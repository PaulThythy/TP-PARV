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
#include <armadillo>
#include <cmath>

using namespace glm;
using namespace std;

char presse;
int anglex, angley, x, y, xold, yold;

float timePassed = 0.0f;

void affichage();
void clavier(unsigned char touche, int x, int y);
void reshape(int x, int y);
void idle();
void mouse(int bouton, int etat, int x, int y);
void mousemotion(int x, int y);

void anim(int NumTimer);
void updateAngles();

void anim(int NumTimer)
{
  using namespace std::chrono;
  static time_point<system_clock> refTime = system_clock::now();

  time_point<system_clock> currentTime = system_clock::now(); // This and "end"'s type is std::chrono::time_point

  duration<double> deltaTime = currentTime - refTime;

  if (timePassed + deltaTime.count() >= 1.0f)
  {
    timePassed = 0.0f;
  }
  else
  {
    timePassed += deltaTime.count();
  }

  int delatTemps = duration_cast<milliseconds>(deltaTime).count();

  refTime = currentTime;

  updateAngles();
  glutTimerFunc(500, anim, 1);
  glutPostRedisplay();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(200, 200);
  glutInitWindowSize(1500, 1500);
  glutCreateWindow("Particle System");

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


void affichage()
{
  int i, j;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glShadeModel(GL_SMOOTH);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0., 0., -5.);
  glRotatef(angley, 1.0, 0.0, 0.0);
  glRotatef(anglex, 0.0, 1.0, 0.0);

  // axe x en rouge
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0, 0, 0.0);
  glVertex3f(1, 0, 0.0);
  glEnd();
  // axe des y en vert
  glBegin(GL_LINES);
  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0, 0, 0.0);
  glVertex3f(0, 1, 0.0);
  glEnd();
  // axe des z en bleu
  glBegin(GL_LINES);
  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0, 0, 0.0);
  glVertex3f(0, 0, 1.0);
  glEnd();

  glFlush();

  glutSwapBuffers();
}

void clavier(unsigned char touche, int x, int y)
{
  switch (touche)
  {
  case 'p':
    /* affichage du carre plein */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glutPostRedisplay();
    break;
  case 'f':
    /* affichage en mode fil de fer */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glutPostRedisplay();
    break;
  case 's':
    /* Affichage en mode sommets seuls */
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glutPostRedisplay();
    break;
  case 'd':
    glEnable(GL_DEPTH_TEST);
    glutPostRedisplay();
    break;
  case 'D':
    glDisable(GL_DEPTH_TEST);
    glutPostRedisplay();
    break;
  case 'q':
    exit(0);
  }
}

void reshape(int x, int y)
{
  if (x < y)
    glViewport(0, (y - x) / 2, x, x);
  else
    glViewport((x - y) / 2, 0, y, y);
}

void mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    presse = 1;
    xold = x;
    yold = y;
  }
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    presse = 0;
}

void mousemotion(int x, int y)
{
  if (presse)
  {
    anglex = anglex + (x - xold);
    angley = angley + (y - yold);
    glutPostRedisplay(); /* on demande un rafraichissement de l'affichage */
  }
  // else
  //{
  //     pointCible.x= ;
  //     pointCible.y= ;
  // }

  xold = x;
  yold = y;
}
