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

#include "triangulate.h"
#include <MeshLib/GMesh.h>
#include <GeomLib/GObject.h>
#include <GeomLib/geom.h>

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
GMesh* triangulate(GTriangulate& c)
{
	assert(c.Nodes() == c.Edges());

	int N = c.Nodes();

	GMesh* pm = new GMesh;
	pm->Create(N, N-2, N);

	// create the Nodes
	for (int i=0; i<N; ++i)
	{
		GMesh::NODE& n = pm->Node(i);
		n.r = to_vec3f(c.Node(i).r);
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
		int N0 = N;
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

				// update earity of diagonal endpoints
				v1.ntag = abs(v1.ntag); v1.ntag *= Diagonal(c, i-2, i+1);
				v3.ntag = abs(v3.ntag); v3.ntag *= Diagonal(c, i-1, i+2);

				// cut-off ear v2
				c.DeleteNode(i);
				N = c.Nodes();
				break;
			}
		}
		if (N == N0)
		{
			// Hmm, no ear was cut off. This is a problem and now we're stuck
			// in an infinite loop, so let's just abort. 
			// The most likely cause is a duplicate node in the original GTriangulate object. 
			break;
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

//-----------------------------------------------------------------------------
GMesh* triangulate(GFace& face)
{
	assert(face.m_ntype == FACE_POLYGON);

	GBaseObject& obj = *face.Object();

	GTriangulate c;
	c.Clear();

	const int M = 50;

	// find the (approximate) face normal
	vec3d fn(0, 0, 0);
	int ne = face.Edges();
	for (int i = 0; i < ne - 1; ++i)
	{
		GEdge& e0 = *obj.Edge(face.m_edge[i].nid);
		int w0 = face.m_edge[i].nwn;

		GEdge& e1 = *obj.Edge(face.m_edge[i + 1].nid);
		int w1 = face.m_edge[i + 1].nwn;

		vec3d t0 = (w0 > 0 ? e0.Tangent(1) : -e0.Tangent(0));
		vec3d t1 = (w1 > 0 ? e1.Tangent(0) : -e1.Tangent(1));

		fn += (t0 ^ t1);
	}
	fn.Normalize();

	// find the rotation to bring it back to the x-y plane
	quatd q(fn, vec3d(0, 0, 1));
	if (q.Norm() == 0.0)
		q = quatd(PI, vec3d(1, 0, 0));
	
	quatd qi = q.Inverse();

	// assume the first point is also on the plane
	vec3d rc = obj.Node(face.m_node[0])->LocalPosition();

	// create all nodes
	for (int i = 0; i < ne; ++i)
	{
		GEdge& e = *obj.Edge(face.m_edge[i].nid);
		int ew = face.m_edge[i].nwn;
		int en0 = (ew == 1 ? e.m_node[0] : e.m_node[1]);
		int en1 = (ew == 1 ? e.m_node[1] : e.m_node[0]);
		int n0 = obj.Node(en0)->GetLocalID();

		// Not sure if this will always work, but if the face is inverted (e.g. bottom face of cylinder)
		// then q will also invert the winding of the GM_CIRCLE_ARC, and so we need to flip the edges orientation. 
		int orient = (fn * vec3d(0, 0, 1) > 0 ? e.m_orient : -e.m_orient);

		switch (e.m_ntype)
		{
		case EDGE_LINE:
		{
			vec3d r = obj.Node(en0)->LocalPosition() - rc;
			q.RotateVector(r);
			c.AddNode(r, n0);
		}
		break;
		case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = obj.Node(e.m_cnode[0])->LocalPosition() - rc;
			vec3d r1 = obj.Node(e.m_node[0])->LocalPosition() - rc;
			vec3d r2 = obj.Node(e.m_node[1])->LocalPosition() - rc;
			q.RotateVector(r0);
			q.RotateVector(r1);
			q.RotateVector(r2);

			vec2d a0(r0.x, r0.y);
			vec2d a1(r1.x, r1.y);
			vec2d a2(r2.x, r2.y);

			GM_CIRCLE_ARC ca(a0, a1, a2, orient);

			if (ew == 1) c.AddNode(r1, n0); else c.AddNode(r2, n0);

			int j0, j1, ji;
			if (face.m_edge[i].nwn == 1) { j0 = 1; j1 = M; ji = 1; }
			else { j0 = M - 1; j1 = 0; ji = -1; }
			for (int j = j0; j != j1; j += ji)
			{
				double l = (double)j / (double)M;
				c.AddNode(ca.Point(l), -1);
			}
		}
		break;
		case EDGE_3P_ARC:
		{
			vec3d r0 = obj.Node(e.m_cnode[0])->LocalPosition() - rc;
			vec3d r1 = obj.Node(e.m_node[0])->LocalPosition() - rc;
			vec3d r2 = obj.Node(e.m_node[1])->LocalPosition() - rc;
			q.RotateVector(r0);
			q.RotateVector(r1);
			q.RotateVector(r2);

			vec2d a0(r0.x, r0.y);
			vec2d a1(r1.x, r1.y);
			vec2d a2(r2.x, r2.y);

			GM_ARC ca(a0, a1, a2);

			if (ew == 1) c.AddNode(r1, n0); else c.AddNode(r2, n0);

			int j0, j1, ji;
			if (face.m_edge[i].nwn == 1) { j0 = 1; j1 = M; ji = 1; }
			else { j0 = M - 1; j1 = 0; ji = -1; }
			for (int j = j0; j != j1; j += ji)
			{
				double l = (double)j / (double)M;
				c.AddNode(ca.Point(l), -1);
			}
		}
		break;
		default:
			assert(false);
		}
	}

	// create all edges
	int NN = c.Nodes();
	int m = 0;
	for (int i = 0; i < ne; ++i)
	{
		GEdge& e = *obj.Edge(face.m_edge[i].nid);
		int eid = e.GetLocalID();
		switch (e.m_ntype)
		{
		case EDGE_LINE:
		{
			int n0 = m++;
			int n1 = (n0 + 1) % NN;
			c.AddEdge(n0, n1, eid);
		}
		break;
		case EDGE_3P_CIRC_ARC:
			for (int j = 0; j < M; ++j)
			{
				int n0 = m++;
				int n1 = (n0 + 1) % NN;
				c.AddEdge(n0, n1, eid);
			}
			break;
		case EDGE_3P_ARC:
			for (int j = 0; j < M; ++j)
			{
				int n0 = m++;
				int n1 = (n0 + 1) % NN;
				c.AddEdge(n0, n1, eid);
			}
			break;
		default:
			assert(false);
		}
	}

	GMesh* pm = triangulate(c);

	// Position the face at the correct position
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		vec3d r = to_vec3d(pm->Node(i).r);
		qi.RotateVector(r);
		r += rc;
		pm->Node(i).r = to_vec3f(r);
	}

	// set the proper face IDs
	for (int i = 0; i < pm->Faces(); ++i) pm->Face(i).pid = face.GetLocalID();

	return pm;
}

