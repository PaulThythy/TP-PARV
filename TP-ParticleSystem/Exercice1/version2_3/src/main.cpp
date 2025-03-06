/********************************************************/
/*                     CubeVBOShader.cpp                         */
/********************************************************/
/* Premiers pas avec OpenGL.                            */
/* Objectif : afficher a l'ecran uncube avec ou sans shader    */
/********************************************************/

/* inclusion des fichiers d'en-tete Glut */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include "shader.hpp"
#include <string.h>

#include <chrono>

// Include GLM
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"

#include <vector>

using namespace glm;
using namespace std;

struct alignas(16) Particle
{
  alignas(16) vec3 position;
  alignas(16) vec3 velocity;
  alignas(16) vec3 color;
  alignas(4) float weight;
  alignas(4) float radius;
  alignas(4) float restitution;
};

vector<Particle> particles;

typedef struct
{
  const vec3 position; // position
  const float V0;      // initial speed
} Emitter;

Emitter emitter = {vec3(0.0f, 0.0f, 1.0f), 3.0f};

typedef struct
{
  const float cubeMinX, cubeMaxX;
  const float cubeMinY, cubeMaxY;
  const float cubeMinZ, cubeMaxZ;
} ParticlesContainer;

ParticlesContainer particlesContainer = {
  -1.0f, 1.0f,
  -1.0f, 1.0f,
  0.0f, 2.0f
};

void anim(int NumTimer);

// initialisations

void createSSBO();
void updateSSBO();
void emitParticles(int nbParticles);
void deleteSSBO();
void traceObjet();

// fonctions de rappel de glut
void affichage();
void clavier(unsigned char, int, int);
void mouse(int, int, int, int);
void mouseMotion(int, int);
void reshape(int, int);
// misc
void drawString(const char *str, int x, int y, float color[4], void *font);
void showInfo();
void *font = GLUT_BITMAP_8_BY_13; // pour afficher des textes 2D sur l'ecran
// variables globales pour OpenGL
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance = 1.;

// constantes pour la simulation
vec3 gravity = vec3(0.0f, 0.0f, -9.91f);
const float Cx = 0.5f;  // coefficient de résistance aérodynamique (exemple)
const float rho = 1.2f; // masse volumique de l'air en kg/m³ (environ)
const float S = 0.01f;  // section droite en m² (exemple)

const float alpha = glm::radians(15.0f); // déviation max de ±15° (en radians)

// variables Handle d'opengl
//--------------------------
GLuint programID;                                                     // handle pour le shader
GLuint computeProgramID;                                              // handle pour le shader de calcul
GLuint MatrixIDMVP, MatrixIDView, MatrixIDModel, MatrixIDPerspective; // handle pour la matrice MVP
GLuint ssboParticles;

GLint dtLocation, gravityLocation;
GLint particleCountLocation;
GLint cxLocation, rhoLocation, sLocation;

GLint cubeMinXLocation, cubeMaxXLocation;
GLint cubeMinYLocation, cubeMaxYLocation;
GLint cubeMinZLocation, cubeMaxZLocation;

// variable pour paramétrage eclairage
//--------------------------------------
vec3 cameraPosition(2., 0., 0.);
// le matériau
//---------------
GLfloat materialShininess = 3.;

// la lumière
//-----------
vec3 LightPosition(1., 0., .5);
vec3 LightIntensities(1., 1., 1.); // couleur la lumiere
GLfloat LightAttenuation = 1.;
GLfloat LightAmbientCoefficient = .1;

glm::mat4 MVP;                     // justement la voilà
glm::mat4 Model, View, Projection; // Matrices constituant MVP

int screenHeight = 1500;
int screenWidth = 1500;

