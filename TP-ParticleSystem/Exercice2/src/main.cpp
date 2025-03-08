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

struct Particle {
  alignas(16) glm::vec3 position;
  alignas(16) glm::vec3 previousPosition;
  alignas(16) glm::vec3 velocity;
  float weight;
  int isFixed;
};

vector<Particle> particles;
GLuint vaoID = 0;
GLuint indexBufferID = 0;
std::vector<GLuint> indices;

void anim(int NumTimer);

// initialisations
void createSSBO();
void emitParticles(int nbParticles);
void deleteSSBO();
void traceObjet();

// fonctions de rappel de glut
void affichage();
void drawAxes();
void clavier(unsigned char, int, int);
void mouse(int, int, int, int);
void mouseMotion(int, int);
void reshape(int, int);

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
float springConstant = 200.0f;              // Constante de ressort (k)
float restLength = 0.5f;                 // Longueur de repos (espacement initial)
vec3 windForce = vec3(10.0f, 5.0f, 0.0f);  // Force du vent (à ajuster)
float damping = 5.0f;

const int NX = 20;
const int NY = 20;
const int TOTAL_PARTICLES = NX * NY;

// variables Handle d'opengl
//--------------------------
GLuint programID;                                                     // handle pour le shader
GLuint computeProgramID;                                              // handle pour le shader de calcul
GLuint MatrixIDMVP, MatrixIDView, MatrixIDModel, MatrixIDPerspective; // handle pour la matrice MVP
GLuint ssboParticles;

GLint dtLocation, gravityLocation;
GLint NXLocation, NYLocation;
GLint kLocation, restLengthLocation, windLocation;
GLint dampingLocation;

// variable pour paramétrage eclairage
//--------------------------------------
vec3 cameraPosition(0.0f, -10.0f, 0.5f);
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

void drawAxes() {
  glUseProgram(0);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixf(&Projection[0][0]);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glm::mat4 modelView = View * Model;
  glLoadMatrixf(&modelView[0][0]);

  // Dessiner les axes
  glLineWidth(2.0f);
  glBegin(GL_LINES);
    // Axe X en rouge
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 0.0f, 0.0f);
    
    // Axe Y en vert
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    
    // Axe Z en bleu
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
  glEnd();

  // Restaurer les matrices précédentes
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}


//-------------------------
void initFlag()
{
  particles.resize(TOTAL_PARTICLES);
  for (int i = 0; i < NY; i++)
  {
    for (int j = 0; j < NX; j++)
    {
      int idx = i * NX + j;
      Particle &p = particles[idx];
      p.position = vec3(0.0f, j * restLength, i * restLength);
      p.previousPosition = p.position;
      p.velocity = vec3(0.0f);
      p.weight = 1.0f;
      p.isFixed = (j == 0) ? 1 : 0;
    }
  }
}

void createIndexBuffer() {
  // Pour chaque cellule (de NX-1 par NY-1), on crée 2 triangles.
  for (int i = 0; i < NY - 1; i++) {
      for (int j = 0; j < NX - 1; j++) {
          // Calculer les indices des 4 coins de la cellule
          GLuint topLeft     = i * NX + j;
          GLuint topRight    = topLeft + 1;
          GLuint bottomLeft  = (i + 1) * NX + j;
          GLuint bottomRight = bottomLeft + 1;
          // Premier triangle : topLeft, bottomLeft, topRight
          indices.push_back(topLeft);
          indices.push_back(bottomLeft);
          indices.push_back(topRight);
          // Deuxième triangle : topRight, bottomLeft, bottomRight
          indices.push_back(topRight);
          indices.push_back(bottomLeft);
          indices.push_back(bottomRight);
      }
  }

  // Créer un VAO (même si nous n'utilisons pas de vertex attributes classiques,
  // il est nécessaire pour glDrawElements)
  glGenVertexArrays(1, &vaoID);
  glBindVertexArray(vaoID);
  
  // Créer et remplir le buffer d’indices
  glGenBuffers(1, &indexBufferID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indices.size() * sizeof(GLuint),
               indices.data(),
               GL_STATIC_DRAW);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
  
  glBindVertexArray(0);
}

//----------------------------------------
void initOpenGL(void)
//----------------------------------------
{
  glCullFace(GL_BACK);    // on spécifie queil faut éliminer les face arriere
  glDisable(GL_CULL_FACE); // on active l'élimination des faces qui par défaut n'est pas active
  glEnable(GL_DEPTH_TEST);
  // le shader
  programID = LoadShaders("shaders/vertex.vert", "shaders/fragment.frag");
  computeProgramID = LoadComputeShader("shaders/compute.comp");

  initFlag();
  createSSBO();
  createIndexBuffer();

  glEnable(GL_PROGRAM_POINT_SIZE);
  glPointSize(5);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Get  handles for our matrix transformations "MVP" VIEW  MODELuniform
  MatrixIDMVP = glGetUniformLocation(programID, "MVP");
  MatrixIDView = glGetUniformLocation(programID, "VIEW");
  MatrixIDModel = glGetUniformLocation(programID, "MODEL");
  MatrixIDPerspective = glGetUniformLocation(programID, "PERSPECTIVE");

  dtLocation = glGetUniformLocation(computeProgramID, "dt");
  gravityLocation = glGetUniformLocation(computeProgramID, "gravity");
  NXLocation = glGetUniformLocation(computeProgramID, "NX");
  NYLocation = glGetUniformLocation(computeProgramID, "NY");
  kLocation = glGetUniformLocation(computeProgramID, "k");
  restLengthLocation = glGetUniformLocation(computeProgramID, "restLength");
  windLocation = glGetUniformLocation(computeProgramID, "wind");
  dampingLocation = glGetUniformLocation(computeProgramID, "damping");

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

  glUseProgram(computeProgramID);

  glUniform1f(dtLocation, dt);
  glUniform3f(gravityLocation, gravity.x, gravity.y, gravity.z);

  glUniform1i(NXLocation, NX);
  glUniform1i(NYLocation, NY);
  glUniform1f(kLocation, springConstant);
  glUniform1f(restLengthLocation, restLength);
  glUniform3f(windLocation, windForce.x, windForce.y, windForce.z);
  glUniform1f(dampingLocation, damping);

  GLuint groups = (TOTAL_PARTICLES + 255) / 256;
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

void createSSBO()
{
  glCreateBuffers(1, &ssboParticles);
  glNamedBufferStorage(ssboParticles,
                        TOTAL_PARTICLES * sizeof(Particle),
                        particles.data(),
                        GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
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
                     glm::vec3(0, 0, 0), // and looks at the origin
                     glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  Model = glm::mat4(1.0f);
  Model = glm::rotate(Model, glm::radians(cameraAngleX), glm::vec3(1, 0, 0));
  Model = glm::rotate(Model, glm::radians(cameraAngleY), glm::vec3(0, 1, 0));
  Model = glm::scale(Model, glm::vec3(.8, .8, .8) * cameraDistance);
  MVP = Projection * View * Model;
  traceObjet(); // trace VBO avec ou sans shader

  drawAxes();

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
  glUniformMatrix4fv(MatrixIDView, 1, GL_FALSE, &View[0][0]);
  glUniformMatrix4fv(MatrixIDModel, 1, GL_FALSE, &Model[0][0]);
  glUniformMatrix4fv(MatrixIDPerspective, 1, GL_FALSE, &Projection[0][0]);

  glBindVertexArray(vaoID);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  
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