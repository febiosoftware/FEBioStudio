/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "FEFace.h"
#include <assert.h>

//-----------------------------------------------------------------------------
FSFace::FSFace()
{
	m_type = FE_FACE_INVALID_TYPE;
	m_elem[0].eid = m_elem[1].eid = m_elem[2].eid = -1;
	m_elem[0].lid = m_elem[1].lid = m_elem[2].lid = -1;
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
bool FSFace::operator == (const FSFace& f) const
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
void FSFace::SetType(FEFaceType type)
{
	assert(type != FE_FACE_INVALID_TYPE);
	m_type = type;
}

//-----------------------------------------------------------------------------
int FSFace::Shape() const
{
	static int S[] {FE_FACE_INVALID_SHAPE, FE_FACE_TRI, FE_FACE_QUAD, FE_FACE_TRI, FE_FACE_TRI, FE_FACE_QUAD, FE_FACE_QUAD, FE_FACE_TRI};
	assert(m_type != FE_FACE_INVALID_TYPE);
	return S[m_type];
}

//-----------------------------------------------------------------------------
int FSFace::Nodes() const
{
	static int N[] = {0, 3, 4, 6, 7, 8, 9, 10};
	assert(m_type != FE_FACE_INVALID_TYPE);
	return N[m_type];
}

//-----------------------------------------------------------------------------
int FSFace::Edges() const
{
	static int E[] = {0, 3, 4, 3, 3, 4, 4, 3};
	assert(m_type != FE_FACE_INVALID_TYPE);
	return E[m_type];
}

//-----------------------------------------------------------------------------
bool FSFace::HasEdge(int n1, int n2)
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
int FSFace::FindEdge(const FSEdge& edge)
{
	FSEdge edgei;
	for (int i=0; i<Edges(); ++i)
	{
		edgei = GetEdge(i);
		if (edgei == edge) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// See if this face has node i
bool FSFace::HasNode(int i)
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
int FSFace::FindNode(int i)
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
int FSFace::GetEdgeNodes(int i, int* en) const
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
FSEdge FSFace::GetEdge(int i) const
{
	FSEdge edge;
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
void FSFace::shape(double* H, double r, double s)
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
//! Evaluate the derivatives of shape function values at the iso-parametric point (r,s)
void FSFace::shape_deriv(double* Hr, double* Hs, double r, double s)
{
    switch (m_type)
    {
        case FE_FACE_TRI3:
        {
            Hr[0] = -1; Hs[0] = -1;
            Hr[1] =  1; Hs[1] =  0;
            Hr[2] =  0; Hs[2] =  1;
        }
            break;
        case FE_FACE_QUAD4:
        {
            Hr[0] = -0.25*(1-s); Hs[0] = -0.25*(1-r);
            Hr[1] =  0.25*(1-s); Hs[1] = -0.25*(1+r);
            Hr[2] =  0.25*(1+s); Hs[2] =  0.25*(1+r);
            Hr[3] = -0.25*(1+s); Hs[3] =  0.25*(1-r);
        }
            break;
        case FE_FACE_TRI6:
        {
            Hr[0] = -3.0 + 4.0*r + 4.0*s;
            Hr[1] =  4.0*r - 1.0;
            Hr[2] =  0.0;
            Hr[3] =  4.0 - 8.0*r - 4.0*s;
            Hr[4] =  4.0*s;
            Hr[5] = -4.0*s;
            
            Hs[0] = -3.0 + 4.0*s + 4.0*r;
            Hs[1] =  0.0;
            Hs[2] =  4.0*s - 1.0;
            Hs[3] = -4.0*r;
            Hs[4] =  4.0*r;
            Hs[5] =  4.0 - 8.0*s - 4.0*r;
        }
            break;
        case FE_FACE_TRI7:
        {
            Hr[6] = 27.0*s*(1.0 - 2.0*r - s);
            Hr[0] = -3.0 + 4.0*r + 4.0*s     + Hr[6]/9.0;
            Hr[1] =  4.0*r - 1.0             + Hr[6]/9.0;
            Hr[2] =  0.0                     + Hr[6]/9.0;
            Hr[3] =  4.0 - 8.0*r - 4.0*s - 4.0*Hr[6]/9.0;
            Hr[4] =  4.0*s               - 4.0*Hr[6]/9.0;
            Hr[5] = -4.0*s               - 4.0*Hr[6]/9.0;
            
            Hs[6] = 27.0*r*(1.0 - r - 2.0*s);
            Hs[0] = -3.0 + 4.0*s + 4.0*r     + Hs[6]/9.0;
            Hs[1] =  0.0                     + Hs[6]/9.0;
            Hs[2] =  4.0*s - 1.0             + Hs[6]/9.0;
            Hs[3] = -4.0*r               - 4.0*Hs[6]/9.0;
            Hs[4] =  4.0*r               - 4.0*Hs[6]/9.0;
            Hs[5] =  4.0 - 8.0*s - 4.0*r - 4.0*Hs[6]/9.0;
        }
            break;
        case FE_FACE_QUAD8:
        {
            Hr[4] = -r*(1 - s);
            Hr[5] = 0.5*(1 - s*s);
            Hr[6] = -r*(1 + s);
            Hr[7] = -0.5*(1 - s*s);
            
            Hr[0] = -0.25*(1 - s) - 0.5*(Hr[4] + Hr[7]);
            Hr[1] =  0.25*(1 - s) - 0.5*(Hr[4] + Hr[5]);
            Hr[2] =  0.25*(1 + s) - 0.5*(Hr[5] + Hr[6]);
            Hr[3] = -0.25*(1 + s) - 0.5*(Hr[6] + Hr[7]);
            
            Hs[4] = -0.5*(1 - r*r);
            Hs[5] = -s*(1 + r);
            Hs[6] = 0.5*(1 - r*r);
            Hs[7] = -s*(1 - r);
            
            Hs[0] = -0.25*(1 - r) - 0.5*(Hs[4] + Hs[7]);
            Hs[1] = -0.25*(1 + r) - 0.5*(Hs[4] + Hs[5]);
            Hs[2] =  0.25*(1 + r) - 0.5*(Hs[5] + Hs[6]);
            Hs[3] =  0.25*(1 - r) - 0.5*(Hs[6] + Hs[7]);
            
        }
            break;
        case FE_FACE_QUAD9:
        {
            double R[3] = {0.5*r*(r-1.0), 0.5*r*(r+1.0), 1.0 - r*r};
            double S[3] = {0.5*s*(s-1.0), 0.5*s*(s+1.0), 1.0 - s*s};
            double DR[3] = {r-0.5, r+0.5, -2.0*r};
            double DS[3] = {s-0.5, s+0.5, -2.0*s};
            
            Hr[0] = DR[0]*S[0];
            Hr[1] = DR[1]*S[0];
            Hr[2] = DR[1]*S[1];
            Hr[3] = DR[0]*S[1];
            Hr[4] = DR[2]*S[0];
            Hr[5] = DR[1]*S[2];
            Hr[6] = DR[2]*S[1];
            Hr[7] = DR[0]*S[2];
            Hr[8] = DR[2]*S[2];
            
            Hs[0] = R[0]*DS[0];
            Hs[1] = R[1]*DS[0];
            Hs[2] = R[1]*DS[1];
            Hs[3] = R[0]*DS[1];
            Hs[4] = R[2]*DS[0];
            Hs[5] = R[1]*DS[2];
            Hs[6] = R[2]*DS[1];
            Hs[7] = R[0]*DS[2];
            Hs[8] = R[2]*DS[2];
        }
        default:
            assert(false);
    }
}

//-----------------------------------------------------------------------------
double FSFace::eval(double* d, double r, double s)
{
	double H[FSFace::MAX_NODES];
	shape(H, r, s);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return a;
}

//-----------------------------------------------------------------------------
vec3f FSFace::eval(vec3f* d, double r, double s)
{
	double H[FSFace::MAX_NODES];
	shape(H, r, s);
	vec3f a(0, 0, 0);
	for (int i = 0; i<Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}

//-----------------------------------------------------------------------------
vec3d FSFace::eval(vec3d* d, double r, double s)
{
	double H[FSFace::MAX_NODES];
	shape(H, r, s);
	vec3d a(0, 0, 0);
	for (int i = 0; i < Nodes(); ++i) a += d[i] * H[i];
	return a;
}

//-----------------------------------------------------------------------------
double FSFace::eval_deriv1(double* d, double r, double s)
{
    double Hr[FSFace::MAX_NODES], Hs[FSFace::MAX_NODES];
    shape_deriv(Hr, Hs, r, s);
    double a = 0.0;
    for (int i = 0; i<Nodes(); ++i) a += Hr[i] * d[i];
    return a;
}

//-----------------------------------------------------------------------------
double FSFace::eval_deriv2(double* d, double r, double s)
{
    double Hr[FSFace::MAX_NODES], Hs[FSFace::MAX_NODES];
    shape_deriv(Hr, Hs, r, s);
    double a = 0.0;
    for (int i = 0; i<Nodes(); ++i) a += Hs[i] * d[i];
    return a;
}

//-----------------------------------------------------------------------------
vec3d FSFace::eval_deriv1(vec3d* d, double r, double s)
{
    double Hr[FSFace::MAX_NODES], Hs[FSFace::MAX_NODES];
    shape_deriv(Hr, Hs, r, s);
    vec3d a(0, 0, 0);
    for (int i = 0; i<Nodes(); ++i) a += d[i] * Hr[i];
    return a;
}

//-----------------------------------------------------------------------------
vec3d FSFace::eval_deriv2(vec3d* d, double r, double s)
{
    double Hr[FSFace::MAX_NODES], Hs[FSFace::MAX_NODES];
    shape_deriv(Hr, Hs, r, s);
    vec3d a(0, 0, 0);
    for (int i = 0; i<Nodes(); ++i) a += d[i] * Hs[i];
    return a;
}

//-----------------------------------------------------------------------------
int FSFace::gauss(double* gr, double* gs, double* gw)
{
    int nint = 0;
    
    switch (m_type)
    {
        case FE_FACE_TRI3:
        {
            nint = 3;
            const double a = 1.0 / 6.0;
            const double b = 2.0 / 3.0;
            gr[0] = a; gs[0] = a; gw[0] = a;
            gr[1] = b; gs[1] = a; gw[1] = a;
            gr[2] = a; gs[2] = b; gw[2] = a;
        }
            break;
        case FE_FACE_QUAD4:
        {
            nint = 4;
            const double a = 1.0 / sqrt(3.0);
            gr[0] = -a; gs[0] = -a; gw[0] = 1;
            gr[1] =  a; gs[1] = -a; gw[1] = 1;
            gr[2] =  a; gs[2] =  a; gw[2] = 1;
            gr[3] = -a; gs[3] =  a; gw[3] = 1;
        }
            break;
        case FE_FACE_TRI6:
        {
            nint = 4;
            const double a = 1.0/3.0;
            const double b = 1.0/5.0;
            const double c = 3.0/5.0;
            gr[0] = a; gs[0] = a; gw[0] = -27.0/96.0;
            gr[1] = c; gs[1] = b; gw[1] =  25.0/96.0;
            gr[2] = b; gs[2] = b; gw[2] =  25.0/96.0;
            gr[3] = b; gs[3] = c; gw[3] =  25.0/96.0;
        }
            break;
        case FE_FACE_TRI7:
        {
            nint = 4;
            const double a = 1.0/3.0;
            const double b = 1.0/5.0;
            const double c = 3.0/5.0;
            gr[0] = a; gs[0] = a; gw[0] = -27.0/96.0;
            gr[1] = c; gs[1] = b; gw[1] =  25.0/96.0;
            gr[2] = b; gs[2] = b; gw[2] =  25.0/96.0;
            gr[3] = b; gs[3] = c; gw[3] =  25.0/96.0;
        }
            break;
        case FE_FACE_QUAD8:
        {
            nint = 9;
            const double a = sqrt(0.6);
            const double w1 = 25.0/81.0;
            const double w2 = 40.0/81.0;
            const double w3 = 64.0/81.0;
            gr[ 0] = -a; gs[ 0] = -a;  gw[ 0] = w1;
            gr[ 1] =  0; gs[ 1] = -a;  gw[ 1] = w2;
            gr[ 2] =  a; gs[ 2] = -a;  gw[ 2] = w1;
            gr[ 3] = -a; gs[ 3] =  0;  gw[ 3] = w2;
            gr[ 4] =  0; gs[ 4] =  0;  gw[ 4] = w3;
            gr[ 5] =  a; gs[ 5] =  0;  gw[ 5] = w2;
            gr[ 6] = -a; gs[ 6] =  a;  gw[ 6] = w1;
            gr[ 7] =  0; gs[ 7] =  a;  gw[ 7] = w2;
            gr[ 8] =  a; gs[ 8] =  a;  gw[ 8] = w1;
        }
            break;
        case FE_FACE_QUAD9:
        {
            nint = 9;
            const double a = sqrt(0.6);
            const double w1 = 25.0/81.0;
            const double w2 = 40.0/81.0;
            const double w3 = 64.0/81.0;
            gr[ 0] = -a; gs[ 0] = -a;  gw[ 0] = w1;
            gr[ 1] =  0; gs[ 1] = -a;  gw[ 1] = w2;
            gr[ 2] =  a; gs[ 2] = -a;  gw[ 2] = w1;
            gr[ 3] = -a; gs[ 3] =  0;  gw[ 3] = w2;
            gr[ 4] =  0; gs[ 4] =  0;  gw[ 4] = w3;
            gr[ 5] =  a; gs[ 5] =  0;  gw[ 5] = w2;
            gr[ 6] = -a; gs[ 6] =  a;  gw[ 6] = w1;
            gr[ 7] =  0; gs[ 7] =  a;  gw[ 7] = w2;
            gr[ 8] =  a; gs[ 8] =  a;  gw[ 8] = w1;
        }
        default:
            assert(false);
    }
    
    return nint;
}
