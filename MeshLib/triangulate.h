/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include <FSCore/math3d.h>
#include <vector>

class GLMesh;
class GFace;

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
	std::vector<NODE>	m_Node;
	std::vector<EDGE>	m_Edge;
};

//-----------------------------------------------------------------------------
// --- Helper functions for 2D triangulation algorithms ---

// this is used as a tolerance for comparing to zero
#define ZERO_TOL	1e-12

// returns twice the (signed) area of the triangle <a,b,c>
inline double Area2(const vec2d& a, const vec2d& b, const vec2d& c)
{
	return (b.x() - a.x())*(c.y() - a.y()) - (c.x() - a.x())*(b.y() - a.y());
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

	if (a.x() != b.x()) return ((a.x() <= c.x()) && (c.x() <= b.x())) || ((a.x() >= c.x()) && (c.x() >= b.x()));
	else return ((a.y() <= c.y()) && (c.y() <= b.y())) || ((a.y() >= c.y()) && (c.y() >= b.y()));
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
GLMesh* triangulate(GFace& face);

//-----------------------------------------------------------------------------
std::vector<vec3d> convex_hull2d(const std::vector<vec3d>& p);
