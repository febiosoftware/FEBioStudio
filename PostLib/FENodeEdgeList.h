#pragma once
#include "FEElement.h"
#include <vector>

namespace Post {

//-----------------------------------------------------------------------------
// Forward declaration of the mesh class
class FEMeshBase;

//-----------------------------------------------------------------------------
// For each node of the mesh stores a list of edges that connect to that node
class FENodeEdgeList
{
public:
	// constructor
	FENodeEdgeList(void);

	// build the list from a mesh
	void Build(FEMeshBase* pm);

	// clear the list
	void Clear();

	// is the list empty or not
	bool Empty();

	// return the valence of a node, i.e. the number of edges connecting to that node
	int Valence(int node);

	// return the list of edges connecting to a node (the lenght of this list is the node valence)
	int* EdgeList(int node);

protected:
	FEMeshBase*		m_pm;
	vector<int>		m_index;	// offset in edge list
	vector<int>		m_edge;		// edge list
};

}
