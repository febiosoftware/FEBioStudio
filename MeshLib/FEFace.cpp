#include "FEFace.h"

//-----------------------------------------------------------------------------
FEFace::FEFace()
{
	m_type = FE_FACE_INVALID_TYPE;
	m_elem[0] = m_elem[1] = -1; 
	m_sid = 0;

	n[0] = -1; n[1] = -1; n[2] = -1; n[3] = -1;
	n[4] = -1; n[5] = -1; n[6] = -1; n[7] = -1;
	n[8] = -1;
	n[9] = -1;
	m_nbr[0] = -1;
	m_nbr[1] = -1;
	m_nbr[2] = -1;
	m_nbr[3] = -1;

	m_edge[0] = -1;
	m_edge[1] = -1;
	m_edge[2] = -1;
	m_edge[3] = -1;
}

//-----------------------------------------------------------------------------
bool FEFace::operator == (const FEFace& f) const
{
	assert(m_type != FE_FACE_INVALID_TYPE);
	if (m_type != f.m_type) return false;
	switch (m_type)
	{
	case FE_FACE_TRI3:
		{
			if ((n[0] != f.n[0]) && (n[0] != f.n[1]) && (n[0] != f.n[2])) return false;
			if ((n[1] != f.n[0]) && (n[1] != f.n[1]) && (n[1] != f.n[2])) return false;
			if ((n[2] != f.n[0]) && (n[2] != f.n[1]) && (n[2] != f.n[2])) return false;
		}
		break;
	case FE_FACE_QUAD4:
		{
			if ((n[0] != f.n[0]) && (n[0] != f.n[1]) && (n[0] != f.n[2]) && (n[0] != f.n[3])) return false;
			if ((n[1] != f.n[0]) && (n[1] != f.n[1]) && (n[1] != f.n[2]) && (n[1] != f.n[3])) return false;
			if ((n[2] != f.n[0]) && (n[2] != f.n[1]) && (n[2] != f.n[2]) && (n[2] != f.n[3])) return false;
			if ((n[3] != f.n[0]) && (n[3] != f.n[1]) && (n[3] != f.n[2]) && (n[3] != f.n[3])) return false;
		}
		break;
	case FE_FACE_TRI6:
		{
			if ((n[0] != f.n[0]) && (n[0] != f.n[1]) && (n[0] != f.n[2])) return false;
			if ((n[1] != f.n[0]) && (n[1] != f.n[1]) && (n[1] != f.n[2])) return false;
			if ((n[2] != f.n[0]) && (n[2] != f.n[1]) && (n[2] != f.n[2])) return false;

			if ((n[3] != f.n[3]) && (n[3] != f.n[4]) && (n[3] != f.n[5])) return false;
			if ((n[4] != f.n[3]) && (n[4] != f.n[4]) && (n[4] != f.n[5])) return false;
			if ((n[5] != f.n[3]) && (n[5] != f.n[4]) && (n[5] != f.n[5])) return false;
		}
		break;
	case FE_FACE_TRI7:
		{
			if ((n[0] != f.n[0]) && (n[0] != f.n[1]) && (n[0] != f.n[2])) return false;
			if ((n[1] != f.n[0]) && (n[1] != f.n[1]) && (n[1] != f.n[2])) return false;
			if ((n[2] != f.n[0]) && (n[2] != f.n[1]) && (n[2] != f.n[2])) return false;

			if ((n[3] != f.n[3]) && (n[3] != f.n[4]) && (n[3] != f.n[5])) return false;
			if ((n[4] != f.n[3]) && (n[4] != f.n[4]) && (n[4] != f.n[5])) return false;
			if ((n[5] != f.n[3]) && (n[5] != f.n[4]) && (n[5] != f.n[5])) return false;

			if ((n[6] != f.n[6])) return false;
		}
		break;
	case FE_FACE_QUAD8:
		{
			if ((n[0] != f.n[0]) && (n[0] != f.n[1]) && (n[0] != f.n[2]) && (n[0] != f.n[3])) return false;
			if ((n[1] != f.n[0]) && (n[1] != f.n[1]) && (n[1] != f.n[2]) && (n[1] != f.n[3])) return false;
			if ((n[2] != f.n[0]) && (n[2] != f.n[1]) && (n[2] != f.n[2]) && (n[2] != f.n[3])) return false;
			if ((n[3] != f.n[0]) && (n[3] != f.n[1]) && (n[3] != f.n[2]) && (n[3] != f.n[3])) return false;

			if ((n[4] != f.n[4]) && (n[4] != f.n[5]) && (n[4] != f.n[6]) && (n[4] != f.n[7])) return false;
			if ((n[5] != f.n[4]) && (n[5] != f.n[5]) && (n[5] != f.n[6]) && (n[5] != f.n[7])) return false;
			if ((n[6] != f.n[4]) && (n[6] != f.n[5]) && (n[6] != f.n[6]) && (n[6] != f.n[7])) return false;
			if ((n[7] != f.n[4]) && (n[7] != f.n[5]) && (n[7] != f.n[6]) && (n[7] != f.n[7])) return false;
		}
		break;
	case FE_FACE_QUAD9:
		{
			if ((n[0] != f.n[0]) && (n[0] != f.n[1]) && (n[0] != f.n[2]) && (n[0] != f.n[3])) return false;
			if ((n[1] != f.n[0]) && (n[1] != f.n[1]) && (n[1] != f.n[2]) && (n[1] != f.n[3])) return false;
			if ((n[2] != f.n[0]) && (n[2] != f.n[1]) && (n[2] != f.n[2]) && (n[2] != f.n[3])) return false;
			if ((n[3] != f.n[0]) && (n[3] != f.n[1]) && (n[3] != f.n[2]) && (n[3] != f.n[3])) return false;

			if ((n[4] != f.n[4]) && (n[4] != f.n[5]) && (n[4] != f.n[6]) && (n[4] != f.n[7])) return false;
			if ((n[5] != f.n[4]) && (n[5] != f.n[5]) && (n[5] != f.n[6]) && (n[5] != f.n[7])) return false;
			if ((n[6] != f.n[4]) && (n[6] != f.n[5]) && (n[6] != f.n[6]) && (n[6] != f.n[7])) return false;
			if ((n[7] != f.n[4]) && (n[7] != f.n[5]) && (n[7] != f.n[6]) && (n[7] != f.n[7])) return false;

			if (n[8] != f.n[8]) return false;
		}
		break;
	case FE_FACE_TRI10:
		{
			if ((n[0] != f.n[0]) && (n[0] != f.n[1]) && (n[0] != f.n[2])) return false;
			if ((n[1] != f.n[0]) && (n[1] != f.n[1]) && (n[1] != f.n[2])) return false;
			if ((n[2] != f.n[0]) && (n[2] != f.n[1]) && (n[2] != f.n[2])) return false;

			if ((n[3] != f.n[3]) && (n[3] != f.n[4]) && (n[3] != f.n[5]) && (n[3] != f.n[6]) && (n[3] != f.n[7]) && (n[3] != f.n[8])) return false;
			if ((n[4] != f.n[3]) && (n[4] != f.n[4]) && (n[4] != f.n[5]) && (n[4] != f.n[6]) && (n[4] != f.n[7]) && (n[4] != f.n[8])) return false;
			if ((n[5] != f.n[3]) && (n[5] != f.n[4]) && (n[5] != f.n[5]) && (n[5] != f.n[6]) && (n[5] != f.n[7]) && (n[5] != f.n[8])) return false;
			if ((n[6] != f.n[3]) && (n[6] != f.n[4]) && (n[6] != f.n[5]) && (n[6] != f.n[6]) && (n[6] != f.n[7]) && (n[6] != f.n[8])) return false;
			if ((n[7] != f.n[3]) && (n[7] != f.n[4]) && (n[7] != f.n[5]) && (n[7] != f.n[6]) && (n[7] != f.n[7]) && (n[7] != f.n[8])) return false;
			if ((n[8] != f.n[3]) && (n[8] != f.n[4]) && (n[8] != f.n[5]) && (n[8] != f.n[6]) && (n[8] != f.n[7]) && (n[8] != f.n[8])) return false;

			if ((n[9] != f.n[9])) return false;
		}
		break;
	default:
		assert(false);
	}

	return true;
}

