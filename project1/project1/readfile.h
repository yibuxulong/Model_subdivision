#ifndef READ_FILE
#define READ_FILE
#include<iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>
#include "halfEdge_structure.h"
void findTwin(unsigned halfedgeRead)
{
	//once twin has been found, two half-edges will be set as "foundTwin"
	//so these loops will not execute too long
	for (unsigned i = 0; i < halfedgeRead && !halfedge[i].foundTwin; i++) {
		for (unsigned j = i + 1; j < halfedgeRead; j++) {
			if (halfedge[i].foundTwin)
				break;
			if (halfedge[i].origin == (*halfedge[j].nextEdge).origin && halfedge[j].origin == (*halfedge[i].nextEdge).origin) {
				halfedge[i].twin = &halfedge[j];
				halfedge[i].foundTwin = true;
				halfedge[j].twin = &halfedge[i];
				halfedge[j].foundTwin = true;
			}
		}
	}
}

//check the head
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

	while (getline(wrlFile, tempLine)) a++;
	return a;
}
//read file, generate vertex table, halfedge table and face table
int readWrl(string file)
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

	while (wrlFile >> sTemp) {
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
				vertex.push_back(vertex_temp);
			}
			count_vertex_read++;
		}
		//set face and halfedge
		if (pointSettled && !dataSettled) {
			if ("]" == sTemp) {
				findTwin(count_halfedge_read);
				//findOrigin(vector<HalfEdge_halfedge> halfedge, vector<HalfEdge_vertex> vertex);
				dataSettled = true;
				continue;
			}
			else if ("-1" == sTemp) {
				HalfEdge_halfedge edge_temp1;
				HalfEdge_halfedge edge_temp2;
				HalfEdge_halfedge edge_temp3;
				HalfEdge_face face_temp;
				halfedge.push_back(edge_temp1);
				halfedge.push_back(edge_temp2);
				halfedge.push_back(edge_temp3);
				//face
				face.push_back(face_temp);
				face[count_face_read].boundary = &halfedge[count_halfedge_read - 3];
				//halfedge's origin & vertexs
				halfedge[count_halfedge_read - 3].origin = &vertex[edge[0]];
				if (!vertex[edge[0]].beenOrigined) {
					vertex[edge[0]].asOrigin = &halfedge[count_halfedge_read - 3];
					vertex[edge[0]].beenOrigined = true;
				}
				halfedge[count_halfedge_read - 2].origin = &vertex[edge[1]];
				if (!vertex[edge[1]].beenOrigined) {
					vertex[edge[1]].asOrigin = &halfedge[count_halfedge_read - 2];
					vertex[edge[1]].beenOrigined = true;
				}
				halfedge[count_halfedge_read - 1].origin = &vertex[edge[2]];
				if (!vertex[edge[2]].beenOrigined) {
					vertex[edge[2]].asOrigin = &halfedge[count_halfedge_read - 1];
					vertex[edge[2]].beenOrigined = true;
				}
				//halfedge's nextedge
				halfedge[count_halfedge_read - 3].nextEdge = &halfedge[count_halfedge_read - 2];
				halfedge[count_halfedge_read - 2].nextEdge = &halfedge[count_halfedge_read - 1];
				halfedge[count_halfedge_read - 1].nextEdge = &halfedge[count_halfedge_read - 3];
				//halfedge's preedge
				halfedge[count_halfedge_read - 3].preEdge = &halfedge[count_halfedge_read - 1];
				halfedge[count_halfedge_read - 2].preEdge = &halfedge[count_halfedge_read - 3];
				halfedge[count_halfedge_read - 1].preEdge = &halfedge[count_halfedge_read - 2];
				//halfedge's incidentface
				halfedge[count_halfedge_read - 3].incidentFace = &face[count_face_read];
				halfedge[count_halfedge_read - 2].incidentFace = &face[count_face_read];
				halfedge[count_halfedge_read - 1].incidentFace = &face[count_face_read];
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
			if (checkEnd(fireEnd))
				break;
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
#endif