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
#include "FEStrainMap.h"
#include "FEMeshData_T.h"
#include "FEPostModel.h"
#include "tools.h"

using namespace Post;
using namespace std;

//-----------------------------------------------------------------------------
void FEStrainMap::Surface::BuildNodeList(FSMesh& mesh)
{
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

	// create the local node list
	m_lnode.resize(Faces() * 4);
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		if (f.Nodes() == 4)
		{
			m_lnode[4 * i] = mesh.Node(f.n[0]).m_ntag; assert(m_lnode[4 * i] >= 0);
			m_lnode[4 * i + 1] = mesh.Node(f.n[1]).m_ntag; assert(m_lnode[4 * i + 1] >= 0);
			m_lnode[4 * i + 2] = mesh.Node(f.n[2]).m_ntag; assert(m_lnode[4 * i + 2] >= 0);
			m_lnode[4 * i + 3] = mesh.Node(f.n[3]).m_ntag; assert(m_lnode[4 * i + 3] >= 0);
		}
		else if (f.Nodes() == 3)
		{
			m_lnode[4 * i] = mesh.Node(f.n[0]).m_ntag; assert(m_lnode[4 * i] >= 0);
			m_lnode[4 * i + 1] = mesh.Node(f.n[1]).m_ntag; assert(m_lnode[4 * i + 1] >= 0);
			m_lnode[4 * i + 2] = mesh.Node(f.n[2]).m_ntag; assert(m_lnode[4 * i + 2] >= 0);
			m_lnode[4 * i + 3] = m_lnode[4 * i + 2];
		}
		else assert(false);
	}

	// create the node-facet look-up table
	m_NLT.resize(Nodes());
	for (int i = 0; i<Faces(); ++i)
	{
		FSFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int inode = m_lnode[4 * i + j];
			m_NLT[inode].push_back(m_face[i]);
		}
	}
}

// constructor
FEStrainMap::FEStrainMap()
{
	m_tol = 0.01;
}

// assign selections
void FEStrainMap::SetFrontSurface1(std::vector<int>& s)
{
	m_front1.m_face = s;
}

void FEStrainMap::SetBackSurface1(std::vector<int>& s)
{
	m_back1.m_face = s;
}

void FEStrainMap::SetFrontSurface2(std::vector<int>& s)
{
	m_front2.m_face = s;
}

void FEStrainMap::SetBackSurface2(std::vector<int>& s)
{
	m_back2.m_face = s;
}

// apply the map
void FEStrainMap::Apply(FEPostModel& fem)
{
	// relative distance to allow penetration
	double distTol = 1.0;

	static int ncalls = 0; ncalls++;
	char szname[64];
	if (ncalls == 1)
		sprintf(szname, "strain map");
	else
		sprintf(szname, "strain map (%d)", ncalls);

	// store the model
	m_fem = &fem;

	// add a new data field
	fem.AddDataField(new FEDataField_T<FEFaceData<float, DATA_NODE> >(&fem, EXPORT_DATA), szname);
	int NDATA = fem.GetDataManager()->DataFields() - 1;

	// get the mesh
	FSMesh& mesh = *fem.GetFEMesh(0);

	// build the node lists
	m_front1.BuildNodeList(mesh);
	m_back1.BuildNodeList(mesh);
	m_front2.BuildNodeList(mesh);
	m_back2.BuildNodeList(mesh);
	int N = mesh.Nodes();

	// repeat for all steps
	int nstep = fem.GetStates();
	for (int n = 0; n<nstep; ++n)
	{
		UpdateNodePositions(m_front1, n);
		UpdateNodePositions(m_back1, n);
		UpdateNodePositions(m_front2, n);
		UpdateNodePositions(m_back2, n);

		BuildNormalList(m_front1);
		BuildNormalList(m_back1);
		BuildNormalList(m_front2);
		BuildNormalList(m_back2);

		FEState* ps = fem.GetState(n);
		FEFaceData<float, DATA_NODE>& df = dynamic_cast<FEFaceData<float, DATA_NODE>&>(ps->m_Data[NDATA]);

		// loop over all nodes of surface 1
		vector<float> D1(m_front1.Nodes(), 0.f);
		vec3f q;
		for (int i = 0; i<m_front1.Nodes(); ++i)
		{
			int inode = m_front1.m_node[i];
			FSNode& node = mesh.Node(inode);
			vec3f r = m_front1.m_pos[i];
			if (project(m_front2, r, m_front1.m_norm[i], q))
			{
				D1[i] = (q - r).Length();
				double s = (q - r)*m_front1.m_norm[i];
				if (s < 0) D1[i] = -D1[i];
			}
		}

		vector<float> L1(m_front1.Nodes());
		for (int i = 0; i<m_front1.Nodes(); ++i)
		{
			int inode = m_front1.m_node[i];
			if (D1[i] < 0)
			{
				FSNode& node = mesh.Node(inode);
				vec3f r = m_front1.m_pos[i];
				if (project(m_back1, r, m_front1.m_norm[i], q))
				{
					L1[i] = (q - r).Length();
					if (fabs(D1[i]) <= distTol*fabs(L1[i]))
					{
						double s = (r - q)*m_front1.m_norm[i];
						if (s < 0) L1[i] = -L1[i];
					}
					else L1[i] = 1e+34f;
				}
				else L1[i] = 1e+34f;	// really large number, such that the strain is zero
			}
			else L1[i] = 0.0;
		}

		vector<float> s1(m_front1.Nodes());
		for (int i=0; i<m_front1.Nodes(); ++i)
		{
			if (D1[i] < 0.0)
				s1[i] = 0.5f*D1[i] / L1[i];
			else
				s1[i] = 0.f;
		}

		vector<int> nf1(m_front1.Faces());
		for (int i = 0; i<m_front1.Faces(); ++i) nf1[i] = 4; //mesh.Face(m_surf1.m_face[i]).Nodes();
		df.add(s1, m_front1.m_face, m_front1.m_lnode, nf1);

		// loop over all nodes of surface 2
		vector<float> D2(m_front2.Nodes());
		for (int i = 0; i<m_front2.Nodes(); ++i)
		{
			int inode = m_front2.m_node[i];
			FSNode& node = mesh.Node(inode);
			vec3f r = m_front2.m_pos[i];
			if (project(m_front1, r, m_front2.m_norm[i], q))
			{
				D2[i] = (q - r).Length();
				double s = (q - r)*m_front2.m_norm[i];
				if (s < 0) D2[i] = -D2[i];
			}
			else D2[i] = 0.f;
		}

		vector<float> L2(m_front2.Nodes());
		for (int i = 0; i<m_front2.Nodes(); ++i)
		{
			int inode = m_front2.m_node[i];
			if (D2[i] < 0)
			{
				FSNode& node = mesh.Node(inode);
				vec3f r = m_front2.m_pos[i];
				if (project(m_back2, r, m_front2.m_norm[i], q))
				{
					L2[i] = (q - r).Length();
					if (fabs(D2[i]) <= distTol*fabs(L2[i]))
					{
						double s = (r - q)*m_front2.m_norm[i];
						if (s < 0) L2[i] = -L2[i];
					}
					else L2[i] = 1e+34f;	// really large number, such that the strain is zero
				}
				else L2[i] = 1e+34f;	// really large number, such that the strain is zero
			}
			else L2[i] = 0.f;
		}

		vector<float> s2(m_front2.Nodes());
		for (int i = 0; i<m_front2.Nodes(); ++i)
		{
			if (D2[i] < 0.0)
				s2[i] = 0.5f*D2[i] / L2[i];
			else
				s2[i] = 0.f;
		}

		vector<int> nf2(m_front2.Faces());
		for (int i = 0; i<m_front2.Faces(); ++i) nf2[i] = 4; //mesh.Face(m_surf2.m_face[i]).Nodes();
		df.add(s2, m_front2.m_face, m_front2.m_lnode, nf2);
	}
}


