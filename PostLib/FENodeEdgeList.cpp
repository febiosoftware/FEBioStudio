#include "stdafx.h"
#include "FENodeEdgeList.h"
#include "FEMesh.h"

using namespace Post;

FENodeEdgeList::FENodeEdgeList(void)
{
	m_pm = 0; 
}

void FENodeEdgeList::Clear()
{
	m_pm = 0;
	m_index.clear();
	m_edge.clear();
}

bool FENodeEdgeList::Empty()
{ 
	return (m_pm == 0);
}

int FENodeEdgeList::Valence(int node)
{
	return (m_index[node + 1] - m_index[node]);
}

int* FENodeEdgeList::EdgeList(int node)
{
	return &(m_edge[ m_index[node] ]);
}

void FENodeEdgeList::Build(FEMeshBase* pm)
{
	m_pm = pm;
	if (pm == 0) { Clear(); return; }

	int NN = pm->Nodes();
	int NE = pm->Edges();

	m_index.resize(NN + 1, 0);

	int nsize = 0;
	for (int i=0; i<NE; ++i)
	{
		FEEdge& edge = pm->Edge(i);
		int n0 = edge.node[0];
		int n1 = edge.node[1];

		m_index[n0]++;
		m_index[n1]++;
		nsize += 2;
	}

	m_edge.resize(nsize);

	int m = m_index[0];
	m_index[0] = 0;
	for (int i=1; i<=NN; ++i)
	{
		int n = m_index[i];
		m_index[i] = m_index[i-1] + m;
		m = n;
	}

	vector<int> tmp(NN, 0);

	for (int i = 0; i<NE; ++i)
	{
		FEEdge& edge = pm->Edge(i);
		int n0 = edge.node[0];
		int n1 = edge.node[1];

		m_edge[ m_index[n0] + tmp[n0] ] = i; tmp[n0]++;
		m_edge[ m_index[n1] + tmp[n1] ] = i; tmp[n1]++;
	}
}
