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

#include "TriMesh.h"
#include "MeshTools.h"
#include <stack>

//=============================================================================
// TriMesh::EDGE
//=============================================================================

//-----------------------------------------------------------------------------
// constructor
TriMesh::EDGE::EDGE()
{
	nedge[0] = -1;
	nedge[1] = -1;
	ntag = 0;
	gid = -1;
}

//-----------------------------------------------------------------------------
// Attach the face to an edge
void TriMesh::EDGE::addFace(TriMesh::FACEP f, int lid)
{
	// find an open slot
	assert((lid >= 0) && (lid <= 2));
	if      (nedge[0] == -1) { face[0] = f; nedge[0] = lid; }
	else if (nedge[1] == -1) { face[1] = f; nedge[1] = lid; }
	else { assert(false); }

#ifndef NDEBUG
	NODEP n[2] = { f->node[lid], f->node[(lid + 1) % 3] };
	if ((node[0] != n[0]) && (node[0] != n[1])) assert(false);
	if ((node[1] != n[0]) && (node[1] != n[1])) assert(false);
#endif
}

//-----------------------------------------------------------------------------
// Remove the face from the edge
void TriMesh::EDGE::removeFace(TriMesh::FACEP p)
{
	if      ((nedge[0] != -1) && (p == face[0])) nedge[0] = -1;
	else if ((nedge[1] != -1) && (p == face[1])) nedge[1] = -1;
	else { assert(false); }
}

//=============================================================================
// TriMesh::FACE
//=============================================================================

TriMesh::FACE::FACE()
{
	gid = 0;
}

//=============================================================================
// TriMesh
//=============================================================================

TriMesh::TriMesh()
{
}

//-----------------------------------------------------------------------------
// Add a node to the mesh.
// The node will not be attached to any edge or face
TriMesh::NODEP TriMesh::addNode(const vec3d& r, int ntag)
{
	NODE node(r);
	node.ntag = ntag;
	return m_Node.insert(m_Node.end(), node);
}

//-----------------------------------------------------------------------------
// Add a new edge to the mesh
TriMesh::EDGEP TriMesh::addEdge(NODEP n0, NODEP n1, int ntag)
{
	assert(n0 != n1);
	EDGE e;
	e.node[0] = n0;
	e.node[1] = n1;
	e.ntag = ntag;

	n0->eval++;
	n1->eval++;

	return m_Edge.insert(m_Edge.end(), e);
}

//-----------------------------------------------------------------------------
// Add a new face to the mesh
// It is assumed that the nodes and edges are defined.
TriMesh::FACEP TriMesh::addFace(const FACE& f)
{
	assert((f.edge[0]->node[0] == f.node[0]) || (f.edge[0]->node[1] == f.node[0]));
	assert((f.edge[0]->node[1] == f.node[1]) || (f.edge[0]->node[0] == f.node[1]));

	assert((f.edge[1]->node[0] == f.node[1]) || (f.edge[1]->node[1] == f.node[1]));
	assert((f.edge[1]->node[1] == f.node[2]) || (f.edge[1]->node[0] == f.node[2]));

	assert((f.edge[2]->node[0] == f.node[2]) || (f.edge[2]->node[1] == f.node[2]));
	assert((f.edge[2]->node[1] == f.node[0]) || (f.edge[2]->node[0] == f.node[0]));

	// insert it in the list
	FACEP fp = m_Face.insert(m_Face.end(), f);

	// update additional face data
	updateFace(fp);

	return fp;
}

//-----------------------------------------------------------------------------
// Helper class for setting additional face data after inserting a face
void TriMesh::updateFace(TriMesh::FACEP fp)
{
	// attach the face to the edges
	fp->edge[0]->addFace(fp, 0);
	fp->edge[1]->addFace(fp, 1);
	fp->edge[2]->addFace(fp, 2);

	// check the edge winding
	for (int j = 0; j<3; ++j)
	{
		EDGEP ep = fp->edge[j];
		NODEP n0 = fp->node[j];
		NODEP n1 = fp->node[(j + 1) % 3];
		if      ((ep->node[0] == n0) && (ep->node[1] == n1)) fp->w[j] =  1;
		else if ((ep->node[0] == n1) && (ep->node[1] == n0)) fp->w[j] = -1;
		else
			assert(false);
	}

	// calculate normal
	vec3d n = (fp->node[1]->r - fp->node[0]->r) ^ (fp->node[2]->r - fp->node[0]->r);
	n.Normalize();
	fp->normal = n;
}

//-----------------------------------------------------------------------------
// Remove a face from the mesh
void TriMesh::removeFace(TriMesh::FACEP f, bool removeEdges)
{
	// detach the edges
	f->edge[0]->removeFace(f);
	f->edge[1]->removeFace(f);
	f->edge[2]->removeFace(f);

	// see if we should erase the edges as well
	if (removeEdges)
	{
		if ((f->edge[0]->nedge[0] == -1) && (f->edge[0]->nedge[1] == -1) && (f->edge[0]->gid == -1)) removeEdge(f->edge[0]);
		if ((f->edge[1]->nedge[0] == -1) && (f->edge[1]->nedge[1] == -1) && (f->edge[1]->gid == -1)) removeEdge(f->edge[1]);
		if ((f->edge[2]->nedge[0] == -1) && (f->edge[2]->nedge[1] == -1) && (f->edge[2]->gid == -1)) removeEdge(f->edge[2]);
	}

	// erase the face
	m_Face.erase(f);
}

