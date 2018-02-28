/*********************************/
/* Bezier Bicubic  Surface       */
/* (C) Bedrich Benes 2018        */
/* bbenes ~ at ~ purdue.edu      */
/* Press +,- to add/remove points*/
/*       r to randomize          */
/*       s to change rotation    */
/*       c to render curve       */
/*       t to render tangents    */
/*       p to render points      */
/*       s to change rotation    */
/*       w to savel to OBJ file  */
/*********************************/

#include <stdio.h>
#include <iostream>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <string>
#include <vector>			//Standard template library class
#include <GL/freeglut.h>

//in house created libraries
#include "math/vect3d.h"    //for vector manipulation
#include "math/triangle.h"  //triangles
#include "helper.h"         
#include "objGen.h"         //to save OBJ file format for 3D printing

#pragma warning(disable : 4996)
#pragma comment(lib, "freeglut.lib")

using namespace std;

GLuint points=0;  //number of points to display the object
bool needRedisplay=false;
GLfloat  sign=+1; //diretcion of rotation
const GLfloat defaultIncrement=0.7f; //speed of rotation
GLfloat  angleIncrement=defaultIncrement;
GLuint stacks = 5, slices = 5;

vector <TriangleC> v;   //all the triangles will be stored here

Vect3d cv[4][4]; //control polygon 4x4
Vect3d cv2[4][4]; //control polygon 4x4
Vect3d cv3[4][4]; //control polygon 4x4
Vect3d cv4[4][4]; //control polygon 4x4
Vect3d red(1, 0, 0), green(0, 1, 0), blue(0, 0, 1), almostBlack(0.1f, 0.1f, 0.1f), yellow(1, 1, 0);

//window size
GLint wWindow=1200;
GLint hWindow=800;

//this defines what will be rendered
//see Key() how is it controlled
bool pointsFlag = false;
bool drawFlag = true;


void DisplayPolygon(Vect3d cv[4][4], Vect3d cv2[4][4], Vect3d cv3[4][4], Vect3d cv4[4][4]) {

	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 3; j++)
		DrawLine(cv[i][j], cv[i][j + 1], yellow);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			DrawLine(cv[i][j], cv[i + 1][j], yellow);

	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 3; j++)
		DrawLine(cv2[i][j], cv2[i][j + 1], yellow);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			DrawLine(cv2[i][j], cv2[i + 1][j], yellow);

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
			DrawLine(cv3[i][j], cv3[i][j + 1], yellow);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			DrawLine(cv3[i][j], cv3[i + 1][j], yellow);

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
			DrawLine(cv4[i][j], cv4[i][j + 1], yellow);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			DrawLine(cv4[i][j], cv4[i + 1][j], yellow);

	glPointSize(10);
	glColor3fv(blue);
	glBegin(GL_POINTS);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			glVertex3fv(cv[i][j]);
	glEnd();

}


//returns point on Bezier surface defined by cv at [u,v]
Vect3d Q(float u, float v, Vect3d cv[4][4])
{
	std::vector<double> Bu;
	std::vector<double> Bv;
	Bu.push_back(pow(1 - u, 3));
	Bu.push_back(3 * u * pow(1 - u, 2));
	Bu.push_back(3 * pow(u, 2)*(1 - u));
	Bu.push_back(pow(u, 3));

	Bv.push_back(pow(1 - v, 3));
	Bv.push_back(3 * v * pow(1 - v, 2));
	Bv.push_back(3 * pow(v, 2)*(1 - v));
	Bv.push_back(pow(v, 3));

	Vect3d tmp;
	tmp.Set(0, 0, 0);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tmp += cv[i][j] * Bu[i] * Bv[j];
		
		}
	}
	
	return tmp;
}



//Tesselate the Bezier Surface and 
//fill the <TriangleC> *a with data. 
void CreateSurface(vector <TriangleC> *tri, unsigned int stacks, unsigned int slices, Vect3d cv[4][4])
{
	TriangleC tmp;

	Vect3d a,b,c;

	GLfloat deltau = 1.f / (GLfloat)stacks;
	GLfloat deltav = 1.f / (GLfloat)slices;
	for (GLuint i = 0; i<stacks; i++)
	{
		GLfloat u = i * deltau;
		for (GLuint j = 0; j<slices; j++)
		{
			GLfloat v = j * deltav;
			//the first triangle
			a.Set(Q(u,          v, cv));
			b.Set(Q(u + deltau, v, cv));
			c.Set(Q(u,          v + deltav, cv));
			tmp.Set(a, b, c);
			tri->push_back(tmp);

			//the second triangle
			a.Set(Q(u + deltau, v + deltav, cv));
			tmp.Set(c, b, a	);
			tri->push_back(tmp);
		}
	}

}


