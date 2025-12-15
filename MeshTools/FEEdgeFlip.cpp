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
#include "FEEdgeFlip.h"
#include <MeshLib/MeshTools.h>
#include <MeshLib/FSSurfaceMesh.h>
using namespace std;

FEEdgeFlip::FEEdgeFlip() : FESurfaceModifier("Flip Edges")
{
	m_EL = 0;
	m_FEL = 0;
	m_EFL = 0;
}

FEEdgeFlip::~FEEdgeFlip()
{
	Cleanup();
}

void FEEdgeFlip::Cleanup()
{
	if (m_EL ) { delete m_EL ; m_EL  = 0; }
	if (m_FEL) { delete m_FEL; m_FEL = 0; }
	if (m_EFL) { delete m_EFL; m_EFL = 0; }
}

FSSurfaceMesh* FEEdgeFlip::Apply(FSSurfaceMesh* pm)
{	
	// make sure this is a tri-mesh
	if (pm->IsType(FE_FACE_TRI3) == false) return nullptr;

	// copy the mesh
	FSSurfaceMesh* newMesh = new FSSurfaceMesh(*pm);

	// allocate the data structures we'll need
	// create the edge list
	m_EL = new EdgeList(*newMesh);
	int NE = m_EL->size();
	m_tag.assign(NE, 1);

	// create the edge-face list
	m_FEL = new FSFaceEdgeList(*newMesh, *m_EL);

	// create the edge-face list
	m_EFL = new FSEdgeFaceList(*newMesh);

	// see if any edges are selected
	bool forceFlip = false;
	bool selectedOnly = false;
	int selectedEdges = newMesh->CountSelectedEdges();

	// if there is only one selected edge, let's take a shortcut
	if (selectedEdges == 1)
	{
		// find the selected mesh
		int isel = -1;
		for (int i = 0; i < newMesh->Edges(); ++i)
		{
			if (newMesh->Edge(i).IsSelected())
			{
				isel = i;
				break;
			}
		}
		assert(isel != -1);
		if (isel == -1) return nullptr;

		// flip this edge
		FlipEdge(isel, newMesh, true);
	}
	else 
	{
		if (selectedEdges > 0)
		{
			selectedOnly = true;
			forceFlip = true;
		}

		// mark the edges we don't want to flip
		MarkEdges(newMesh, selectedOnly);

		// repeat until we can't find any more edges to flip
		bool bdone = false;
		while (bdone == false)
		{
			// assume we are done
			bdone = true;

			// try all edges
			for (int i = 0; i < NE; ++i)
			{
				// only flip edges that are not marked
				if (m_tag[i] == 1)
				{
					// Try to flip this edge
					if (FlipEdge(i, newMesh, forceFlip))
					{
						// The edge was flipped, so we're not done yet
						bdone = false;
					}
				}
			}
		}
	}

	// update the new mesh data
	newMesh->UpdateFaces();

	// don't forget to clean up
	Cleanup();
	
	// all done
	return newMesh;
}

// Mark the edges the we can and cannot flip.
// This means don't flip feature edges.
void FEEdgeFlip::MarkEdges(FSSurfaceMesh* mesh, bool selectedOnly)
{
	EdgeList& EL = *m_EL;
	FSFaceEdgeList& FEL = *m_FEL;
	FSEdgeFaceList& EFL = *m_EFL;

	// number of (surface) edges
	int NE = EFL.size(); assert(NE == EL.size());

	vector<int> edgeList;

	// mark the edges we don't want to flip
	mesh->TagAllNodes(0);
	for (int i = 0; i<mesh->Edges(); ++i)
	{
		FSEdge& edge = mesh->Edge(i);
		if (edge.m_gid >= 0)
		{
			mesh->Node(edge.n[0]).m_ntag = 1;
			mesh->Node(edge.n[1]).m_ntag = 1;

			edgeList.push_back(i);
		}
		else if (selectedOnly && (edge.IsSelected() == false))
		{
			mesh->Node(edge.n[0]).m_ntag = 1;
			mesh->Node(edge.n[1]).m_ntag = 1;
			edgeList.push_back(i);
		}
	}

	for (int i = 0; i<NE; ++i)
	{
		std::pair<int, int>& edge = EL[i];
		int n0 = edge.first;
		int n1 = edge.second;
		if ((mesh->Node(n0).m_ntag == 1) && (mesh->Node(n1).m_ntag == 1))
		{
			for (int j = 0; j<(int)edgeList.size(); ++j)
			{
				FSEdge& ej = mesh->Edge(edgeList[j]);
				if (((ej.n[0] == n0) && (ej.n[1] == n1)) || ((ej.n[0] == n1) && (ej.n[1] == n0)))
				{
					m_tag[i] = 0;
					break;
				}
			}
		}

		// we won't flip edges that are on the outside
		if (EFL[i].size() == 1)
		{
			m_tag[i] = 0;
		}
	}
}

