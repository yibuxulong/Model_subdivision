/***************************************************
 * by:LiXiangxian                                  *
 * 2018.11.12                                      *  
 * Loop Subdivision:                               *  
 * Original vertices are called even vertexs       *
 * Created vertices are called odd vertexs         *
 ***************************************************/
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include "LoopSubdivision.h"
#include "halfEdge_structure.h"

using namespace std;

void findTwin(HalfEdge_mesh *mesh, unsigned halfedge_count);
double adjustFomular(HalfEdge_vertex vertex, unsigned valence, char direction, vector<HalfEdge_vertex> neighbour);
double loopFomular(HalfEdge_halfedge insertHalfedge, char direction);
void setNormal(HalfEdge_mesh *mesh);
void setCrease(HalfEdge_mesh *mesh);

void loopSubdivision(HalfEdge_mesh *inputMesh,HalfEdge_mesh *outputMesh)
{	
	unsigned vertex_count = 0;
	/*******************************************************************
	 *   generate odd vertexs and assign old hlafedge insert points    *
	 *******************************************************************/
	for (auto it = (*inputMesh).halfedge.begin(); it != (*inputMesh).halfedge.end(); it++) {
		if ((*it).insert) continue;
		HalfEdge_vertex insertV;
		insertV.x = loopFomular((*it), 'x');
		insertV.y = loopFomular((*it), 'y');
		insertV.z = loopFomular((*it), 'z');
		(*outputMesh).vertex.push_back(insertV);
		(*outputMesh).vertex[vertex_count].number = vertex_count;
		if ((*it).twin) {
			(*it).insert = &(*outputMesh).vertex[vertex_count];
			(*(*it).twin).insert = &(*outputMesh).vertex[vertex_count];
		}
		else
			(*it).insert = &(*outputMesh).vertex[vertex_count];
		vertex_count++;
	}
	/*******************************************************************
	 *               adjust even vertexs' position                     *
	 *******************************************************************/
	unsigned vertex_count_old = 0;
	for (auto it = (*inputMesh).vertex.begin(); it != (*inputMesh).vertex.end(); it++) {
		unsigned valences = 0;
		vector<HalfEdge_vertex> neighbour; // save neighbour
		HalfEdge_halfedge *find = (*it).asOrigin;
		do {
			valences++;
			//the nearest vertexs are odd vertexs
			neighbour.push_back(*((*find).insert));
			//get one end, research the other direction
			if (!(*find).twin) {
				HalfEdge_halfedge *findBack = (*(*it).asOrigin).preEdge;
				while (findBack) {
					valences++;
					neighbour.push_back(*(*findBack).insert);
					//get the other end, research is done
					if (!(*findBack).twin) break;
					findBack = (*(*findBack).twin).preEdge;
				}
				break;
			}
			find = (*(*find).twin).nextEdge;
		} while (find != (*it).asOrigin);	//stop when got back to start
		
		HalfEdge_vertex vertexUpdate;
		vertexUpdate = (*it);
		vertexUpdate.x = adjustFomular(vertexUpdate, valences, 'x', neighbour);
		vertexUpdate.y = adjustFomular(vertexUpdate, valences, 'y', neighbour);
		vertexUpdate.z = adjustFomular(vertexUpdate, valences, 'z', neighbour);
		(*outputMesh).vertex.push_back(vertexUpdate);
		(*outputMesh).vertex[vertex_count].number = vertex_count;
		(*inputMesh).vertex[vertex_count_old].adjust = &(*outputMesh).vertex[vertex_count];
		vertex_count++;
		vertex_count_old++;
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
		/*assign halfedge for vertexs
		if (!(*e_origin.insert).asOrigin)
			(*e_origin.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 12];
		if (!(*((*e_next.origin).adjust)).asOrigin)
			(*((*e_next.origin).adjust)).asOrigin = &(*outputMesh).halfedge[halfedge_count - 11];
		if (!(*e_next.insert).asOrigin)
			(*e_next.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 10];
		if (!(*(*e_previous.origin).adjust).asOrigin)
			(*(*e_previous.origin).adjust).asOrigin = &(*outputMesh).halfedge[halfedge_count - 8];
		if (!(*e_previous.insert).asOrigin)
			(*e_previous.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 7];
		if (!(*(*e_origin.origin).adjust).asOrigin)
			(*(*e_origin.origin).adjust).asOrigin = &(*outputMesh).halfedge[halfedge_count - 5];
			*/
		(*e_origin.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 12];
		(*((*e_next.origin).adjust)).asOrigin = &(*outputMesh).halfedge[halfedge_count - 11];
		(*e_next.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 10];
		(*(*e_previous.origin).adjust).asOrigin = &(*outputMesh).halfedge[halfedge_count - 8];
		(*e_previous.insert).asOrigin = &(*outputMesh).halfedge[halfedge_count - 7];
		(*(*e_origin.origin).adjust).asOrigin = &(*outputMesh).halfedge[halfedge_count - 5];
		
	}
	//find twin
	findTwin(outputMesh,halfedge_count);
	//compute normal for every face
	setNormal(outputMesh);
	//set crease
	setCrease(outputMesh);
	//done
}

