#pragma once

#include <vector>
using namespace std;

#include "FECoreMesh.h"

//-----------------------------------------------------------------------------
// the first index is the element number
// the second index is the local node index of the element
struct NodeElemRef {
	int		eid;	// element index in mesh
	int		nid;	// local node index of the element
	FEElement_*	pe;	// pointer to element
};

class FENodeElementList
{
public:
	FENodeElementList();
	~FENodeElementList();

	void Build(FECoreMesh* pm);

	void Clear();

	bool IsEmpty() const;

	int Valence(int n) const { return (int)m_elem[n].size(); }
	FEElement_* Element(int n, int j) { return m_elem[n][j].pe; }
	int ElementIndex(int n, int j) const { return m_elem[n][j].eid; }

	bool HasElement(int node, int iel) const;

	vector<int> ElementIndexList(int n) const;
	const vector<NodeElemRef>& ElementList(int n) const { return m_elem[n]; }

protected:
	FECoreMesh*	m_pm;
	vector< vector<NodeElemRef> >	m_elem;
};
