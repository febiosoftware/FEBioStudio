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
#include "FEPostModel.h"
#include "FEDistanceMap.h"
#include "FEMeshData_T.h"
#include <stdio.h>
#include "tools.h"
#include "constants.h"
using namespace std;

//-----------------------------------------------------------------------------
Post::FEDistanceMap::FEDistanceMap(Post::FEPostModel* fem, int flags) : Post::ModelDataField(fem, DATA_FLOAT, DATA_NODE, CLASS_FACE, 0)
{ 
	m_tol = 0.01; 
	m_bsigned = false; 
}

//-----------------------------------------------------------------------------
Post::ModelDataField* Post::FEDistanceMap::Clone() const
{
	FEDistanceMap* pd = new FEDistanceMap(m_fem, 0);
	pd->SetName(GetName());
	pd->m_surf1 = m_surf1;
	pd->m_surf2 = m_surf2;
	pd->m_tol = m_tol;
	pd->m_bsigned = m_bsigned;
	return pd;
}

//-----------------------------------------------------------------------------
void Post::FEDistanceMap::InitSurface(int n)
{
	Post::FEPostMesh& mesh = *m_fem->GetFEMesh(0);

	vector<int> L;
	for (int i = 0; i<mesh.Faces(); ++i) if (mesh.Face(i).IsSelected()) L.push_back(i);

	if (n == 0) SetSelection1(L);
	if (n == 1) SetSelection2(L);
}

//-----------------------------------------------------------------------------
int Post::FEDistanceMap::GetSurfaceSize(int i)
{
	return (i == 0 ? m_surf1.Faces() : m_surf2.Faces());
}

//-----------------------------------------------------------------------------
void Post::FEDistanceMap::Surface::BuildNodeList(Post::FEPostMesh& mesh)
{
	// tag all nodes that belong to this surface
	int N = mesh.Nodes();
	for (int i=0; i<N; ++i) mesh.Node(i).m_ntag = -1;
	int nn = 0;
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) 
		{
			FSNode& node = mesh.Node(f.n[j]);
			if (node.m_ntag == -1) node.m_ntag = nn++;
		}
	}

	// create the global node list
	m_node.resize(nn);
	for (int i=0; i<N; ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.m_ntag >= 0) m_node[node.m_ntag] = i;
	}

	// create the local node list
	const int MN = FSFace::MAX_NODES;
	m_lnode.resize(Faces()*MN);
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) m_lnode[MN * i + j] = mesh.Node(f.n[j]).m_ntag;
	}

	// create the node-facet look-up table
	m_NLT.resize(Nodes());
	for (int i=0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j)
		{
			int inode = m_lnode[MN*i+j];
			m_NLT[inode].push_back(m_face[i]);
		}
	}
}

//-----------------------------------------------------------------------------
void Post::FEDistanceMap::BuildNormalList(Post::FEDistanceMap::Surface& s)
{
	// get the mesh
	Post::FEPostMesh& mesh = *m_fem->GetFEMesh(0);

	int NF = s.Faces();
	int NN = s.Nodes();
	s.m_norm.resize(NN);

	const int MN = FSFace::MAX_NODES;
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(s.m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) 
		{
			int n = s.m_lnode[MN*i + j]; assert(n>=0);
			s.m_norm[n] = f.m_nn[j];
		}
	}
}

//-----------------------------------------------------------------------------
Post::FEMeshData* Post::FEDistanceMap::CreateData(Post::FEState* pstate)
{
	return new Post::FEFaceData<float, DATA_NODE>(pstate, this);
}

