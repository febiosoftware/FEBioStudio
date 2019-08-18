#pragma once
#include "FEItem.h"
#include "math3d.h"

namespace Post {

//-----------------------------------------------------------------------------
// Different edge types
enum FEEdgeType {
	EDGE_LINE2,
	EDGE_LINE3,
	EDGE_LINE4
};

//-----------------------------------------------------------------------------
// Class describing an edge of the mesh. The edges identify the smooth boundaries
class FEEdge : public FEItem
{
public:
	enum { MAX_NODES = 4 };

	FEEdgeType Type() const { return m_type; }

public:
	int node[MAX_NODES];
	FEEdgeType	m_type;

public:
	FEEdge();

	bool operator == (const FEEdge& e)
	{
		if ((node[0] == e.node[0]) && (node[1] == e.node[1])) return true;
		if ((node[0] == e.node[1]) && (node[1] == e.node[0])) return true;
		return false;
	}

	int Nodes() const
	{
		const int N[] = { 2, 3, 4 };
		return N[m_type];
	}

	// evaluate shape function at iso-parameteric point (r,s)
	void shape(double* H, double r);

	// evaluate a vector expression at iso-points (r,s)
	double eval(double* d, double r);

	// evaluate a vector expression at iso-points (r,s)
	vec3f eval(vec3f* v, double r);
};
}
