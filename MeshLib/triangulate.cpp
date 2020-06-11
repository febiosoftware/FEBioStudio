/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "triangulate.h"
#include <MeshTools/GLMesh.h>

#ifdef LINUX
#define abs(a) ((a)>=0?(a):(-(a)))
#endif

//-----------------------------------------------------------------------------
void GTriangulate::Clear()
{
	m_Node.clear();
	m_Edge.clear();
}

//-----------------------------------------------------------------------------
void GTriangulate::AddNode(vec3d r, int nid)
{
	NODE n;
	n.r = r;
	n.nid = nid;
	m_Node.push_back(n);
}

//-----------------------------------------------------------------------------
void GTriangulate::AddEdge(int n0, int n1, int id)
{
	EDGE e;
	e.n[0] = n0;
	e.n[1] = n1;
	e.nid = id;
	m_Edge.push_back(e);
}

//-----------------------------------------------------------------------------
GTriangulate::NODE& GTriangulate::NodeCycle(int n)
{
	int N = (int)m_Node.size();
	if (n < 0) return Node(N + n);
	if (n >= N) return Node(n - N);
	return Node(n);
}

//-----------------------------------------------------------------------------
void GTriangulate::DeleteNode(int n)
{
	vector<NODE>::iterator p = m_Node.begin() + n;
	m_Node.erase(p);
}

//-----------------------------------------------------------------------------
void EarInit(GTriangulate& c);
int Diagonal(GTriangulate& c, int na, int nb);
bool InCone(GTriangulate& c, int na, int nb);
bool Diagonalie(GTriangulate& c, int na, int nb);
bool Intersect(vec3d& a, vec3d& b, vec3d& c, vec3d& d);
bool InCone(GTriangulate& c, int na, int nb);
bool Between(vec3d& a, vec3d& b, vec3d& c);
bool IntersectProp(vec3d& a, vec3d& b, vec3d& c, vec3d& d);

inline bool Xor(bool x, bool y) { return (!x)^(!y); }

double Area2(vec3d& a, vec3d& b, vec3d& c);
inline bool Left     (vec3d& a, vec3d& b, vec3d& c) { return Area2(a, b, c) >  0; }
inline bool LeftOn   (vec3d& a, vec3d& b, vec3d& c) { return Area2(a, b, c) >= 0; }
inline bool Collinear(vec3d& a, vec3d& b, vec3d& c) { return Area2(a, b, c) == 0; }

//-----------------------------------------------------------------------------
GLMesh* triangulate(GTriangulate& c)
{
	assert(c.Nodes() == c.Edges());

	int N = c.Nodes();

	GLMesh* pm = new GLMesh;
	pm->Create(N, N-2, N);

	// create the Nodes
	for (int i=0; i<N; ++i)
	{
		GMesh::NODE& n = pm->Node(i);
		n.r = c.Node(i).r;
		n.pid = c.Node(i).nid;
		c.Node(i).ntag = i+1;
	}

	// create the edges
	int NE = pm->Edges();
	for (int i=0; i<NE; ++i)
	{
		GMesh::EDGE& e = pm->Edge(i);
		e.pid = c.Edge(i).nid;
		e.n[0] = i;
		e.n[1] = (i+1)%NE;
	}

	// initialize ears
	EarInit(c);

	int NF = 0;
	while (N > 3)
	{
		for (int i=0; i<N; ++i)
		{
			GTriangulate::NODE& v2 = c.Node(i);
			if (v2.ntag > 0)
			{
				// find the neighbors
			  GTriangulate::NODE& v3 = c.NodeCycle(i+1); //GTriangulate::NODE& v4 = c.NodeCycle(i+2); 
			  GTriangulate::NODE& v1 = c.NodeCycle(i-1); //GTriangulate::NODE& v0 = c.NodeCycle(i-2); 

				// add a triangle to the mesh
				GMesh::FACE& f = pm->Face(NF++);
				f.n[0] = abs(v1.ntag)-1;
				f.n[1] = abs(v2.ntag)-1;
				f.n[2] = abs(v3.ntag)-1;
				f.pid = 0;
				f.sid = 0;

				// update earity of diangonal endpoints
				v1.ntag = abs(v1.ntag); v1.ntag *= Diagonal(c, i-2, i+1);
				v3.ntag = abs(v3.ntag); v3.ntag *= Diagonal(c, i-1, i+2);

				// cut-off ear v2
				c.DeleteNode(i);
				N = c.Nodes();
				break;
			}
		}
	}

	// add the last face
	GTriangulate::NODE& v1 = c.Node(0);
	GTriangulate::NODE& v2 = c.Node(1);
	GTriangulate::NODE& v3 = c.Node(2);

	GMesh::FACE& f = pm->Face(NF++);
	f.n[0] = abs(v1.ntag)-1;
	f.n[1] = abs(v2.ntag)-1;
	f.n[2] = abs(v3.ntag)-1;
	f.pid = 0;
	f.sid = 0;

	pm->Update();

	return pm;
}

