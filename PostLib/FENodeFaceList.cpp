#include "stdafx.h"
#include "FENodeFaceList.h"
#include "FEMesh.h"

using namespace Post;

//-----------------------------------------------------------------------------
void FENodeFaceList::Build(Post::FEMeshBase* pm)
{
	m_pm = pm;

	int N = pm->Nodes();
	m_NFL.resize(N);

	int NF = pm->Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		int n = f.Nodes();
		for (int j=0; j<n; ++j) m_NFL[f.n[j]].push_back(pair<int,short>(i, j));
	}
}


//-----------------------------------------------------------------------------
// This function will fail if the facet could not be found. 
// The most likely cause would be if the facet is an interal fact, since PostView does not
// process internal facets (e.g. facets between two materials).
// \todo perhaps I should modify PostView so that it stores internal facets as well.
int FENodeFaceList::FindFace(int inode, int n[10], int m)
{
	FEFace ft;
	for (int i = 0; i<m; ++i) ft.n[i] = n[i];
	switch (m)
	{
	case 3: ft.m_type = FE_FACE_TRI3; break;
	case 4: ft.m_type = FE_FACE_QUAD4; break;
	case 6: ft.m_type = FE_FACE_TRI6; break;
	case 7: ft.m_type = FE_FACE_TRI7; break;
	case 8: ft.m_type = FE_FACE_QUAD8; break;
	case 9: ft.m_type = FE_FACE_QUAD9; break;
	case 10: ft.m_type = FE_FACE_TRI10; break;
	default:
		assert(false);
	};

	vector<NodeFaceRef>& ni = m_NFL[inode];
	int nf = (int)ni.size();
	for (int i = 0; i<nf; ++i)
	{
		FEFace& f = m_pm->Face(ni[i].first);
		if (f == ft) return ni[i].first;
	}
	return -1;
}