//-----------------------------------------------------------------------------
void Post::FEDistanceMap::Apply()
{
	// store the model
	Post::FEPostModel& fem = *m_fem;

	// get the mesh
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	const int MN = FSFace::MAX_NODES;

	// build the node lists
	m_surf1.BuildNodeList(mesh);
	m_surf2.BuildNodeList(mesh);
	int N = mesh.Nodes();

	if (m_bsigned)
	{
		BuildNormalList(m_surf1);
		BuildNormalList(m_surf2);
	}

	// get the field index
	int nfield = FIELD_CODE(GetFieldID());

	for (int n = 0; n < fem.GetStates(); ++n)
	{
		FEState* ps = fem.GetState(n);
		Post::FEFaceData<float, DATA_NODE>* df = dynamic_cast<Post::FEFaceData<float, DATA_NODE>*>(&ps->m_Data[nfield]);

		// loop over all nodes of surface 1
		vector<float> a(m_surf1.Nodes());
		for (int i = 0; i < m_surf1.Nodes(); ++i)
		{
			int inode = m_surf1.m_node[i];
			FSNode& node = mesh.Node(inode);
			vec3f r = fem.NodePosition(inode, n);
			vec3f q = project(m_surf2, r, n);
			a[i] = (q - r).Length();
			if (m_bsigned)
			{
				double s = (q - r)*m_surf1.m_norm[i];
				if (s < 0) a[i] = -a[i];
			}
		}
		vector<int> nf1(m_surf1.Faces());
		for (int i = 0; i < m_surf1.Faces(); ++i) nf1[i] = MN; //mesh.Face(m_surf1.m_face[i]).Nodes();
		df->add(a, m_surf1.m_face, m_surf1.m_lnode, nf1);

		// loop over all nodes of surface 2
		vector<float> b(m_surf2.Nodes());
		for (int i = 0; i < m_surf2.Nodes(); ++i)
		{
			int inode = m_surf2.m_node[i];
			FSNode& node = mesh.Node(inode);
			vec3f r = fem.NodePosition(inode, n);
			vec3f q = project(m_surf1, r, n);
			b[i] = (q - r).Length();
			if (m_bsigned)
			{
				double s = (q - r)*m_surf2.m_norm[i];
				if (s < 0) b[i] = -b[i];
			}
		}
		vector<int> nf2(m_surf2.Faces());
		for (int i = 0; i < m_surf2.Faces(); ++i) nf2[i] = MN; //mesh.Face(m_surf2.m_face[i]).Nodes();
		df->add(b, m_surf2.m_face, m_surf2.m_lnode, nf2);
	}
}

//-----------------------------------------------------------------------------
vec3f Post::FEDistanceMap::project(Post::FEDistanceMap::Surface& surf, vec3f& r, int ntime)
{
	Post::FEPostModel& fem = *GetModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	// find the closest surface node
	vec3f q = fem.NodePosition(surf.m_node[0], ntime);
	float Dmin = (q - r)*(q - r);
	int imin = 0;
	for (int i=1; i<surf.Nodes(); ++i)
	{
		vec3f p = fem.NodePosition(surf.m_node[i], ntime);
		float D = (p - r)*(p - r);
		if (D < Dmin)
		{
			q = p;
			Dmin = D;
			imin = i;
		}
	}

	// loop over all facets connected to this node
	vector<int>& FT = surf.m_NLT[imin];
	for (int i=0; i<(int) FT.size(); ++i)
	{
		// get the i-th facet
		FSFace& face = mesh.Face(FT[i]);

		// project r onto the the facet
		vec3f p;
		if (ProjectToFacet(face, r, ntime, p))
		{
			// return the closest projection
			float D = (p - r)*(p - r);
			if (D < Dmin)
			{
				q = p;
				Dmin = D;
			}
		}
	}

	return q;
}

//-----------------------------------------------------------------------------
bool Post::FEDistanceMap::ProjectToFacet(FSFace& f, vec3f& x, int ntime, vec3f& q)
{
	Post::FEPostModel& fem = *GetModel();

	// get the mesh to which this surface belongs
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
	
	// number of element nodes
	int nf = f.Nodes();
	
	// get the elements nodal positions
	const int MN = FSFace::MAX_NODES;
	vec3f y[MN];
	
	// calculate normal projection of x onto element
	switch (f.Type())
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		{
			for (int i = 0; i<3; ++i) y[i] = fem.NodePosition(f.n[i], ntime);
			return ProjectToTriangle(y, x, q, m_tol);
		}
		break;
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		{
			for (int i = 0; i<4; ++i) y[i] = fem.NodePosition(f.n[i], ntime);
			return ProjectToQuad(y, x, q, m_tol);
		}
		break;
	default:
		assert(false);
	}
	return false;
}