//-----------------------------------------------------------------------------
// Remove the edge
// This will also remove the faces.
void TriMesh::removeEdge(TriMesh::EDGEP p)
{
	// first remove the faces
	if (p->nedge[0] != -1) removeFace(p->face[0]);
	if (p->nedge[1] != -1) removeFace(p->face[1]);

	// decrease valence counter
	p->node[0]->eval--; assert(p->node[0]->eval >= 0);
	p->node[1]->eval--; assert(p->node[1]->eval >= 0);

	// remove the edge
	m_Edge.erase(p);
}

//-----------------------------------------------------------------------------
TriMesh::NODEP TriMesh::opposingNode(EDGEP e, int nface)
{
	assert((nface == 0) || (nface == 1));
	int ne = e->nedge[nface];
	if (ne == -1) return m_Node.end();
	FACEP f = e->face[nface];
	NODEP n = f->node[(ne + 2) % 3];
	assert((e->node[0] != n) && (e->node[1] != n));
	return n;
}

//-----------------------------------------------------------------------------
TriMesh::EDGEP TriMesh::nextEdge(TriMesh::EDGEP e, int nface)
{
	assert((nface == 0) || (nface == 1));
	int ne = e->nedge[nface];
	if (ne == -1) return m_Edge.end();
	FACEP f = e->face[nface];
	return f->edge[(ne + 1) % 3];
}

//-----------------------------------------------------------------------------
TriMesh::EDGEP TriMesh::prevEdge(TriMesh::EDGEP e, int nface)
{
	assert((nface == 0) || (nface == 1));
	int ne = e->nedge[nface];
	if (ne == -1) return m_Edge.end();
	FACEP f = e->face[nface];
	return f->edge[(ne + 2) % 3];
}

//-----------------------------------------------------------------------------
TriMesh::EDGEP TriMesh::opposingEdge(TriMesh::FACEP f, TriMesh::NODEP n)
{
	EDGEP e = m_Edge.end();
	if ((f->edge[0]->node[0] != n) && (f->edge[0]->node[1] != n)) e = f->edge[0];
	if ((f->edge[1]->node[0] != n) && (f->edge[1]->node[1] != n)) e = f->edge[1];
	if ((f->edge[2]->node[0] != n) && (f->edge[2]->node[1] != n)) e = f->edge[2];

	assert(e != m_Edge.end());
	return e;
}

//-----------------------------------------------------------------------------
// split a face at a node
void TriMesh::splitFace(FACEP f, NODEP p)
{
	// create three new edges,
	// connecting the face nodes to node p
	EDGEP newEdge[3];
	for (int i = 0; i<3; ++i) newEdge[i] = addEdge(p, f->node[i]);

	// setup the three new faces
	FACE face1;
	face1.node[0] = f->node[0];
	face1.node[1] = f->node[1];
	face1.node[2] = p;
	face1.edge[0] = f->edge[0];
	face1.edge[1] = newEdge[1];
	face1.edge[2] = newEdge[0];
	face1.ntag = f->ntag;

	FACE face2;
	face2.node[0] = f->node[1];
	face2.node[1] = f->node[2];
	face2.node[2] = p;
	face2.edge[0] = f->edge[1];
	face2.edge[1] = newEdge[2];
	face2.edge[2] = newEdge[1];
	face2.ntag = f->ntag;

	FACE face3;
	face3.node[0] = f->node[2];
	face3.node[1] = f->node[0];
	face3.node[2] = p;
	face3.edge[0] = f->edge[2];
	face3.edge[1] = newEdge[0];
	face3.edge[2] = newEdge[2];
	face3.ntag = f->ntag;

	// first remove the old face
	// this will also detach this face from the edges
	removeFace(f);

	// add the three new faces
	addFace(face1);
	addFace(face2);
	addFace(face3);
}

//-----------------------------------------------------------------------------
// Split the edge at node p.
// This also splits the adjoining faces.
void TriMesh::splitEdge(TriMesh::EDGEP e, TriMesh::NODEP p)
{
	EDGEP newEdge[3];

	// create the first two new edges by splitting this edge at p
	newEdge[0] = addEdge(e->node[0], p, e->ntag);
	newEdge[1] = addEdge(p, e->node[1], e->ntag);

	// create new edges and faces
	FACE newFace[4];
	bool bface[4] = { false, false, false, false };
	for (int i = 0; i<2; ++i)
	{
		// add new faces
		if (e->nedge[i] != -1)
		{
			// get the facet
			FACEP f = e->face[i];
			int ne = e->nedge[i];

			// get the nodes of the face f, but reordered
			// such that n0 is the node opposing the edge
			NODEP n[3];
			n[0] = opposingNode(e, i); assert(n[0] != m_Node.end());
			n[1] = (f->w[ne] == 1 ? e->node[0] : e->node[1]);
			n[2] = (f->w[ne] == 1 ? e->node[1] : e->node[0]);

			// create the edge that splits the face
			newEdge[2] = addEdge(n[0], p);

			// build the edge loop
			EDGEP ed[4];
			ed[0] = (f->w[ne] == 1 ? newEdge[0] : newEdge[1]);
			ed[1] = (f->w[ne] == 1 ? newEdge[1] : newEdge[0]);
			ed[2] = nextEdge(e, i); assert(ed[2] != m_Edge.end());
			ed[3] = prevEdge(e, i); assert(ed[3] != m_Edge.end());

			// build the new faces
			newFace[2 * i].node[0] = n[0];
			newFace[2 * i].node[1] = n[1];
			newFace[2 * i].node[2] = p;
			newFace[2 * i].edge[0] = ed[3];
			newFace[2 * i].edge[1] = ed[0];
			newFace[2 * i].edge[2] = newEdge[2];
			newFace[2 * i].ntag = f->ntag;
			bface[2 * i] = true;

			newFace[2 * i + 1].node[0] = n[0];
			newFace[2 * i + 1].node[1] = p;
			newFace[2 * i + 1].node[2] = n[2];
			newFace[2 * i + 1].edge[0] = newEdge[2];
			newFace[2 * i + 1].edge[1] = ed[1];
			newFace[2 * i + 1].edge[2] = ed[2];
			newFace[2 * i + 1].ntag = f->ntag;
			bface[2 * i + 1] = true;
		}
	}

	// remove the old edge first
	// this will also remove the old faces
	removeEdge(e);

	// add new faces
	if (bface[0]) addFace(newFace[0]);
	if (bface[1]) addFace(newFace[1]);
	if (bface[2]) addFace(newFace[2]);
	if (bface[3]) addFace(newFace[3]);
}

