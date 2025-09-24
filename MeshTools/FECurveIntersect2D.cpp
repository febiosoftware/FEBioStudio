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

#include "stdafx.h"
#include "FECurveIntersect2D.h"
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FSCurveMesh.h>
#include <MeshLib/FSFaceEdgeList.h>
using namespace std;

class DynamicMesh2D
{
public:
	struct NODE;
	struct EDGE;
	struct FACE;

	typedef list<NODE>::iterator NODEP;
	typedef list<EDGE>::iterator EDGEP;
	typedef list<FACE>::iterator FACEP;

	struct NODE
	{
		vec3d	r;		// spatial position
		int		ntag;	// node index (0-based)

		NODE()
		{
			ntag = 0;
		}
	};

	struct EDGE
	{
		NODEP	node[2];		// pointer to nodes
		FACEP	face[2];		// pointer to faces
		int		nedge[2];		// local edge index into faces (-1 if face is not set)

		EDGE()
		{
			nedge[0] = -1;
			nedge[1] = -1;
		}

		void addFace(FACEP f, int lid)
		{
			assert((lid>=0)&&(lid<=2));
			if      (nedge[0] == -1) { face[0] = f; nedge[0] = lid; }
			else if (nedge[1] == -1) { face[1] = f; nedge[1] = lid; }
			else { assert(false); }

#ifndef NDEBUG
			NODEP n[2] = {f->node[lid], f->node[(lid+1)%3]};
			if ((node[0] != n[0]) && (node[0] != n[1])) assert(false);
			if ((node[1] != n[0]) && (node[1] != n[1])) assert(false);
#endif
		}

		void removeFace(FACEP p)
		{
			if      ((nedge[0] != -1) && (p == face[0])) nedge[0] = -1;
			else if ((nedge[1] != -1) && (p == face[1])) nedge[1] = -1;
			else { assert(false); }
		}
	};

	struct FACE
	{
		NODEP	node[3];	// pointer to nodes
		EDGEP	edge[3];	// pointer to edges
		int		w[3];		// winding flags
	};

	class NodeIterator
	{
	public:
		NodeIterator(DynamicMesh2D& mesh) : m_mesh(mesh)
		{
			m_node = mesh.m_Node.begin();
		}

		void operator ++ () { m_node++; }


		NODEP operator -> () { return m_node; }

		bool isValid() { return (m_node != m_mesh.m_Node.end()); }

		void reset() { m_node = m_mesh.m_Node.begin(); }

	private:
		DynamicMesh2D&	m_mesh;
		NODEP			m_node;
	};
	friend class NodeIterator;

	class EdgeIterator
	{
	public:
		EdgeIterator(DynamicMesh2D& mesh) : m_mesh(mesh)
		{
			m_edge = mesh.m_Edge.begin();
		}

		void operator ++ () { m_edge++; }

		void operator = (EDGEP e) { m_edge = e; }
		EDGEP operator -> () { return m_edge; }
		operator EDGEP() { return m_edge; }

		bool isValid() { return (m_edge != m_mesh.m_Edge.end()); }

	private:
		DynamicMesh2D&	m_mesh;
		EDGEP			m_edge;
	};
	friend class NodeIterator;

	class FaceIterator
	{
	public:
		FaceIterator(DynamicMesh2D& mesh) : m_mesh(mesh)
		{
			m_face = mesh.m_Face.begin();
		}

		void operator ++ () { m_face++; }


		FACEP operator -> () { return m_face; }

		bool isValid() { return (m_face != m_mesh.m_Face.end()); }

		void reset() { m_face = m_mesh.m_Face.begin(); }

	private:
		DynamicMesh2D&	m_mesh;
		FACEP			m_face;
	};
	friend class FaceIterator;

private:
	list<NODE>	m_Node;
	list<EDGE>	m_Edge;
	list<FACE>	m_Face;

public:
	DynamicMesh2D()
	{
	}

	int Nodes() const { return (int) m_Node.size(); }
	int Edges() const { return (int) m_Edge.size(); }
	int Faces() const { return (int) m_Face.size(); }

	NODEP addNode(const vec3d& r)
	{	
		NODE node;
		node.r = r;
		return m_Node.insert(m_Node.end(), node);
	}

	void removeFace(FACEP f)
	{
		f->edge[0]->removeFace(f);
		f->edge[1]->removeFace(f);
		f->edge[2]->removeFace(f);

		m_Face.erase(f);
	}

