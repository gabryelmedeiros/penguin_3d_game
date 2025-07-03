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
#include <iostream>
#include <random>
#include <chrono> 

int frame = 1;

int min_val = -6;
int max_val = 6;
std::mt19937 gerador(std::chrono::system_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> distribuicao(min_val, max_val);

float cameraMoveX = 0;
float cameraMoveY = 0;
float cameraMoveZ = 0;

int cameraSelected = 1;

struct PenguinAttrib
{
  float dimensionX1 = cameraMoveX + 0.6;
  float dimensionX2 = cameraMoveX - 0.6;
  float dimensionZ1 = cameraMoveZ + 0.6;
  float dimensionZ2 = cameraMoveZ - 0.6;
};

PenguinAttrib penguinDimensions;

struct HoleAttrib
{
  float posX;
  float posY;
  float posZ;

  float dimensionX1 = posX + 1;
  float dimensionX2 = posX - 1;
  float dimensionZ1 = posZ + 1;
  float dimensionZ2 = posZ - 1;

  bool redraw = true;
};

HoleAttrib hole1;
HoleAttrib hole2;
HoleAttrib hole3;

struct FishAttributes
{
  float posX;
  float posY;
  float posZ;

  float dimensionX1 = posX + 0.8;
  float dimensionX2 = posX - 0.8;
  float dimensionZ1 = posZ + 0.4;
  float dimensionZ2 = posZ - 0.4;

  bool redraw = true;
};

FishAttributes fish1;
FishAttributes fish2;
FishAttributes fish3;
FishAttributes fish4;

bool checkHoleCollision(const PenguinAttrib& penguin, const HoleAttrib& hole)
{
  // Verifica a sobreposição no eixo X
  bool overlapX = (penguin.dimensionX1 > hole.dimensionX2 && penguin.dimensionX2 < hole.dimensionX1);

  // Verifica a sobreposição no eixo Z
  bool overlapZ = (penguin.dimensionZ1 > hole.dimensionZ2 && penguin.dimensionZ2 < hole.dimensionZ1);

  return overlapX && overlapZ;
}

bool checkFishCollision(const PenguinAttrib& penguin, const FishAttributes& fish)
{
  // Verifica a sobreposição no eixo X
  bool overlapX = (penguin.dimensionX1 > fish.dimensionX2 && penguin.dimensionX2 < fish.dimensionX1);

  // Verifica a sobreposição no eixo Z
  bool overlapZ = (penguin.dimensionZ1 > fish.dimensionZ2 && penguin.dimensionZ2 < fish.dimensionZ1);

  return overlapX && overlapZ;
}

void generateRandomHoleAttributes(){

  if(hole1.redraw == true){
    hole1.posX = distribuicao(gerador);
    hole1.posZ = distribuicao(gerador);

    hole1.dimensionX1 = hole1.posX + 1;
    hole1.dimensionX2 = hole1.posX - 1;
    hole1.dimensionZ1 = hole1.posZ + 1;
    hole1.dimensionZ2 = hole1.posZ - 1;

    hole1.redraw = false;
  }

  if(hole2.redraw == true){
    hole2.posX = distribuicao(gerador);
    hole2.posZ = distribuicao(gerador);

    hole2.dimensionX1 = hole2.posX + 1;
    hole2.dimensionX2 = hole2.posX - 1;
    hole2.dimensionZ1 = hole2.posZ + 1;
    hole2.dimensionZ2 = hole2.posZ - 1;

    hole2.redraw = false;
  }

  if(hole3.redraw == true){
    hole3.posX = distribuicao(gerador);
    hole3.posZ = distribuicao(gerador);

    hole3.dimensionX1 = hole3.posX + 1;
    hole3.dimensionX2 = hole3.posX - 1;
    hole3.dimensionZ1 = hole3.posZ + 1;
    hole3.dimensionZ2 = hole3.posZ - 1;

    hole3.redraw = false;
  }
}

void generateRandomFishAttributes(){
  if(fish1.redraw == true){
    fish1.posX = distribuicao(gerador);
    fish1.posZ = distribuicao(gerador);

    fish1.dimensionX1 = fish1.posX + 0.8;
    fish1.dimensionX2 = fish1.posX - 0.8;
    fish1.dimensionZ1 = fish1.posZ + 0.4;
    fish1.dimensionZ2 = fish1.posZ - 0.4;

    fish1.redraw = false;
  }

  if(fish2.redraw == true){
    fish2.posX = distribuicao(gerador);
    fish2.posZ = distribuicao(gerador);

    fish2.dimensionX1 = fish2.posX + 0.8;
    fish2.dimensionX2 = fish2.posX - 0.8;
    fish2.dimensionZ1 = fish2.posZ + 0.4;
    fish2.dimensionZ2 = fish2.posZ - 0.4;

    fish2.redraw = false;
  }

  if(fish3.redraw == true){
    fish3.posX = distribuicao(gerador);
    fish3.posZ = distribuicao(gerador);

    fish3.dimensionX1 = fish3.posX + 0.8;
    fish3.dimensionX2 = fish3.posX - 0.8;
    fish3.dimensionZ1 = fish3.posZ + 0.4;
    fish3.dimensionZ2 = fish3.posZ - 0.4;

    fish3.redraw = false;
  }

  if(fish4.redraw == true){
    fish4.posX = distribuicao(gerador);
    fish4.posZ = distribuicao(gerador);

    fish4.dimensionX1 = fish4.posX + 0.8;
    fish4.dimensionX2 = fish4.posX - 0.8;
    fish4.dimensionZ1 = fish4.posZ + 0.4;
    fish4.dimensionZ2 = fish4.posZ - 0.4;

    fish4.redraw = false;
  }
}

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
  glTranslated(0 + cameraMoveX,1.3 + cameraMoveY,0 + cameraMoveZ);
  glScaled(0.6,1,0.6);
  glutSolidSphere(1,30,30);
  glPopMatrix();

  glPopAttrib();
}

