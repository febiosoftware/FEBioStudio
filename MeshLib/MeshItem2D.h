#pragma once
#include <MathLib/math3d.h>
#include <list>

//-----------------------------------------------------------------------------
// forward declarations of the classed defined here.
// These classes are used by the various 2D meshing classes
class Node2D;
class Edge2D;
class Tri2D;
class Quad2D;

//-----------------------------------------------------------------------------
// The 2D mesh classes use lists to store all the mesh items.
// Therefore we defined these reference classes for accessing mesh items
typedef std::list<Node2D>::iterator Node2DIterator;
typedef std::list<Edge2D>::iterator Edge2DIterator;
typedef std::list<Tri2D >::iterator Tri2DIterator;
typedef std::list<Quad2D>::iterator Quad2DIterator;

//-----------------------------------------------------------------------------
// This class represents a node in 2D
class Node2D
{
public:
	// constructor
	Node2D() : tag(0) {}
	explicit Node2D(const vec2d& r) : pos(r), tag(0) {}

public:
	vec2d	pos;	// spatial position
	int		tag;	// user tag
};

//-----------------------------------------------------------------------------
// This class represents an edge
class Edge2D
{
public:
	// constructor
	Edge2D();

public:
	Node2DIterator	node[2];
};
