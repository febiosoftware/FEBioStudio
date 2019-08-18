#pragma once
#include <vector>

class FELineMesh;
class FEEdge;

class FENodeEdgeList
{
public:
	FENodeEdgeList(const FELineMesh& mesh);

	// Return the number of edges for a given node
	int Edges(int node) const { return m_val[node]; }

	// Return the edge for a given node
	const FEEdge* Edge(int node, int edge) const;

	int EdgeIndex(int node, int edge) const { return m_edge[m_off[node] + edge]; }

private:
	const FELineMesh&	m_mesh;
	std::vector<int>	m_val;		// Valence list
	std::vector<int>	m_off;		// Offset into edge array
	std::vector<int>	m_edge;		// edge list
};
