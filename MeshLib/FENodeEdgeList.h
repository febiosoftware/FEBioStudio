#pragma once
#include <vector>

class FELineMesh;
class FEEdge;

class FENodeEdgeList
{
public:
	FENodeEdgeList(FELineMesh* mesh = nullptr);

	void Build(FELineMesh* mesh, bool segsOnly = false);

	void Clear();

	bool IsEmpty() const;

	// Return the number of edges for a given node
	int Edges(int node) const { return (int) m_edge[node].size(); }

	// Return the edge for a given node
	const FEEdge* Edge(int node, int edge) const;

	// return the edge index
	int EdgeIndex(int node, int edge) const;

	const std::vector<int>& EdgeIndexList(int node) const;

private:
	FELineMesh*			m_mesh;
	std::vector< std::vector<int> >	m_edge;		// edge list
};