	void removeEdge(EDGEP p)
	{
		if (p->nedge[0] != -1) removeFace(p->face[0]);
		if (p->nedge[1] != -1) removeFace(p->face[1]);
		m_Edge.erase(p);
	}

	void addFace(const FACE& f)
	{
		FACEP fp = m_Face.insert(m_Face.end(), f);
		updateFace(fp);
	}

	void updateFace(FACEP fp)
	{
		fp->edge[0]->addFace(fp, 0);
		fp->edge[1]->addFace(fp, 1);
		fp->edge[2]->addFace(fp, 2);

		for (int j = 0; j<3; ++j)
		{
			EDGEP ep = fp->edge[j];
			NODEP n0 = fp->node[j];
			NODEP n1 = fp->node[(j + 1) % 3];
			if ((ep->node[0] == n0) && (ep->node[1] == n1)) fp->w[j] = 1;
			else if ((ep->node[0] == n1) && (ep->node[1] == n0)) fp->w[j] = -1;
			else
				assert(false);
		}
	}

	void splitFace(FACEP f, NODEP p)
	{
		EDGEP newEdge[3];
		for (int i = 0; i<3; ++i)
		{
			EDGE edge;
			edge.node[0] = p;
			edge.node[1] = f->node[i];
			newEdge[i] = m_Edge.insert(m_Edge.end(), edge);
		}

		FACE face1;
		face1.node[0] = f->node[0];
		face1.node[1] = f->node[1];
		face1.node[2] = p;
		face1.edge[0] = f->edge[0];
		face1.edge[1] = newEdge[1];
		face1.edge[2] = newEdge[0];

		FACE face2;
		face2.node[0] = f->node[1];
		face2.node[1] = f->node[2];
		face2.node[2] = p;
		face2.edge[0] = f->edge[1];
		face2.edge[1] = newEdge[2];
		face2.edge[2] = newEdge[1];

		FACE face3;
		face3.node[0] = f->node[2];
		face3.node[1] = f->node[0];
		face3.node[2] = p;
		face3.edge[0] = f->edge[2];
		face3.edge[1] = newEdge[0];
		face3.edge[2] = newEdge[2];

		removeFace(f);

		addFace(face1);
		addFace(face2);
		addFace(face3);
	}

	NODEP opposingNode(EDGEP e, int nface)
	{
		assert((nface == 0) || (nface == 1));
		int ne = e->nedge[nface];
		if (ne == -1) return m_Node.end();
		FACEP f = e->face[nface];
		return f->node[(ne + 2) % 3];
	}

	EDGEP nextEdge(EDGEP e, int nface)
	{
		assert((nface == 0) || (nface == 1));
		int ne = e->nedge[nface];
		if (ne == -1) return m_Edge.end();
		FACEP f = e->face[nface];
		return f->edge[(ne + 1) % 3];
	}

	EDGEP prevEdge(EDGEP e, int nface)
	{
		assert((nface == 0) || (nface == 1));
		int ne = e->nedge[nface];
		if (ne == -1) return m_Edge.end();
		FACEP f = e->face[nface];
		return f->edge[(ne + 2) % 3];
	}

	EDGEP addEdge(NODEP n0, NODEP n1)
	{
		EDGE e;
		e.node[0] = n0;
		e.node[1] = n1;
		return m_Edge.insert(m_Edge.end(), e);
	}

	void splitEdge(EDGEP e, NODEP p)
	{
		EDGEP newEdge[3];
		newEdge[0] = addEdge(e->node[0], p);
		newEdge[1] = addEdge(p, e->node[1]);

		FACE newFace[4];
		bool bface[4] = {false, false, false, false};
		for (int i = 0; i<2; ++i)
		{
			// add new faces
			if (e->nedge[i] != -1)
			{
				FACEP f = e->face[i];
				int ne = e->nedge[i];

				NODEP n[3];
				n[0] = opposingNode(e, i); assert(n[0] != m_Node.end());
				n[1] = (f->w[ne] == 1 ? e->node[0] : e->node[1]);
				n[2] = (f->w[ne] == 1 ? e->node[1] : e->node[0]);

				newEdge[2] = addEdge(n[0], p);

				EDGEP ed[4];
				ed[0] = (f->w[ne] == 1 ? newEdge[0] : newEdge[1]);
				ed[1] = (f->w[ne] == 1 ? newEdge[1] : newEdge[0]);
				ed[2] = nextEdge(e, i);
				ed[3] = prevEdge(e, i);

				newFace[2*i].node[0] = n[0];
				newFace[2*i].node[1] = n[1];
				newFace[2*i].node[2] = p;
				newFace[2*i].edge[0] = ed[3];
				newFace[2*i].edge[1] = ed[0];
				newFace[2*i].edge[2] = newEdge[2];
				bface[2*i] = true;

				newFace[2*i+1].node[0] = n[0];
				newFace[2*i+1].node[1] = p;
				newFace[2*i+1].node[2] = n[2];
				newFace[2*i+1].edge[0] = newEdge[2];
				newFace[2*i+1].edge[1] = ed[1];
				newFace[2*i+1].edge[2] = ed[2];
				bface[2*i+1] = true;
			}
		}

		// remove the old edge
		removeEdge(e);

		// add new faces
		if (bface[0]) addFace(newFace[0]);
		if (bface[1]) addFace(newFace[1]);
		if (bface[2]) addFace(newFace[2]);
		if (bface[3]) addFace(newFace[3]);
	}

