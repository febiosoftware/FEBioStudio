#pragma once
#include <list>
#include <MathLib/math3d.h>
using namespace std;

//-----------------------------------------------------------------------------
// Class that represents a triangle mesh in 2D
//
class TriMesh2D
{
private:
	// forward declaration of node, edge, and face classes
	class NODE;
	class EDGE;
	class FACE;

	// "pointers" to the items
	typedef list<NODE>::iterator NODEP;
	typedef list<EDGE>::iterator EDGEP;
	typedef list<FACE>::iterator FACEP;

	// class representing a vertex
	// only stores the vertex coordinates
	class NODE
	{
	public:
		vec2d	pos;	// spatial position
		int		tag;	// user tag

	public:
		NODE() : tag(0) {}
		explicit NODE(const vec2d& v) : pos(v), tag(0) {}
	};

	class EDGE
	{
	public:
		NODEP	node[2];		
	};

public:
	// constructor. constructs and empty mesh
	TriMesh2D();

private:
};