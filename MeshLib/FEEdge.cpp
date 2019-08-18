#include "FEEdge.h"
#include <assert.h>

//-----------------------------------------------------------------------------
FEEdge::FEEdge()
{
	m_elem = -1;
	n[0] = n[1] = n[2] = n[3] = -1;
	m_nbr[0] = m_nbr[1] = -1;
	m_face[0] = m_face[1] = -1;
	m_type = FE_EDGE_INVALID;
}

//-----------------------------------------------------------------------------
FEEdge::FEEdge(const FEEdge& e) : FEItem(e)
{
	m_type = e.m_type;
	m_elem = e.m_elem;
	n[0] = e.n[0];
	n[1] = e.n[1];
	n[2] = e.n[2];
	n[3] = e.n[3];
	m_nbr[0] = e.m_nbr[0];
	m_nbr[1] = e.m_nbr[1];
	m_face[0] = e.m_face[0];
	m_face[1] = e.m_face[1];
}

//-----------------------------------------------------------------------------
void FEEdge::operator = (const FEEdge& e)
{
	m_type = e.m_type;
	m_elem = e.m_elem;
	n[0] = e.n[0];
	n[1] = e.n[1];
	n[2] = e.n[2];
	n[3] = e.n[3];
	m_nbr[0] = e.m_nbr[0];
	m_nbr[1] = e.m_nbr[1];
	m_face[0] = e.m_face[0];
	m_face[1] = e.m_face[1];
	FEItem::operator=(e);
}

//-----------------------------------------------------------------------------
// Tests equality between edges
bool FEEdge::operator == (const FEEdge& e) const
{
	if (e.m_type != m_type) return false;
	assert(m_type != FE_EDGE_INVALID);
	if ((n[0] != e.n[0]) && (n[0] != e.n[1])) return false;
	if ((n[1] != e.n[0]) && (n[1] != e.n[1])) return false;

	if (m_type == FE_EDGE3) 
	{
		if (n[2] != e.n[2]) return false;
	}
	else if (m_type == FE_EDGE4)
	{
		if ((n[2] != e.n[2]) && (n[2] != e.n[3])) return false;
		if ((n[3] != e.n[2]) && (n[3] != e.n[3])) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Returns the local index for a given node number.
int FEEdge::FindNodeIndex(int node) const
{
	assert(m_type != FE_EDGE_INVALID);
	if (node == n[0]) return 0;
	if (node == n[1]) return 1;
	if (node == n[2]) return 2;
	if (node == n[3]) return 3;
	return -1;
}

//-----------------------------------------------------------------------------
void FEEdge::SetType(FEEdgeType type)
{ 
	assert(m_type == FE_EDGE_INVALID);
	assert(type != FE_EDGE_INVALID);
	m_type = type;
}

//-----------------------------------------------------------------------------
int FEEdge::Nodes() const
{ 
	static int nodeCount[] = {2, 3, 4, 0};
	assert(m_type != FE_EDGE_INVALID);
	return nodeCount[m_type];
}
