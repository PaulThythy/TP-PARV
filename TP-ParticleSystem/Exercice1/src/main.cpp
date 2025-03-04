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

typedef struct
{
  GLfloat position[3];
  GLfloat vitesse[3];
  GLfloat masse;
  GLfloat couleur[3];
} Particule;

vector<Particule> listeParticules;

typedef struct
{
  GLfloat position;   //position
  const float V0;           //initial speed

  typedef struct
  {
    const float cubeMinX, cubeMaxX;
    const float cubeMinY, cubeMaxY;
    const float cubeMinZ, cubeMaxZ;
  } ParticlesContainer;
} Emitter;

void anim(int NumTimer);

// initialisations

void genereVBO();
void emitParticules(int nbParticules);
void deleteVBO();
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
const float m = 0.01f;
vec3 gravity = vec3(0.0f, 0.0f, -9.91f);
const float Cx = 0.5f;      // coefficient de résistance aérodynamique (exemple)
const float rho = 1.2f;     // masse volumique de l'air en kg/m³ (environ)
const float S = 0.01f;      // section droite en m² (exemple)

const float emitterCubeSide = 0.1f;  // côté du cube émetteur
const float halfCube = emitterCubeSide / 2.0f;
const float alpha = glm::radians(15.0f);  // déviation max de ±15° (en radians)
const float v0 = 3.0f;   // vitesse initiale (en m/s)

// variables Handle d'opengl
//--------------------------
GLuint programID;                                                     // handle pour le shader
GLuint MatrixIDMVP, MatrixIDView, MatrixIDModel, MatrixIDPerspective; // handle pour la matrice MVP
GLuint VBO_sommets, VBO_normales, VBO_indices, VBO_UVtext, VAO;

// location des VBO
//------------------
GLuint indexVertex = 0, indexUVTexture = 2, indexNormale = 3, indexVitesse = 4, indexCouleur = 5;

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