void babyPenguin(){
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
  glTranslated(0, 0.85, 0);
  glScaled(0.3,0.5,0.3);
  glutSolidSphere(1,30,30);
  glPopMatrix();

  glPopAttrib();
}

void fish(){
  glPushAttrib(GL_LIGHTING_BIT);
  float mat_ambient[] = { 0.4f, 0.4f, 0.1f, 1.0f };
  float mat_diffuse[] = { 0.8f, 0.8f, 0.2f, 1.0f };
  float mat_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
  float shine = 10.0f;

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, shine);

  glPushMatrix();
  glTranslated(0, 1, 0);
  glScaled(0.3,0.2,0.2);
  glutSolidSphere(1,30,30);
  glPopMatrix();

  glPushMatrix();
  glTranslated(-0.5,1,0);
  glRotated(90, 0, 1, 0);
  glScaled(0.2, 0.2, 0.2);
  glutSolidCone(1, 1, 10, 10); 
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
  glScaled(15, 1, 15);
  glutSolidCube(1);
  glPopMatrix();

  glPopAttrib();
}

void hole(){
  glPushAttrib(GL_LIGHTING_BIT); 
  float mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };   
  float mat_diffuse[] = { 0.0f, 0.0f, 0.0f, 1.0f };   
  float mat_specular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 
  float shine = 0.0f; 

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, shine);

  glPushMatrix();
  glTranslated(0,0.3,0);
  glScaled(1,0.1,1);
  glutSolidSphere(1,30,30);
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

  if(cameraSelected == 1){
    //camera posição livre
    glLoadIdentity();
    gluLookAt(5 + cameraMoveX,5 + cameraMoveY,5 + cameraMoveZ, 0 + cameraMoveX, 0 + cameraMoveY, 0 + cameraMoveZ,0,1,0);
  }
  else if(cameraSelected == 2){
    //camera vista de cima
    glLoadIdentity();
    gluLookAt(0 + cameraMoveX,15 + cameraMoveY,0 + cameraMoveZ,0 + cameraMoveX, 0 + cameraMoveY, 0 + cameraMoveZ,-1,0,0);
  }
  else if(cameraSelected == 3){
    //camera de frente pra cena
    glLoadIdentity();
    gluLookAt(0 + cameraMoveX,3 + cameraMoveY,7 + cameraMoveZ,0 + cameraMoveX, 0 + cameraMoveY, 0 + cameraMoveZ,0,1,0);
  }
  else if(cameraSelected == 4){
    //camera ao lado da cena
    glLoadIdentity();
    gluLookAt(7 + cameraMoveX,3 + cameraMoveY,0 + cameraMoveZ,0 + cameraMoveX, 0 + cameraMoveY, 0 + cameraMoveZ,0,1,0);
  }

  generateRandomHoleAttributes();
  generateRandomFishAttributes();

  penguin();
  babyPenguin();

  //Hole1
  glPushMatrix();
  glTranslated(hole1.posX,0,hole1.posZ);
  hole();
  glPopMatrix();

  //Hole2
  glPushMatrix();
  glTranslated(hole2.posX,0,hole2.posZ);
  hole();
  glPopMatrix();

  //Hole3
  glPushMatrix();
  glTranslated(hole3.posX,0,hole3.posZ);
  hole();
  glPopMatrix();

  //fish 1
  glPushMatrix();
  glTranslated(fish1.posX,0,fish1.posZ);
  fish();
  glPopMatrix();

  //fish 2
  glPushMatrix();
  glTranslated(fish2.posX,0,fish2.posZ);
  fish();
  glPopMatrix();

  //fish 3
  glPushMatrix();
  glTranslated(fish3.posX,0,fish3.posZ);
  fish();
  glPopMatrix();

  //fish 4
  glPushMatrix();
  glTranslated(fish4.posX,0,fish4.posZ);
  fish();
  glPopMatrix();

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

  GLfloat direction[] = { 20, 20, 20.0, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, direction);
  glEnable(GL_LIGHT0); 

}

