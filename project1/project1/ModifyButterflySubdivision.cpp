/***************************************************
* by:LiXiangxian                                   *
* 2018.11.26                                       *
* Modify Butterfly Subdivision:                    *
* 2018.11.27                                       *
* finish                                           *
****************************************************/
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include "ModifyButterflySubdivision.h"
#include "halfEdge_structure.h"
//functions
void ModifyButterflySubdivision(HalfEdge_mesh *inputMesh, HalfEdge_mesh *outputMesh);
double butterflyFormular(char direction, HalfEdge_vertex *v0, HalfEdge_vertex *v1, HalfEdge_halfedge *thisEdge,unsigned valence0, unsigned valence1, std::vector<HalfEdge_vertex> neibour0, std::vector<HalfEdge_vertex> neibour1);
void findTwin(HalfEdge_mesh *mesh, unsigned halfedge_count);
void setNormal(HalfEdge_mesh *mesh);
void setCrease(HalfEdge_mesh *mesh);
void ModifyButterflySubdivision(HalfEdge_mesh *inputMesh, HalfEdge_mesh *outputMesh)
{
	/*******************************************************************
	*              compute the position of new vertexs                 *
	********************************************************************/
	unsigned vertex_count = 0;
	for (auto it = (*inputMesh).halfedge.begin(); it != (*inputMesh).halfedge.end(); it++) {
		if ((*it).insert) continue; //from twin
		HalfEdge_vertex insert; //new vertex
		//two vertex of this halfedge
		HalfEdge_vertex *v0 = (*it).origin;
		HalfEdge_vertex *v1 = (*(*it).nextEdge).origin;
		unsigned valence0 = 0, valence1 = 0;
		//------------------count valence of v0-----------------------
		std::vector<HalfEdge_vertex> neighbour0; // save neighbour
		HalfEdge_halfedge *find0 = &(*it);
		do{
			valence0++;
			neighbour0.push_back(*(*(*find0).nextEdge).origin);
			if (!(*(*find0).preEdge).twin) {
				valence0++;// one halfedge valence, if it counts
				neighbour0.push_back(*(*(*find0).preEdge).origin);
				HalfEdge_halfedge *findBack = (*it).twin;
				while (findBack) {
					valence0++;
					neighbour0.push_back((*(*(*findBack).preEdge).origin));
					//get the other end, research is done
					if (!(*(*findBack).nextEdge).twin) break;
					findBack = (*(*findBack).nextEdge).twin;
				}
				break;
			}
			//find0 = (*(*find0).twin).nextEdge;
			find0 = (*(*find0).preEdge).twin;
		} while (find0 != &(*it));	//stop when got back to start
		//------------------count valence of v1-----------------------
		std::vector<HalfEdge_vertex> neighbour1; // save neighbour
		HalfEdge_halfedge *find1 = &(*(*it).nextEdge);
		do {
			valence1++;
			neighbour1.push_back(*(*(*find1).preEdge).origin);
			if (!(*find1).twin) {
				valence1++;// one halfedge valence, if it counts
				neighbour1.push_back(*(*(*find1).nextEdge).origin);
				HalfEdge_halfedge *findBack = (*it).twin;
				while (findBack) {
					valence1++;
					neighbour1.push_back(*(*(*findBack).preEdge).origin);
					//get the other end, research is done
					if (!(*(*findBack).preEdge).twin) break;
					findBack = (*(*findBack).preEdge).twin;
				}
				break;
			}
			find1 = (*(*find1).twin).nextEdge;
		} while (find1 != &(*(*it).nextEdge));	//stop when got back to start
		//use formular
		insert.x = butterflyFormular('x', v0, v1, &(*it), valence0, valence1, neighbour0, neighbour1);
		insert.y = butterflyFormular('y', v0, v1, &(*it), valence0, valence1, neighbour0, neighbour1);
		insert.z = butterflyFormular('z', v0, v1, &(*it), valence0, valence1, neighbour0, neighbour1);
		(*outputMesh).vertex.push_back(insert);
		(*outputMesh).vertex[vertex_count].number = vertex_count;
		if (!(*it).twin) 
			(*it).insert = &(*outputMesh).vertex[vertex_count];
		else {
			(*it).insert = &(*outputMesh).vertex[vertex_count];
			(*(*it).twin).insert = &(*outputMesh).vertex[vertex_count];
		}
		vertex_count++;
	}
	/*******************************************************************
	*              copy the position of old vertexs                    *
	********************************************************************/
	unsigned vertex_count_old = 0;
	for (auto it = (*inputMesh).vertex.begin(); it != (*inputMesh).vertex.end(); it++) {
		HalfEdge_vertex adjustV;
		adjustV.x = (*it).x;
		adjustV.y = (*it).y;
		adjustV.z = (*it).z;
		(*outputMesh).vertex.push_back(adjustV);
		(*outputMesh).vertex[vertex_count].number = vertex_count;
		(*inputMesh).vertex[vertex_count_old].adjust = &(*outputMesh).vertex[vertex_count];
		vertex_count_old++;
		vertex_count++;
	}
	/*******************************************************************
	*							remesh                                 *
	*******************************************************************/
	//All vertexs are already positioned in last two loops
	//now connect them to creat edges and faces
	unsigned face_count = 0;
	unsigned halfedge_count = 0;

	for (auto it = (*inputMesh).face.begin(); it != (*inputMesh).face.end(); it++) {
		//1 face remeshes to 4 new faces, each of them has 3 halfedge
		HalfEdge_halfedge e[4][3];
		HalfEdge_face f[4];

		HalfEdge_halfedge e_origin = *(*it).boundary;
		HalfEdge_halfedge e_previous = *e_origin.preEdge;
		HalfEdge_halfedge e_next = *e_origin.nextEdge;
		//assign origin for each halfedge

		e[0][0].origin = e_origin.insert;
		e[0][1].origin = (*e_next.origin).adjust;
		e[0][2].origin = e_next.insert;

		e[1][0].origin = e_next.insert;
		e[1][1].origin = (*e_previous.origin).adjust;
		e[1][2].origin = e_previous.insert;

		e[2][0].origin = e_previous.insert;
		e[2][1].origin = (*e_origin.origin).adjust;
		e[2][2].origin = e_origin.insert;

		e[3][0].origin = e_origin.insert;
		e[3][1].origin = e_next.insert;
		e[3][2].origin = e_previous.insert;

		/*--------------------------------------------*/
		for (unsigned i = 0; i < 4; i++, face_count++)
			for (unsigned j = 0; j < 3; j++) {
				(*outputMesh).halfedge.push_back(e[i][j]);
				halfedge_count++;
				if (j == 2) {
					(*outputMesh).face.push_back(f[i]);

					(*outputMesh).halfedge[halfedge_count - 3].incidentFace = &(*outputMesh).face[face_count];
					(*outputMesh).halfedge[halfedge_count - 2].incidentFace = &(*outputMesh).face[face_count];
					(*outputMesh).halfedge[halfedge_count - 1].incidentFace = &(*outputMesh).face[face_count];

					(*outputMesh).halfedge[halfedge_count - 3].nextEdge = &(*outputMesh).halfedge[halfedge_count - 2];
					(*outputMesh).halfedge[halfedge_count - 2].nextEdge = &(*outputMesh).halfedge[halfedge_count - 1];
					(*outputMesh).halfedge[halfedge_count - 1].nextEdge = &(*outputMesh).halfedge[halfedge_count - 3];

					(*outputMesh).halfedge[halfedge_count - 3].preEdge = &(*outputMesh).halfedge[halfedge_count - 1];
					(*outputMesh).halfedge[halfedge_count - 2].preEdge = &(*outputMesh).halfedge[halfedge_count - 3];
					(*outputMesh).halfedge[halfedge_count - 1].preEdge = &(*outputMesh).halfedge[halfedge_count - 2];

					(*outputMesh).face[face_count].boundary = &(*outputMesh).halfedge[halfedge_count - 3];
				}
			}
		(*e_origin.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 12];
		(*((*e_next.origin).adjust)).asOrigin = &(*outputMesh).halfedge[halfedge_count - 11];
		(*e_next.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 10];
		(*(*e_previous.origin).adjust).asOrigin = &(*outputMesh).halfedge[halfedge_count - 8];
		(*e_previous.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 7];
		(*(*e_origin.origin).adjust).asOrigin = &(*outputMesh).halfedge[halfedge_count - 5];

	}
	//find twin
	findTwin(outputMesh, halfedge_count);
	//compute normal for every face
	setNormal(outputMesh);
	//crease
	setCrease(outputMesh);
	//done
}