	// returns:
	// -1: point does not lie in face
	// 0,1,2: point coincide with node i (within tolerance)
	// 3,4,5: point falls on edge (i-3)
	// 6: point lies in face
	int intersectFace(FACE& f, const vec3d& p, vec3d& q)
	{
		const double eps = 1e-6;

		// edge Lenghts
		double L[3];
		L[0] = (f.edge[0]->node[0]->r - f.edge[0]->node[1]->r).Length();
		L[1] = (f.edge[1]->node[0]->r - f.edge[1]->node[1]->r).Length();
		L[2] = (f.edge[2]->node[0]->r - f.edge[2]->node[1]->r).Length();
		double Lmax = L[0];
		if (L[1] > Lmax) Lmax = L[1];
		if (L[2] > Lmax) Lmax = L[2];
		if (Lmax == 0.0) Lmax = 1.0;

		// see if it coincides with a node first
		double d1 = (f.node[0]->r - p).Length(); if (d1 / Lmax < eps) return 0;
		double d2 = (f.node[1]->r - p).Length(); if (d2 / Lmax < eps) return 1;
		double d3 = (f.node[2]->r - p).Length(); if (d3 / Lmax < eps) return 2;

		// see if it projects on an edge
		for (int i=0; i<3; ++i)
		{
			EDGEP edge = f.edge[i];

			vec3d t = (edge->node[1]->r - edge->node[0]->r);
			vec3d r = p - edge->node[0]->r; 
			double R = r.Length();
			r.Normalize();
			double c = r * t / L[i];
			if ((c > 1.0 - eps) && (R / L[i] >= -eps) && (R / L[i] <= 1 + eps)) return i + 3;
		}

		vec3d e[2], o = f.node[0]->r;
		e[0] = f.node[1]->r - f.node[0]->r;
		e[1] = f.node[2]->r - f.node[0]->r;

		// next, we decompose q into its components
		// in the triangle basis
		// we need to create the dual basis
		// first, we calculate the metric tensor
		double G[2][2];
		G[0][0] = e[0] * e[0]; G[0][1] = e[0] * e[1];
		G[1][0] = e[1] * e[0]; G[1][1] = e[1] * e[1];

		// and its inverse
		double D = G[0][0] * G[1][1] - G[0][1] * G[1][0];
		double Gi[2][2];
		Gi[0][0] = 1 / D*G[1][1]; Gi[0][1] = -1 / D*G[0][1];
		Gi[1][0] = -1 / D*G[1][0]; Gi[1][1] = 1 / D*G[0][0];

		// build dual basis
		vec3d E[2];
		E[0] = e[0] * Gi[0][0] + e[1] * Gi[0][1];
		E[1] = e[0] * Gi[1][0] + e[1] * Gi[1][1];

		// get the components
		double rp = E[0] * (p - o);
		double sp = E[1] * (p - o);

		q = o + (e[0]*rp + e[1]*sp);

		if ((rp>=-eps)&&(sp>=-eps)&&(rp+sp<=1.0+eps)) return 6;

		return -1;
	}

	NODEP insertPoint(const vec3d& r)
	{
		vec3d q;
		for (FACEP f = m_Face.begin(); f != m_Face.end(); ++f)
		{
			int ncase = intersectFace(*f, r, q);
			switch (ncase)
			{
			case 0:
			case 1:
			case 2:
				{
					// point coincides with node
					// Don't do anything
					return f->node[ncase];
				}
				break;
			case 3:
			case 4:
			case 5:
				{
					// point coincide with edge
					int eid = ncase - 3;
					EDGEP edge = f->edge[eid];

					// create a new node					
					NODEP newNode = addNode(q);

					splitEdge(edge, newNode);

					return newNode;
				}
				break;
			case 6:
				{
					// create a new node					
					NODEP newNode = addNode(q);

					splitFace(f, newNode);

					return newNode;
				}
				break;
			}
		}

		// if we get here, let's just add the node
		return addNode(r);
	}

