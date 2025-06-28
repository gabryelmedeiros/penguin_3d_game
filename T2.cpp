// https://cs.lmu.edu/~ray/notes/openglexamples/
// g++ litsolids.cpp -lX11 -lGL -lGLU -lglut -g -Wall -O2 -o r.exe
//
// This program shows three cyan objects illuminated with a single yellow
// light source.  It illustrates several of the lighting parameters.

#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>   
#include <math.h>   
#include <ctime>  

void penguin(){
  glPushAttrib(GL_LIGHTING_BIT); //isso daqui é a mesma coisa do pushMatrix, porém para materiais
  float mat_ambient[] ={0.02f, 0.02f, 0.02f, 1.0f };
  float mat_diffuse[] ={0.01f, 0.01f, 0.01f, 1.0f };
  float mat_specular[] ={0.4f, 0.4f, 0.4f, 1.0f };
  float shine =32.0f ;

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, shine);

  glPushMatrix();
  glTranslated(0,1,0);
  glScaled(0.6,1,0.6);
  glutSolidSphere(1,30,30);
  glPopMatrix();

  glPopAttrib();
}

void icePlatform(){
  glPushAttrib(GL_LIGHTING_BIT);

  float mat_ambient[] = { 0.1f, 0.15f, 0.2f, 0.3f }; 
  float mat_diffuse[] = { 0.1f, 0.15f, 0.2f, 0.3f }; 
  float mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  float shine = 128.0f;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);

  glPushMatrix();
  glScaled(10, 1, 10);
  glutSolidCube(1);
  glPopMatrix();

  glPopAttrib();
}

void water(){
  glPushAttrib(GL_LIGHTING_BIT);
  float mat_ambient[] ={ 0.0f,0.1f,0.06f ,1.0f};
  float mat_diffuse[] ={ 0.0f,0.50980392f,0.50980392f,1.0f};
  float mat_specular[] ={0.50196078f,0.50196078f,0.50196078f,1.0f };
  float shine =32.0f;

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, shine);

  glPushMatrix();
  glScaled(100,0.2,100);
  glutSolidCube(1);
  glPopMatrix();

  glPopAttrib();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(5,5,5,0,0,0,0,1,0);

  penguin();
  water();
  glDepthMask(GL_FALSE);  // desativa escrita no buffer de profundidade
  icePlatform();          // desenha o objeto transparente
  glDepthMask(GL_TRUE);
   
  glutSwapBuffers();
}

void reshape(GLint w, GLint h) {
  glMatrixMode(GL_PROJECTION);
  GLfloat aspect = GLfloat(w) / GLfloat(h);
  glLoadIdentity();

  gluPerspective (45,aspect, 0.1, 40);

}

void init() {
  glMatrixMode(GL_MODELVIEW);

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_NORMALIZE);

  glShadeModel(GL_SMOOTH);

  glEnable(GL_LIGHTING);

  GLfloat direction[] = { 20, 20, 0.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, direction);
  glEnable(GL_LIGHT0); 

}

int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowPosition(80, 80);
  glutInitWindowSize(800, 600);
  glutCreateWindow("Trabalho 2");
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  init();
  glutMainLoop();
}