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

#include "stdafx.h"
#include "FEAreaCoverage.h"
#include "FEPostModel.h"
#include "FEMeshData_T.h"
#include <MeshLib/Intersect.h>
#include "constants.h"
using namespace Post;

//-----------------------------------------------------------------------------
void FEAreaCoverage::Surface::Create(Post::FEPostMesh& mesh)
{
	// this assumes that the m_face member has initialized
	int NF = (int)m_face.size();
	m_fnorm.resize(NF, vec3f(0.f, 0.f, 0.f));

	// tag all nodes that belong to this surface
	int N = mesh.Nodes();
	for (int i = 0; i<N; ++i) mesh.Node(i).m_ntag = -1;
	int nn = 0;
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			FENode& node = mesh.Node(f.n[j]);
			if (node.m_ntag == -1) node.m_ntag = nn++;
		}
	}

	// create the global node list
	m_node.resize(nn);
	for (int i = 0; i<N; ++i)
	{
		FENode& node = mesh.Node(i);
		if (node.m_ntag >= 0) m_node[node.m_ntag] = i;
	}
	m_pos.resize(nn);

	// create the local node list
	const int MN = FEFace::MAX_NODES;
	m_lnode.resize(Faces() * MN);
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j = 0; j < nf; ++j) m_lnode[MN*i + j] = mesh.Node(f.n[j]).m_ntag;
	}

	// create the node-facet look-up table
	m_NLT.resize(Nodes());
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int inode = m_lnode[MN * i + j];
			m_NLT[inode].push_back(m_face[i]);
		}
	}
}


//-----------------------------------------------------------------------------
FEAreaCoverage::FEAreaCoverage(Post::FEPostModel* fem) : Post::FEDataField("area coverage", DATA_FLOAT, DATA_NODE, CLASS_FACE, 0)
{
	m_fem = fem;
	m_bignoreBackIntersections = true;
}

//-----------------------------------------------------------------------------
void FEAreaCoverage::IgnoreBackIntersection(bool b)
{
	m_bignoreBackIntersections = b;
}

//-----------------------------------------------------------------------------
bool FEAreaCoverage::IgnoreBackIntersection() const
{
	return m_bignoreBackIntersections;
}

//-----------------------------------------------------------------------------
Post::FEDataField* Post::FEAreaCoverage::Clone() const
{
	FEAreaCoverage* pd = new FEAreaCoverage(m_fem);
	pd->m_surf1 = m_surf1;
	pd->m_surf2 = m_surf2;
	return pd;
}

//-----------------------------------------------------------------------------
void Post::FEAreaCoverage::InitSurface(int n)
{
	Post::FEPostMesh& mesh = *m_fem->GetFEMesh(0);

	vector<int> L;
	for (int i = 0; i<mesh.Faces(); ++i) if (mesh.Face(i).IsSelected()) L.push_back(i);

	if (n == 0) SetSelection1(L);
	if (n == 1) SetSelection2(L);
}

//-----------------------------------------------------------------------------
int Post::FEAreaCoverage::GetSurfaceSize(int i)
{
	return (i == 0 ? m_surf1.Faces() : m_surf2.Faces());
}

//-----------------------------------------------------------------------------
Post::FEMeshData* Post::FEAreaCoverage::CreateData(Post::FEState* pstate)
{
	return new Post::FEFaceData<float, DATA_NODE>(pstate, this);
}

//-----------------------------------------------------------------------------
void FEAreaCoverage::Apply(Post::FEPostModel* fem)
{
	m_fem = fem;
	Apply();
}

