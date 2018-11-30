#include <math.h>
#include <iostream>
#include "halfEdge_structure.h"

//find the twin of halfedges
void findTwin(HalfEdge_mesh *mesh, unsigned halfedge_count)
{
	for (unsigned i = 0; i < halfedge_count; i++)
		for (unsigned j = i + 1; j < halfedge_count; j++) {
			if ((((*mesh).halfedge)[i]).twin)
				break;
			if ((((*mesh).halfedge)[j]).twin)
				continue;
			if ((((*mesh).halfedge)[i]).origin == (*(((*mesh).halfedge)[j]).nextEdge).origin && (((*mesh).halfedge)[j]).origin == (*(((*mesh).halfedge)[i]).nextEdge).origin) {
				(((*mesh).halfedge)[i]).twin = &((*mesh).halfedge)[j];
				(((*mesh).halfedge)[j]).twin = &((*mesh).halfedge)[i];
			}
		}
}
//compute normal
void setNormal(HalfEdge_mesh *mesh)
{
	unsigned i = 0;
	for (auto it = (*mesh).face.begin(); it != (*mesh).face.end(); it++, i++) {
		HalfEdge_halfedge *origin = (*it).boundary;
		// v1 is first vector of the face, v2 is the second vector, vn is the normal
		HalfEdge_normal v1, v2, vn; 
		HalfEdge_vertex p0 = *(*origin).origin;
		HalfEdge_vertex p1 = *(*(*origin).nextEdge).origin;
		HalfEdge_vertex p2 = *(*(*origin).preEdge).origin;
		double length;
		//compute normal
		v1.x = p0.x - p1.x;
		v1.y = p0.y - p1.y;
		v1.z = p0.z - p1.z;
		v2.x = p0.x - p2.x;
		v2.y = p0.y - p2.y;
		v2.z = p0.z - p2.z;
		vn.x = v1.y * v2.z - v1.z * v2.y;
		vn.y = v1.z * v2.x - v1.x * v2.z;
		vn.z = v1.x * v2.y - v1.y * v2.x;
		length = sqrt((vn.x * vn.x) + (vn.y * vn.y) + (vn.z * vn.z));
		if (length == 0.0) {
			std::cout << "Wrong number, a vector's length is 0.";
			return;
		}
		vn.x /= length;
		vn.y /= length;
		vn.z /= length;
		vn.number = i;
		(*mesh).normal.push_back(vn);
		(*it).normal = &(*mesh).normal[i];
	}
}
//compute cos(angle) of two vectors
double vectorAngle(HalfEdge_normal v0, HalfEdge_normal v1)
{
	double length1 = sqrt((v0.x * v0.x) + (v0.y * v0.y) + (v0.z * v0.z));
	double length2 = sqrt((v1.x * v1.x) + (v1.y * v1.y) + (v1.z * v1.z));
	return ((v0.x * v1.x + v0.y * v1.y + v0.z * v1.z) / (length1 * length2));
}
//set crease
void setCrease(HalfEdge_mesh *mesh)
{
	for (auto it = (*mesh).halfedge.begin(); it != (*mesh).halfedge.end(); it++) {
		if ((*it).CreaseChecked) continue;
		HalfEdge_normal normal0 = *(*(*it).incidentFace).normal;
		HalfEdge_normal normal1 = *(*(*(*it).twin).incidentFace).normal;
		//the function acos return Arc system
		double angle = acos(vectorAngle(normal0, normal1)) * 180.0 / Pi;
		/*how to know it is a crease*/
		if (angle >= 150.0) {
			(*it).isCrease = true;
			(*(*it).twin).isCrease = true;
		}
		(*it).CreaseChecked = true;
		(*(*it).twin).CreaseChecked = true;
	}
}