// pour la texcture
//-------------------
GLuint image;
GLuint bufTexture, bufNormalMap;
GLuint locationTexture, locationNormalMap;
//-------------------------
void emitParticules(int nbParticules)
{
  vec3 emitterCenter(0.0f, 0.0f, 1.0f);

  for (int i = 0; i < nbParticules; i++)
  {
    Particule p;

    float rx = ((float)rand() / (float)RAND_MAX) * emitterCubeSide - halfCube;
    float ry = ((float)rand() / (float)RAND_MAX) * emitterCubeSide - halfCube;
    float rz = ((float)rand() / (float)RAND_MAX) * emitterCubeSide - halfCube;

    vec3 pos = emitterCenter + vec3(rx, ry, rz);
    p.position[0] = pos.x;
    p.position[1] = pos.y;
    p.position[2] = pos.z;

    // Calcul de la vitesse initiale
    // Pour obtenir une direction quasi verticale, on souhaite que l'angle entre la vitesse et l'axe z soit dans [0, alpha].
    // On génère d'abord u uniformément dans [cos(alpha), 1] puis on calcule theta = arccos(u).
    float u = ((float)rand() / (float)RAND_MAX) * (1.0f - cos(alpha)) + cos(alpha);
    float theta = acos(u);
    // Phi est tiré uniformément dans [0, 2*pi]
    float phi = ((float)rand() / (float)RAND_MAX) * 2.0f * M_PI;
    // Conversion de coordonnées sphériques en cartésiennes (axe z vertical)
    float vx = v0 * sin(theta) * cos(phi);
    float vy = v0 * sin(theta) * sin(phi);
    float vz = v0 * cos(theta);
    p.vitesse[0] = vx;
    p.vitesse[1] = vy;
    p.vitesse[2] = vz;
    
    p.masse = m;
    
    // Attribution d'une couleur aléatoire
    p.couleur[0] = (float)rand() / (float)RAND_MAX;
    p.couleur[1] = (float)rand() / (float)RAND_MAX;
    p.couleur[2] = (float)rand() / (float)RAND_MAX;
    
    listeParticules.push_back(p);
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
  programID = LoadShaders("shaders/vertex.vert", "shaders/fragment.frag");
  glEnable(GL_PROGRAM_POINT_SIZE);
  //  glPointSize(30.);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // Get  handles for our matrix transformations "MVP" VIEW  MODELuniform
  MatrixIDMVP = glGetUniformLocation(programID, "MVP");
  //  MatrixIDView = glGetUniformLocation(programID, "VIEW");
  MatrixIDModel = glGetUniformLocation(programID, "MODEL");
  // MatrixIDPerspective = glGetUniformLocation(programID, "PERSPECTIVE");

  // Projection matrix : 65 Field of View, 1:1 ratio, display range : 1 unit <-> 1000 units
  // ATTENTIOn l'angle est donné en radians si f GLM_FORCE_RADIANS est défini sinon en degré
  Projection = glm::perspective(glm::radians(60.f), 1.0f, 1.0f, 1000.0f);
}

void anim(int NumTimer)
{
  using namespace std::chrono;
  static time_point<system_clock> refTime = system_clock::now();

  time_point<system_clock> currentTime = system_clock::now(); // This and "end"'s type is std::chrono::time_point

  duration<double> deltaTime = currentTime - refTime;

  int delatTemps = duration_cast<milliseconds>(deltaTime).count(); // temps  écoulé en millisecondes depuis le dernier appel de anim

  refTime = currentTime;

  // dt en secondes (attention : deltaTime est en secondes)
  float dt = static_cast<float>(deltaTime.count());

  const float restitution = 0.8f;

  emitParticules(10);

  for(size_t i = 0;i < listeParticules.size();i++) {
    vec3 velocity(listeParticules[i].vitesse[0],
                  listeParticules[i].vitesse[1],
                  listeParticules[i].vitesse[2]);
    vec3 pos(listeParticules[i].position[0],
             listeParticules[i].position[1],
             listeParticules[i].position[2]);

    vec3 F_gravity = listeParticules[i].masse * gravity;

    float speed = length(velocity);
    vec3 F_drag(0.0f);
    if (speed > 0.0f) {
      F_drag = -0.5f * Cx * rho * S * speed * speed * normalize(velocity);
    }

    // Force nette
    vec3 F_net = F_gravity + F_drag;

    // Accélération
    vec3 acceleration = F_net / listeParticules[i].masse;

    // Mise à jour de la vitesse (méthode d'Euler)
    velocity += acceleration * dt;

    // Mise à jour de la position
    pos += velocity * dt;

    // Définition des bornes du cube
    const float cubeMinX = -1.0f, cubeMaxX = 1.0f;
    const float cubeMinY = -1.0f, cubeMaxY = 1.0f;
    const float cubeMinZ = 0.0f,  cubeMaxZ = 2.0f;

    // Rebond sur les parois du cube
    if (pos.x < cubeMinX) {
      pos.x = cubeMinX;
      velocity.x = -velocity.x * restitution;
    } else if (pos.x > cubeMaxX) {
      pos.x = cubeMaxX;
      velocity.x = -velocity.x * restitution;
    }
    if (pos.y < cubeMinY) {
      pos.y = cubeMinY;
      velocity.y = -velocity.y * restitution;
    } else if (pos.y > cubeMaxY) {
      pos.y = cubeMaxY;
      velocity.y = -velocity.y * restitution;
    }
    if (pos.z < cubeMinZ) {
      pos.z = cubeMinZ;
      velocity.z = -velocity.z * restitution;
    } else if (pos.z > cubeMaxZ) {
      pos.z = cubeMaxZ;
      velocity.z = -velocity.z * restitution;
    }

    listeParticules[i].vitesse[0] = velocity.x;
    listeParticules[i].vitesse[1] = velocity.y;
    listeParticules[i].vitesse[2] = velocity.z;

    listeParticules[i].position[0] = pos.x;
    listeParticules[i].position[1] = pos.y;
    listeParticules[i].position[2] = pos.z;
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO_sommets);

  glBufferData(GL_ARRAY_BUFFER, listeParticules.size() * sizeof(Particule), listeParticules.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glutPostRedisplay();
  glutTimerFunc(25, anim, 1);
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

  genereVBO();

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
  deleteVBO();
  return 0;
}

void genereVBO()
{

  if (glIsBuffer(VBO_sommets) == GL_TRUE)
    glDeleteBuffers(1, &VBO_sommets);
  glGenBuffers(1, &VBO_sommets);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_sommets);

  glBufferData(GL_ARRAY_BUFFER, listeParticules.size() * sizeof(Particule), listeParticules.data(), GL_DYNAMIC_DRAW);
  glGenBuffers(1, &VAO);
  glEnableVertexAttribArray(indexVertex);
  glEnableVertexAttribArray(indexVitesse);
  glEnableVertexAttribArray(indexCouleur);

  glBindVertexArray(VAO); // ici on bind le VAO , c'est lui qui recupèrera les configurations des VBO glVertexAttribPointer , glEnableVertexAttribArray...
  glBindBuffer(GL_ARRAY_BUFFER, VBO_sommets);

  glVertexAttribPointer(indexVertex, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void *>(offsetof(Particule, position)));
  glVertexAttribPointer(indexVitesse, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void *>(offsetof(Particule, vitesse)));
  glVertexAttribPointer(indexCouleur, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), reinterpret_cast<void *>(offsetof(Particule, couleur)));

  // une fois la config terminée
  // on désactive le dernier VBO et le VAO pour qu'ils ne soit pas accidentellement modifié
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}
//-----------------
void deleteVBO()
//-----------------
{
  glDeleteBuffers(1, &VBO_sommets);
  glDeleteBuffers(1, &VBO_normales);
  glDeleteBuffers(1, &VBO_indices);
  glDeleteBuffers(1, &VBO_UVtext);
  glDeleteBuffers(1, &VAO);
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
  // glUniformMatrix4fv(MatrixIDView, 1, GL_FALSE,&View[0][0]);
  glUniformMatrix4fv(MatrixIDModel, 1, GL_FALSE, &Model[0][0]);
  // glUniformMatrix4fv(MatrixIDPerspective, 1, GL_FALSE, &Projection[0][0]);

  // pour l'affichage

  glBindVertexArray(VAO); // on active le VAO
  glDrawArrays(GL_POINTS, 0, listeParticules.size());
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