//-------------------------
void emitParticles(int nbParticles)
{
  for (int i = 0; i < nbParticles; i++)
  {
    Particle p;

    // Génération d'un offset aléatoire dans l'intervalle [-0.05, 0.05] pour chaque coordonnée
    float rx = ((float)rand() / (float)RAND_MAX) * 0.1f - 0.05f;
    float ry = ((float)rand() / (float)RAND_MAX) * 0.1f - 0.05f;
    float rz = ((float)rand() / (float)RAND_MAX) * 0.1f - 0.05f;

    // La position initiale est l'addition de la position de l'émetteur et de l'offset
    vec3 pos = emitter.position + vec3(rx, ry, rz);
    p.position.x = pos.x;
    p.position.y = pos.y;
    p.position.z = pos.z;

    // Calcul de la vitesse initiale
    // On souhaite que l'angle entre la vitesse et l'axe z soit dans [0, alpha]
    float u = ((float)rand() / (float)RAND_MAX) * (1.0f - cos(alpha)) + cos(alpha);
    float theta = acos(u);
    float phi = ((float)rand() / (float)RAND_MAX) * 2.0f * M_PI;
    float vx = emitter.V0 * sin(theta) * cos(phi);
    float vy = emitter.V0 * sin(theta) * sin(phi);
    float vz = emitter.V0 * cos(theta);
    p.velocity.x = vx;
    p.velocity.y = vy;
    p.velocity.z = vz;

    p.weight = 0.01f;
    p.radius = 0.005f; // Ajustez cette valeur selon vos besoins
    p.restitution = 0.8f;

    // Attribution d'une couleur aléatoire
    p.color.x = (float)rand() / (float)RAND_MAX;
    p.color.y = (float)rand() / (float)RAND_MAX;
    p.color.z = (float)rand() / (float)RAND_MAX;

    particles.push_back(p);
  }
}

//----------------------------------------
void initOpenGL(void)
//----------------------------------------
{
  glCullFace(GL_BACK);    // on spécifie queil faut éliminer les face arriere
  glEnable(GL_CULL_FACE); // on active l'élimination des faces qui par défaut n'est pas active
  glEnable(GL_DEPTH_TEST);
  // le shader
  programID = LoadShaders("shaders/vertex.vert", "shaders/fragment.frag", "shaders/geometry.geom");
  computeProgramID = LoadComputeShader("shaders/compute.comp");

  glEnable(GL_PROGRAM_POINT_SIZE);
  glPointSize(1);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Get  handles for our matrix transformations "MVP" VIEW  MODELuniform
  MatrixIDMVP = glGetUniformLocation(programID, "MVP");
  MatrixIDView = glGetUniformLocation(programID, "VIEW");
  MatrixIDModel = glGetUniformLocation(programID, "MODEL");
  MatrixIDPerspective = glGetUniformLocation(programID, "PERSPECTIVE");

  dtLocation = glGetUniformLocation(computeProgramID, "dt");
  gravityLocation = glGetUniformLocation(computeProgramID, "gravity");

  cxLocation = glGetUniformLocation(computeProgramID, "Cx");
  rhoLocation = glGetUniformLocation(computeProgramID, "rho");
  sLocation = glGetUniformLocation(computeProgramID, "S");

  particleCountLocation = glGetUniformLocation(computeProgramID, "particleCount");

  cubeMinXLocation = glGetUniformLocation(computeProgramID, "particleContainer.cubeMinX");
  cubeMaxXLocation = glGetUniformLocation(computeProgramID, "particleContainer.cubeMaxX");
  cubeMinYLocation = glGetUniformLocation(computeProgramID, "particleContainer.cubeMinY");
  cubeMaxYLocation = glGetUniformLocation(computeProgramID, "particleContainer.cubeMaxY");
  cubeMinZLocation = glGetUniformLocation(computeProgramID, "particleContainer.cubeMinZ");
  cubeMaxZLocation = glGetUniformLocation(computeProgramID, "particleContainer.cubeMaxZ");


  // Projection matrix : 65 Field of View, 1:1 ratio, display range : 1 unit <-> 1000 units
  // ATTENTIOn l'angle est donné en radians si f GLM_FORCE_RADIANS est défini sinon en degré
  Projection = glm::perspective(glm::radians(60.f), 1.0f, 1.0f, 1000.0f);
}