	void insertEdge(NODEP n0, NODEP n1, vector<NODEP>& nodeList)
	{
		// loop over all edges
		for (EDGEP e = m_Edge.begin(); e != m_Edge.end(); ++e)
		{	
			// see if the edges intersect
			vec3d q;
			int ncase = edgeIntersect(e->node[0], e->node[1], n0, n1, q);
			switch (ncase)
			{
			case 1: // same edge
				// edge already exists, so return
				return;
			case 2: // proper intersection
				{
					NODEP newNode = addNode(q);
					nodeList.push_back(newNode);
					splitEdge(e, newNode);

					// insert the two sub-edges
					insertEdge(n0, newNode, nodeList);
					insertEdge(newNode, n1, nodeList);

					return;
				}
				break;
			case 3: // edges coincide at node
				// skip this edge
				break;
			}
		}
	}

	void flipEdge(EDGEP e)
	{
		// make sure this edge can be flipped
		if ((e->nedge[0]==-1)||(e->nedge[1]==-1)) return;

		NODEP node[4];
		node[0] = (e->face[0]->w[e->nedge[0]] == 1 ? e->node[0] : e->node[1]);
		node[1] = (e->face[0]->w[e->nedge[0]] == 1 ? e->node[1] : e->node[0]);
		node[2] = opposingNode(e, 0);
		node[3] = opposingNode(e, 1);

		EDGEP edge[4];
		edge[0] = nextEdge(e, 0);
		edge[1] = prevEdge(e, 0);
		edge[2] = nextEdge(e, 1);
		edge[3] = prevEdge(e, 1);

		FACEP face[2];
		face[0] = e->face[0];
		face[1] = e->face[1];

		// disconnect faces
		e->removeFace(face[0]);
		e->removeFace(face[1]);
		edge[0]->removeFace(face[0]);
		edge[1]->removeFace(face[0]);
		edge[2]->removeFace(face[1]);
		edge[3]->removeFace(face[1]);

		// rotate edge
		e->node[0] = node[2];
		e->node[1] = node[3];

		// rotate faces
		face[0]->node[0] = node[0];
		face[0]->node[1] = node[3];
		face[0]->node[2] = node[2];
		face[0]->edge[0] = edge[2];
		face[0]->edge[1] = e;
		face[0]->edge[2] = edge[1];

		face[1]->node[0] = node[1];
		face[1]->node[1] = node[2];
		face[1]->node[2] = node[3];
		face[1]->edge[0] = edge[0];
		face[1]->edge[1] = e;
		face[1]->edge[2] = edge[3];

		updateFace(face[0]);
		updateFace(face[1]);

		assert((edge[0]->nedge[0] != -1) && (edge[0]->nedge[1] != -1));
		assert((edge[1]->nedge[0] != -1) && (edge[1]->nedge[1] != -1));
		assert((edge[2]->nedge[0] != -1) && (edge[2]->nedge[1] != -1));
		assert((edge[3]->nedge[0] != -1) && (edge[3]->nedge[1] != -1));
		assert((e->nedge[0] != -1) && (e->nedge[1] != -1));
	}

	// returns:
	// 0 = no intersection
	// 1 = same edge
	// 2 = proper intersection
	// 3 = edges coincide at point
	int edgeIntersect(NODEP n0, NODEP n1, NODEP n2, NODEP n3, vec3d& q)
	{
		// see if this edge already exists
		if ((n0 == n2) && (n1 == n3)) return 1;
		if ((n0 == n3) && (n1 == n2)) return 1;

		// see if any nodes coincides
		if ((n0 == n2) || (n0 == n3)) return 3;
		if ((n1 == n2) || (n1 == n3)) return 3;

		vec3d a = n0->r;
		vec3d b = n1->r;
		vec3d c = n2->r;
		vec3d d = n3->r;

		vec3d t = b - a;
		vec3d r = d - c;

		double A[2][2], B[2];
		A[0][0] = t.x; A[0][1] = -r.x; B[0] = c.x - a.x;
		A[1][0] = t.y; A[1][1] = -r.y; B[1] = c.y - a.y;

		// calculate inverse
		double D = A[0][0]*A[1][1] - A[0][1]*A[1][0];
		if (D != 0.0)
		{
			double Ai[2][2];
			Ai[0][0] =  A[1][1] / D; Ai[0][1] = -A[0][1]/D;
			Ai[1][0] = -A[1][0] / D; Ai[1][1] =  A[0][0]/D;

			double lm = Ai[0][0]*B[0] + Ai[0][1]*B[1];
			double mu = Ai[1][0]*B[0] + Ai[1][1]*B[1];

			if ((lm>0) && (lm<1) && (mu>0) && (mu<1))
			{
				// it's a proper intersection
				q = a + t*lm;
				return 2;
			}
		}

		return 0;
	}
};

