#include "stdafx.h"
#include "FEEdge.h"
#include <assert.h>
using namespace Post;

//-----------------------------------------------------------------------------
FEEdge::FEEdge()
{
	node[0] = node[1] = node[2] = -1;
}

//-----------------------------------------------------------------------------
//! Evaluate the shape function values at the iso-parametric point r = [0,1]
void FEEdge::shape(double* H, double r)
{
	switch (m_type)
	{
	case EDGE_LINE2:
		H[0] = 1.0 - r;
		H[1] = r;
		break;
	case EDGE_LINE3:
		H[0] = (1 - r)*(2 * (1 - r) - 1);
		H[1] = r*(2 * r - 1);
		H[2] = 4 * (1 - r)*r;
		break;
	case EDGE_LINE4:
		H[0] = 0.5f*(1.f - r)*(3.f*r - 1.f)*(3.f*r - 2.f);
		H[1] = 0.5f*r*(3.f*r - 1.f)*(3.f*r - 2.f);
		H[2] = 9.f / 2.f*r*(r - 1.f)*(3.f*r - 2.f);
		H[3] = 9.f / 2.f*r*(1.f - r)*(3.f*r - 1.f);
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
double FEEdge::eval(double* d, double r)
{
	double H[FEEdge::MAX_NODES];
	shape(H, r);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return a;
}

//-----------------------------------------------------------------------------
vec3f FEEdge::eval(vec3f* d, double r)
{
	double H[FEEdge::MAX_NODES];
	shape(H, r);
	vec3f a(0, 0, 0);
	for (int i = 0; i<Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}
