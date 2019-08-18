#pragma once
#include <MathLib/math3d.h>
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
// Class that represents a line mesh in 2D composed of linear segments
//
class LineMesh2D
{
private:
	// forward type declarations
	class NODE;
	class EDGE;

public:
	// reference types for accessing mesh items
	typedef list<NODE>::iterator Node;
	typedef list<EDGE>::iterator Edge;

private:
	// class that represents a node
	class NODE
	{
	public:
		vec2d	pos;
		explicit NODE(const vec2d& r) : pos(r) {}
	};

	// class that represents an edge
	class EDGE
	{
	public:
		Node	node[2];	// the two nodes this edge connects.
		Edge	edge[2];	// two neighbor edges
	};

public:
	// constructor. 
	LineMesh2D();

	// add a point to the mesh
	// returns a Node reference
	Node AddNode(const vec2d& r);

	// add an edge between two nodes
	Edge AddEdge(Node node0, Node node1);

private:
	list<NODE>	m_Node;	// list of nodes
	list<EDGE>	m_Edge;	// list of edges
};
