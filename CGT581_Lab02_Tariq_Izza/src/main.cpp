/*********************************/
/* CGT 581G Lab framework        */
/* (C) Bedrich Benes 2018        */
/* bbenes ~ at ~ purdue.edu      */
/* Press +,- to add/remove points*/
/*       r to randomize          */
/*       s to change rotation    */
/*       c to render curve       */
/*       t to render tangents    */
/*       p to render points      */
/*       s to change rotation    */
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
#include "helper.h"         //for vector manipulation

#pragma warning(disable : 4996)
#pragma comment(lib, "freeglut.lib")

using namespace std;

GLuint points = 0;  //number of points to display the object
int steps = 20;     //# of subdivisions
bool needRedisplay = false;
GLfloat  sign = +1; //diretcion of rotation
const GLfloat defaultIncrement = 0.7f; //speed of rotation
GLfloat  angleIncrement = defaultIncrement;
float deltaT = 1.0 / steps;
vector <Vect3d> v;   //all the control points will be stored here

					 //window size
GLint wWindow = 1200;
GLint hWindow = 800;

//this defines what will be rendered
//see Key() how is it controlled
bool pointsFlag = false;
bool curveFlag = true;
bool de_Cas = false;

int numOfseg = 0;
int numOfCpts = 0;
/**********************
LAB related MODIFY
***********************/


//This fills the <vector> *a with data. 
void CreateCurve(vector <Vect3d> *a, int n)
{
	Vect3d tmp;

	tmp.Set(0, 0, 0);
	a->push_back(tmp);
	tmp.Set(0, 3, 3);
	a->push_back(tmp);
	tmp.Set(3, 3, -3);
	a->push_back(tmp);
	tmp.Set(3, 0, 0);
	a->push_back(tmp);

	numOfseg++;
}

//Call THIS for a new curve. It clears the old one first
void InitArray(int n)
{
	v.clear();
	CreateCurve(&v, n);
}



//returns random number from <-1,1>
inline float random11() {
	return 2.f*rand() / (float)RAND_MAX - 1.f;
}

//randomizes an existing curve by adding random number to each coordinate
void Randomize(vector <Vect3d> *a) {
	const float intensity = 0.01f;
	for (unsigned int i = 0; i < a->size(); i++) {
		Vect3d r(random11(), random11(), random11());
		a->at(i) = a->at(i) + intensity*r;
	}
}

Vect3d Q(float t, Vect3d p0, Vect3d p1, Vect3d p2, Vect3d p3) {
	double b0 = pow(1 - t, 3);
	double b1 = 3 * t * pow(1 - t, 2);
	double b2 = 3 * pow(t, 2)*(1 - t);
	double b3 = pow(t, 3);
	return (p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3);

}

//Adds a random control point to curve
void addControlPoint(vector <Vect3d> *a, bool C1) {
	Vect3d tmp;

	if (C1) {
		tmp = a->at(a->size() - 1) + (a->at(a->size() - 1) - a->at(a->size() - 2));
	}
	else {
		tmp.Set(random11() * 3, random11() * 3, random11() * 3);
	}

	a->push_back(tmp);

	numOfCpts++;
}

void PopVector(vector <Vect3d> *a) {
	if (a->size() > 1) {
		if (numOfCpts == 0) {
			numOfCpts = 2;
			numOfseg--;
		}
		else {
			numOfCpts--;
		}
		a->pop_back();

	}

}

bool Condition1(vector <Vect3d> P) {     // check if distance between the 1st and last point of polygon is less than length of pixel
	if ((P[3] - P[0]).Length() <= 0.05) {
		return true;
	}
	return false;
}

bool Condition2(vector <Vect3d> P) {  // check if area of polygon is small enough
	float area = 0.0f;

	for (int i = 0; i < 4; i++) {
		area += P[i].x()*P[(i + 1) % 4].y() - P[(i + 1) % 4].x()*P[i].y();
	}

	if (abs(area / 2.0f) <= 0.0005f) {
		return true;
	}

	return false;
}

bool FlatEnough(vector <Vect3d> P) {
	return Condition1(P);
	//return Condition2(P);
}

void SplitCurve(vector<Vect3d> P, int p0, vector<Vect3d> *left, vector<Vect3d> *right) {
	Vect3d hlp;
	left->at(0) = P[p0];
	left->at(1) = (P[p0] + P[p0 + 1]) / 2.f;
	hlp = (P[p0 + 1] + P[p0 + 2]) / 2.f;
	left->at(2) = (left->at(1) + hlp) / 2.f;
	right->at(3) = P[p0 + 3];
	right->at(2) = (P[p0 + 2] + P[p0 + 3]) / 2.f;
	right->at(1) = (right->at(2) + hlp) / 2.f;
	left->at(3) = right->at(0) = (left->at(2) + right->at(1)) / 2.f;
}