// Try to flip an edge
// Will return true on success (the edge was flipped), or false on failure
bool FEEdgeFlip::FlipEdge(int iedge, FSSurfaceMesh* mesh, bool forceFlip)
{
	int a[3], b[3];
	vec3d ra[3], rb[3];

	EdgeList& EL = *m_EL;
	FSFaceEdgeList& FEL = *m_FEL;
	FSEdgeFaceList& EFL = *m_EFL;

	// get the edge and its nodes
	std::pair<int, int>& edge = EL[iedge];
	int en0 = edge.first;
	int en1 = edge.second;

	// get the face list of the node
	std::vector<int>& faceList = EFL[iedge];

	// we only consider edges with two adjacent faces
	if (faceList.size() != 2) return false;

	// get the two adjacent faces
	FSFace& f0 = mesh->Face(faceList[0]);
	FSFace& f1 = mesh->Face(faceList[1]);

	// we need to find start node for each face
	int k0 = -1;
	for (int k = 0; k<3; ++k)
	{
		int m0 = f0.n[k];
		int m1 = f0.n[(k + 1) % 3];

		if (((m0 == en0) && (m1 == en1)) || ((m0 == en1) && (m1 == en0)))
		{
			k0 = k;
			break;
		}
	}
	assert(k0 != -1);

	int k1 = -1;
	for (int k = 0; k<3; ++k)
	{
		int m0 = f1.n[k];
		int m1 = f1.n[(k + 1) % 3];

		if (((m0 == en0) && (m1 == en1)) || ((m0 == en1) && (m1 == en0)))
		{
			k1 = k;
			break;
		}
	}
	assert(k1 != -1);

	// get the nodes in the correct order (a0, a1 form the edge; same for b)
	a[0] = f0.n[k0];
	a[1] = f0.n[(k0 + 1) % 3];
	a[2] = f0.n[(k0 + 2) % 3];

	b[0] = f1.n[k1];
	b[1] = f1.n[(k1 + 1) % 3];
	b[2] = f1.n[(k1 + 2) % 3];

	assert(a[0] == b[1]);
	assert(a[1] == b[0]);

	// see if we should flip the edge
	if (forceFlip == false)
	{
		bool flip = ShouldFlip(a, b, mesh);
		if (flip == false) return false;
	}
	else m_tag[iedge] = 0;

	// Do the actual edge flipping
	DoFlipEdge(iedge, a, b, k0, k1, mesh);

	// the edge was flipped, so return true
	return true;
}

