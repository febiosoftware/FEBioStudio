#include "stdafx.h"
#include "FENodeFaceList.h"
#include "FEMesh.h"

using namespace Post;

//-----------------------------------------------------------------------------
void FENodeFaceList::Build(FEMeshBase* pm)
{
	int N = pm->Nodes();
	m_NFL.resize(N);

	int NF = pm->Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		int n = f.Nodes();
		for (int j=0; j<n; ++j) m_NFL[f.node[j]].push_back(pair<int,short>(i, j));
	}
}