//-----------------------------------------------------------------------------
void FEStrainMap::UpdateNodePositions(FEStrainMap::Surface& s, int ntime)
{
	int NN = s.Nodes();
	s.m_pos.resize(NN);
	for (int i = 0; i < NN; ++i) s.m_pos[i] = m_fem->NodePosition(s.m_node[i], ntime);
}

//-----------------------------------------------------------------------------
void FEStrainMap::BuildNormalList(FEStrainMap::Surface& s)
{
	// get the mesh
	FSMesh& mesh = *m_fem->GetFEMesh(0);

	int NF = s.Faces();
	int NN = s.Nodes();
	s.m_norm.resize(NN);
	for (int i = 0; i < NN; ++i) s.m_norm[i] = vec3f(0, 0, 0);

	vec3f r[3];
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(s.m_face[i]);
		int nf = f.Nodes();
		r[0] = s.m_pos[s.m_lnode[4*i   ]];
		r[1] = s.m_pos[s.m_lnode[4*i + 1]];
		r[2] = s.m_pos[s.m_lnode[4*i + 2]];
		vec3f fn = (r[1] - r[0]) ^ (r[2] - r[0]);

		for (int j = 0; j<nf; ++j)
		{
			int n = s.m_lnode[4 * i + j]; assert(n >= 0);
			s.m_norm[n] += fn;
		}
	}

	for (int i = 0; i < NN; ++i) s.m_norm[i].Normalize();
}

//-----------------------------------------------------------------------------
bool FEStrainMap::project(FEStrainMap::Surface& surf, vec3f& r, vec3f& t, vec3f& q)
{
	FSMesh& mesh = *m_fem->GetFEMesh(0);

	// loop over all facets
	float Dmin = 0.f;
	bool bfound = false;
	vec3f rf[4];
	for (int i = 0; i<surf.Faces(); ++i)
	{
		// get the i-th facet
		FSFace& face = mesh.Face(surf.m_face[i]);
		rf[0] = surf.m_pos[surf.m_lnode[4*i    ]];
		rf[1] = surf.m_pos[surf.m_lnode[4*i + 1]];
		rf[2] = surf.m_pos[surf.m_lnode[4*i + 2]];
		rf[3] = surf.m_pos[surf.m_lnode[4*i + 3]];

		// project r onto the the facet along its normal
		vec3f p;
		if (ProjectToFacet(rf, face.Nodes(), r, t, p))
		{
			// return the closest projection
			float D = (p - r)*(p - r);
			if ((D < Dmin) || (bfound == false))
			{
				q = p;
				Dmin = D;
				bfound = true;
			}
		}
	}

	return bfound;
}

//-----------------------------------------------------------------------------
bool FEStrainMap::ProjectToFacet(vec3f* y, int nf, vec3f& x, vec3f& t, vec3f& q)
{
	// get the mesh to which this surface belongs
	FSMesh& mesh = *m_fem->GetFEMesh(0);

	// calculate normal projection of x onto element
	switch (nf)
	{
	case 3: return ProjectToTriangle(y, x, t, q, m_tol); break;
	case 4: return ProjectToQuad(y, x, t, q, m_tol); break;
	default:
		assert(false);
	}
	return false;
}
