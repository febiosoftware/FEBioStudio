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
#include "FEAdvancingFrontMesher2D.h"
#include <MeshLib/FSMesh.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FSCurveMesh.h>
#include "FEMMGRemesh.h"
#include "FECurveMesher.h"
#include <MeshLib/triangulate.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <GLLib/GLMesh.h>
#include <list>
//using namespace std;

#ifdef HAS_MMG
#endif

struct FRONT_NODE
{
	vec2d	r;
	int		nid;
	int		tag;
};

struct FRONT_EDGE
{
	int		node[2];
};

struct FRONT_FACE
{
	int		node[3];
};

//-----------------------------------------------------------------------------
double distanceToEdge(const vec2d& ra, const vec2d& rb, const vec2d& r)
{
	// project r onto edge
	vec2d e = rb - ra;
	double L = e*e;
	if (L == 0.0) return (r - ra).norm();

	double l = e*(r - ra) / L;
	if (l <= 0.0) return (r - ra).norm();
	if (l >= 1.0) return (r - rb).norm();

	vec2d q = ra + e*l;
	return (r - q).norm();
}

//-----------------------------------------------------------------------------
int findClosestNode(const vec2d& ra, const vec2d& rb, vector<int>& front, vector<FRONT_NODE>& nodeList)
{
	int N = (int)front.size();
	int nmin = -1;
	double Dmin = 0.0;
	for (int i=0; i<N; ++i)
	{
		FRONT_NODE& node = nodeList[front[i]];
//		if (node.tag == 0)
		{
			if (IsLeft(ra, rb, node.r))
			{
				double D = distanceToEdge(ra, rb, node.r);
				if ((D < Dmin) || (nmin == -1))
				{
					nmin = i;
					Dmin = D;
				}
			}
		}
	}

	return nmin;
}

//-----------------------------------------------------------------------------
FEAdvancingFrontMesher2D::FEAdvancingFrontMesher2D()
{
	AddBoolParam(false, "insert new nodes", "insert new nodes");
	AddDoubleParam(0.1, "Element size", "Element size");
}