//Call THIS for a new object. It clears the old one first
void InitArray(unsigned int stacks, unsigned int slices)
{
	v.clear();
	for (int i = 0; i<4; i++)
	for (int j = 0; j<4; j++)
		cv[i][j].Set(i, 0, j);
	cv[1][0].Set(1, 3, 0);
	cv[2][0].Set(2, 3, 0);
	cv[2][2].Set(2, 3, 2);
	cv[1][1].Set(1, 3, 1); 
	cv[3][3].Set(3, 1, 3); 

	CreateSurface(&v, stacks, slices, cv);

	//////////////////////////////  2nd surface

	for(int i = 0; i < 4; i++) {
		cv2[0][i].Set(cv[3][i]);
		cv2[1][i].Set(2*cv[3][i] - cv[2][i]);
	}

	for (int i = 2; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cv2[i][j].Set(i+3, 0, j);
		}
	}
	CreateSurface(&v, stacks, slices, cv2);

	//////////////////////////////  3rd surface

	for (int i = 0; i < 4; i++) {
		cv3[i][0].Set(cv[i][3]);
		cv3[i][1].Set(2*cv[i][3] - cv[i][2]);
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 2; j < 4; j++) {
			cv3[i][j].Set(i, 0, j+3);
		}
	}
	CreateSurface(&v, stacks, slices, cv3);

	//////////////////////////////  4th surface

	for (int i = 0; i < 4; i++) {
		cv4[i][0].Set(cv2[i][3]);
		cv4[i][1].Set(2 * cv2[i][3] - cv2[i][2]);
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 2; j < 4; j++) {
			cv4[i][j].Set(i+3, 0, j + 3);
		}
	}
	CreateSurface(&v, stacks, slices, cv4);

}

//returns random number from <-1,1>
inline float random11() { 
	return 2.f*rand() / (float)RAND_MAX - 1.f;
}

//randomizes an existing curve by adding random number to each coordinate
void Randomize(vector <TriangleC> *a) {
	const float intensity = 0.01f;
	for (unsigned int i = 0; i < a->size(); i++) {
		Vect3d r(random11(), random11(), random11());
		a->at(i).a = a->at(i).a + intensity * r;
		r.Set(random11(), random11(), random11());
		a->at(i).b = a->at(i).b + intensity * r;
		r.Set(random11(), random11(), random11());
		a->at(i).c = a->at(i).c + intensity * r;
	}
}

//Display previously stored triangles
void DrawSurface(vector <TriangleC> *v, Vect3d color) {
	glColor3fv(color);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < v->size(); i++) {
		glVertex3fv(v->at(i).a);
		glVertex3fv(v->at(i).b);
		glVertex3fv(v->at(i).c);
	}
	glEnd();

}

//this is the actual code for the lab
void Lab03() {
	Vect3d a,b,c;
	Vect3d origin(0, 0, 0);


	CoordSyst();
	if (pointsFlag) DisplayPolygon(cv, cv2, cv3, cv4);
	//draw the curve
	if (drawFlag){
		DrawSurface(&v, almostBlack);
	}

}

//the main rendering function
void RenderObjects()
{
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
//call the student's code from here
	Lab03();
}

//Add here if you want to control some global behavior
//see the pointFlag and how is it used
void Kbd(unsigned char a, int x, int y)//keyboard callback
{
	switch (a)
		{
		case 27: exit(0); break;
		case 'p': pointsFlag = !pointsFlag; break;
		case 'c': drawFlag = !drawFlag; break;
		case 32: {
			if (angleIncrement == 0) angleIncrement = defaultIncrement;
			else angleIncrement = 0;
			break;
		}
		case 's': {sign = -sign; break; }
		case 'w': {SaveOBJ(&v,"geometry.obj"); break; }
		case 'r': {
			Randomize(&v);
			break;
		}
		case 'a': {stacks++; InitArray(stacks, slices); break; }
		case 'A': {
			stacks--;
			if (stacks < 2) stacks = 2;
			InitArray(stacks, slices);
			break;
		}
		case 'b': {slices++; InitArray(stacks, slices); break; }
		case 'B': {
			slices--;
			if (slices < 2) slices = 2;
			InitArray(stacks, slices);
			break;
		}
	}
	cout << "[stacks,slices]=[" << stacks << "," << slices << "]" << endl;
	glutPostRedisplay();
}


/*******************
OpenGL code. Do not touch.
******************/
void Idle(void)
{
  glClearColor(0.5f,0.5f,0.5f,1); //background color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  GLMessage("Bezier Surface - CGT 581G");
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40,(GLfloat)wWindow/(GLfloat)hWindow,0.01,100); //set the camera
  glMatrixMode(GL_MODELVIEW); //set the scene
  glLoadIdentity();
  gluLookAt(0,10,10,0,0,0,0,1,0); //set where the camera is looking at and from. 
  static GLfloat angle=0;
  angle+=angleIncrement;
  if (angle>=360.f) angle=0.f;
  glRotatef(sign*angle,0,1,0);
  RenderObjects();
  glutSwapBuffers();  
}


int main(int argc, char **argv)
{ 
  glutInitDisplayString("stencil>=2 rgb double depth samples");
  glutInit(&argc, argv);
  glutInitWindowSize(wWindow,hWindow);
  glutInitWindowPosition(500,100);
  glutCreateWindow("Surface of Revolution");
  //GLenum err = glewInit();
  // if (GLEW_OK != err){
  // fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  //}
  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(Kbd); //+ and -
  glutMouseFunc(NULL);
  glutSpecialUpFunc(NULL); 
  glutSpecialFunc(NULL);
  InitArray(stacks, slices);
  glutMainLoop();
  return 0;        
}
