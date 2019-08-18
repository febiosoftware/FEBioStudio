#include "FENodeEdgeList.h"
#include "FELineMesh.h"

FENodeEdgeList::FENodeEdgeList(const FELineMesh& mesh) : m_mesh(mesh)
{
	// allocate valence array
	int N = m_mesh.Nodes();
	if (N == 0) return;
	m_val.resize(N, 0);

	// fill valence array
	int NE = m_mesh.Edges();
	for (int i=0; i<NE; ++i)
	{
		const FEEdge& edge = m_mesh.Edge(i);

		int n0 = edge.n[0]; assert(n0 >= 0);
		int n1 = edge.n[1]; assert(n1 >= 0);

		m_val[n0]++;
		m_val[n1]++;
	}

	// fill offset array
	m_off.resize(N, 0);
	for (int i=1; i<N; ++i)
	{
		m_off[i] = m_off[i-1] + m_val[i-1];
	}

	// allocate edge array
	int nsize = m_off[N-1] + m_val[N-1];
	m_edge.resize(nsize, -1);

	// fill edge array
	vector<int> tmp(N, 0);
	for (int i=0; i<NE; ++i)
	{
		const FEEdge& edge = m_mesh.Edge(i);
		int n0 = edge.n[0];
		int n1 = edge.n[1];

		m_edge[m_off[n0] + tmp[n0]] = i; tmp[n0]++; assert(tmp[n0] <= m_val[n0]);
		m_edge[m_off[n1] + tmp[n1]] = i; tmp[n1]++; assert(tmp[n1] <= m_val[n1]);
	}
}

// Return the edge for a given node
const FEEdge* FENodeEdgeList::Edge(int node, int edge) const
{
	return m_mesh.EdgePtr(m_edge[m_off[node] + edge]);
}