//-----------------------------------------------------------------------------
bool TriMesh::flipEdge(TriMesh::EDGEP e)
{
	// Don't know if this function works correctly yet
//	assert(false);

	// make sure this edge can be flipped
	if ((e->nedge[0] == -1) || (e->nedge[1] == -1)) return false;

	// get the four nodes
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

	return true;
}

//-------------------------------------------------------------------
TriMesh::EDGEP TriMesh::findEdge(TriMesh::NODEP n0, TriMesh::NODEP n1)
{
	TriMesh::EdgeIterator it(*this);
	while (it.isValid())
	{
		if ((it->node[0] == n0) && (it->node[1] == n1)) return it;
		if ((it->node[1] == n0) && (it->node[0] == n1)) return it;
		++it;
	}

	return it;
}

//-------------------------------------------------------------------
TriMesh::FACEP TriMesh::faceNeighbor(TriMesh::FACEP f, int i)
{
	EDGEP e = f->edge[i];
	assert((e->face[0] == f) || (e->face[1] == f));
	if (e->face[0] == f)
	{
		return (e->nedge[1] != -1 ? e->face[1] : m_Face.end());
	}
	else
	{
		return (e->nedge[0] != -1 ? e->face[0] : m_Face.end());
	}
}

//-------------------------------------------------------------------
int TriMesh::edgeWinding(TriMesh::FACEP f, TriMesh::EDGEP e)
{
	if (e == f->edge[0]) return f->w[0];
	if (e == f->edge[1]) return f->w[1];
	if (e == f->edge[2]) return f->w[2];

	assert(false);

	return 0;
}

//-------------------------------------------------------------------
void TriMesh::tagAllEdges(int ntag)
{
	EdgeIterator it(*this);
	while (it.isValid())
	{
		it->ntag = ntag;
		++it;
	}
}

//-------------------------------------------------------------------
void TriMesh::tagAllFaces(int ntag)
{
	FaceIterator it(*this);
	while (it.isValid())
	{
		it->ntag = ntag;
		++it;
	}
}

//-------------------------------------------------------------------
void TriMesh::PartitionSurface()
{
	FaceIterator faceIt(*this);
	while (faceIt.isValid())
	{
		faceIt->ntag = 0;
		faceIt->gid = 0;
		++faceIt;
	}

	int np = 0;
	faceIt.reset();
	while (1)
	{
		// find an unprocessed face
		FACEP f = m_Face.end();
		while (faceIt.isValid())
		{
			if (faceIt->ntag == 0)
			{
				f = *faceIt;
				++faceIt;
				break;
			}
			else ++faceIt;
		}

		if (f == m_Face.end()) break;

		std::stack<FACEP> S;
		S.push(f);
		f->ntag = 1;
		while (S.empty() == false)
		{
			FACEP f = S.top(); S.pop();

			f->gid = np;

			for (int i=0; i<3; ++i)
			{
				EDGEP ei = f->edge[i];
				if (ei->gid == -1)
				{
					FACEP fi = faceNeighbor(f, i);
					if ((fi != m_Face.end()) && (fi->ntag == 0))
					{
						fi->ntag = 1;
						S.push(fi);
					}
				}
			}
		}

		np++;
	}
}


//=============================================================================
// TriMesh algorithms
//=============================================================================

// project a point onto the plane defined by point c and normal N
vec3d projectToPlane(const vec3d& N, const vec3d& c, const vec3d& p)
{
	vec3d r = p - c;
	double l = r*N;
	vec3d q = c + (r - N*l);
	return q;
}