//-----------------------------------------------------------------------------
bool intersectEdge(int nodea, vec2d& rb, vector<int>& front, vector<FRONT_NODE>& nodeList)
{
	vec2d ra = nodeList[nodea].r;

	int N = (int)front.size();
	for (int i=0; i<N; ++i)
	{
		int n0 = front[i];
		int n1 = front[(i+1)%N];

		if ((n0 != nodea)&&(n1 != nodea))
		{
			vec2d& c = nodeList[n0].r;
			vec2d& d = nodeList[n1].r;

			if (Intersect(ra, rb, c, d)) return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
bool encroach(vec2d& rp, vector<int>& front, vector<FRONT_NODE>& nodeList, double tol)
{
	int N = (int)front.size();
	for (int i = 0; i<N; ++i)
	{
		int n0 = front[i];
		int n1 = front[(i + 1) % N];

		vec2d& c = nodeList[n0].r;
		vec2d& d = nodeList[n1].r;

		if (distanceToEdge(c,d, rp) < tol) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool IsInside(vec2d& p, vector<int>& front, vector<FRONT_NODE>& nodeList)
{
	vec2d r(0.12319, 0.45890); r.unit();
	vec2d q = p + r*100; // todo: make this size dependent on the bounding box
	int count = 0;
	int N = (int)front.size();
	for (int i=0; i<N; ++i)
	{
		vec2d& a = nodeList[front[i]].r;
		vec2d& b = nodeList[front[(i+1)%N]].r;
		if (Intersect(r, p, a, b)) count++;
	}

	return (count%2 == 1);
}

//-----------------------------------------------------------------------------
void BuildFrontMesh(vector<int> front, vector<FRONT_NODE>& nodeList, vector<FRONT_FACE>& faceList, bool insertNewNodes)
{
	// loop until we only have two edges left
	int node = 0;
	int insertedNodes = 0;
	while (front.size() > 2)
	{
		// keep track of the front size
		int NN = (int)front.size();

		int m0 = node;
		int m1 = (node+1)%NN;

		// get the two edge nodes
		int n0 = front[m0];
		int n1 = front[m1];
		vec2d ra = nodeList[n0].r;
		vec2d rb = nodeList[n1].r;

		// tag the edge nodes to make sure they won't be considered as possible candidates
		// (NOTE: This does nothing at the moment)
		nodeList[n0].tag = 1;
		nodeList[n1].tag = 1;

		// propose a new node, positioned such that it creates a "perfect" triangle
		vec2d e = rb - ra;
		vec2d t(-e.y(), e.x()); // rotate 90 degrees counter-clockwise
		vec2d c = (ra + rb)*0.5;
		double w = e.norm();
		double h = sqrt(3.0)*0.5;	// height of equilateral triangle
		vec2d rp = c + t*h;	// opposite corner

		// see if this node can be inserted
		bool insert = true; // assume all is well

		if (insertNewNodes)
		{
			// see if the new edges that would created would intersect any existing edge
			if (IsInside(rp, front, nodeList))
			{
				if (intersectEdge(n0, rp, front, nodeList) ||
					intersectEdge(n1, rp, front, nodeList)) insert = false;
				else
				{
					// see if this new node would encroach on a boundary edge
					if (encroach(rp, front, nodeList, w*0.8)) insert = false;
				}
			}
			else insert = false;
		}
		else insert = false;

		// see if we should insert the node
		if (insert)
		{
			insertedNodes++;

			// add the new node
			FRONT_NODE newNode;
			newNode.r = rp;
			newNode.nid = (int)nodeList.size();
			newNode.tag = 0;
			nodeList.push_back(newNode);

			// add a triangle
			FRONT_FACE f;
			f.node[0] = nodeList[n0].nid;
			f.node[1] = nodeList[n1].nid;
			f.node[2] = newNode.nid;
			faceList.push_back(f);

			// add this node to the front
			front.insert(front.begin() + m1, newNode.nid);
			node++;
			if (node >= front.size()) node = 0;
		}
		else
		{
			// find the node that is left and closest to this edge
			int closestNode = findClosestNode(ra, rb, front, nodeList);
			if (closestNode == -1) return;
			assert(closestNode != -1);
			assert((closestNode != m0)&&(closestNode != m1));
			if (closestNode == -1) return;
			int n2 = front[closestNode];

			// build a face
			FRONT_FACE f;
			f.node[0] = nodeList[n0].nid;
			f.node[1] = nodeList[n1].nid;
			f.node[2] = nodeList[n2].nid;
			faceList.push_back(f);

			if (closestNode == (m1 + 1) % NN)
			{
				front.erase(front.begin() + m1);
				node++;
				if (node >= front.size()) node = 0;
			}
			else if (closestNode == (m0 + (NN - 1)) % NN)
			{
				front.erase(front.begin() + m0);
				node++;
				if (node >= front.size()) node = 0;
			}
			else
			{
				// split the front
				vector<int> leftFront;
				vector<int> rightFront;

				int n = closestNode;
				do
				{
					leftFront.push_back(front[n]);
					n = (n + 1) % NN;
				}
				while (n != m0);
				leftFront.push_back(front[m0]);

				n = m1;
				do
				{
					rightFront.push_back(front[n]);
					n = (n + 1) % NN;
				}
				while (n != closestNode);
				rightFront.push_back(front[closestNode]);

				BuildFrontMesh(leftFront, nodeList, faceList, insertNewNodes);
				BuildFrontMesh(rightFront, nodeList, faceList, insertNewNodes);
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// generate the mesh
FSMesh* FEAdvancingFrontMesher2D::BuildMesh(GObject* po)
{
	// make sure we a valid object
	if (po == nullptr) return nullptr;

//---> HACK:
	// get the one-and-only surface
	FECurveMesher curveMesher;
	curveMesher.SetElementSize(GetFloatValue(1));
	FSCurveMesh* curve = new FSCurveMesh;
	int NS = po->Faces();
	if (NS > 0)
	{
		for (int n=0; n<NS; ++n)
		{
			GFace* surf = po->Face(n);
			int surfEdges = surf->Edges();
			for (int i=0; i<surfEdges; ++i)
			{
				GEdge* e = po->Edge(surf->m_edge[i].nid);
				FSCurveMesh* edgeMesh = curveMesher.BuildMesh(e); assert(edgeMesh);
				curve->Attach(*edgeMesh);
				delete edgeMesh;
			}
		}
	}
	else
	{
		// this is for curve mesh objects
		int NC = po->Edges();
		for (int i=0; i<NC; ++i)
		{
			GEdge* e = po->Edge(i);
			FSCurveMesh* edgeMesh = curveMesher.BuildMesh(e); assert(edgeMesh);
			curve->Attach(*edgeMesh);
			delete edgeMesh;
		}
	}
//---->

	// make sure we have a mesh
	if (curve == 0) return 0;

	// make sure this curve has a minimum of three nodes
	if (curve->Nodes() < 3) return 0;

	// make sure this curve is sorted
	curve->Sort();

	// make sure it's closed
	if (curve->Type() != FSCurveMesh::CLOSED_CURVE) return 0;

	// now we can get started
	// copy the nodes 
	int NN = curve->Nodes();
	vector<FRONT_NODE> nodeList(NN);
	for (int i=0; i<NN; ++i)
	{
		vec3d r = curve->Node(i).pos();
		nodeList[i].r = vec2d(r.x, r.y);
		nodeList[i].nid = i;
		nodeList[i].tag = 0;
	}

	// create the front
	// and add all nodes to the front
	vector<int> front(NN);
	for (int i=0; i<NN; ++i) front[i] = i;

	// this is where we'll store the nodes
	vector<FRONT_FACE> face;
	BuildFrontMesh(front, nodeList, face, GetBoolValue(0));

	// Build the mesh
	NN = (int)nodeList.size();
	int NE = (int)face.size();
	FSMesh* mesh = new FSMesh;
	mesh->Create(NN, NE);
	for (int i = 0; i<NN; ++i)
	{
		mesh->Node(i).r = nodeList[i].r;
	}

	for (int i = 0; i<NE; ++i)
	{
		FRONT_FACE& f = face[i];
		FSElement& el = mesh->Element(i);
		el.SetType(FE_TRI3);
		el.m_node[0] = f.node[0];
		el.m_node[1] = f.node[1];
		el.m_node[2] = f.node[2];
	}

	mesh->RebuildMesh();

	// clean up
	delete curve;

	return mesh;
}

//================================================================================
FSSurfaceMesh* GLMeshToSurfaceMesh(GLMesh& m)
{
	int NN = m.Nodes();
	int NF = m.Faces();
	int NE = m.Edges();

	FSSurfaceMesh* sm = new FSSurfaceMesh();
	sm->Create(NN, NE, NF);
	for (int i = 0; i < NN; ++i)
	{
		FSNode& node = sm->Node(i);
		GLMesh::NODE& gnode = m.Node(i);
		node.r = to_vec3d(gnode.r);
		node.m_gid = gnode.pid;
	}

	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = sm->Edge(i);
		GLMesh::EDGE& gedge = m.Edge(i);
		edge.n[0] = gedge.n[0];
		edge.n[1] = gedge.n[1];
		edge.m_gid = gedge.pid;
	}

	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = sm->Face(i);
		GLMesh::FACE& gface = m.Face(i);
		face.SetType(FE_FACE_TRI3);
		face.n[0] = gface.n[0];
		face.n[1] = gface.n[1];
		face.n[2] = gface.n[2];
		face.m_gid = gface.pid;
	}

	sm->Update();

	return sm;
}

//================================================================================
FEMMG2DMesher::FEMMG2DMesher()
{
	AddDoubleParam(0.1, "Element size", "Element size");
}

FSMesh* FEMMG2DMesher::BuildMesh(GObject* po)
{
	if (po == nullptr) return nullptr;

	// MMG needs a base mesh, so let's create one by doing a rough triangulation of the shape.
	assert(po->Faces() == 1);
	GFace& face = *po->Face(0);
	GLMesh* gm = triangulate(face);

	// MMG needs a FSSurfaceMesh, so convert
	FSSurfaceMesh* pm = GLMeshToSurfaceMesh(*gm);

	// Now, let's use MMG to remesh
	double h = GetFloatValue(0);
	MMG2DRemesh mmg;
	mmg.SetFloatValue(0, h);

	FSSurfaceMesh* newMesh = mmg.Apply(pm);

	delete pm;

	return new FSMesh(*newMesh);
}