//-----------------------------------------------------------------------------
void FEAreaCoverage::Apply()
{
	FEPostModel& fem = *m_fem;

	// get the mesh
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	// build the node lists
	m_surf1.Create(mesh);
	m_surf2.Create(mesh);

	const int MN = FEFace::MAX_NODES;

	// get the field index
	int nfield = FIELD_CODE(GetFieldID());

	// repeat for all steps
	int nstep = fem.GetStates();
	for (int n = 0; n<nstep; ++n)
	{
		// build the normal lists
		UpdateSurface(m_surf1, n);
		UpdateSurface(m_surf2, n);

		FEState* ps = fem.GetState(n);
		FEFaceData<float, DATA_NODE>& df = dynamic_cast<FEFaceData<float, DATA_NODE>&>(ps->m_Data[nfield]);

		// repeat over all nodes of surface 1
		vector<float> a(m_surf1.Nodes(), 0.f);
		for (int i = 0; i<m_surf1.Nodes(); ++i)
		{
			int inode = m_surf1.m_node[i];
			FENode& node = mesh.Node(inode);
			vec3f ri = fem.NodePosition(inode, n);
			vec3f Ni = m_surf1.m_norm[i];

			// see if it intersects the other surface
			if (intersect(ri, Ni, m_surf2))
			{
				a[i] = 1.f;
			}
		}
		vector<int> nf1(m_surf1.Faces());                     // TODO: The reason I have to comment this out is because m_lnode has a fixed size per face
		for (int i = 0; i < m_surf1.Faces(); ++i) nf1[i] = MN;// mesh.Face(m_surf1.m_face[i]).Nodes();
		df.add(a, m_surf1.m_face, m_surf1.m_lnode, nf1);


		// repeat over all nodes of surface 2
		vector<float> b(m_surf2.Nodes(), 0.f);
		for (int i = 0; i<m_surf2.Nodes(); ++i)
		{
			int inode = m_surf2.m_node[i];
			FENode& node = mesh.Node(inode);
			vec3f ri = fem.NodePosition(inode, n);
			vec3f Ni = m_surf2.m_norm[i];

			// see if it intersects the other surface
			if (intersect(ri, Ni, m_surf1))
			{
				b[i] = 1.f;
			}
		}
		vector<int> nf2(m_surf2.Faces());
		for (int i = 0; i < m_surf2.Faces(); ++i) nf2[i] = MN;// mesh.Face(m_surf2.m_face[i]).Nodes();
		df.add(b, m_surf2.m_face, m_surf2.m_lnode, nf2);
	}
}

//-----------------------------------------------------------------------------
void FEAreaCoverage::UpdateSurface(FEAreaCoverage::Surface& s, int nstate)
{
	// get the mesh
	Post::FEPostMesh& mesh = *m_fem->GetFEMesh(0);
	FEState& state = *m_fem->GetState(nstate);
	int NF = s.Faces();
	int NN = s.Nodes();

	// update nodal positions
	for (int i=0; i<NN; ++i)
	{
		s.m_pos[i] = m_fem->NodePosition(s.m_node[i], nstate);
	}

	// update face normals
	s.m_fnorm.assign(NF, vec3f(0.f, 0.f, 0.f));
	s.m_norm.assign(NN, vec3f(0.f,0.f,0.f));
	const int MN = FEFace::MAX_NODES;
	vec3f r[3];
	for (int i = 0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(s.m_face[i]);

		r[0] = s.m_pos[s.m_lnode[i*MN    ]];
		r[1] = s.m_pos[s.m_lnode[i*MN + 1]];
		r[2] = s.m_pos[s.m_lnode[i*MN + 2]];

		vec3f N = (r[1] - r[0])^(r[2] - r[0]);

		s.m_fnorm[i] = N;
		s.m_fnorm[i].Normalize();

		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int n = s.m_lnode[MN * i + j]; assert(n >= 0);
			s.m_norm[n] += N;
		}
	}
	for (int i=0; i<(int)s.m_norm.size(); ++i) s.m_norm[i].Normalize();
}

//-----------------------------------------------------------------------------
bool FEAreaCoverage::intersect(const vec3f& r, const vec3f& N, FEAreaCoverage::Surface& surf)
{
	// create the ray
	Ray ray = {r, N};

	// loop over all facets connected to this node
	Intersection q;
	for (int i = 0; i<(int)surf.m_face.size(); ++i)
	{
		// see if the ray intersects this face
		if (faceIntersect(surf, ray, i))
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------
bool FEAreaCoverage::faceIntersect(FEAreaCoverage::Surface& surf, const Ray& ray, int nface)
{
	Intersection q;
	q.m_index = -1;
	Post::FEPostMesh& mesh = *m_fem->GetFEMesh(0);

	vec3f rn[4];
	FEFace& face = mesh.Face(surf.m_face[nface]);

	const int MN = FEFace::MAX_NODES;

	bool bfound = false;
	switch (face.m_type)
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
	{
		for (int i = 0; i<3; ++i)
		{
			rn[i] = surf.m_pos[surf.m_lnode[MN * nface + i]];
		}

		Triangle tri = { rn[0], rn[1], rn[2], surf.m_fnorm[nface] };
		bfound = IntersectTriangle(ray, tri, q, false);
	}
	break;
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
	{
		for (int i = 0; i<4; ++i)
		{
			rn[i] = surf.m_pos[surf.m_lnode[MN * nface + i]];
		}

		Quad quad = { rn[0], rn[1], rn[2], rn[3] };
		bfound = FastIntersectQuad(ray, quad, q);
	}
	break;
	}

	if (bfound && (m_bignoreBackIntersections))
	{
		// make sure the projection is in the direction of the ray
		bfound = (ray.direction*(q.point - ray.origin) > 0.f);
	}

	return bfound;
}