//-----------------------------------------------------------------------------
// See if point p projects inside the face
//
// returns:
//    -1: point does not lie in face
// 0,1,2: point coincide with node i (within tolerance)
// 3,4,5: point falls on edge (i-3)
//     6: point lies in face
//
int projectToFace(TriMesh::FACE& f, const vec3d& p, vec3d& q, const double eps)
{
	// project the point p onto the triangle's plane
	q = projectToPlane(f.normal, f.node[0]->r, p);

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
	double d1 = (f.node[0]->r - q).Length(); if (d1 / Lmax < eps) { q = f.node[0]->r; return 0; }
	double d2 = (f.node[1]->r - q).Length(); if (d2 / Lmax < eps) { q = f.node[1]->r; return 1; }
	double d3 = (f.node[2]->r - q).Length(); if (d3 / Lmax < eps) { q = f.node[2]->r; return 2; }

	// see if it projects on an edge
	for (int i = 0; i<3; ++i)
	{
		TriMesh::EDGEP edge = f.edge[i];

		vec3d t = (edge->node[1]->r - edge->node[0]->r);
		vec3d r = q - edge->node[0]->r;
		double c = (r * t) / (t*t);
		vec3d n = r - t*c;
		double h = n.Length() / L[i];
		if ((c > 0.0) && (c < 1.0) && (h <= eps)) 
		{
			q = edge->node[0]->r + t*c;
			return i + 3;
		}
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
	double rp = E[0] * (q - o);
	double sp = E[1] * (q - o);

	// we need a tighter tolerance for this test
	const double tol = 1e-6;
	if ((rp >= -tol) && (sp >= -tol) && (rp + sp <= 1.0 + tol)) 
	{
		return 6;
	}

	return -1;
}

//-----------------------------------------------------------------------------
double projectToEdge(TriMesh::EDGE& edge, const vec3d& r, vec3d& q)
{
	// project onto the edge
	vec3d& c = edge.node[0]->r;
	vec3d t = edge.node[1]->r - c;
	vec3d p = r - c;
	double l = t*p / (t*t);
	q = c + t*l;

	return l;
}

//-----------------------------------------------------------------------------
// This function inserts a new node at the position r. If necessary, edges or faces
// are split in order to retain a valid mesh.
TriMesh::NODEP insertPoint(TriMesh& mesh, const vec3d& r, const double eps)
{
	// find the closest node first
	// if the projection on the faces or edges fail, the closest node will be returned.
	double Lmin = 1e99;
	vec3d closestPoint;
	TriMesh::NODEP closestNode = mesh.m_Node.end();
	for (TriMesh::NODEP n = mesh.m_Node.begin(); n != mesh.m_Node.end(); ++n)
	{
		double L = (r - n->r).SqrLength();
		if (L < Lmin)
		{
			Lmin = L;
			closestPoint = n->r;
			closestNode = n;
		}	
	}

	// If the point coincided with a node, no need to proceed
	if (Lmin < 1e-16) return closestNode;

	// try to project the node onto a face
	TriMesh::FACEP closestFace = mesh.m_Face.end();
	TriMesh::EDGEP closestEdge = mesh.m_Edge.end();
	for (TriMesh::FACEP f = mesh.m_Face.begin(); f != mesh.m_Face.end(); ++f)
	{
		vec3d q;
		int ncase = projectToFace(*f, r, q, eps);
		double L = (q - r).SqrLength();
		switch (ncase)
		{
		case 0:
		case 1:
		case 2:
			{
				// point coincides with node
				if (L < Lmin)
				{
					closestPoint = q;
					closestNode = f->node[ncase];
					closestEdge = mesh.m_Edge.end();
					closestFace = mesh.m_Face.end();
				}
			}
			break;
		case 3:
		case 4:
		case 5:
			{
				// point falls on edge
				int eid = ncase - 3;
				TriMesh::EDGEP edge = f->edge[eid];

				if (L < Lmin)
				{
					closestFace = f;
					closestNode = mesh.m_Node.end();
					closestEdge = edge;
					closestPoint = q;
					Lmin = L;
				}
			}
			break;
		case 6:
			{
				// general case, where node projects to inside of face
				if (L < Lmin)
				{
					closestFace = f;
					closestEdge = mesh.m_Edge.end();
					closestNode = mesh.m_Node.end();
					closestPoint = q;
					Lmin = L;
				}
			}
			break;
		}
	}

	// if the closest point is a node, just return the node
	if (closestNode != mesh.m_Node.end())
	{
		return closestNode;
	}
	
	// insert the node in the closest face
	if (closestFace != mesh.m_Face.end())
	{
		// create a new node
		TriMesh::NODEP newNode = mesh.addNode(closestPoint);

		// If the node fell on an edge, split the edge
		// else split the face
		if (closestEdge != mesh.m_Edge.end())
		{
			mesh.splitEdge(closestEdge, newNode);
		}
		else mesh.splitFace(closestFace, newNode);

		// return the new node
		return newNode;
	}

	// It didn't project onto a face, so try to project it on an edge.
	// (Note the edge projections above were still projections on the plane of a face. Here
	//  the edge projections are closest point projections on the edge directly.)
	TriMesh::EDGEP minEdge = mesh.m_Edge.end();
	double minDist = 1e99;
	vec3d qmin, q;
	for (TriMesh::EDGEP edge = mesh.m_Edge.begin(); edge != mesh.m_Edge.end(); ++edge)
	{
		double l = projectToEdge(*edge, r, q);
		if ((l>0.0) && (l<1.0))
		{
			double D = (q - r).SqrLength();
			if (D < minDist)
			{
				qmin = q;
				minEdge = edge;
				minDist = D;
			}
		}
	}

	// If we found it, insert it.
	if (minEdge != mesh.m_Edge.end())
	{
		// create a new node					
		TriMesh::NODEP newNode = mesh.addNode(qmin);

		mesh.splitEdge(minEdge, newNode);

		return newNode;
	}

	// if we get here, let's just add the node
	assert(false);
	return closestNode;
}

void insertEdge(TriMesh& mesh, TriMesh::NODEP n0, TriMesh::NODEP n1, std::vector<TriMesh::NODEP>& nodeList, int tag, const double eps)
{
	std::stack<NodePair> S;
	S.push(NodePair(n0, n1));

	const int MAXITER = 50;
	int niter = 0;
	while (S.empty() == false)
	{
		NodePair nodePair = S.top(); S.pop();

		TriMesh::NODEP n0 = nodePair.first;
		TriMesh::NODEP n1 = nodePair.second;

		// loop over all faces that contain one of these nodes
		TriMesh::NODEP newNode = mesh.m_Node.end();
		TriMesh::EDGEP closestEdge = mesh.m_Edge.end();
		vec3d closestPoint;
		double Dmin = 1e99;
		bool bfound = false;
		for (TriMesh::FACEP f = mesh.m_Face.begin(); f != mesh.m_Face.end(); ++f)
		{
			// first see if this edge already exists
			TriMesh::EDGEP e = mesh.m_Edge.end();
			for (int i = 0; i<3; ++i)
			{
				TriMesh::EDGEP ei = f->edge[i];
				if (((ei->node[0] == n0) && (ei->node[1] == n1)) ||
					((ei->node[0] == n1) && (ei->node[1] == n0)))
				{
					ei->ntag = tag; e = ei;
					closestEdge = mesh.m_Edge.end();
					newNode = mesh.m_Node.end();
					bfound = true;
					break;
				}
			}

			// if the edge does not exist we need to dig a little deeper
			if (e == mesh.m_Edge.end())
			{
				// if node 0 or 1 belongs to the face, we find the opposing edge.
				if ((f->node[0] == n0) || (f->node[1] == n0) || (f->node[2] == n0))
				{
					// find the edge opposing the node 0
					e = mesh.opposingEdge(f, n0);
				}
				else if ((f->node[0] == n1) || (f->node[1] == n1) || (f->node[2] == n1))
				{
					// find the edge opposing the node 1
					e = mesh.opposingEdge(f, n1);
				}

				// if we found the edge, figure out where to split it
				if (e != mesh.m_Edge.end())
				{
					// get the edge "normal" (i.e. average normal of the adjoining faces)
					vec3d N(0, 0, 0);
					if (e->nedge[0] != -1) N += e->face[0]->normal;
					if (e->nedge[1] != -1) N += e->face[1]->normal;
					N.Normalize();

					// see if the edges intersect
					vec3d q;
					double L;
					int ncase = edgeIntersect(e->node[0], e->node[1], n0, n1, N, q, L, eps);
					switch (ncase)
					{
					case 1: // intersection at node 0
						newNode = e->node[0];
						e = mesh.m_Edge.end();
						bfound = true;
						break;
					case 2: // intersection at node 1
						newNode = e->node[1];
						e = mesh.m_Edge.end();
						bfound = true;
						break;
					case 3: // proper intersection
						{
							if (L < Dmin)
							{
								Dmin = L;
								closestPoint = q;
								closestEdge = e;
								bfound = true;
							}
						}
						break;
					case 4: // edges coincide at node
						// this should never happen
						assert(false);
						break;
					case 5: // same edge
						// edge already exists, so return
						e->ntag = tag;
						e = closestEdge = mesh.m_Edge.end();
						break;
					};

					if (e == mesh.m_Edge.end()) break;
				}
			}
			else break; // the edge already exists so we're done
		}

		// it is possible that no edge could be found
		// In that case, as a fallback, we proceed along an existing edge that makes the 
		// smallest angle with the current search edge
		// TODO: Got to make this faster
		if (bfound == false)
		{
			TriMesh::EDGEP bestEdge = mesh.m_Edge.end();
			double cosMax = -1.0;
			for (TriMesh::FACEP f = mesh.m_Face.begin(); f != mesh.m_Face.end(); ++f)
			{
				for (int i = 0; i<3; ++i)
				{
					TriMesh::EDGEP ei = f->edge[i];
					if (ei->ntag == 0)
					{
						if (ei->node[0] == n0)
						{
							vec3d e1 = n1->r - n0->r; e1.Normalize();
							vec3d e2 = ei->node[1]->r - ei->node[0]->r; e2.Normalize();
							double cosi = e1*e2;
							if (cosi > cosMax)
							{
								cosMax = cosi;
								newNode = ei->node[1];
								bfound = true;
							}
						}
						else if (ei->node[1] == n0)
						{
							vec3d e1 = n1->r - n0->r; e1.Normalize();
							vec3d e2 = ei->node[0]->r - ei->node[1]->r; e2.Normalize();
							double cosi = e1*e2;
							if (cosi > cosMax)
							{
								cosMax = cosi;
								newNode = ei->node[0];
								bfound = true;
							}
						}
					}
				}
			}

			assert(bfound);
		}

		if (newNode != mesh.m_Node.end())
		{
			// insert the two sub-edges
			S.push(NodePair(n0, newNode));
			S.push(NodePair(newNode, n1));
		}
		else if (closestEdge != mesh.m_Edge.end())
		{
			TriMesh::NODEP newNode = mesh.addNode(closestPoint);
			nodeList.push_back(newNode);
			mesh.splitEdge(closestEdge, newNode);

			// insert the two sub-edges
			S.push(NodePair(n0, newNode));
			S.push(NodePair(newNode, n1));
		}

		niter++;
	}
}

//-----------------------------------------------------------------------------
// See if two edges intersect in plane (n0, N)
// returns:
// 0 = no intersection
// 1 = intersects at node n0
// 2 = intersects at node n1
// 3 = proper intersection
// 4 = edges coincide at points
// 5 = edges are identical
int edgeIntersect(TriMesh::NODEP n0, TriMesh::NODEP n1, TriMesh::NODEP n2, TriMesh::NODEP n3, vec3d N, vec3d& q, double& L, const double eps)
{
	// see if the edges are identical
	if ((n0 == n2) && (n1 == n3)) return 5;
	if ((n0 == n3) && (n1 == n2)) return 5;

	// see if any nodes coincides
	if ((n0 == n2) || (n0 == n3)) return 4;
	if ((n1 == n2) || (n1 == n3)) return 4;

	// get the coordinates
	vec3d r0 = n0->r;
	vec3d r1 = n1->r;
	vec3d r2 = n2->r;
	vec3d r3 = n3->r;

	double tol = eps;

	//		vec3d T = N^(r1 - r0);
	//		N = (r1 - r0) ^ T;

	// project all points onto the plane defined by (c0;N)
	vec3d a = r1 - N*((r1 - r0)*N);
	vec3d b = r2 - N*((r2 - r0)*N);
	vec3d c = r3 - N*((r3 - r0)*N);

	vec3d t = a - r0;
	vec3d r = c - b;
	vec3d p = b - r0;

	// Rotate the vectors into this plane
	quatd Q(N, vec3d(0,0,1)), Qi = Q.Inverse();
	Q.RotateVector(t);
	Q.RotateVector(r);
	Q.RotateVector(p);

	double A[2][2], B[2];
	A[0][0] = t.x; A[0][1] = -r.x; B[0] = p.x;
	A[1][0] = t.y; A[1][1] = -r.y; B[1] = p.y;

	// calculate inverse
	double D = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	if (D != 0.0)
	{
		double Ai[2][2];
		Ai[0][0] = A[1][1] / D; Ai[0][1] = -A[0][1] / D;
		Ai[1][0] = -A[1][0] / D; Ai[1][1] = A[0][0] / D;

		double lm = Ai[0][0] * B[0] + Ai[0][1] * B[1];
		double mu = Ai[1][0] * B[0] + Ai[1][1] * B[1];

		if ((mu >= 0) && (mu <= 1))
		{
			if ((lm >= 0) && (lm <= tol))
			{
				q = n0->r;
				return 1;
			}

			if ((lm <= 1.0) && (lm >= 1.0 - tol))
			{
				q = n1->r;
				return 2;
			}

			if ((lm > tol) && (lm < 1 - tol))
			{
				// it's a proper intersection
				q = r0 + (r1 - r0)*lm;

				// calculate the 
				vec3d s = n2->r + (n3->r - n2->r)*mu;
				L = (s - q).Length();

				return 3;
			}
		}
	}

	return 0;
}

double triSize(TriMesh::FACEP f)
{
	vec3d& r0 = f->node[0]->r;
	vec3d& r1 = f->node[1]->r;
	vec3d& r2 = f->node[2]->r;

	double L1 = (r1 - r0).SqrLength();
	double L2 = (r2 - r1).SqrLength();
	double L3 = (r0 - r2).SqrLength();

	return sqrt((L1+L2+L3)/3.0);
}

// calculate (squared) distance from edge [a,b] to point c
double edgeToPointDistanceSqr(const vec3d& a, const vec3d& b, const vec3d& c)
{
	// project c on edge
	double L2 = (b - a).SqrLength();
	if (L2 == 0.0) return (a - c).SqrLength();
	double w = (b-a)*(c-a)/L2;
	if (w < 0.0) return (c - a).SqrLength();
	if (w > 1.0) return (c - b).SqrLength();
	
	vec3d q = a + (b - a)*w;
	return (c - q).SqrLength();
}

// see if a face is inside a sphere with center c and radius R
bool insideSphere(TriMesh& mesh, TriMesh::FACEP f, const vec3d& c, double R, int ngid = -1)
{
	vec3d& r0 = f->node[0]->r;
	vec3d& r1 = f->node[1]->r;
	vec3d& r2 = f->node[2]->r;

	// check the nodes first 
	double R2 = R*R;
	double D[3];
	D[0] = (r0 - c).SqrLength();
	D[1] = (r1 - c).SqrLength();
	D[2] = (r2 - c).SqrLength();
	int nmin = 0;
	if ((D[1] < D[0]) && (D[1] < D[2])) nmin = 1;
	else if ((D[2] < D[0]) && (D[2] < D[1])) nmin = 2;

	if (D[nmin] < R2) return ((ngid == -1) || (f->node[nmin]->gid != ngid));

	// next, see if any of the edges intersect the sphere
	double L[3] = {0};
	L[0] = edgeToPointDistanceSqr(r0, r1, c);
	L[1] = edgeToPointDistanceSqr(r1, r2, c);
	L[2] = edgeToPointDistanceSqr(r2, r0, c);

	int emin = 0;
	if ((L[1] < L[0]) && (L[1] < L[2])) emin = 1;
	else if ((L[2] < L[0]) && (L[2] < L[1])) emin = 2;

	if (L[emin] < R2) 
	{
		if (f->edge[emin]->gid != -1) return false;

		TriMesh::NODEP n0 = f->edge[emin]->node[0];
		TriMesh::NODEP n1 = f->edge[emin]->node[1];
		return ((ngid==-1) || ((n0->gid!= ngid)&&(n1->gid != ngid)));
	}

	// TODO: I should check if the face itself intersects the sphere, but I don't think
	// I need that predicate so I'm just going to return here.

	return false;
}

bool checkMesh(TriMesh& mesh)
{
	TriMesh::EdgeIterator edge(mesh);
	while (edge.isValid())
	{
		if ((edge->nedge[0] == -1)&&(edge->nedge[1] == -1)) return false;
		++edge;
	}
	return true;
}

TriMesh::NODEP insertDelaunyPoint(TriMesh& mesh, const vec3d& r, bool remesh, const double eps)
{
	// find the closest node first
	// if the projection on the faces or edges fail, the closest node will be returned.
	double Lmin = 1e99;
	vec3d closestPoint;
	TriMesh::NODEP closestNode = mesh.m_Node.end();
	for (TriMesh::NODEP n = mesh.m_Node.begin(); n != mesh.m_Node.end(); ++n)
	{
		if (n->eval > 0)
		{
			double L = (r - n->r).SqrLength();
			if (L < Lmin)
			{
				Lmin = L;
				closestPoint = n->r;
				closestNode = n;
			}
		}
	}

	// If the point coincided with a node, no need to proceed
	if (Lmin < 1e-16)
	{
		return closestNode;
	}
	Lmin = 1e99;

	// try to project the node onto a face
	TriMesh::FACEP closestFace = mesh.m_Face.end();
	vec3d q;
	for (TriMesh::FACEP f = mesh.m_Face.begin(); f != mesh.m_Face.end(); ++f)
	{
		vec3d p;
		int ncase = projectToFace(*f, r, p, eps);
		if (ncase != -1)
		{
			double L = (p - r).SqrLength();
			if (L < Lmin)
			{
				closestFace = f;
				q = p;
				Lmin = L;
			}
		}
	}

//	if (closestFace == mesh.m_Face.end())
	{
		// It didn't project onto a face, so try to project it on an edge.
		// (Note the edge projections above were still projections on the plane of a face. Here
		//  the edge projections are closest point projections on the edge directly.)
		TriMesh::EDGEP minEdge = mesh.m_Edge.end();
		vec3d p;
		for (TriMesh::EDGEP edge = mesh.m_Edge.begin(); edge != mesh.m_Edge.end(); ++edge)
		{
			double l = projectToEdge(*edge, r, p);
			if ((l>0.0) && (l<1.0))
			{
				double L = (p - r).SqrLength();
				if (L < Lmin)
				{
					q = p;
					minEdge = edge;
					Lmin = L;

					if (edge->nedge[0] != -1) closestFace = edge->face[0];
					else 
					{
						assert(edge->nedge[1] != -1);
						closestFace = edge->face[1];
					}
				}
			}
		}
	}

	// if we found the face, we proceed
	if (closestFace != mesh.m_Face.end())
	{
		// calculate size metric for this face (which is the root-mean-sqr edge length)
		double L = 0.5*triSize(closestFace);
//		if (R != 0.0) L = (R < L ? R : L);

		// tag all the faces within distance L for removal
		mesh.tagAllFaces(0);
		std::vector<TriMesh::FACEP> removalList;
		closestFace->ntag = 2;
		removalList.push_back(closestFace);

		// create a stack
		std::stack<TriMesh::FACEP> stack;

		// add all unmarked neighbors to stack
		for (int i = 0; i<3; ++i)
		{
			TriMesh::FACEP fi = mesh.faceNeighbor(closestFace, i);
			if ((fi != mesh.m_Face.end()) && (fi->ntag == 0))
			{
				fi->ntag = 1;
				stack.push(fi);
			}
		}

		while (stack.empty() == false)
		{
			TriMesh::FACEP f = stack.top(); stack.pop();
			if (insideSphere(mesh, f, q, L, 0))
			{
				// add it to the list of faces to remove
				removalList.push_back(f);
				f->ntag = 2;

				// add all unmarked neighbors to stack
				for (int i=0; i<3; ++i)
				{
					TriMesh::FACEP fi = mesh.faceNeighbor(f, i);
					if ((fi != mesh.m_Face.end()) && (fi->ntag == 0)) 
					{
						fi->ntag = 1;
						stack.push(fi);
					}
				}
			}
		}

		// if we only end up with one facet, we'll add all the neighbors unless it crosses a tagged edge
		if (removalList.size() == 1)
		{
			TriMesh::FACEP f0 = removalList[0];
			for (int j=0; j<3; ++j)
			{
				TriMesh::EDGEP ej = f0->edge[j];
				if (ej->gid == -1)
				{
					int n0 = ej->node[0]->eval;
					int n1 = ej->node[1]->eval;

					if ((n0>3)&&(n1>3))
					{
						TriMesh::FACEP fj = mesh.faceNeighbor(f0, j);
						fj->ntag = 2;
						removalList.push_back(fj);
					}
				}
			}
		}

		// extract the outside boundary of the removal list
		std::vector<TriMesh::EDGEP> border;
		for (int i=0; i<(int)removalList.size(); ++i)
		{
			TriMesh::FACEP fi = removalList[i];
			for (int j=0; j<3; ++j)
			{
				TriMesh::FACEP fj = mesh.faceNeighbor(fi, j);
				if ((fj == mesh.m_Face.end()) || (fj->ntag != 2)) border.push_back(fi->edge[j]);
			}
		}

		// create a sorted point list
		TriMesh::EDGEP e0 = border[0];
		// determine winding
		int w = 1;
		if ((e0->face[0] != mesh.m_Face.end()) && (e0->face[0]->ntag == 2))
		{
			w = mesh.edgeWinding(e0->face[0], e0);
		}
		else
		{
			assert(e0->face[1] != mesh.m_Face.end());
			assert(e0->face[1]->ntag == 2);
			w = mesh.edgeWinding(e0->face[1], e0);
		}

		// create the node list
		std::vector<TriMesh::NODEP> nodeList;
		if (w == 1) 
		{
			nodeList.push_back(e0->node[0]);
			nodeList.push_back(e0->node[1]);
		}
		else
		{
			nodeList.push_back(e0->node[1]);
			nodeList.push_back(e0->node[0]);
		}
		TriMesh::NODEP n2 = nodeList[1];

		std::vector<TriMesh::EDGEP> contour;
		contour.push_back(e0);

		for (int i=0; i<border.size(); ++i) border[i]->ntag = 0;
		e0->ntag = 1;
		while (nodeList[0] != n2)
		{
			bool bfound = false;
			for (int i=0; i<border.size(); ++i)
			{
				TriMesh::EDGEP ei = border[i];
				if (ei->ntag != 1)
				{
					if (ei->node[0] == n2)
					{
						ei->ntag = 1;
						n2 = ei->node[1];
						nodeList.push_back(n2);
						contour.push_back(ei);
						bfound = true;
						break;
					}

					if (ei->node[1] == n2)
					{
						ei->ntag = 1;
						n2 = ei->node[0];
						nodeList.push_back(n2);
						contour.push_back(ei);
						bfound = true;
						break;
					}
				}
			}

			assert(bfound);
		}

		// remove the last node from the node list since it is duplicated
		assert(nodeList[0] == nodeList[nodeList.size()-1]);
		nodeList.erase(nodeList.begin() + nodeList.size() - 1);

		// calculate average face normal
		vec3d fN(0,0,0);
		for (int i=0; i<removalList.size(); ++i)
		{
			fN += removalList[i]->normal;
		}
		fN.Normalize();

		// remove faces
		for (int i=0; i<(int)removalList.size(); ++i) mesh.removeFace(removalList[i], true);

		if (remesh == false)
		{
			return mesh.m_Node.end();
		}

		// insert the new point
		TriMesh::NODEP newNode = mesh.addNode(q);

		// first, cut-off the ears of the node list
		if (contour.size() > 3)
		{
			int N = (int)nodeList.size();
			int m = 0;
			double Lmin = 1e99;
			for (int i=0; i<N; ++i)
			{
				double L = (nodeList[i]->r - q).SqrLength();
				if (L < Lmin)
				{
					m = i;
					Lmin = L;
				}
			}

			vec3d rA[3], rB[3], rC[3], rD[3];
			for (int i=0; i<nodeList.size(); ++i)
			{
				vec3d a = nodeList[(i+m)%N]->r;
				vec3d b = nodeList[(i+m+1)%N]->r;
				vec3d c = nodeList[(i+m+2)%N]->r;

				double A = area_triangle(a, b, c);
				if (A > 1e-5)
				{
					vec3d fNi = (a - q) ^ (b - q);

					rA[0] = a; rA[1] = b; rA[2] = c;
					rB[0] = a; rB[1] = c; rB[2] = q;
					rC[0] = a; rC[1] = b; rC[2] = q;
					rD[0] = b; rD[1] = c; rD[2] = q;

					vec3d nA = faceNormal(rA);
					vec3d nB = faceNormal(rB);
					vec3d nC = faceNormal(rC);
					vec3d nD = faceNormal(rD);

					double QA = TriangleQuality(rA);
					double QB = TriangleQuality(rB);
					double QC = TriangleQuality(rC);
					double QD = TriangleQuality(rD);

					double Q1 = (QA < QB ? QA : QB);
					double Q2 = (QC < QD ? QC : QD);

					double L1 = (q - b).SqrLength();
					double L2 = (c - a).SqrLength();

					double cAB = nA*nB;
					double cCD = nC*nD;

					if ((nA*nB > 0.8) && ((nC*nD < 1e-7) || (Q1 > Q2)))
//					if ((cCD < 0.01) || (Q1 > Q2))
					{
						int na = (i + m) % N;
						int nb = (i + m + 1) % N;
						int nc = (i + m + 2) % N;

						// create a new edge
						TriMesh::EDGEP newEdge = mesh.addEdge(nodeList[na], nodeList[nc]);

						TriMesh::FACE newFace;
						newFace.node[0] = nodeList[na];
						newFace.node[1] = nodeList[nb];
						newFace.node[2] = nodeList[nc];

						newFace.edge[0] = contour[na];
						newFace.edge[1] = contour[nb];
						newFace.edge[2] = newEdge;
						mesh.addFace(newFace);

						// delete the middle node from the node list
						nodeList.erase(nodeList.begin() + nb);

						// delete the two edges from the contour
						contour[na] = newEdge;
						contour.erase(contour.begin() + nb);

						i--;
						N--;
						if (N <= 4) break;
					}
				}
			}
		}

		assert(nodeList.size() == contour.size());

		TriStar star;
		star.m_node = newNode;

		// create new edges
		std::vector<TriMesh::EDGEP> newEdges;
		for (int i=0; i<nodeList.size(); ++i)
		{
			TriMesh::EDGEP newEdge = mesh.addEdge(nodeList[i], newNode);
			newEdges.push_back(newEdge);
			star.m_edge.push_back(newEdge);
		}

		// triangulate the hole
		int N = (int)nodeList.size();
		for (int i=0; i<N; ++i)
		{
			TriMesh::FACE face;
			face.node[0] = nodeList[i];
			face.node[1] = nodeList[(i+1)%N];
			face.node[2] = newNode;

			vec3d a = face.node[0]->r;
			vec3d b = face.node[1]->r;
			vec3d c = face.node[2]->r;

			face.edge[0] = contour[i];
			face.edge[1] = newEdges[(i+1)%N];
			face.edge[2] = newEdges[i];

			TriMesh::FACEP fp = mesh.addFace(face);

			star.m_face.push_back(fp);
		}

		return newNode;
	}
	else
	{
		return closestNode;
	}
}

TriMesh::EDGEP insertDelaunyEdge(TriMesh& mesh, TriMesh::NODEP n0, TriMesh::NODEP n1)
{
	TriMesh::EDGEP e = mesh.findEdge(n0, n1);

/*	if (e == mesh.m_Edge.end())
	{
		TriMesh::EdgeIterator it(mesh);
		int n = 0;
		while (it.isValid())
		{
			TriMesh::FACEP f0 = (it->nedge[0] != -1 ? it->face[0] : mesh.m_Face.end());
			TriMesh::FACEP f1 = (it->nedge[1] != -1 ? it->face[1] : mesh.m_Face.end());

			if ((f0 != mesh.m_Face.end()) && (f1 != mesh.m_Face.end()))
			{
				TriMesh::NODEP na = mesh.opposingNode(it, 0);
				TriMesh::NODEP nb = mesh.opposingNode(it, 1);

				if (((na == n0) && (nb == n1)) || ((na == n1) && (nb == n0)))
				{
					mesh.flipEdge(it);
					return it;
				}
			}

			++it;
			++n;
		}
	}
*/
	return e;
}