// Do the actual edge flipping
void FEEdgeFlip::DoFlipEdge(int iedge, int a[3], int b[3], int k0, int k1, FSSurfaceMesh* mesh)
{
	EdgeList& EL = *m_EL;
	FSFaceEdgeList& FEL = *m_FEL;
	FSEdgeFaceList& EFL = *m_EFL;
	vector<int> tmp0(3), tmp1(3);

	// flip the edge
	std::pair<int, int>& edge = EL[iedge];
	edge.first = a[2];
	edge.second = b[2];

	FSEdge& e = mesh->Edge(iedge);
	e.n[0] = a[2];
	e.n[1] = b[2];

	// get the face list of the node
	std::vector<int>& faceList = EFL[iedge];
	FSFace& f0 = mesh->Face(faceList[0]);
	FSFace& f1 = mesh->Face(faceList[1]);

	// flip the faces
	f0.n[0] = b[2];
	f0.n[1] = a[2];
	f0.n[2] = a[0];

	f1.n[0] = a[2];
	f1.n[1] = b[2];
	f1.n[2] = a[1];

	// find participating edges
	vector<int>& el0 = FEL[faceList[0]]; assert(el0.size() == 3);
	vector<int>& el1 = FEL[faceList[1]]; assert(el1.size() == 3);
	assert(el0[k0] == el1[k1]);

	vector<int>& efl0 = EFL[el0[(k0 + 1) % 3]];
	vector<int> _efl0 = efl0;
	bool bfound = false;
	for (int j = 0; j<efl0.size(); ++j)
	{
		if (efl0[j] == faceList[0])
		{
			efl0[j] = faceList[1];
			bfound = true;
			break;
		}
	}
	assert(bfound);

	vector<int>& efl1 = EFL[el1[(k1 + 1) % 3]];
	vector<int> _efl1 = efl1;
	bfound = false;
	for (int j = 0; j<efl1.size(); ++j)
	{
		if (efl1[j] == faceList[1])
		{
			efl1[j] = faceList[0];
			bfound = true;
			break;
		}
	}
	assert(bfound);

	tmp0 = el0;
	tmp1 = el1;

	el0[0] = iedge;
	el0[1] = tmp0[(k0 + 2) % 3];
	el0[2] = tmp1[(k1 + 1) % 3];

	el1[0] = iedge;
	el1[1] = tmp1[(k1 + 2) % 3];
	el1[2] = tmp0[(k0 + 1) % 3];
}

// See if a qaud [A,B,C,D] is convex. Since the points A,B,C,D won't be necessarily coplanar, this
// is only approximate.
bool isConvex(const vec3d& A, const vec3d& B, const vec3d& C, const vec3d& D)
{
	// calculate edge vectors
	vec3d e1 = (B - A).Normalize();
	vec3d e2 = (C - B).Normalize();
	vec3d e3 = (D - C).Normalize();
	vec3d e4 = (A - D).Normalize();

	// calculate (approximate) normal
	vec3d N = e1 ^ (-e4); N.Normalize();

	// now see if each point lies on the negative side of the plane with normal e_i x N. 
	if ((e1^N)*e2 >= 0.0) return false;
	if ((e2^N)*e3 >= 0.0) return false;
	if ((e3^N)*e4 >= 0.0) return false;
	if ((e4^N)*e1 >= 0.0) return false;

	return true;
}

// helper function to see if an edge should be flipped
bool FEEdgeFlip::ShouldFlip(int a[3], int b[3], FSSurfaceMesh* mesh)
{
	// get the nodal coordinates
	vec3d A = mesh->Node(a[0]).r;
	vec3d B = mesh->Node(b[2]).r;
	vec3d C = mesh->Node(a[1]).r;
	vec3d D = mesh->Node(a[2]).r;

	// we only proceed if the quad formed by A,B,C,D is convex
	if (isConvex(A, B, C, D) == false) return false;

	// get the nodal coordinates
	vec3d ra[3], rb[3];
	ra[0] = A;
	ra[1] = C;
	ra[2] = D;

	rb[0] = C;
	rb[1] = A;
	rb[2] = B;

	double Qa1 = TriangleQuality(ra);
	double Qb1 = TriangleQuality(rb);
	double Qmin1 = (Qa1 < Qb1 ? Qa1 : Qb1);

	// flip the edge
	ra[0] = D;
	ra[1] = B;
	ra[2] = C;

	rb[0] = B;
	rb[1] = D;
	rb[2] = A;

	double Qa2 = TriangleQuality(ra);
	double Qb2 = TriangleQuality(rb);
	double Qmin2 = (Qa2 < Qb2 ? Qa2 : Qb2);

	const double eps = 1e-12;
	return (Qmin2 > Qmin1 + eps);
}