void anim(int NumTimer)
{
    using namespace std::chrono;
    static time_point<system_clock> refTime = system_clock::now();
    time_point<system_clock> currentTime = system_clock::now();
    duration<double> deltaTime = currentTime - refTime;
    refTime = currentTime;

    float dt = static_cast<float>(deltaTime.count());

    emitParticles(100);

    updateSSBO();

    glUseProgram(computeProgramID);

    glUniform1f(dtLocation, dt);
    glUniform3f(gravityLocation, 0.0f, 0.0f, -9.91f);
    glUniform1f(cxLocation, 0.5f);
    glUniform1f(rhoLocation, 1.2f);
    glUniform1f(sLocation,  0.01f);

    glUniform1f(cubeMinXLocation, -1.0f); glUniform1f(cubeMaxXLocation, 1.0f);
    glUniform1f(cubeMinYLocation, -1.0f); glUniform1f(cubeMaxYLocation, 1.0f);
    glUniform1f(cubeMinZLocation, 0.0f); glUniform1f(cubeMaxZLocation, 2.0f);

    unsigned int N = particles.size();
    glUniform1ui(particleCountLocation, N);

    GLuint groups = (N + 255) / 256;
    glDispatchCompute(groups, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glutPostRedisplay();
    glutTimerFunc(20, anim, 1);
}

//----------------------------------------
int main(int argc, char **argv)
//----------------------------------------
{

  /* initialisation de glut et creation
     de la fenetre */

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowPosition(200, 200);
  glutInitWindowSize(screenWidth, screenHeight);
  glutCreateWindow("Particle System");

  // Initialize GLEW
  if (glewInit() != GLEW_OK)
  {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }

  // info version GLSL
  std::cout << "***** Info GPU *****" << std::endl;
  std::cout << "Fabricant : " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "Carte graphique: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "Version : " << glGetString(GL_VERSION) << std::endl;
  std::cout << "Version GLSL : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl
            << std::endl;

  initOpenGL();

  createSSBO();

  /* enregistrement des fonctions de rappel */
  glutDisplayFunc(affichage);
  glutKeyboardFunc(clavier);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutTimerFunc(200, anim, 1);
  /* Entree dans la boucle principale glut */
  glutMainLoop();

  glDeleteProgram(programID);
  deleteSSBO();
  return 0;
}

void createSSBO() {
  // Génération et remplissage du SSBO avec les particules
  glCreateBuffers(1, &ssboParticles);
  glNamedBufferStorage(ssboParticles, particles.size() * sizeof(Particle),
               particles.data(), GL_DYNAMIC_STORAGE_BIT);
  // On le lie à l'unité de binding 0
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
}

void updateSSBO() {
  glNamedBufferData(ssboParticles, particles.size() * sizeof(Particle),
               particles.data(), GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
}

//-----------------
void deleteSSBO()
//-----------------
{
  glDeleteBuffers(1, &ssboParticles);
  ssboParticles = 0;
}

/* fonction d'affichage */
void affichage()
{

  glClearColor(1., 1., 1., 0.0);
  glClearDepth(10.0f); // 0 is near, >0 is far
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(1.0, 1.0, 1.0);
  glPointSize(2.0);

  View = glm::lookAt(cameraPosition,     // Camera is at (0,0,3), in World Space
                     glm::vec3(0, 0, 1), // and looks at the origin
                     glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  Model = glm::mat4(1.0f);
  Model = glm::rotate(Model, glm::radians(cameraAngleX), glm::vec3(1, 0, 0));
  Model = glm::rotate(Model, glm::radians(cameraAngleY), glm::vec3(0, 1, 0));
  Model = glm::scale(Model, glm::vec3(.8, .8, .8) * cameraDistance);
  MVP = Projection * View * Model;
  traceObjet(); // trace VBO avec ou sans shader

  glutPostRedisplay();
  glutSwapBuffers();
}

//-------------------------------------
void traceObjet()
//-------------------------------------
{
  // Use  shader & MVP matrix   MVP = Projection * View * Model;
  glUseProgram(programID);

  // on envoie les données necessaires aux shaders */
  glUniformMatrix4fv(MatrixIDMVP, 1, GL_FALSE, &MVP[0][0]);
  glUniformMatrix4fv(MatrixIDView, 1, GL_FALSE,&View[0][0]);
  glUniformMatrix4fv(MatrixIDModel, 1, GL_FALSE, &Model[0][0]);
  glUniformMatrix4fv(MatrixIDPerspective, 1, GL_FALSE, &Projection[0][0]);

  // pour l'affichage
  glDrawArrays(GL_POINTS, 0, particles.size());
  glBindVertexArray(0); // on desactive les VAO

  glUseProgram(0); // et le pg
}

void reshape(int w, int h)
{
  // set viewport to be the entire window
  glViewport(0, 0, (GLsizei)w, (GLsizei)h); // ATTENTION GLsizei important - indique qu'il faut convertir en entier non négatif

  // set perspective viewing frustum
  float aspectRatio = (float)w / h;

  Projection = glm::perspective(glm::radians(60.0f), (float)(w) / (float)h, 1.0f, 1000.0f);
}

void clavier(unsigned char touche, int x, int y)
{
  switch (touche)
  {
  case 'f': /* affichage du carre plein */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glutPostRedisplay();
    break;
  case 'e': /* affichage en mode fil de fer */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glutPostRedisplay();
    break;
  case 'v': /* Affichage en mode sommets seuls */
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glutPostRedisplay();
    break;
  case 's': /* Affichage en mode sommets seuls */
    materialShininess -= .5;
    glutPostRedisplay();
    break;
  case 'S': /* Affichage en mode sommets seuls */
    materialShininess += .5;
    glutPostRedisplay();
    break;
  case 'x': /* Affichage en mode sommets seuls */
    LightPosition.x -= .2;
    glutPostRedisplay();
    break;
  case 'X': /* Affichage en mode sommets seuls */
    LightPosition.x += .2;
    glutPostRedisplay();
    break;
  case 'y': /* Affichage en mode sommets seuls */
    LightPosition.y -= .2;
    glutPostRedisplay();
    break;
  case 'Y': /* Affichage en mode sommets seuls */
    LightPosition.y += .2;
    glutPostRedisplay();
    break;
  case 'z': /* Affichage en mode sommets seuls */
    LightPosition.z -= .2;
    glutPostRedisplay();
    break;
  case 'Z': /* Affichage en mode sommets seuls */
    LightPosition.z += .2;
    glutPostRedisplay();
    break;
  case 'a': /* Affichage en mode sommets seuls */
    LightAmbientCoefficient -= .1;
    glutPostRedisplay();
    break;
  case 'A': /* Affichage en mode sommets seuls */
    LightAmbientCoefficient += .1;
    glutPostRedisplay();
    break;

  case 'q': /*la touche 'q' permet de quitter le programme */
    exit(0);
  }
}

void mouse(int button, int state, int x, int y)
{
  mouseX = x;
  mouseY = y;

  if (button == GLUT_LEFT_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      mouseLeftDown = true;
    }
    else if (state == GLUT_UP)
      mouseLeftDown = false;
  }

  else if (button == GLUT_RIGHT_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      mouseRightDown = true;
    }
    else if (state == GLUT_UP)
      mouseRightDown = false;
  }

  else if (button == GLUT_MIDDLE_BUTTON)
  {
    if (state == GLUT_DOWN)
    {
      mouseMiddleDown = true;
    }
    else if (state == GLUT_UP)
      mouseMiddleDown = false;
  }
}

void mouseMotion(int x, int y)
{
  if (mouseLeftDown)
  {
    cameraAngleY += (x - mouseX);
    cameraAngleX += (y - mouseY);
    mouseX = x;
    mouseY = y;
  }
  if (mouseRightDown)
  {
    cameraDistance += (y - mouseY) * 0.2f;
    mouseY = y;
  }

  glutPostRedisplay();
}