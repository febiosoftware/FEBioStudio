#include "stdafx.h"
#include "FEDistanceMap.h"
#include "FEMeshData_T.h"
#include <stdio.h>
#include "tools.h"
using namespace Post;

//-----------------------------------------------------------------------------
void FEDistanceMap::Surface::BuildNodeList(Post::FEPostMesh& mesh)
{
	// tag all nodes that belong to this surface
	int N = mesh.Nodes();
	for (int i=0; i<N; ++i) mesh.Node(i).m_ntag = -1;
	int nn = 0;
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) 
		{
			FENode& node = mesh.Node(f.n[j]);
			if (node.m_ntag == -1) node.m_ntag = nn++;
		}
	}

	// create the global node list
	m_node.resize(nn);
	for (int i=0; i<N; ++i)
	{
		FENode& node = mesh.Node(i);
		if (node.m_ntag >= 0) m_node[node.m_ntag] = i;
	}

	// create the local node list
	const int MN = FEFace::MAX_NODES;
	m_lnode.resize(Faces()*MN);
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) m_lnode[MN * i + j] = mesh.Node(f.n[j]).m_ntag;
	}

	// create the node-facet look-up table
	m_NLT.resize(Nodes());
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& f = mesh.Face(m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j)
		{
			int inode = m_lnode[MN*i+j];
			m_NLT[inode].push_back(m_face[i]);
		}
	}
}

//-----------------------------------------------------------------------------
void FEDistanceMap::BuildNormalList(FEDistanceMap::Surface& s)
{
	// get the mesh
	Post::FEPostMesh& mesh = *m_pfem->GetFEMesh(0);

	int NF = s.Faces();
	int NN = s.Nodes();
	s.m_norm.resize(NN);

	const int MN = FEFace::MAX_NODES;
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = mesh.Face(s.m_face[i]);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) 
		{
			int n = s.m_lnode[MN*i + j]; assert(n>=0);
			s.m_norm[n] = f.m_nn[j];
		}
	}
}

//-----------------------------------------------------------------------------
void FEDistanceMap::Apply(FEModel& fem)
{
	static int ncalls = 0; ncalls++;
	char szname[64];
	if (ncalls==1)
		sprintf(szname, "distance map");
	else
		sprintf(szname, "distance map (%d)", ncalls);

	// store the model
	m_pfem = &fem;

	// add a new data field
	fem.AddDataField(new FEDataField_T<FEFaceData<float, DATA_NODE> >(szname, EXPORT_DATA));
	int NDATA = fem.GetDataManager()->DataFields()-1;

	// get the mesh
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	const int MN = FEFace::MAX_NODES;

	// build the node lists
	m_surf1.BuildNodeList(mesh);
	m_surf2.BuildNodeList(mesh);
	int N = mesh.Nodes();

	if (m_bsigned)
	{
		BuildNormalList(m_surf1);
		BuildNormalList(m_surf2);
	}

	// repeat for all steps
	int nstep = fem.GetStates();
	for (int n=0; n<nstep; ++n)
	{
		FEState* ps = fem.GetState(n);
		FEFaceData<float,DATA_NODE>& df = dynamic_cast<FEFaceData<float,DATA_NODE>&>(ps->m_Data[NDATA]);

		// loop over all nodes of surface 1
		vector<float> a(m_surf1.Nodes());
		for (int i=0; i<m_surf1.Nodes(); ++i)
		{
			int inode = m_surf1.m_node[i];
			FENode& node = mesh.Node(inode);
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
		for (int i=0; i<m_surf1.Faces(); ++i) nf1[i] = MN; //mesh.Face(m_surf1.m_face[i]).Nodes();
		df.add(a, m_surf1.m_face, m_surf1.m_lnode, nf1);

		// loop over all nodes of surface 2
		vector<float> b(m_surf2.Nodes());
		for (int i=0; i<m_surf2.Nodes(); ++i)
		{
			int inode = m_surf2.m_node[i];
			FENode& node = mesh.Node(inode);
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
		for (int i = 0; i<m_surf2.Faces(); ++i) nf2[i] = MN; //mesh.Face(m_surf2.m_face[i]).Nodes();
		df.add(b, m_surf2.m_face, m_surf2.m_lnode, nf2);
	}
}

//-----------------------------------------------------------------------------
vec3f FEDistanceMap::project(FEDistanceMap::Surface& surf, vec3f& r, int ntime)
{
	Post::FEPostMesh& mesh = *m_pfem->GetFEMesh(0);

	// find the closest surface node
	vec3f q = m_pfem->NodePosition(surf.m_node[0], ntime);
	float Dmin = (q - r)*(q - r);
	int imin = 0;
	for (int i=1; i<surf.Nodes(); ++i)
	{
		vec3f p = m_pfem->NodePosition(surf.m_node[i], ntime);
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
		FEFace& face = mesh.Face(FT[i]);

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
bool FEDistanceMap::ProjectToFacet(FEFace& f, vec3f& x, int ntime, vec3f& q)
{
	// get the mesh to which this surface belongs
	Post::FEPostMesh& mesh = *m_pfem->GetFEMesh(0);
	
	// number of element nodes
	int nf = f.Nodes();
	
	// get the elements nodal positions
	const int MN = FEFace::MAX_NODES;
	vec3f y[MN];
	
	// calculate normal projection of x onto element
	switch (f.Type())
	{
	case FE_FACE_TRI3:
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	case FE_FACE_TRI10:
		{
			for (int i = 0; i<3; ++i) y[i] = m_pfem->NodePosition(f.n[i], ntime);
			return ProjectToTriangle(y, x, q, m_tol);
		}
		break;
	case FE_FACE_QUAD4:
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		{
			for (int i = 0; i<4; ++i) y[i] = m_pfem->NodePosition(f.n[i], ntime);
			return ProjectToQuad(y, x, q, m_tol);
		}
		break;
	default:
		assert(false);
	}
	return false;
}
