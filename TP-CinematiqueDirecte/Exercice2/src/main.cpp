/********************************************************/
/*                     cube.cpp                         */                    
/********************************************************/
/*                Affiche a l'ecran un cube en 3D       */              
/********************************************************/


#ifdef __APPLE__
  #include <GLUT/glut.h>
#else 
  #include <GL/glut.h>   
#endif 

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/io.hpp>
#include <chrono>

using namespace glm;
using namespace std;

//****************************************
#define NB_BRAS 4
//****************************************

char presse;
int anglex, angley, x, y, xold, yold;

float timePassed = 0.0f;

// Variables globales pour les rotations et les longueurs
float angleBase = 0.0f;    // Angle du segment de base (cube rouge)
float angleCoude = 0.0f;   // Angle pour le coude (cube vert)
float anglePoignet = 0.0f; // Angle pour le poignet (cube bleu)

float longueurBase = 1.0f;
float longueurCoude = 2.0f;
float longueurPoignet = 1.0f;


void affichage();
void clavier(unsigned char touche, int x, int y);
void reshape(int x, int y);
void idle();
void mouse(int bouton, int etat, int x, int y);
void mousemotion(int x, int y);

void anim(int NumTimer);

void anim(int NumTimer) {
  using namespace std::chrono;
  static time_point < system_clock > refTime = system_clock::now();

  time_point < system_clock > currentTime = system_clock::now(); // This and "end"'s type is std::chrono::time_point

  duration < double > deltaTime = currentTime - refTime;

  cout << "deltaTime : " << deltaTime.count() << endl;

  if (timePassed + deltaTime.count() >= 1.0f) {
    timePassed = 0.0f;
  } else {
    timePassed += deltaTime.count(); 
  }

  int delatTemps = duration_cast < milliseconds > (deltaTime).count();

  refTime = currentTime;

  glutPostRedisplay();
  glutTimerFunc(100, anim, 1);

}

int main(int argc, char ** argv) {
  glutInit( & argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(200, 200);
  glutInitWindowSize(1500, 1500);
  glutCreateWindow("Cinématique directe");

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glColor3f(1.0, 1.0, 1.0);
  glPointSize(10.0);
  glEnable(GL_DEPTH_TEST);

  glutDisplayFunc(affichage);
  glutKeyboardFunc(clavier);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mousemotion);
  glutTimerFunc(200, anim, 1);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, 1, .1, 30.);

  glutMainLoop();
  return 0;
}

void bras() {
    glPushMatrix();
    // --------------------------
    // Segment de base (cube rouge)
    // --------------------------
    // Rotation par rapport à un axe (ici l'axe z par exemple)
    glRotatef(angleBase, 0.0f, 0.0f, 1.0f);
    glPushMatrix();
      glColor3f(1.0f, 0.0f, 0.0f);
      // Mise à l'échelle pour obtenir la bonne longueur
      glScalef(longueurBase, 0.2f, 0.2f);
      // Déplacement pour dessiner le cube centré sur le segment
      glTranslatef(0.5f, 0.0f, 0.0f);
      glutSolidCube(1.0);
    glPopMatrix();
    
    // Se déplacer à l'extrémité du segment de base
    glTranslatef(longueurBase, 0.0f, 0.0f);
    
    // --------------------------
    // Segment du coude (cube vert)
    // --------------------------
    glRotatef(angleCoude, 0.0f, 0.0f, 1.0f);
    glPushMatrix();
      glColor3f(0.0f, 1.0f, 0.0f);
      glScalef(longueurCoude, 0.2f, 0.2f);
      glTranslatef(0.5f, 0.0f, 0.0f);
      glutSolidCube(1.0);
    glPopMatrix();
    
    // Se déplacer à l'extrémité du coude
    glTranslatef(longueurCoude, 0.0f, 0.0f);
    
    // --------------------------
    // Segment du poignet (cube bleu)
    // --------------------------
    glRotatef(anglePoignet, 0.0f, 0.0f, 1.0f);
    glPushMatrix();
      glColor3f(0.0f, 0.0f, 1.0f);
      glScalef(longueurPoignet, 0.2f, 0.2f);
      glTranslatef(0.5f, 0.0f, 0.0f);
      glutSolidCube(1.0);
    glPopMatrix();
    
    glPopMatrix();
}


void affichage() {
  int i, j;
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glShadeModel(GL_SMOOTH);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0., 0., -5.);
  glRotatef(angley, 1.0, 0.0, 0.0);
  glRotatef(anglex, 0.0, 1.0, 0.0);

  bras();

  //axe x en rouge
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0, 0, 0.0);
  glVertex3f(1, 0, 0.0);
  glEnd();
  //axe des y en vert
  glBegin(GL_LINES);
  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0, 0, 0.0);
  glVertex3f(0, 1, 0.0);
  glEnd();
  //axe des z en bleu
  glBegin(GL_LINES);
  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0, 0, 0.0);
  glVertex3f(0, 0, 1.0);
  glEnd();

  glFlush();

  glutSwapBuffers();
}

void clavier(unsigned char touche, int x, int y) {
  switch(touche) {
    // Modifier l'angle de la base avec 'a' et 'z'
    case 'a':
      angleBase += 5.0f;
      break;
    case 'A':
      angleBase -= 5.0f;
      break;
      
    // Modifier l'angle du coude avec 's' et 'x'
    case 's':
      angleCoude += 5.0f;
      break;
    case 'S':
      angleCoude -= 5.0f;
      break;
      
    // Modifier l'angle du poignet avec 'd' et 'c'
    case 'd':
      anglePoignet += 5.0f;
      break;
    case 'D':
      anglePoignet -= 5.0f;
      break;
      
    case 'q':
      exit(0);
      break;
      
    // Autres commandes (affichage, mode fil de fer, etc.)
    case 'p':
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
    case 'f':
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case 'u':
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    case 'x':
      glEnable(GL_DEPTH_TEST);
      break;
    case 'X':
      glDisable(GL_DEPTH_TEST);
      break;
  }
  glutPostRedisplay();
}



void reshape(int x, int y) {
  if (x < y)
    glViewport(0, (y - x) / 2, x, x);
  else
    glViewport((x - y) / 2, 0, y, y);
}

void mouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    presse = 1; 
    xold = x;
    yold = y;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    presse = 0;
}

void mousemotion(int x, int y) {
  if (presse) {
    anglex = anglex + (x - xold);
    angley = angley + (y - yold);
    glutPostRedisplay(); /* on demande un rafraichissement de l'affichage */
  }
  //else
  //{
  //    pointCible.x= ;
  //    pointCible.y= ;
  //}

  xold = x;
  yold = y;
}