FECurveIntersect2D::FECurveIntersect2D() : FEModifier("Intersect Curve2D")
{
	m_pc = 0;
}

void FECurveIntersect2D::SetCurveMesh(GCurveMeshObject* pc)
{
	m_pc = pc;
}

// This is defined in triangulate.cpp
double Area2(vec3d& a, vec3d& b, vec3d& c);

void angleCosines(vec3d& a, vec3d& b, vec3d& c, double w[3])
{
	vec3d e0 = (b - a); e0.Normalize();
	vec3d e1 = (c - b); e1.Normalize();
	vec3d e2 = (a - c); e2.Normalize();

	w[0] = -e0*e2;
	w[1] = -e0*e1;
	w[2] = -e1*e2;
}

double maxCosine(vec3d& a, vec3d& b, vec3d& c)
{
	double w[3];
	angleCosines(a,b,c,w);
	double maxw = w[0];
	if (w[1] > maxw) maxw = w[1];
	if (w[2] > maxw) maxw = w[2];
	return maxw;
}

double Area2(DynamicMesh2D::FACEP fp)
{
	return Area2(fp->node[0]->r, fp->node[1]->r, fp->node[2]->r);
}

FSMesh* FECurveIntersect2D::Apply(FSMesh* pm)
{
	// make sure this is a triangle mesh
	if (pm->IsType(FE_TRI3) == false) return 0;

	// make sure we have a curve
	if (m_pc == 0) return 0;

	DynamicMesh2D dyna;
	BuildMesh(dyna, pm);

	GObject* pd = pm->GetGObject();

	FSCurveMesh* ps = m_pc->GetCurveMesh();

	// insert all the nodes
	int N = ps->Nodes();
	vector<DynamicMesh2D::NODEP> nodeList;
	for (int i = 0; i<N; ++i)
	{
		vec3d r = ps->Node(i).r;
		r = m_pc->GetTransform().LocalToGlobal(r);
		if (pd) r = pd->GetTransform().GlobalToLocal(r);
		DynamicMesh2D::NODEP node = dyna.insertPoint(r);
		nodeList.push_back(node);
	}

	// insert all the edges
	int NE = ps->Edges();
	for (int i=0; i<NE; ++i)
	{
		FSEdge& e = ps->Edge(i);
		dyna.insertEdge(nodeList[e.n[0]], nodeList[e.n[1]], nodeList);
	}

	// flip edges 
/*	for (DynamicMesh2D::EdgeIterator edge(dyna); edge.isValid(); ++edge)
	{	
		if ((edge->nedge[0] != -1) && (edge->nedge[1] != -1))
		{
			DynamicMesh2D::NODEP node[4];
			node[0] = (edge->face[0]->w[edge->nedge[0]] == 1 ? edge->node[0] : edge->node[1]);
			node[1] = (edge->face[0]->w[edge->nedge[0]] == 1 ? edge->node[1] : edge->node[0]);
			node[2] = dyna.opposingNode(edge, 0);
			node[3] = dyna.opposingNode(edge, 1);

			vec3d r[4];
			r[0] = node[0]->r;
			r[1] = node[1]->r;
			r[2] = node[2]->r;
			r[3] = node[3]->r;


			// maximize min area
			double A[4], minA, minB;
			A[0] = Area2(r[0], r[1], r[2]); assert(A[0] > 0.0);
			A[1] = Area2(r[0], r[3], r[1]); assert(A[1] > 0.0);
			minA = (A[0] < A[1] ? A[0] : A[1]);
			A[2] = Area2(r[0], r[3], r[2]);
			A[3] = Area2(r[1], r[2], r[3]);
			minB = (A[2] < A[3] ? A[2] : A[3]);
			if (minB > minA)
			{
				dyna.flipEdge(edge);
			}

			// maximize min angle (= minimize max cosine)
			double A[4], maxA, maxB;
			A[0] = maxCosine(r[0], r[1], r[2]); assert(A[0] > 0.0);
			A[1] = maxCosine(r[0], r[3], r[1]); assert(A[1] > 0.0);
			maxA = (A[0] > A[1] ? A[0] : A[1]);
			A[2] = maxCosine(r[0], r[3], r[2]); assert(A[2] > 0.0);
			A[3] = maxCosine(r[1], r[2], r[3]); assert(A[3] > 0.0);
			maxB = (A[0] > A[1] ? A[0] : A[1]);
			if (maxB < maxA)
			{
				dyna.flipEdge(edge);
			}
		}
	}
*/
	// tag all new nodes
	int nodes = pm->Nodes();
	for (size_t i = 0; i < nodeList.size(); ++i) 
	{
		// Note that some nodes will already have their tag set
		// These nodes are nodes that coincide with nodes of the original mesh
		if (nodeList[i]->ntag == 0)
			nodeList[i]->ntag = nodes++;
	}

	return BuildFEMesh(dyna);
}