//-----------------------------------------------------------------------------
void EarInit(GTriangulate& c)
{
	int N = c.Nodes();
	for (int i=0; i<N; ++i)
	{
		GTriangulate::NODE& v1 = c.Node(i);
		v1.ntag *= Diagonal(c, i-1, i+1);
	}
}

//-----------------------------------------------------------------------------
int Diagonal(GTriangulate& c, int na, int nb)
{
	bool b = InCone(c, na, nb) && InCone(c, nb, na) && Diagonalie(c, na, nb);
	return (b?1:-1);
}

//-----------------------------------------------------------------------------
bool InCone(GTriangulate& c, int na, int nb)
{
	GTriangulate::NODE& a = c.NodeCycle(na);
	GTriangulate::NODE& b = c.NodeCycle(nb);
	GTriangulate::NODE& a1 = c.NodeCycle(na+1);
	GTriangulate::NODE& a0 = c.NodeCycle(na-1);

	if (LeftOn(a.r, a1.r, a0.r))
		return Left(a.r, b.r, a0.r) && Left(b.r, a.r, a1.r);
	return !(LeftOn(a.r, b.r, a1.r) && LeftOn(b.r, a.r, a0.r));
}

//-----------------------------------------------------------------------------
bool Diagonalie(GTriangulate& c, int na, int nb)
{
	GTriangulate::NODE& a = c.NodeCycle(na);
	GTriangulate::NODE& b = c.NodeCycle(nb);
	int N = c.Nodes();
	for (int i=0; i<N; ++i)
	{
		GTriangulate::NODE& c0 = c.Node(i);
		GTriangulate::NODE& c1 = c.NodeCycle(i+1);
		if ((&c0 != &a) && (&c1 != &a) && (&c0 != &b) && (&c1 != &b) && Intersect(a.r, b.r, c0.r, c1.r)) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
bool Intersect(vec3d& a, vec3d& b, vec3d& c, vec3d& d)
{
	if (IntersectProp(a, b, c, d)) return true;
	else if ( Between(a, b, c) || Between(a, b, d) || Between(c, d, a) || Between(c, d, b)) return true;
	else return false;
}

//-----------------------------------------------------------------------------
bool Between(vec3d& a, vec3d& b, vec3d& c)
{
	if (!Collinear(a, b, c)) return false;

	if (a.x != b.x) return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

//-----------------------------------------------------------------------------
bool IntersectProp(vec3d& a, vec3d& b, vec3d& c, vec3d& d)
{
	if (Collinear(a, b, c) || Collinear(a, b, d) || Collinear(c, d, a) || Collinear(c, d, b)) return false;
	return Xor(Left(a, b, c), Left(a, b, d)) && Xor(Left(c, d, a), Left(c, d, b));
}

//-----------------------------------------------------------------------------
double Area2(vec3d& a, vec3d& b, vec3d& c)
{
	return (b.x - a.x)*(c.y - a.y) - (c.x - a.x)*(b.y - a.y);
}
