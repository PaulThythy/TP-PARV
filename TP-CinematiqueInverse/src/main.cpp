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
#include <armadillo>
#include <cmath>

using namespace glm;
using namespace std;

char presse;
int anglex, angley, x, y, xold, yold;

float timePassed = 0.0f;

// init angles between articulations
float theta0 = 0.0f;
float theta1 = -M_PI / 2;
float theta2 = M_PI / 2;
float theta3 = 0.0f;

// length of articulations
float L1 = 1.0f, L2 = 2.0f, L3 = 1.0f, L4 = 1.0f;

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
  glutCreateWindow("cube");

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
    
    // Appliquer la rotation de la base (theta0)
    glRotatef(theta0 * 180.0 / M_PI, 0.0, 0.0, 1.0);

    // --- Segment rouge (premier segment) ---
    glPushMatrix();
      glColor3f(1.0, 0.0, 0.0);
      glScalef(L1, 0.2, 0.5);
      glTranslatef(0.5, 0.0, 0.0);
      glutSolidCube(1.0);
    glPopMatrix();
    
    // Se déplacer à l'extrémité du segment rouge
    glTranslatef(L1, 0.0, 0.0);

    // --- Rotation de l'articulation verte (theta1) ---
    glRotatef(theta1 * 180.0 / M_PI, 0.0, 0.0, 1.0);

    // --- Segment vert ---
    glPushMatrix();
      glColor3f(0.0, 1.0, 0.0);
      glScalef(L2, 0.2, 0.5);
      glTranslatef(0.5, 0.0, 0.0);
      glutSolidCube(1.0);
    glPopMatrix();
    
    // Se déplacer à l'extrémité du segment vert
    glTranslatef(L2, 0.0, 0.0);

    // --- Rotation de l'articulation bleue (theta2) ---
    glRotatef(theta2 * 180.0 / M_PI, 0.0, 0.0, 1.0);

    // --- Segment bleu ---
    glPushMatrix();
      glColor3f(0.0, 0.0, 1.0);
      glScalef(L3, 0.2, 0.5);
      glTranslatef(0.5, 0.0, 0.0);
      glutSolidCube(1.0);
    glPopMatrix();

    // Se déplacer à l'extrémité du segment bleu
    glTranslatef(L3, 0.0, 0.0);

    // --- Rotation de l'articulation cyan (theta3) ---
    glRotatef(theta3 * 180.0 / M_PI, 0.0, 0.0, 1.0);

    // --- Segment cyan ---
    glPushMatrix();
      glColor3f(0.0, 1.0, 1.0);
      glScalef(L4, 0.2, 0.5);
      glTranslatef(0.5, 0.0, 0.0);
      glutSolidCube(1.0);
    glPopMatrix();
    
    glPopMatrix();
}


arma::vec computeEndEffectorPosition()
{
  float x = L1 * cos(theta0) + L2 * cos(theta0 + theta1) + L3 * cos(theta0 + theta1 + theta2) + L4 * cos(theta0 + theta1 + theta2 + theta3);

  float y = L1 * sin(theta0) + L2 * sin(theta0 + theta1) + L3 * sin(theta0 + theta1 + theta2) + L4 * sin(theta0 + theta1 + theta2 + theta3);

  return {x, y, 0};
}

arma::mat computeJacobian() {
    float J11 = -L1 * sin(theta0) - L2 * sin(theta0 + theta1) - L3 * sin(theta0 + theta1 + theta2) - L4 * sin(theta0 + theta1 + theta2 + theta3);
    float J12 = -L2 * sin(theta0 + theta1) - L3 * sin(theta0 + theta1 + theta2) - L4 * sin(theta0 + theta1 + theta2 + theta3);
    float J13 = -L3 * sin(theta0 + theta1 + theta2) - L4 * sin(theta0 + theta1 + theta2 + theta3);
    float J14 = -L4 * sin(theta0 + theta1 + theta2 + theta3);

    float J21 = L1 * cos(theta0) + L2 * cos(theta0 + theta1) + L3 * cos(theta0 + theta1 + theta2) + L4 * cos(theta0 + theta1 + theta2 + theta3);
    float J22 = L2 * cos(theta0 + theta1) + L3 * cos(theta0 + theta1 + theta2) + L4 * cos(theta0 + theta1 + theta2 + theta3);
    float J23 = L3 * cos(theta0 + theta1 + theta2) + L4 * cos(theta0 + theta1 + theta2 + theta3);
    float J24 = L4 * cos(theta0 + theta1 + theta2 + theta3);

    arma::mat J = {{J11, J12, J13, J14},
                   {J21, J22, J23, J24}};
    return J;
}


void updateAngles() {
    arma::vec target = {0, 2}; // Position désirée
    arma::vec currentPos = computeEndEffectorPosition().head(2);
    arma::vec error = target - currentPos; // Erreur sur x et y

    arma::mat J = computeJacobian();
    arma::mat J_pinv = pinv(J); // Pseudo-inverse Moore-Penrose

    float K = 0.1;                            // empêche de diverger
    arma::vec dTheta = K * (J_pinv * error);  // Variation des angles

    theta0 += dTheta(0);
    theta1 += dTheta(1);
    theta2 += dTheta(2);
    theta3 += dTheta(3);

    glutPostRedisplay(); // Redessiner la scène après mise à jour
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

  bras();

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