void keyPressed(int key, int x, int y){
  switch(key){
    case GLUT_KEY_UP:
      if(cameraSelected != 3){
        cameraMoveX -= 0.2;
      }
      else{
        cameraMoveZ -= 0.2;
      }
      break;
    case GLUT_KEY_DOWN:
      if(cameraSelected != 3){
        cameraMoveX += 0.2;
      }
      else{
        cameraMoveZ += 0.2;
      }
      break;
    case GLUT_KEY_LEFT:
      if(cameraSelected != 3){
        cameraMoveZ += 0.2;
      }
      else{
        cameraMoveX += 0.2;
      }
      break;
    case GLUT_KEY_RIGHT:
      if(cameraSelected != 3){
        cameraMoveZ -= 0.2;
      }
      else{
        cameraMoveX -=0.2;
      }
      break;
  }

  penguinDimensions.dimensionX1 = cameraMoveX + 0.6;
  penguinDimensions.dimensionX2 = cameraMoveX - 0.6;
  penguinDimensions.dimensionZ1 = cameraMoveZ + 0.6;
  penguinDimensions.dimensionZ2 = cameraMoveZ - 0.6;

  bool holeCollision;

  holeCollision = checkHoleCollision(penguinDimensions, hole1);
  if (holeCollision){
    glutDestroyWindow(glutGetWindow());
    hole1.redraw = true;
  }

  holeCollision = checkHoleCollision(penguinDimensions, hole2);
  if (holeCollision){
    glutDestroyWindow(glutGetWindow());
    hole2.redraw = true;
  }

  holeCollision = checkHoleCollision(penguinDimensions, hole3);
  if (holeCollision){
    glutDestroyWindow(glutGetWindow());
    hole3.redraw = true;
  }

  bool fishCollision;

  fishCollision = checkFishCollision(penguinDimensions, fish1);
  if (fishCollision){
    fish1.redraw = true;
  }

  fishCollision = checkFishCollision(penguinDimensions, fish2);
  if (fishCollision){
    fish2.redraw = true;
  }

  fishCollision = checkFishCollision(penguinDimensions, fish3);
  if (fishCollision){
    fish3.redraw = true;
  }

  fishCollision = checkFishCollision(penguinDimensions, fish4);
  if (fishCollision){
    fish4.redraw = true;
  }

  glutPostRedisplay();
}

void keyPressedNormal(unsigned char key, int x, int y) {
   switch (key) {
      case '1':
        cameraSelected = 1;
        break;
      case '2':
        cameraSelected = 2;
        break;
      case '3':
        cameraSelected = 3;
        break;
      case '4':
        cameraSelected = 4;
        break;
    }

  glutPostRedisplay();
}

void ticker(int value){
  frame ++;

  if((frame % 500) == 0){
    hole1.redraw = true;
    hole2.redraw = true;
    hole3.redraw = true;
  }

  glutPostRedisplay();
  glutTimerFunc(20, ticker, 0);
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
  glutTimerFunc(20, ticker, 0);
  glutSpecialFunc(keyPressed);
  glutKeyboardFunc(keyPressedNormal);
  glutMainLoop();
}
