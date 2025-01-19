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

// FSMeshBase.cpp: implementation of the FSMeshBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEPostMesh.h"
#include "FEState.h"
#include <MeshLib/MeshTools.h>
#include <MeshLib/FEFaceEdgeList.h>
#include <stack>
using namespace std;

//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
Post::FEPostMesh::FEPostMesh()
{
}

//-----------------------------------------------------------------------------
// Destructor
Post::FEPostMesh::~FEPostMesh()
{
	CleanUp();
}

//-----------------------------------------------------------------------------
// clean all depdendant structures
void Post::FEPostMesh::CleanUp()
{
	m_NEL.Clear();
	m_NFL.Clear();

	ClearDomains();
	ClearElemSets();
	ClearSurfaces();
	ClearNodeSets();
}

//-----------------------------------------------------------------------------
// clear everything
void Post::FEPostMesh::ClearAll()
{
	CleanUp();
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	m_Elem.clear();
}

//-----------------------------------------------------------------------------
// Clear all the domains
void Post::FEPostMesh::ClearDomains()
{
	for (int i=0; i<(int) m_Dom.size(); ++i) delete m_Dom[i];
	m_Dom.clear();
}

//-----------------------------------------------------------------------------
// Clear all the parts
void Post::FEPostMesh::ClearElemSets()
{
	m_pFEElemSet.Clear();
}

//-----------------------------------------------------------------------------
// Clear all the surfaces
void Post::FEPostMesh::ClearSurfaces()
{
	m_pFESurface.Clear();
}

//-----------------------------------------------------------------------------
// Clear all the node sets
void Post::FEPostMesh::ClearNodeSets()
{
	m_pFENodeSet.Clear();
}

//-----------------------------------------------------------------------------
void Post::FEPostMesh::Create(int nodes, int elems, int faces, int edges)
{
	if (nodes)
	{
		CleanUp();

		m_Node.resize(nodes);

		// assign default IDs
		for (int i=0; i<nodes; ++i) m_Node[i].SetID(i+1);
	}

	if (elems)
	{
		m_Elem.resize(elems);

		// set default element ID's
		for (int i=0; i<elems; i++) 
		{
			FEElement_& el = ElementRef(i);
			el.SetID(i+1); 
			el.m_lid = i;	
		}

		// allocate storage for element data
		m_data.Clear();
	}

	if (faces) m_Face.resize(faces);
	if (edges) m_Edge.resize(edges);
}

//-----------------------------------------------------------------------------
// Build the parts
void Post::FEPostMesh::UpdateDomains()
{
	ClearDomains();

	// figure out how many domains there are
	int ndom = 0;
	int NE = Elements();
	for (int i=0; i<NE; ++i) if (ElementRef(i).m_MatID > ndom) ndom = ElementRef(i).m_MatID;
	++ndom;

	// figure out the domain sizes
	vector<int> elemSize(ndom, 0);
	vector<int> faceSize(ndom, 0);

	for (int i = 0; i<NE; ++i)
	{
		FEElement_& el = ElementRef(i);
		elemSize[el.m_MatID]++;
	}

	int NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = Face(i);

		int ma = ElementRef(face.m_elem[0].eid).m_MatID;
		int mb = (face.m_elem[1].eid >= 0 ? ElementRef(face.m_elem[1].eid).m_MatID : -1);

		faceSize[ma]++;
		if (mb >= 0) faceSize[mb]++;
	}

	m_Dom.resize(ndom);
	for (int i=0; i<ndom; ++i) 
	{
		m_Dom[i] = new MeshDomain(this);
		m_Dom[i]->SetMatID(i);
		m_Dom[i]->Reserve(elemSize[i], faceSize[i]);
	}

	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = ElementRef(i);
		m_Dom[el.m_MatID]->AddElement(i);
	}

	for (int i=0; i<NF; ++i)
	{
		FSFace& face = Face(i);

		int ma = ElementRef(face.m_elem[0].eid).m_MatID;
		int mb = (face.m_elem[1].eid >= 0 ? ElementRef(face.m_elem[1].eid).m_MatID : -1);

		m_Dom[ma]->AddFace(i);
		if (mb >= 0) m_Dom[mb]->AddFace(i);
	}
}

