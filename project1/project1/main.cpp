/**************************************************************
 * by:LiXiangxian                                             *
 * 2018.11.4                                                  *
 * read .wrl file in cmd line:                                *
 * ./execute.exe name_of_model.wrl name_of_new_model.wrl      *
 * 2018.11.23                                                 *
 * change the way to input model.                             *
 * this .exe will creat a folder named "model_subdivision"    *
 * in project directory， then save model in it               *
 * 2018.11.25                                                 *
 * add visualization                                          *
 **************************************************************/
#include<iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include <io.h>
#include <direct.h>
#include<windows.h>
#include <GL\GL.h>
#include <GL\glut.h>
#include "halfEdge_structure.h"
#include "LoopSubdivision.h"

using namespace std;

void loopSubdivision(HalfEdge_mesh *inputMesh, HalfEdge_mesh *outputMesh);
void ModifyButterflySubdivision(HalfEdge_mesh *inputMesh, HalfEdge_mesh *outputMesh);
void setNormal(HalfEdge_mesh *mesh);
void findTwin(HalfEdge_mesh *mesh, unsigned halfedge_count);
void setCrease(HalfEdge_mesh *mesh);

std::string fireHead = "";
string faceStart = "";
string fireEnd = "";
unsigned vertexNumber = 0;
unsigned faceNumber = 0;
unsigned halfedgeNumber = 0;

unsigned winWidth = 800, winHeight = 600;
GLuint faceList, wireList;
int mousetate = 0;
GLfloat oldX = 0.0f;
GLfloat oldY = 0.0f;
float xRotate = 0.0f;
float yRotate = 0.0f;
float ty = 0.0f;
float scale = 1.0f;

int showstate = 1;
bool showFace = true;
bool showWire = false;
bool showFlatlines = false;

//check the head,just for this file version
bool checkHead(string s)
{
	return(s == "#VRMLV2.0utf8(ConvertedtoASCII)Shape{geometryIndexedFaceSet{coordCoordinate{point[");
}

//check string between vertexs and faces
bool checkFace(string s)
{
	return(s == "}coordIndex[");
}

//check end
bool checkEnd(string s)
{
	return(s == "}}");
}

//convert string to double
double stringToDouble(const string& str)
{
	istringstream iss(str);
	double num;
	iss >> num;
	return num;
}