//even vertexs formular
double adjustFomular(HalfEdge_vertex vertex, unsigned valence, char direction, vector<HalfEdge_vertex> neighbour)
{
	double position = 0.0;
	double beta = 0.0;
	double temp = (double)valence;
	if ((*(vertex.asOrigin)).twin == nullptr || (*(vertex.asOrigin)).isCrease) {
		switch (direction) {
		case 'x':
			return(((*(*vertex.asOrigin).insert).x + (*(*(*vertex.asOrigin).preEdge).insert).x) * 1.0 / 8.0 + vertex.x * 3.0 / 4.0);
		case 'y':
			return(((*(*vertex.asOrigin).insert).y + (*(*(*vertex.asOrigin).preEdge).insert).y) * 1.0 / 8.0 + vertex.y * 3.0 / 4.0);
		case 'z':
			return(((*(*vertex.asOrigin).insert).z + (*(*(*vertex.asOrigin).preEdge).insert).z) * 1.0 / 8.0 + vertex.z * 3.0 / 4.0);
		}
	}
	/*Warren's way*/
	/*if (valence == 3) 
	*	beta = 3.0 / 16.0;
	*else if (valence > 3) beta = 3.0 / (8.0 * temp);
	*/
	beta = (3.0 / 8.0) + (cos(2.0 * Pi / temp) / 4.0);
	beta = beta * beta;
	beta = ((5.0 / 8.0) - beta) / temp;
	switch (direction) {
	case 'x':{
		for (auto it = neighbour.begin(); it != neighbour.end(); it++) position += (*it).x;
		return((1.0 - temp * beta) * vertex.x + beta * position);
	}
	case 'y':{
		for (auto it = neighbour.begin(); it != neighbour.end(); it++) position += (*it).y;
		return((1.0 - temp * beta) * vertex.y + beta * position);
	}
	case 'z':{
		for (auto it = neighbour.begin(); it != neighbour.end(); it++) position += (*it).z;
		return((1.0 - temp * beta) * vertex.z + beta * position);
	}
	}
}

//odd vertexs formular
double loopFomular(HalfEdge_halfedge insertHalfedge, char direction)
{
	if (insertHalfedge.twin == nullptr || insertHalfedge.isCrease) {
		switch (direction) {
			case 'x':{
				return(((*insertHalfedge.origin).x + (*(*insertHalfedge.nextEdge).origin).x)/ 2.0);
			}
			case 'y':{
				return(((*insertHalfedge.origin).y + (*(*insertHalfedge.nextEdge).origin).y) / 2.0);
			}
			case 'z':{
				return(((*insertHalfedge.origin).z + (*(*insertHalfedge.nextEdge).origin).z) / 2.0);
			}
		}
	}
	else {
		switch (direction) {
			case 'x':{
				return(
					(((*insertHalfedge.origin).x * 3.0) +
					((*(*insertHalfedge.nextEdge).origin).x * 3.0) +
					(*(*insertHalfedge.preEdge).origin).x +
					(*(*(*insertHalfedge.twin).preEdge).origin).x) / 8.0);
			}
			case 'y':{
				return(
					(((*insertHalfedge.origin).y * 3.0) +
					((*(*insertHalfedge.nextEdge).origin).y * 3.0) +
					(*(*insertHalfedge.preEdge).origin).y +
					(*(*(*insertHalfedge.twin).preEdge).origin).y) / 8.0);
			}
			case 'z':{
				return(
					(((*insertHalfedge.origin).z * 3.0) +
					((*(*insertHalfedge.nextEdge).origin).z * 3.0) +
					(*(*insertHalfedge.preEdge).origin).z +
					(*(*(*insertHalfedge.twin).preEdge).origin).z) / 8.0);
			}
		}
	}
}