//-----------------------------------------------------------------------------
void FEFace::SetType(FEFaceType type)
{
	assert(type != FE_FACE_INVALID_TYPE);
	m_type = type;
}

//-----------------------------------------------------------------------------
int FEFace::Shape() const
{
	static int S[] {FE_FACE_INVALID_SHAPE, FE_FACE_TRI, FE_FACE_QUAD, FE_FACE_TRI, FE_FACE_TRI, FE_FACE_QUAD, FE_FACE_QUAD, FE_FACE_TRI};
	assert(m_type != FE_FACE_INVALID_TYPE);
	return S[m_type];
}

//-----------------------------------------------------------------------------
int FEFace::Nodes() const
{
	static int N[] = {0, 3, 4, 6, 7, 8, 9, 10};
	assert(m_type != FE_FACE_INVALID_TYPE);
	return N[m_type];
}

//-----------------------------------------------------------------------------
int FEFace::Edges() const
{
	static int E[] = {0, 3, 4, 3, 3, 4, 4, 3};
	assert(m_type != FE_FACE_INVALID_TYPE);
	return E[m_type];
}

//-----------------------------------------------------------------------------
bool FEFace::HasEdge(int n1, int n2)
{
	assert(m_type != FE_FACE_INVALID_TYPE);
	int shape = Shape();
	if (shape == FE_FACE_QUAD)
	{
		if (((n[0]==n1) && (n[1]==n2)) || ((n[1]==n1) && (n[0]==n2))) return true;
		if (((n[1]==n1) && (n[2]==n2)) || ((n[2]==n1) && (n[1]==n2))) return true;
		if (((n[2]==n1) && (n[3]==n2)) || ((n[3]==n1) && (n[2]==n2))) return true;
		if (((n[3]==n1) && (n[0]==n2)) || ((n[0]==n1) && (n[3]==n2))) return true;
	}
	else if (shape == FE_FACE_TRI)
	{
		if (((n[0]==n1) && (n[1]==n2)) || ((n[1]==n1) && (n[0]==n2))) return true;
		if (((n[1]==n1) && (n[2]==n2)) || ((n[2]==n1) && (n[1]==n2))) return true;
		if (((n[2]==n1) && (n[0]==n2)) || ((n[0]==n1) && (n[2]==n2))) return true;
	}
	else assert(false);
	return false;
}