//-----------------------------------------------------------------------------
// Update the FE data
void Post::FEPostMesh::BuildMesh()
{
	// build mesh data
	FSMesh::RebuildMesh(60.0);

	// Build the node-element list
	if (m_NEL.IsEmpty()) m_NEL.Build(this);

	// build the node-face list
	m_NFL.Build(this);

	// create the parts
	UpdateDomains();
}

//-----------------------------------------------------------------------------
bool Post::FindElementInReferenceFrame(FSCoreMesh& m, const vec3f& p, int& nelem, double r[3])
{
	vec3f y[FSElement::MAX_NODES];
	int NE = m.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& e = m.ElementRef(i);
		int ne = e.Nodes();
		nelem = i;

		// do a quick bounding box test
		vec3f r0 = to_vec3f(m.Node(e.m_node[0]).r);
		vec3f r1 = r0;
		for (int j = 1; j<ne; ++j)
		{
			vec3f rj = to_vec3f(m.Node(e.m_node[j]).r);
			if (rj.x < r0.x) r0.x = rj.x;
			if (rj.y < r0.y) r0.y = rj.y;
			if (rj.z < r0.z) r0.z = rj.z;
			if (rj.x > r1.x) r1.x = rj.x;
			if (rj.y > r1.y) r1.y = rj.y;
			if (rj.z > r1.z) r1.z = rj.z;
		}

		float dx = fabs(r0.x - r1.x);
		float dy = fabs(r0.y - r1.y);
		float dz = fabs(r0.z - r1.z);

		float R = dx;
		if (dy > R) R = dy;
		if (dz > R) R = dz;
		float eps = R*0.001f;

		r0.x -= eps;
		r0.y -= eps;
		r0.z -= eps;

		r1.x += eps;
		r1.y += eps;
		r1.z += eps;

		if ((p.x >= r0.x) && (p.x <= r1.x) &&
			(p.y >= r0.y) && (p.y <= r1.y) &&
			(p.z >= r0.z) && (p.z <= r1.z))
		{
			if (ProjectInsideReferenceElement(m, e, p, r)) return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
double Post::IntegrateNodes(Post::FEPostMesh& mesh, const std::vector<int>& nodeList, Post::FEState* ps)
{
	double res = 0.0;
	int N = (int) nodeList.size();
	for (int i = 0; i<N; ++i)
	{
		FSNode& node = mesh.Node( nodeList[i] );
		if (ps->m_NODE[i].m_ntag > 0)
		{
			res += ps->m_NODE[i].m_val;
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
double Post::IntegrateEdges(Post::FEPostMesh& mesh, const std::vector<int>& edgeList, Post::FEState* ps)
{
	assert(false);
	return 0.0;
}

// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double Post::IntegrateFaces(Post::FEPostMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps)
{
	double res = 0.0;
	float v[FSFace::MAX_NODES];
	vec3f r[FSFace::MAX_NODES];
	int NF = (int)faceList.size();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face( faceList[i] );
		if (f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal values
			for (int j = 0; j<nn; ++j) v[j] = ps->m_NODE[f.n[j]].m_val;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				v[3] = v[2];
				break;
			}

			// get the nodal coordinates
			for (int j = 0; j<nn; ++j) r[j] = ps->m_NODE[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// add to integral
			res += IntegrateQuad(r, v);
		}
	}
	return res;
}

// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double Post::IntegrateReferenceFaces(Post::FEPostMesh& mesh, const std::vector<int>& faceList, Post::FEState* ps)
{
	Post::FERefState& ref = *ps->m_ref;
	double res = 0.0;
	float v[FSFace::MAX_NODES];
	vec3f r[FSFace::MAX_NODES];
	int NF = (int)faceList.size();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& f = mesh.Face( faceList[i] );
		if (f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal values
			for (int j = 0; j < nn; ++j) v[j] = ps->m_NODE[f.n[j]].m_val;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				v[3] = v[2];
				break;
			}

			// get the (reference!) nodal coordinates
			for (int j = 0; j < nn; ++j) r[j] = ref.m_Node[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// add to integral
			res += IntegrateQuad(r, v);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
vec3d Post::IntegrateSurfaceNormal(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	vec3d res(0,0,0);
	float vx[FSFace::MAX_NODES], vy[FSFace::MAX_NODES], vz[FSFace::MAX_NODES];
	vec3f r[FSFace::MAX_NODES];
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& f = mesh.Face(i);
		vec3d N = to_vec3d(f.m_fn);

		if (f.IsSelected() && f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal coordinates
			for (int j = 0; j < nn; ++j) r[j] = ps->m_NODE[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// get the nodal values
			for (int j = 0; j < nn; ++j)
			{
				double v = ps->m_NODE[f.n[j]].m_val;
				vx[j] = N.x*v;
				vy[j] = N.y*v;
				vz[j] = N.z*v;
			}

			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				vx[3] = vx[2];
				vy[3] = vy[2];
				vz[3] = vz[2];
				break;
			}

			// add to integral
			res.x += IntegrateQuad(r, vx);
			res.y += IntegrateQuad(r, vy);
			res.z += IntegrateQuad(r, vz);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double Post::IntegrateReferenceElems(Post::FEPostMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps)
{
	Post::FERefState& ref = *ps->m_ref;
	double res = 0.0;
	float v[FSElement::MAX_NODES];
	vec3f r[FSElement::MAX_NODES];
	int NE = (int)elemList.size();
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& e = mesh.ElementRef( elemList[i] );
		if (e.IsSolid() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and (reference!) coordinates
			for (int j = 0; j<nn; ++j) v[j] = ps->m_NODE[e.m_node[j]].m_val;
			for (int j = 0; j<nn; ++j) r[j] = ref.m_Node[e.m_node[j]].m_rt;
			switch (e.Type())
			{
			case FE_PENTA6:
				v[7] = v[5]; r[7] = r[5];
				v[6] = v[5]; r[6] = r[5];
				v[5] = v[4]; r[5] = r[4];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
				v[7] = v[3]; r[7] = r[3];
				v[6] = v[3]; r[6] = r[3];
				v[5] = v[3]; r[5] = r[3];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			}

			// add to integral
			res += IntegrateHex(r, v);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double Post::IntegrateElems(Post::FEPostMesh& mesh, const std::vector<int>& elemList, Post::FEState* ps)
{
	double res = 0.0;
	float v[FSElement::MAX_NODES] = { 0.f };
	vec3f r[FSElement::MAX_NODES];
	int NE = (int)elemList.size();
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& e = mesh.ElementRef( elemList[i] );
		if (e.IsSolid() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and coordinates
			for (int j = 0; j < nn; ++j) v[j] = ps->m_ElemData.value(i, j);
//			for (int j = 0; j < nn; ++j) v[j] = ps->m_NODE[e.m_node[j]].m_val;

			for (int j = 0; j < nn; ++j) r[j] = ps->m_NODE[e.m_node[j]].m_rt;
			switch (e.Type())
			{
			case FE_PENTA6:
				v[7] = v[5]; r[7] = r[5];
				v[6] = v[5]; r[6] = r[5];
				v[5] = v[4]; r[5] = r[4];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
				v[7] = v[3]; r[7] = r[3];
				v[6] = v[3]; r[6] = r[3];
				v[5] = v[3]; r[5] = r[3];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			}

			// add to integral
			res += IntegrateHex(r, v);
		}
		else if (e.IsShell() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and coordinates
			for (int j = 0; j < nn; ++j) v[j] = ps->m_ElemData.value(i, j);
			for (int j = 0; j < nn; ++j) r[j] = ps->m_NODE[e.m_node[j]].m_rt;

			switch (e.Type())
			{
			case FE_TRI3:
				v[3] = v[2]; r[3] = r[2];
				break;
			}

			res += IntegrateQuad(r, v);
		}

		// TODO: This was done so that discrete element variables can be added, but I don't think that makes sense
		//       for other element types that are considered "beams", e.g. discrete elements. 
		//       I think the solution is to distinguish between "beams" and "discrete" elements. 
		if (e.IsBeam() && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			double v0 = ps->m_ElemData.value(i, 0);
			double v1 = ps->m_ElemData.value(i, 1);
			res += 0.5 * (v0 + v1);
		}
	}
	return res;
}
