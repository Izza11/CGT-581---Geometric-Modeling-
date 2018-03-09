/********************************/
/* Lab03                        */
/* (C) Bedrich Benes 2018       */
/* bbenes ~ at ~ purdue.edu     */
/********************************/

#include <stdio.h>
#include <iostream>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <string>
#include <vector>			//Standard template library class
#include <GL/glew.h>
#include <GL/glut.h>
//in house created libraries
#include "math/vect3d.h"    //for vector manipulation

#pragma warning(disable : 4996)
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glut32.lib")

using namespace std;

const GLfloat thresholdIncrement = 0.001;
float threshold = 1.f;
bool needRedisplay=false;
GLfloat  sign=+1;
const GLfloat defaultIncrement=0.3;
GLfloat  angleIncrement=defaultIncrement;

const int MAX = 256;
float vox[MAX][MAX][MAX];


//the main window size
GLint wWindow=800;
GLint hWindow=600;



void InitArray()
{
	vector<Vect3d> elements;
	elements.push_back(Vect3d(50, 20, 100));
	elements.push_back(Vect3d(100, 50, 50));
	elements.push_back(Vect3d(70, 100, 50));

	float R = 70;
	float Fr = 0.0f;

	for (int i = 0; i<MAX; i++)
		for (int j = 0; j<MAX; j++)
			for (int k = 0; k < MAX; k++) {

				for (int n = 0; n < 2; n++) {
					//float r = sqrt(pow(i - MAX / (2*n), 2) + pow(j - MAX / (2*n), 2) + pow(k - MAX / (2*n), 2));

					float r = sqrt(pow(i - elements[n].x(), 2) + pow(j - elements[n].y(), 2) + pow(k - elements[n].z(), 2));

					float Fr1 = (4 * pow(r, 6)) / (9 * pow(R, 6));

					float Fr2 = (17 * pow(r, 4)) / (9 * pow(R, 4));

					float Fr3 = (22 * pow(r, 2)) / (9 * pow(R, 2));

					Fr += (1 - Fr1 + Fr2 - Fr3);
				}

				vox[i][j][k] = Fr;
				Fr = 0.0f;
				//comment out the line below to get random values
				//				vox[i][j][k] = rand() / (float)RAND_MAX;
			}
}



/*********************************
Some OpenGL-related functions
**********************************/
//displays the text message in the GL window
void GLMessage(char *message)
{
	static int i;
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.f,100.f,0.f,100.f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3ub(0,0,255);
	glRasterPos2i(10,10);
	for(i=0;i<(int)strlen(message); i++ ) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,message[i]);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

//called when a window is reshaped
void Reshape(int w, int h)
{
  glViewport(0,0,w, h);       
  glEnable(GL_DEPTH_TEST);
//remember the settings for the camera
  wWindow=w;
  hWindow=h;
}


//the main rendering function
void RenderObjects()
{
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glColor3f(0,0,1);
	glPointSize(2);
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	for (int i = 0; i<MAX; i++)
		for (int j = 0; j<MAX; j++)
			for (int k = 0; k < MAX; k++) {
				if (vox[i][j][k] < threshold) continue;
				float x = 3 * (i / (float)MAX - 0.5f);
				float y = 3 * (j / (float)MAX - 0.5f);
				float z = 3 * (k / (float)MAX - 0.5f);
				glVertex3f(x, y, z);
			}
	glEnd();
}
	

void Idle(void)
{
  glClearColor(1.9,1.9,1.9,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  GLMessage("Voxel Array");
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40,(GLfloat)wWindow/(GLfloat)hWindow,0.01,100); //set the camera
  glMatrixMode(GL_MODELVIEW); //set the scene
  glLoadIdentity();
  gluLookAt(0,5,5,0,0,0,0,1,0);
  static GLfloat angle=0;
  angle+=angleIncrement;
  if (angle>=360.f) angle=0.f;
  glRotatef(sign*angle,0,1,0);
  RenderObjects();
  glutSwapBuffers();  
}

void Display(void)
{

}

void Kbd(unsigned char a, int x, int y)//keyboard callback
{
	switch(a)
	{
 	  case 27 : exit(0);break;
	  case 32 : {
		  if (angleIncrement==0) angleIncrement=defaultIncrement;
						    else angleIncrement=0;
		  break;
		}
	  case 's': {sign=-sign;break;}
	  case '-': {
				  threshold -= thresholdIncrement;
				  break;
				}
	  case '+': {
				  threshold += thresholdIncrement;
				  break;
				}
	}
	cout << "[threshold]=["<< threshold << "]" << endl;
	glutPostRedisplay();
}


int main(int argc, char **argv)
{ 
  glutInitDisplayString("stencil>=2 rgb double depth samples");
  glutInit(&argc, argv);
  glutInitWindowSize(wWindow,hWindow);
  glutInitWindowPosition(500,100);
  glutCreateWindow("Lab03");
  GLenum err = glewInit();
  if (GLEW_OK != err){
  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(Kbd); //+ and -
  glutMouseFunc(NULL);
  glutSpecialUpFunc(NULL); 
  glutSpecialFunc(NULL);
  InitArray();
  glutMainLoop();
  return 0;        
}
