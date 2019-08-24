#include "stdafx.h"
#include "FEFace.h"

//-----------------------------------------------------------------------------
Post::FEFace::FEFace()
{
	m_nsg = 0;
	m_ntype = FE_FACE_INVALID_TYPE;
	for (int i = 0; i<MAX_NODES; ++i) node[i] = -1;
}

//-----------------------------------------------------------------------------
//! return the edge
FEEdge Post::FEFace::Edge(int i)
{
	FEEdge e;
	assert(i<Edges());
	switch (m_ntype)
	{
	case FE_FACE_TRI3:
	{
		const int L[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };
		e.n[0] = node[L[i][0]]; e.n[1] = node[L[i][1]];
		e.m_type = FE_EDGE2;
	}
	break;
	case FE_FACE_QUAD4:
	{
		const int L[4][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } };
		e.n[0] = node[L[i][0]]; e.n[1] = node[L[i][1]];
		e.m_type = FE_EDGE2;
	}
	break;
	case FE_FACE_TRI6:
	case FE_FACE_TRI7:
	{
		const int L[3][3] = { { 0, 1, 3 }, { 1, 2, 4 }, { 2, 0, 5 } };
		e.n[0] = node[L[i][0]]; e.n[1] = node[L[i][1]]; e.n[2] = node[L[i][2]];
		e.m_type = FE_EDGE3;
	}
	break;
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
	{
		const int L[4][3] = { { 0, 1, 4 }, { 1, 2, 5 }, { 2, 3, 6 }, { 3, 0, 7 } };
		e.n[0] = node[L[i][0]]; e.n[1] = node[L[i][1]]; e.n[2] = node[L[i][2]];
		e.m_type = FE_EDGE3;
	}
	break;
	case FE_FACE_TRI10:
	{
		const int L[3][4] = { { 0, 1, 3, 4 }, { 1, 2, 5, 6 }, { 2, 0, 8, 7 } };
		e.m_type = FE_EDGE4;
		e.n[0] = node[L[i][0]];
		e.n[1] = node[L[i][1]];
		e.n[2] = node[L[i][2]];
		e.n[3] = node[L[i][3]];
	}
	break;
	default:
		assert(false);
	}
	return e;
}

//-----------------------------------------------------------------------------
//! Evaluate the shape function values at the iso-parametric point (r,s)
void Post::FEFace::shape(double* H, double r, double s)
{
	switch (m_ntype)
	{
	case FE_FACE_TRI3:
	{
		H[0] = 1.0 - r - s;
		H[1] = r;
		H[2] = s;
	}
	break;
	case FE_FACE_QUAD4:
	{
		H[0] = 0.25*(1.0 - r)*(1.0 - s);
		H[1] = 0.25*(1.0 + r)*(1.0 - s);
		H[2] = 0.25*(1.0 + r)*(1.0 + s);
		H[3] = 0.25*(1.0 - r)*(1.0 + s);
	}
	break;
	case FE_FACE_TRI6:
	{
		double r1 = 1.0 - r - s;
		double r2 = r;
		double r3 = s;

		H[0] = r1*(2.0*r1 - 1.0);
		H[1] = r2*(2.0*r2 - 1.0);
		H[2] = r3*(2.0*r3 - 1.0);
		H[3] = 4.0*r1*r2;
		H[4] = 4.0*r2*r3;
		H[5] = 4.0*r3*r1;
	}
	break;
	case FE_FACE_TRI7:
	{
		double r1 = 1.0 - r - s;
		double r2 = r;
		double r3 = s;

		H[6] = 27.0*r1*r2*r3;
		H[0] = r1*(2.0*r1 - 1.0) + H[6] / 9.0;
		H[1] = r2*(2.0*r2 - 1.0) + H[6] / 9.0;
		H[2] = r3*(2.0*r3 - 1.0) + H[6] / 9.0;
		H[3] = 4.0*r1*r2 - 4.0*H[6] / 9.0;
		H[4] = 4.0*r2*r3 - 4.0*H[6] / 9.0;
		H[5] = 4.0*r3*r1 - 4.0*H[6] / 9.0;
	}
	break;
	case FE_FACE_QUAD8:
	{
		H[4] = 0.5*(1 - r*r)*(1 - s);
		H[5] = 0.5*(1 - s*s)*(1 + r);
		H[6] = 0.5*(1 - r*r)*(1 + s);
		H[7] = 0.5*(1 - s*s)*(1 - r);

		H[0] = 0.25*(1 - r)*(1 - s) - 0.5*(H[4] + H[7]);
		H[1] = 0.25*(1 + r)*(1 - s) - 0.5*(H[4] + H[5]);
		H[2] = 0.25*(1 + r)*(1 + s) - 0.5*(H[5] + H[6]);
		H[3] = 0.25*(1 - r)*(1 + s) - 0.5*(H[6] + H[7]);
	}
	break;
	case FE_FACE_QUAD9:
	{
		double R[3] = { 0.5*r*(r - 1.0), 0.5*r*(r + 1.0), 1.0 - r*r };
		double S[3] = { 0.5*s*(s - 1.0), 0.5*s*(s + 1.0), 1.0 - s*s };

		H[0] = R[0] * S[0];
		H[1] = R[1] * S[0];
		H[2] = R[1] * S[1];
		H[3] = R[0] * S[1];
		H[4] = R[2] * S[0];
		H[5] = R[1] * S[2];
		H[6] = R[2] * S[1];
		H[7] = R[0] * S[2];
		H[8] = R[2] * S[2];
	}
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
double Post::FEFace::eval(double* d, double r, double s)
{
	double H[Post::FEFace::MAX_NODES];
	shape(H, r, s);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return a;
}

//-----------------------------------------------------------------------------
vec3f Post::FEFace::eval(vec3f* d, double r, double s)
{
	double H[Post::FEFace::MAX_NODES];
	shape(H, r, s);
	vec3f a(0, 0, 0);
	for (int i = 0; i<Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}