void FECurveIntersect2D::BuildMesh(DynamicMesh2D& dyna, FSMesh* pm)
{
	// build the nodes
	int NN = pm->Nodes();
	vector<DynamicMesh2D::NODEP> nodePtr;
	for (int i = 0; i<NN; ++i)
	{
		DynamicMesh2D::NODEP n = dyna.addNode(pm->Node(i).r);
		n->ntag = i;
		nodePtr.push_back(n);
	}

	// build the edges
	EdgeList ET(*pm);
	int NE = ET.size();
	vector<DynamicMesh2D::EDGEP> edgePtr;
	for (int i = 0; i<NE; ++i)
	{
		pair<int, int>& edge = ET[i];
		DynamicMesh2D::EDGEP e = dyna.addEdge(nodePtr[edge.first], nodePtr[edge.second]);
		edgePtr.push_back(e);
	}

	// build the faces
	FSFaceEdgeList FET(*pm, ET);
	int NF = pm->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = pm->Face(i);
		DynamicMesh2D::FACE f;
		f.node[0] = nodePtr[face.n[0]];
		f.node[1] = nodePtr[face.n[1]];
		f.node[2] = nodePtr[face.n[2]];

		f.edge[0] = edgePtr[FET[i][0]];
		f.edge[1] = edgePtr[FET[i][1]];
		f.edge[2] = edgePtr[FET[i][2]];

		dyna.addFace(f);
	}
}

FSMesh* FECurveIntersect2D::BuildFEMesh(DynamicMesh2D& dyna)
{
	DynamicMesh2D::NodeIterator nodePtr(dyna);
	for (;nodePtr.isValid(); ++nodePtr) nodePtr->ntag = -1;

	DynamicMesh2D::FaceIterator facePtr(dyna);
	for (;facePtr.isValid(); ++facePtr)
	{
		facePtr->node[0]->ntag = 1;
		facePtr->node[1]->ntag = 1;
		facePtr->node[2]->ntag = 1;
	}

	nodePtr.reset();
	int nodes = 0;
	for (; nodePtr.isValid(); ++nodePtr) 
		if (nodePtr->ntag == 1) nodePtr->ntag = nodes++;

	int NN = nodes;
	int NF = dyna.Faces();

	FSMesh* pm = new FSMesh;
	pm->Create(NN, NF);

	nodePtr.reset();
	for (; nodePtr.isValid(); ++nodePtr)
		if (nodePtr->ntag >= 0)
			pm->Node(nodePtr->ntag).r = nodePtr->r;

	facePtr.reset();
	for (int i = 0; i<NF; ++i, ++facePtr)
	{
		FSElement& el = pm->Element(i);
		el.SetType(FE_TRI3);
		el.m_node[0] = facePtr->node[0]->ntag; assert((el.m_node[0] >= 0) && (el.m_node[0] < NN));
		el.m_node[1] = facePtr->node[1]->ntag; assert((el.m_node[1] >= 0) && (el.m_node[1] < NN));
		el.m_node[2] = facePtr->node[2]->ntag; assert((el.m_node[2] >= 0) && (el.m_node[2] < NN));
		el.m_gid = 0;
	}

	// next, we use the auto-mesher to reconstruct all faces, edges and nodes
	pm->RebuildMesh();

	return pm;
}