// TODO: Not sure if this works well, but initial testing appears promising. 
std::vector<vec3d> convex_hull2d(const std::vector<vec3d>& p)
{
	int N = p.size();
	if (N <= 2) return p;

	std::vector<vec3d> q;
	int n0 = 0;
	vec3d a = p[0], b;
	do
	{
		// find another point such that all points are to the left of the line
		int n1 = -1;
		for (int j = 0; j < N; ++j)
		{
			b = p[j];
			bool bok = true;
			for (int i = 0; i < q.size(); ++i)
			{
				vec3d c = q[i];
				if ((b - c).SqrLength() < 1e-12)
				{
					bok = false;
					break;
				}
			}

			if (bok && (j != n0))
			{
				if ((b - a).SqrLength() > 1e-12)
				{
					bool intersects = false;
					for (int k = 0; k < N; ++k)
					{
						if ((k != n0) && (k != j))
						{
							vec3d c = p[k];

							double Lca = (a - c).SqrLength();
							double Lcb = (b - c).SqrLength();

							if ((Lca > 1e-12) && (Lcb > 1e-12))
							{
								double A2 = (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
								if (A2 < 0)
								{
									intersects = true;
									break;
								}
							}
						}
					}

					if (intersects == false)
					{
						n1 = j;
						break;
					}
				}
			}
		}

		if (n1 != -1)
		{
			if (q.empty()) q.push_back(a);
			q.push_back(b);

			n0 = n1;
			a = p[n0];
			n1 = -1;
		}
		else
		{
			if (q.empty() == false) break;
			n0++;
			if (n0 >= N) break;
			a = p[n0];
		}
	} while (true);

	return q;
}
