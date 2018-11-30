#ifndef HALF_EDGE_STRUCTURE
#define HALF_EDGE_STRUCTURE
#include <vector>
#define Pi 3.14159265
struct HalfEdge_vertex;
struct HalfEdge_face;
struct HalfEdge_halfedge;
struct HalfEdge_normal;

struct HalfEdge_vertex
{
	unsigned number = 0;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	HalfEdge_vertex *adjust = nullptr; //new position after adjust
	HalfEdge_halfedge *asOrigin = nullptr;	//halfedge which starts from this point
	HalfEdge_halfedge *asInsert = nullptr;	//after subdivision
	bool beenOrigined = false;
};

struct HalfEdge_face
{
	HalfEdge_halfedge *boundary = nullptr;	//one edge that bound this face
	HalfEdge_normal *normal = nullptr;
};

struct HalfEdge_halfedge
{
	unsigned number = 0;
	HalfEdge_vertex *origin = nullptr;
	HalfEdge_halfedge *twin = nullptr;
	HalfEdge_face *incidentFace = nullptr;
	HalfEdge_halfedge *nextEdge = nullptr;
	HalfEdge_halfedge *preEdge= nullptr;
	HalfEdge_vertex *insert = nullptr; //record new vertex
	bool isCrease = false; // artificially specify a cease or boundary
	bool CreaseChecked = false;
};
struct HalfEdge_normal
{
	unsigned number = 0;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};
struct HalfEdge_mesh
{
	std::vector<HalfEdge_face> face;
	std::vector<HalfEdge_halfedge> halfedge;
	std::vector<HalfEdge_vertex> vertex;
	std::vector<HalfEdge_normal> normal;
};
void findTwin();
void setNormal();
void setCrease();
#endif