//convert string to int
int stringToInt(const string& str)
{
	istringstream iss(str);
	int num;
	iss >> num;
	return num;
}
unsigned countLine(string file)
{
	ifstream wrlFile;
	unsigned a = 0;
	wrlFile.open(file.data());   //connect file stream
	assert(wrlFile.is_open());   //if fail, warning and exit
	string tempLine;
	
	while (getline(wrlFile,tempLine)) a++;
	return a;
}
//read file, generate vertex table, halfedge table and face table
int readWrl(string file, HalfEdge_mesh* mesh)
{
	ifstream wrlFile;
	wrlFile.open(file.data());   //connect file stream
	assert(wrlFile.is_open());   //if fail, warning and exit

	HalfEdge_vertex vertex_temp;
	unsigned count_vertex_read = 0;
	unsigned count_face_read = 0;
	unsigned count_halfedge_read = 0;
	int edge[3];
	string sTemp;
	bool headChecked = false;
	bool vertexEnd = false;
	bool pointSettled = false;
	bool dataSettled = false;
	while (wrlFile >> sTemp)
	{
		//head check
		if (!headChecked) {
			if ("[" == sTemp || "point[" == sTemp) {
				fireHead += sTemp;
				if (checkHead(fireHead)) {
					headChecked = true;
					continue;
				}
				else return -1;
			}
			else fireHead += sTemp;
		}
		//set point coordinate
		if (headChecked && !pointSettled) {
			if (vertexEnd) {
				faceStart += sTemp;
				if (checkFace(faceStart))
					pointSettled = true;
				else if (wrlFile.eof() && !checkFace(faceStart))
					return -2;
				continue;
			}

			if ("]" == sTemp) {
				vertexEnd = true;
				continue;
			}
			if (0 == count_vertex_read % 3)
				vertex_temp.x = stringToDouble(sTemp);
			if (1 == count_vertex_read % 3)
				vertex_temp.y = stringToDouble(sTemp);
			if (2 == count_vertex_read % 3) {
				vertex_temp.z = stringToDouble(sTemp);
				(*mesh).vertex.push_back(vertex_temp);
				(*mesh).vertex[count_vertex_read / 3].number = count_vertex_read / 3;
			}
			count_vertex_read++;
		}
		//set face and halfedge
		if (pointSettled && !dataSettled){
			if ("]" == sTemp) {
				findTwin(mesh,count_halfedge_read);
				//findOrigin(vector<HalfEdge_halfedge> halfedge, vector<HalfEdge_vertex> vertex);
				dataSettled = true;
				continue;
			}
			else if ("-1" == sTemp) {
				HalfEdge_halfedge edge_temp1;
				HalfEdge_halfedge edge_temp2;
				HalfEdge_halfedge edge_temp3;
				HalfEdge_face face_temp;
				(*mesh).halfedge.push_back(edge_temp1);
				(*mesh).halfedge.push_back(edge_temp2);
				(*mesh).halfedge.push_back(edge_temp3);
				//face
				(*mesh).face.push_back(face_temp);
				(*mesh).face[count_face_read].boundary = &(*mesh).halfedge[count_halfedge_read - 3];
				//halfedge's origin & vertexs
				(*mesh).halfedge[count_halfedge_read - 3].origin = &(*mesh).vertex[edge[0]];
				if (!(*mesh).vertex[edge[0]].beenOrigined) {
					(*mesh).vertex[edge[0]].asOrigin = &(*mesh).halfedge[count_halfedge_read - 3];
					(*mesh).vertex[edge[0]].beenOrigined = true;
				}
				((*mesh).halfedge[count_halfedge_read - 2]).origin = &(*mesh).vertex[edge[1]];
				if (!(*mesh).vertex[edge[1]].beenOrigined) {
					(*mesh).vertex[edge[1]].asOrigin = &(*mesh).halfedge[count_halfedge_read - 2];
					(*mesh).vertex[edge[1]].beenOrigined = true;
				}
				(*mesh).halfedge[count_halfedge_read - 1].origin = &(*mesh).vertex[edge[2]];
				if (!(*mesh).vertex[edge[2]].beenOrigined){
					(*mesh).vertex[edge[2]].asOrigin = &(*mesh).halfedge[count_halfedge_read - 1];
					(*mesh).vertex[edge[2]].beenOrigined = true;
				}
				//halfedge's nextedge
				(*mesh).halfedge[count_halfedge_read - 3].nextEdge = &(*mesh).halfedge[count_halfedge_read - 2];
				(*mesh).halfedge[count_halfedge_read - 2].nextEdge = &(*mesh).halfedge[count_halfedge_read - 1];
				(*mesh).halfedge[count_halfedge_read - 1].nextEdge = &(*mesh).halfedge[count_halfedge_read - 3];
				//halfedge's preedge
				(*mesh).halfedge[count_halfedge_read - 3].preEdge = &(*mesh).halfedge[count_halfedge_read - 1];
				(*mesh).halfedge[count_halfedge_read - 2].preEdge = &(*mesh).halfedge[count_halfedge_read - 3];
				(*mesh).halfedge[count_halfedge_read - 1].preEdge = &(*mesh).halfedge[count_halfedge_read - 2];
				//halfedge's incidentface
				(*mesh).halfedge[count_halfedge_read - 3].incidentFace = &(*mesh).face[count_face_read];
				(*mesh).halfedge[count_halfedge_read - 2].incidentFace = &(*mesh).face[count_face_read];
				(*mesh).halfedge[count_halfedge_read - 1].incidentFace = &(*mesh).face[count_face_read];
				//halfedge's name
				(*mesh).halfedge[count_halfedge_read - 3].number = count_halfedge_read - 3;
				(*mesh).halfedge[count_halfedge_read - 2].number = count_halfedge_read - 2;
				(*mesh).halfedge[count_halfedge_read - 1].number = count_halfedge_read - 1;
				count_face_read++;
			}
			//a set contains three vertexs
			else if (0 == count_halfedge_read % 3) {
				edge[0] = stringToInt(sTemp);
				count_halfedge_read++;
			}
			else if (1 == count_halfedge_read % 3) {
				edge[1] = stringToInt(sTemp);
				count_halfedge_read++;
			}
			else if (2 == count_halfedge_read % 3) {
				edge[2] = stringToInt(sTemp);
				count_halfedge_read++;
			}
		}
		//end check
		if (dataSettled) {
			fireEnd += sTemp;
			if (checkEnd(fireEnd)) {
				setNormal(mesh);
				setCrease(mesh);
				break;
			}
			else if (wrlFile.eof() && !checkEnd(fireEnd))
				return -3;
		}
	}
	halfedgeNumber = count_halfedge_read;
	vertexNumber = count_vertex_read / 3;
	faceNumber = count_face_read;
	wrlFile.close();
	return 0;
}
void writeFile(HalfEdge_mesh *outputMesh,string newfile)
{
	std::ofstream outfile(newfile);
	outfile << "#VRML V2.0 utf8 (Converted to ASCII)\nShape {\n\tgeometry IndexedFaceSet\n\t{\n\t\tcoord Coordinate\n\t\t{\n\t\t\tpoint [\n" << std::endl;
	for (auto it = (*outputMesh).vertex.begin(); it != (*outputMesh).vertex.end(); it++) {
		outfile << "\t\t\t" << (*it).x << "  " << (*it).y << "  " << (*it).z << endl;
	}
	outfile << "\t\t\t]\n\t\t}\n\t\tcoordIndex [" << endl;
	for (auto it = (*outputMesh).face.begin(); it != (*outputMesh).face.end(); it++) {
		outfile
			<< "\t\t\t"
			<< (*(*(*it).boundary).origin).number << " "
			<< (*(*(*(*it).boundary).nextEdge).origin).number << " "
			<< (*(*(*(*it).boundary).preEdge).origin).number << " "
			<< "-1" <<endl;
	}
	outfile << "\t\t]\n\t}\n} " << endl;
	outfile.close();
	cout << "File write completed.";
}
/*-------------------------------Visualization----------------------------------------*/
//init mesh
void initMesh(HalfEdge_mesh *mesh)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(2.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0); 

	faceList = glGenLists(1);
	wireList = glGenLists(1);
	
	//draw wire
	glNewList(wireList, GL_COMPILE);
	glDisable(GL_LIGHTING);
	glLineWidth(2.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	for (auto it = (*mesh).halfedge.begin(); it != (*mesh).halfedge.end(); it++) {
		//draw triangles
		glVertex3f((*(*it).origin).x, (*(*it).origin).y, (*(*it).origin).z);
		glVertex3f((*(*(*it).twin).origin).x, (*(*(*it).twin).origin).y, (*(*(*it).twin).origin).z);
	}
	glEnd();
	glEnable(GL_LIGHTING);
	glEndList();

	//draw face
	glNewList(faceList, GL_COMPILE);
	for (auto it = (*mesh).face.begin(); it != (*mesh).face.end(); it++) {
		glBegin(GL_POLYGON);
		HalfEdge_halfedge *e0 = (*it).boundary;
		HalfEdge_halfedge *e1 = (*(*it).boundary).nextEdge;
		HalfEdge_halfedge *e2 = (*(*it).boundary).preEdge;
		glNormal3f((*(*it).normal).x, (*(*it).normal).y, (*(*it).normal).z);
		glVertex3f((*(*e0).origin).x, (*(*e0).origin).y, (*(*e0).origin).z);
		glVertex3f((*(*e1).origin).x, (*(*e1).origin).y, (*(*e1).origin).z);
		glVertex3f((*(*e2).origin).x, (*(*e2).origin).y, (*(*e2).origin).z);
		glEnd();
	}
	glEndList();
}
//mouse event
void myMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mousetate = 1;
		oldX = x;
		oldY = y;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		mousetate = 0;
	glutPostRedisplay();
}
//mouse move
void onMouseMove(int x, int y)
{
	if (mousetate) {
		yRotate += x - oldX;
		glutPostRedisplay();
		oldX = x;
		xRotate += y - oldY;
		glutPostRedisplay();
		oldY = y;
	}
}
//window
void myReshape(GLint w, GLint h)
{
	glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w > h)
		glOrtho(-static_cast<GLdouble>(w) / h, static_cast<GLdouble>(w) / h, -1.0, 1.0, -100.0, 100.0);
	else
		glOrtho(-1.0, 1.0, -static_cast<GLdouble>(h) / w, static_cast<GLdouble>(h) / w, -100.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void myDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glRotatef(xRotate, 1.0f, 0.0f, 0.0f); 
	glRotatef(yRotate, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, ty);
	glScalef(scale, scale, scale); 

	if (showFace)
		glCallList(faceList);
	if (showFlatlines) {
		glCallList(faceList);
		glCallList(wireList);
	}
	if (showWire)
		glCallList(wireList);
	glutSwapBuffers();
}
//keyboard
void mySpecial(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_F1:{
			if (showFace == true) {
				showFace = false;
				showWire = true;
			}
			else if (showWire == true) {
				showWire = false;
				showFlatlines = true;
			}
			else if (showFlatlines == true) {
				showFlatlines = false;
				showFace = true;
			}
			break;
		}
		case GLUT_KEY_F2:
			exit(0);
	}
}
/*---------------------------end-Visualization------------------------------------------*/
//main
int main(int argc, char** argv)
{
	int inputFileNumber;
	int subdivisionType;
	string inputFileName = "",outputFileName = "";
	vector<HalfEdge_mesh> iterationMesh;
	/*--------------------------Choose Model---------------------------------*/
	cout << "-----------------------------" << endl;
	cout << "Enter number to choose model:" << endl;
	cout << "1 : Tetrahedron\n"
		 << "2 : Cube\n"
		 << "3 : T-Shape\n"
		 << "4 : igea_ascii_Simp1000\n"
		 << "5 : igea_ascii_Simp4000" << endl;
	cout << "-----------------------------" << endl;
	if (_access("../model_subdivision", 0) == -1)
	{
		cout << "new folder created";
		_mkdir("../model_subdivision");
	}
	do {
		cin >> inputFileNumber;
		switch (inputFileNumber) {
		case 1:{
			inputFileName = "../model/Tetrahedron.wrl";
			outputFileName = "../model_subdivision/Tetrahedron_1.wrl";
			scale = 0.8;
			break;
		}
		case 2:{
			inputFileName = "../model/Cube.wrl";
			outputFileName = "../model_subdivision/Cube_1.wrl";
			scale = 0.5;
			break;
		}
		case 3:{
			inputFileName = "../model/Tshape.wrl";
			outputFileName = "../model_subdivision/Tshape_1.wrl";
			scale = 0.5;
			break;
		}
		case 4:{
			inputFileName = "../model/igea_ascii_Simp1000.wrl";
			outputFileName = "../model_subdivision/igea_ascii_Simp1000_1.wrl";
			scale = 10;
			break;
		}
		case 5:{
			inputFileName = "../model/igea_ascii_Simp4000.wrl";
			outputFileName = "../model_subdivision/igea_ascii_Simp4000_1.wrl";
			scale = 10;
			break;
		}
		default:{
			cout << "input error!" << endl;
		}
		}
	} while (inputFileName == "");
	//cout << argv[1] << endl;
	//unsigned vectorPreserve = countLine(argv[1]);
	/*--------------------------end model choosing---------------------------*/
	cout << "Choose a subdivision type:\n1.Loop subdivision\n2.Modify Butterfly subdivision" << endl;
	cin >> subdivisionType;
	while (subdivisionType != 1 && subdivisionType != 2) {
		cout << "Choose a subdivision type:\n1.Loop subdivision\n2.Modify Butterfly subdivision" << endl;
		cin >> subdivisionType;
	}
	cout << "Iteration times: ";
	unsigned times;
	cin >> times;
	//model in file is the iteration 0 
	unsigned vectorPreserve = countLine(inputFileName);
	HalfEdge_mesh inputmesh;
	iterationMesh.push_back(inputmesh);
	iterationMesh.reserve(times*6);
	((iterationMesh[0]).vertex).reserve(vectorPreserve);
	((iterationMesh[0]).face).reserve(vectorPreserve);
	((iterationMesh[0]).halfedge).reserve(vectorPreserve * 3);
	((iterationMesh[0]).normal).reserve(vectorPreserve);
	int reportCode = readWrl(inputFileName,&iterationMesh[0]);
	//read file
	if (0 == reportCode) cout << "File parsing completed." << endl;
	else {
		clog << "Error happened! Error number is " + reportCode << endl;
		return -1;
	}
	//init OpenGL
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("subdivision");
	initMesh(&iterationMesh[0]);
	glutMouseFunc(myMouse);
	glutMotionFunc(onMouseMove);
	glutSpecialFunc(&mySpecial);
	glutReshapeFunc(&myReshape);
	glutDisplayFunc(&myDisplay);
	
	//assign edge/cease for tetrahedron,Cube,T-Shape
	/*for tetrahedron
	if (inputFileNumber == 1) {
		for (auto it = (iterationMesh[0]).halfedge.begin(); it != (iterationMesh[0]).halfedge.end(); it++)
			(*it).isCrease = true;
	}
	/*for cube
	if (inputFileNumber == 2) {
		int i = 0;
		for(auto it = (iterationMesh[0]).halfedge.begin(); it != (iterationMesh[0]).halfedge.end(); it++){
			if (i % 3 != 2) (*it).isCrease = true;
		}
		}
	/*for T-Shape, too hard to assign*/
	//loop subdivision
	for (unsigned i = 1; i < times+1; i++) {
		cout << "in " << i << " subdivision...." << endl;
		vectorPreserve *= 8;
		HalfEdge_mesh newMesh;
		iterationMesh.push_back(newMesh);
		((iterationMesh[i]).vertex).reserve(vectorPreserve);
		((iterationMesh[i]).face).reserve(vectorPreserve);
		((iterationMesh[i]).halfedge).reserve(vectorPreserve);
		((iterationMesh[i]).normal).reserve(vectorPreserve);
		if (subdivisionType == 1)
			loopSubdivision(&iterationMesh[i - 1], &iterationMesh[i]);
		else if (subdivisionType == 2)
			ModifyButterflySubdivision(&iterationMesh[i - 1], &iterationMesh[i]);
	}
	
	//wirte file
	HalfEdge_mesh finalOutput = iterationMesh[times];
	initMesh(&finalOutput);
	glutPostRedisplay();
	cout << "**********************************************" << endl;
	cout << "Press F1 to change display mode." << endl;
	cout << "Press F2 to exit display." << endl;
	writeFile(&finalOutput, outputFileName);
	cout << "File has been written in ../model_subdivision." << endl;
	cout << "**********************************************" << endl;
	glutMainLoop();
	return 0;
}