double butterflyFormular
(char direction, 
HalfEdge_vertex *v0, HalfEdge_vertex *v1, 
HalfEdge_halfedge *thisEdge, 
unsigned valence0, unsigned valence1, 
std::vector<HalfEdge_vertex> neibour0, std::vector<HalfEdge_vertex> neibour1)
{
	//case(d), boundary or crease
	if ((*thisEdge).twin == nullptr || (*thisEdge).isCrease) {
		HalfEdge_vertex *v2 = (*(*thisEdge).preEdge).origin;
		HalfEdge_vertex *v3 = (*(*thisEdge).nextEdge).origin;
		switch (direction) {
		case 'x':
			return(((*v0).x + (*v1).x) * (9.0 / 16.0) + ((*v2).x + (*v3).x) * (-1.0 / 16.0));
		case 'y':
			return(((*v0).y + (*v1).y) * (9.0 / 16.0) + ((*v2).y + (*v3).y) * (-1.0 / 16.0));
		case 'z':
			return(((*v0).z + (*v1).z) * (9.0 / 16.0) + ((*v2).z + (*v3).z) * (-1.0 / 16.0));
		}
	}
	//case(a)
	if (valence0 == 6 && valence1 == 6) {
		auto i = neibour0.begin();
		auto j = neibour1.begin();
		/*
		HalfEdge_vertex *v2 = (*(*thisEdge).preEdge).origin;
		HalfEdge_vertex *v3 = (*(*(*thisEdge).twin).preEdge).origin;
		HalfEdge_vertex *v4 = (*(*(*(*thisEdge).preEdge).twin).preEdge).origin;
		HalfEdge_vertex *v5 = (*(*(*(*thisEdge).nextEdge).twin).preEdge).origin;
		HalfEdge_vertex *v6 = (*(*(*(*(*thisEdge).twin).nextEdge).twin).preEdge).origin;
		HalfEdge_vertex *v7 = (*(*(*(*(*thisEdge).twin).preEdge).twin).preEdge).origin;
		*/
		HalfEdge_vertex *v2 = &(*(i + 1));
		HalfEdge_vertex *v3 = &(*(i + 5));
		HalfEdge_vertex *v4 = &(*(i + 2));
		HalfEdge_vertex *v5 = &(*(j + 2));
		HalfEdge_vertex *v6 = &(*(i + 4));
		HalfEdge_vertex *v7 = &(*(j + 4));
		switch (direction) {
			case 'x':
				return((((*v0).x + (*v1).x) * 0.5) + (((*v2).x + (*v3).x) * 0.125) + (((*v4).x + (*v5).x + (*v6).x + (*v7).x) * (-1.0 / 16.0)));
			case 'y':
				return((((*v0).y + (*v1).y) * 0.5) + (((*v2).y + (*v3).y) * 0.125) + (((*v4).y + (*v5).y + (*v6).y + (*v7).y) * (-1.0 / 16.0)));
			case 'z':
				return((((*v0).z + (*v1).z) * 0.5) + (((*v2).z + (*v3).z) * 0.125) + (((*v4).z + (*v5).z + (*v6).z + (*v7).z) * (-1.0 / 16.0)));
		}
	}
	double result = 0.0; //save temp/final result
	//case(b).1, in this case, v1 is main factor
	if (valence1 != 6) {
		if (valence1 < 5) {
			auto j = neibour1.begin();
			HalfEdge_vertex *v2 = &(*(j + 1));
			HalfEdge_vertex *v3 = &(*(j + 2));
			//HalfEdge_vertex v2 = *(*(*thisEdge).preEdge).origin;
			//HalfEdge_vertex v3 = *(*(*(*(*thisEdge).nextEdge).twin).preEdge).origin;
			if (valence1 == 3) {
				switch (direction) {
				case 'x':
					result = ((*v1).x * 0.75 + (*v0).x *(5.0 / 12.0) + ((*v2).x + (*v3).x) * (-1.0 / 12.0));
					break;
				case 'y':
					result = ((*v1).y * 0.75 + (*v0).y *(5.0 / 12.0) + ((*v2).y + (*v3).y) * (-1.0 / 12.0));
					break;
				case 'z':
					result = ((*v1).z * 0.75 + (*v0).z *(5.0 / 12.0) + ((*v2).z + (*v3).z) * (-1.0 / 12.0));
					break;
				}
			}
			else if (valence1 == 4) {
				switch (direction) {
				case 'x':
					result = ((*v1).x * 0.75 + (*v0).x * 0.375 + (*v3).x * (-0.125));
					break;
				case 'y':
					result = ((*v1).y * 0.75 + (*v0).y * 0.375 + (*v3).y * (-0.125));
					break;
				case 'z':
					result = ((*v1).z * 0.75 + (*v0).z * 0.375 + (*v3).z * (-0.125));
					break;
				}
			}
		}
		else if (valence1 >= 5) {
			double beta = 0.0;
			double betaSum = 0.0;
			double position = 0.0;
			double i = 0.0;
			switch (direction) {
			case 'x':
				for (auto it = neibour1.begin(); it != neibour1.end(); it++, i++) {
					beta = (1.0 / (double)valence1)*(0.25 + cos(2.0*i*Pi / (double)valence1) + 0.5*cos(4.0*i*Pi / (double)valence1));
					position += beta * (*it).x;
					betaSum += beta;
				}
				position += (0.75 * (*v1).x);
				break;
			case 'y':
				for (auto it = neibour1.begin(); it != neibour1.end(); it++, i++) {
					beta = (1.0 / (double)valence1)*(0.25 + cos(2.0*i*Pi / (double)valence1) + 0.5*cos(4.0*i*Pi / (double)valence1));
					position += beta * (*it).y;
					betaSum += beta;
				}
				position += (0.75 * (*v1).y);
				break;
			case 'z':
				for (auto it = neibour1.begin(); it != neibour1.end(); it++, i++) {
					beta = (1.0 / (double)valence1)*(0.25 + cos(2.0*i*Pi / (double)valence1) + 0.5*cos(4.0*i*Pi / (double)valence1));
					position += beta * (*it).z;
					betaSum += beta;
				}
				position += (0.75 * (*v1).z);
				break;
			}
			result = position;
		}
	}
	if (valence1 != 6 && valence0 == 6) return result;
	//case(b).2, in this case, v0 is main factor
	if (valence0 != 6) {
		if (valence0 < 5) {
			auto i = neibour0.begin();
			HalfEdge_vertex *v2 = &(*(i + 1));
			HalfEdge_vertex *v3 = &(*(i + 2));
			//HalfEdge_vertex v2 = *(*(*thisEdge).preEdge).origin;
			//HalfEdge_vertex v3 = *(*(*(*(*thisEdge).preEdge).twin).preEdge).origin;
			if (valence0 == 3) {
				switch (direction) {
				case 'x':
					result += ((*v0).x * 0.75 + (*v1).x *(5.0 / 12.0) + ((*v2).x + (*v3).x) * (-1.0 / 12.0));
					break;
				case 'y':
					result += ((*v0).y * 0.75 + (*v1).y *(5.0 / 12.0) + ((*v2).y + (*v3).y) * (-1.0 / 12.0));
					break;
				case 'z':
					result += ((*v0).z * 0.75 + (*v1).z *(5.0 / 12.0) + ((*v2).z + (*v3).z) * (-1.0 / 12.0));
					break;
				}
			}
			else if (valence0 == 4) {
				switch (direction) {
				case 'x':
					result += ((*v0).x * 0.75 + (*v1).x * 0.375 + (*v3).x * (-0.125));
					break;
				case 'y':
					result += ((*v0).y * 0.75 + (*v1).y * 0.375 + (*v3).y * (-0.125));
					break;
				case 'z':
					result += ((*v0).z * 0.75 + (*v1).z * 0.375 + (*v3).z * (-0.125));
					break;
				}
			}
		}
		else if (valence0 >= 5) {
			double beta = 0.0;
			double betaSum = 0.0;
			double position = 0.0;
			double i = 0.0;
			switch (direction) {
			case 'x':
				for (auto it = neibour0.begin(); it != neibour0.end(); it++, i++) {
					beta = (1.0 / (double)valence0)*(0.25 + cos(2.0*i*Pi / (double)valence0) + 0.5*cos(4.0*i*Pi / (double)valence0));
					position += beta * (*it).x;
					betaSum += beta;
				}
				position += ((1.0 - betaSum) * (*v0).x);
				break;
			case 'y':
				for (auto it = neibour0.begin(); it != neibour0.end(); it++, i++) {
					beta = (1.0 / (double)valence0)*(0.25 + cos(2.0*i*Pi / (double)valence0) + 0.5*cos(4.0*i*Pi / (double)valence0));
					position += beta * (*it).y;
					betaSum += beta;
				}
				position += ((1.0 - betaSum) * (*v0).y);
				break;
			case 'z':
				for (auto it = neibour0.begin(); it != neibour0.end(); it++, i++) {
					beta = (1.0 / (double)valence0)*(0.25 + cos(2.0*i*Pi / (double)valence0) + 0.5*cos(4.0*i*Pi / (double)valence0));
					position += beta * (*it).z;
					betaSum += beta;
				}
				position += ((1.0 - betaSum) * (*v0).z);
			}
			result += position;
		}
	}
	if (valence0 != 6 && valence1 == 6) return result;
	//case(c)
	else if (valence0 != 6 && valence1 != 6) return (result * 0.5);
}