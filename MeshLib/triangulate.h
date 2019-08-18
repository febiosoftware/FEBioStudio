#pragma once
#include <MathLib/math3d.h>
#include <vector>
using namespace std;

class GLMesh;

class GTriangulate
{
public:
	struct NODE
	{
		vec3d	r;
		int		ntag;
		int		nid;
	};

	struct EDGE
	{
		int n[2];
		int nid;
	};

public:
	void AddNode(vec3d r, int nid);
	void AddEdge(int n0, int n1, int id);

	int Nodes() { return (int)m_Node.size(); }
	NODE& Node(int i) { return m_Node[i]; }

	int Edges() { return (int)m_Edge.size(); }
	EDGE& Edge(int i) { return m_Edge[i]; }

	NODE& NodeCycle(int n);

	void DeleteNode(int n);

	void Clear();

protected:
	vector<NODE>	m_Node;
	vector<EDGE>	m_Edge;
};

//-----------------------------------------------------------------------------
// --- Helper functions for 2D triangulation algorithms ---

// this is used as a tolerance for comparing to zero
#define ZERO_TOL	1e-12

// returns twice the (signed) area of the triangle <a,b,c>
inline double Area2(const vec2d& a, const vec2d& b, const vec2d& c)
{
	return (b.x - a.x)*(c.y - a.y) - (c.x - a.x)*(b.y - a.y);
}

// returns true if point c lies on the left side of the edge [a,b]
inline bool IsLeft(const vec2d& a, const vec2d& b, const vec2d& c) { return Area2(a, b, c) >  ZERO_TOL; }

// returns true if point c lies on the left side of or on the edge [a,b]
inline bool IsLeftOn(const vec2d& a, const vec2d& b, const vec2d& c) { return Area2(a, b, c) >= 0.0; }

// returns true if point c lies on the edge [a,b]
inline bool IsCollinear(const vec2d& a, const vec2d& b, const vec2d& c) { double A2 = Area2(a,b,c); return ((A2>=-ZERO_TOL)&&(A2<=ZERO_TOL)); }

// see if point c lies between [a,b]
inline bool Between(const vec2d& a, const vec2d& b, const vec2d& c)
{
	if (!IsCollinear(a, b, c)) return false;

	if (a.x != b.x) return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

// see if edges [a,b] properly intersect edge [c,d]
inline bool IntersectProp(const vec2d& a, const vec2d& b, const vec2d& c, const vec2d& d)
{
	// make sure they are not collinear
	if (IsCollinear(a, b, c) || IsCollinear(a, b, d) || IsCollinear(c, d, a) || IsCollinear(c, d, b)) return false;

	// see if either c,d are left of [a,b] but not both
	// and if either a,b are left of [c,d] but not both
	return (IsLeft(a, b, c)^IsLeft(a, b, d)) && (IsLeft(c, d, a)^IsLeft(c, d, b));
}

// see if [a,b] intersects with [c,d]
inline bool Intersect(const vec2d& a, const vec2d& b, const vec2d& c, const vec2d& d)
{
	// first see if they intersect properly (i.e. intersection point is not a,b,c,d)
	if (IntersectProp(a, b, c, d)) return true;

	// see if any nodes fall inside an edge
	if (Between(a, b, c) || Between(a, b, d) || Between(c, d, a) || Between(c, d, b)) return true;

	// no intersection detected
	return false;
}

//-----------------------------------------------------------------------------
GLMesh* triangulate(GTriangulate& c);
