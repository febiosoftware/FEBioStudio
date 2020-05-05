#include "FENodeEdgeList.h"
#include "FELineMesh.h"

FENodeEdgeList::FENodeEdgeList(FELineMesh* mesh) : m_mesh(mesh)
{
	if (mesh) Build(mesh);
}

void FENodeEdgeList::Clear()
{
	m_edge.clear();
}

bool FENodeEdgeList::IsEmpty() const
{
	return m_edge.empty();
}

void FENodeEdgeList::Build(FELineMesh* pmesh, bool segsOnly)
{
	m_mesh = pmesh;
	assert(pmesh);
	FELineMesh& mesh = *m_mesh;

	// allocate valence array
	int N = mesh.Nodes();
	if (N == 0) return;
	m_edge.resize(N);

	// fill edge array
	int NE = mesh.Edges();
	for (int i=0; i<NE; ++i)
	{
		const FEEdge& edge = mesh.Edge(i);
		if ((segsOnly == false) || (edge.m_gid >= 0))
		{
			int n0 = edge.n[0];
			int n1 = edge.n[1];

			vector<int>& l0 = m_edge[n0];
			vector<int>& l1 = m_edge[n1];

			l0.push_back(i);
			l1.push_back(i);
		}
	}
}

// Return the edge for a given node
const FEEdge* FENodeEdgeList::Edge(int node, int edge) const
{
	return m_mesh->EdgePtr(m_edge[node][edge]);
}

int FENodeEdgeList::EdgeIndex(int node, int edge) const 
{ 
	return m_edge[node][edge]; 
}

const std::vector<int>& FENodeEdgeList::EdgeIndexList(int node) const
{
	return m_edge[node];
}
