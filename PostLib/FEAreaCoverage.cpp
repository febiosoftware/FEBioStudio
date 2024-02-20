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
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			FSNode& node = mesh.Node(f.n[j]);
			if (node.m_ntag == -1) node.m_ntag = nn++;
		}
	}

	// create the global node list
	m_node.resize(nn);
	for (int i = 0; i<N; ++i)
	{
		FSNode& node = mesh.Node(i);
		if (node.m_ntag >= 0) m_node[node.m_ntag] = i;
	}
	m_pos.resize(nn);

	// create the local node list
	const int MN = FSFace::MAX_NODES;
	m_lnode.resize(Faces() * MN);
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j = 0; j < nf; ++j) m_lnode[MN*i + j] = mesh.Node(f.n[j]).m_ntag;
	}

	// create the node-facet look-up table
	m_NLT.resize(Nodes());
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int inode = m_lnode[MN * i + j];
			m_NLT[inode].push_back(i);
		}
	}
}

//-----------------------------------------------------------------------------
FEAreaCoverage::FEAreaCoverage(Post::FEPostModel* fem, int flags) : Post::ModelDataField(fem, DATA_SCALAR, DATA_NODE, FACE_DATA, 0)
{
	m_ballowBackIntersections = false;
	m_angleThreshold = 0.0;
	m_backSearchRadius = 0.0;
}

//-----------------------------------------------------------------------------
void FEAreaCoverage::AllowBackIntersection(bool b)
{
	m_ballowBackIntersections = b;
}

//-----------------------------------------------------------------------------
bool FEAreaCoverage::AllowBackIntersection() const
{
	return m_ballowBackIntersections;
}

//-----------------------------------------------------------------------------
void Post::FEAreaCoverage::SetAngleThreshold(double w)
{
	m_angleThreshold = w;
}

//-----------------------------------------------------------------------------
double Post::FEAreaCoverage::GetAngleThreshold() const
{
	return m_angleThreshold;
}

void Post::FEAreaCoverage::SetBackSearchRadius(double R)
{
	m_backSearchRadius = R;
}

double Post::FEAreaCoverage::GetBackSearchRadius() const
{
	return m_backSearchRadius;
}

//-----------------------------------------------------------------------------
Post::ModelDataField* Post::FEAreaCoverage::Clone() const
{
	FEAreaCoverage* pd = new FEAreaCoverage(m_fem, 0);
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

	const int MN = FSFace::MAX_NODES;

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

		// project surface 1 onto surface 2
		vector<float> a(m_surf1.Nodes(), 0.f);
		projectSurface(m_surf1, m_surf2, a);
		vector<int> nf1(m_surf1.Faces());                     // TODO: The reason I have to comment this out is because m_lnode has a fixed size per face
		for (int i = 0; i < m_surf1.Faces(); ++i) nf1[i] = MN;// mesh.Face(m_surf1.m_face[i]).Nodes();
		df.add(a, m_surf1.m_face, m_surf1.m_lnode, nf1);


		// repeat over all nodes of surface 2
		vector<float> b(m_surf2.Nodes(), 0.f);
		projectSurface(m_surf2, m_surf1, b);
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
	const int MN = FSFace::MAX_NODES;
	vec3f r[3];
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(s.m_face[i]);

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
// project a surface onto another surface
void FEAreaCoverage::projectSurface(FEAreaCoverage::Surface& surf1, FEAreaCoverage::Surface& surf2, vector<float>& a)
{
#pragma omp parallel for shared(surf1, surf2, a)
	for (int i = 0; i < surf1.Nodes(); ++i)
	{
		vec3f ri = surf1.m_pos[i];
		vec3f Ni = surf1.m_norm[i];

		// see if it intersects the other surface
		Intersection q;
		if (intersect(ri, Ni, surf2, q))
		{
			vec3f e = to_vec3f(q.point) - ri;
			double L1 = e.Length();
			if (e*Ni < 0.f) L1 = -L1;

			// make sure back intersections are contrained to search radius
			bool bintersect = true;
			if ((L1 < 0) && (m_backSearchRadius > 0))
			{
				if (-L1 > m_backSearchRadius) bintersect = false;
			}

			// if the intersection remains, tag it
			if (bintersect)
			{
				a[i] = 1.f;
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool FEAreaCoverage::intersect(const vec3f& r, const vec3f& N, FEAreaCoverage::Surface& surf, Intersection& qmin)
{
	// create the ray
	vec3d rd = to_vec3d(r);
	Ray ray = {rd, to_vec3d(N)};

	// loop over all facets connected to this node
	Intersection q;
	int imin = -1;
	double Lmin;
	for (int i = 0; i<(int)surf.m_face.size(); ++i)
	{
		// see if the ray intersects this face
		if (faceIntersect(surf, ray, i, q))
		{
			double L = (q.point - rd).Length();
			if ((imin == -1) || (L < Lmin))
			{
				imin = i;
				Lmin = L;
				qmin = q;
			}
		}
	}

	return (imin != -1);
}


//-----------------------------------------------------------------------------
bool FEAreaCoverage::faceIntersect(FEAreaCoverage::Surface& surf, const Ray& ray, int nface, Intersection& q)
{
	q.m_index = -1;
	Post::FEPostMesh& mesh = *m_fem->GetFEMesh(0);

	vec3f rn[4];
	FSFace& face = mesh.Face(surf.m_face[nface]);

	const int MN = FSFace::MAX_NODES;

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

		Triangle tri = { to_vec3d(rn[0]), to_vec3d(rn[1]), to_vec3d(rn[2]), to_vec3d(surf.m_fnorm[nface]) };
		bfound = IntersectTriangle(ray, tri, q, false);

		bfound = (bfound && (ray.direction * tri.fn < -m_angleThreshold));
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

		Quad quad = { to_vec3d(rn[0]), to_vec3d(rn[1]), to_vec3d(rn[2]), to_vec3d(rn[3]) };
		bfound = FastIntersectQuad(ray, quad, q);
	}
	break;
	}

	if (bfound && (m_ballowBackIntersections == false))
	{
		// make sure the projection is in the direction of the ray
		bfound = (ray.direction*(q.point - ray.origin) > 0.f);
	}

	return bfound;
}
