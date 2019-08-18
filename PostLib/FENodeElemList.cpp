#include "stdafx.h"
#include "FENodeElemList.h"
#include "FEMesh.h"

using namespace Post;

//-----------------------------------------------------------------------------
void FENodeElemList::Build(FEMeshBase* pm)
{
	int N = pm->Nodes();
	m_NEL.resize(N);

	int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& e = pm->Element(i);
		int n = e.Nodes();
		for (int j=0; j<n; ++j) m_NEL[e.m_node[j]].push_back(pair<int, short>(i, j));
	}
}