void DeCasteljau(vector <Vect3d> P, int p0, Vect3d color) {  // p0 is the starting index of every new control polygon formed by adding control points

	vector<Vect3d> left(4);
	vector<Vect3d> right(4);

	if (FlatEnough(P)) {

		DrawLine(P[p0], P[p0 + 3], color);

		return;
	}

	SplitCurve(P, p0, &left, &right);
	DeCasteljau(left, 0, color);
	DeCasteljau(right, 0, color);
}

void DrawDeCasteljau(vector <Vect3d> P, Vect3d color) {


	if (numOfCpts >= 3) {  // check if 3 more Control points have been added to form a new segment for Bezeir curve
		numOfCpts = 0;
		numOfseg++;
	}

	for (int i = 0; i < numOfseg * 3; i = i + 3) {
			DeCasteljau(P, i, color);
	}
}


void Bezier(Vect3d p0, Vect3d p1, Vect3d p2, Vect3d p3, Vect3d color) {

	for (int i = 0; i < steps; i++) {
		DrawLine(Q(deltaT*i, p0, p1, p2, p3), Q(deltaT*(i + 1), p0, p1, p2, p3), color);
	}

}

void DrawBezier(vector <Vect3d> a, Vect3d color) {


	if (numOfCpts >= 3) {  // check if 3 more Control points have been added to form a new segment for Bezeir curve
		numOfCpts = 0;
		numOfseg++;
	}

	for (int i = 0; i < numOfseg * 3; i = i + 3) {

		Bezier(a[i], a[i + 1], a[i + 2], a[i + 3], color);

	}
}


//this is the actual code for the lab
void Lab02() {
	Vect3d a, b, c;
	Vect3d origin(0, 0, 0);
	Vect3d red(1, 0, 0), green(0, 1, 0), blue(0, 0, 1), almostBlack(0.1f, 0.1f, 0.1f), yellow(1, 1, 0);


	//	CoordSyst();
	//draw the curve
	if (curveFlag) {
		DrawBezier(v, almostBlack);
	}

	if (de_Cas) {

		DrawDeCasteljau(v, red);
	}

	//draw the control points and the control polygon
	if (pointsFlag) {
		for (unsigned int i = 0; i < v.size(); i++) {
			DrawPoint(v[i], blue);
		}
		for (unsigned int i = 0; i < v.size() - 1; i++) {
			DrawLine(v[i], v[i + 1], yellow);
		}
	}

}

//the main rendering function
void RenderObjects()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//call the student's code from here
	Lab02();
}

//Add here if you want to control some global behavior
//see the pointFlag and how is it used
void Kbd(unsigned char a, int x, int y)//keyboard callback
{
	switch (a)
	{
	case 27: exit(0); break;
	case 'p': pointsFlag = !pointsFlag; break;
	case 'c': curveFlag = !curveFlag; break;
	case 32: {
		if (angleIncrement == 0) angleIncrement = defaultIncrement;
		else angleIncrement = 0;
		break;
	}
	case 's': {sign = -sign; break; }
	case '-': {
		steps--;
		if (steps < 1) steps = 1;
		InitArray(steps);
		break;
	}
	case '+': {
		steps++;
		InitArray(steps);
		break;
	}
	case 'r': {
		Randomize(&v);
		break;
	} case 'A': {
		addControlPoint(&v, false);  // C1 continuity is set to false here

		break;
	} case 'P': {
		addControlPoint(&v, true);  // C1 continuity is set to true here
		break;
	}	
	case 'Z': {		
		PopVector(&v);
		break;
	}
	case 'D': {   // draw by de Casteljau algorithm
		curveFlag = !curveFlag;
		de_Cas = !de_Cas;
		break;
	}

	}  // switch ends here
	   cout << "[points]=[" << steps << "]" << endl;
	glutPostRedisplay();
}


/*******************
OpenGL code. Do not touch.
******************/
void Idle(void)
{
	glClearColor(0.5f, 0.5f, 0.5f, 1); //background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLMessage("Lab 1 - CGT 581G");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, (GLfloat)wWindow / (GLfloat)hWindow, 0.01, 100); //set the camera
	glMatrixMode(GL_MODELVIEW); //set the scene
	glLoadIdentity();
	gluLookAt(0, 10, 10, 0, 0, 0, 0, 1, 0); //set where the camera is looking at and from. 
	static GLfloat angle = 0;
	angle += angleIncrement;
	if (angle >= 360.f) angle = 0.f;
	glRotatef(sign*angle, 0, 1, 0);
	RenderObjects();
	glutSwapBuffers();
}


int main(int argc, char **argv)
{
	glutInitDisplayString("stencil>=2 rgb double depth samples");
	glutInit(&argc, argv);
	glutInitWindowSize(wWindow, hWindow);
	glutInitWindowPosition(500, 100);
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
	InitArray(steps);
	glutMainLoop();
	return 0;
}