//-----------------------------------------------------------------------------
int FEFace::FindEdge(const FEEdge& edge)
{
	FEEdge edgei;
	for (int i=0; i<Edges(); ++i)
	{
		edgei = GetEdge(i);
		if (edgei == edge) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// See if this face has node i
bool FEFace::HasNode(int i)
{
	assert(m_type != FE_FACE_INVALID_TYPE);
	switch (m_type)
	{
	case FE_FACE_TRI3 : return ((n[0]==i)||(n[1]==i)||(n[2]==i));
	case FE_FACE_QUAD4: return ((n[0]==i)||(n[1]==i)||(n[2]==i)||(n[3]==i));
	case FE_FACE_TRI6 : return ((n[0]==i)||(n[1]==i)||(n[2]==i)||(n[3]==i)||(n[4]==i)||(n[5]==i));
	case FE_FACE_TRI7 : return ((n[0]==i)||(n[1]==i)||(n[2]==i)||(n[3]==i)||(n[4]==i)||(n[5]==i)||(n[6]==i));
	case FE_FACE_QUAD8: return ((n[0]==i)||(n[1]==i)||(n[2]==i)||(n[3]==i)||(n[4]==i)||(n[5]==i)||(n[6]==i)||(n[7]==i));
	case FE_FACE_QUAD9: return ((n[0]==i)||(n[1]==i)||(n[2]==i)||(n[3]==i)||(n[4]==i)||(n[5]==i)||(n[6]==i)||(n[7]==i)||(n[8]==i));
	case FE_FACE_TRI10: return ((n[0]==i)||(n[1]==i)||(n[2]==i)||(n[3]==i)||(n[4]==i)||(n[5]==i)||(n[6]==i)||(n[7]==i)||(n[8]==i)||(n[9]==i));
	default:
		assert(false);
	}
	return false;
}

//-----------------------------------------------------------------------------
// Find the array index of node with ID i
int FEFace::FindNode(int i)
{
	assert(m_type != FE_FACE_INVALID_TYPE);
	if (i == -1) return -1;
	if (i == n[0]) return 0; 
	if (i == n[1]) return 1; 
	if (i == n[2]) return 2; 
	if (i == n[3]) return 3;
	if (i == n[4]) return 4; 
	if (i == n[5]) return 5; 
	if (i == n[6]) return 6; 
	if (i == n[7]) return 7;
	if (i == n[8]) return 8;
	if (i == n[9]) return 9;
	return -1;
}

//-----------------------------------------------------------------------------
int FEFace::GetEdgeNodes(int i, int* en) const
{
	assert(m_type != FE_FACE_INVALID_TYPE);
	int nn = 0;
	switch (m_type)
	{
	case FE_FACE_TRI3:
		nn = 2;
		if (i == 0) { en[0] = n[0]; en[1] = n[1]; en[2] = en[3] = -1; }
		if (i == 1) { en[0] = n[1]; en[1] = n[2]; en[2] = en[3] = -1; }
		if (i == 2) { en[0] = n[2]; en[1] = n[0]; en[2] = en[3] = -1; }
		break;
	case FE_FACE_QUAD4:
		nn = 2;
		if (i == 0) { en[0] = n[0]; en[1] = n[1]; en[2] = en[3] = -1; }
		if (i == 1) { en[0] = n[1]; en[1] = n[2]; en[2] = en[3] = -1; }
		if (i == 2) { en[0] = n[2]; en[1] = n[3]; en[2] = en[3] = -1; }
		if (i == 3) { en[0] = n[3]; en[1] = n[0]; en[2] = en[3] = -1; }
		break;
	case FE_FACE_TRI6:
		nn = 3;
		if (i == 0) { en[0] = n[0]; en[1] = n[1]; en[2] = n[3]; en[3] = -1; }
		if (i == 1) { en[0] = n[1]; en[1] = n[2]; en[2] = n[4]; en[3] = -1; }
		if (i == 2) { en[0] = n[2]; en[1] = n[0]; en[2] = n[5]; en[3] = -1; }
		break;
	case FE_FACE_TRI7:
		nn = 3;
		if (i == 0) { en[0] = n[0]; en[1] = n[1]; en[2] = n[3]; en[3] = -1; }
		if (i == 1) { en[0] = n[1]; en[1] = n[2]; en[2] = n[4]; en[3] = -1; }
		if (i == 2) { en[0] = n[2]; en[1] = n[0]; en[2] = n[5]; en[3] = -1; }
		break;
	case FE_FACE_QUAD8:
	case FE_FACE_QUAD9:
		nn = 3;
		if (i == 0) { en[0] = n[0]; en[1] = n[1]; en[2] = n[4]; en[3] = -1; }
		if (i == 1) { en[0] = n[1]; en[1] = n[2]; en[2] = n[5]; en[3] = -1; }
		if (i == 2) { en[0] = n[2]; en[1] = n[3]; en[2] = n[6]; en[3] = -1; }
		if (i == 3) { en[0] = n[3]; en[1] = n[0]; en[2] = n[7]; en[3] = -1; }
		break;
	case FE_FACE_TRI10:
		nn = 4;
		if (i == 0) { en[0] = n[0]; en[1] = n[1]; en[2] = n[3]; en[3] = n[4]; }
		if (i == 1) { en[0] = n[1]; en[1] = n[2]; en[2] = n[5]; en[3] = n[6]; }
		if (i == 2) { en[0] = n[2]; en[1] = n[0]; en[2] = n[8]; en[3] = n[7]; }
		break;
	default:
		assert(false);
	}
	return nn;
}

//-----------------------------------------------------------------------------
FEEdge FEFace::GetEdge(int i) const
{
	FEEdge edge;
	int n = GetEdgeNodes(i, edge.n);
	switch (n)
	{
	case 2: edge.m_type = FE_EDGE2; break;
	case 3: edge.m_type = FE_EDGE3; break;
	case 4: edge.m_type = FE_EDGE4; break;
	default:
		assert(false);
	}
	return edge;
}

//-----------------------------------------------------------------------------
//! Evaluate the shape function values at the iso-parametric point (r,s)
void FEFace::shape(double* H, double r, double s)
{
	switch (m_type)
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
double FEFace::eval(double* d, double r, double s)
{
	double H[FEFace::MAX_NODES];
	shape(H, r, s);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return a;
}

//-----------------------------------------------------------------------------
vec3f FEFace::eval(vec3f* d, double r, double s)
{
	double H[FEFace::MAX_NODES];
	shape(H, r, s);
	vec3f a(0, 0, 0);
	for (int i = 0; i<Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}

