// FEElement.cpp: implementation of the FEElement class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEElement.h"
#include "tet4.h"
#include "penta6.h"
#include "hex8.h"
#include "pyra5.h"
#include "tet10.h"
#include "tet15.h"
#include "tet20.h"
#include "hex20.h"
#include "hex27.h"
#include "penta15.h"
using namespace Post;

//-----------------------------------------------------------------------------
// Face lookup tables
int FT_HEX[6][4] = {
	{0, 1, 5, 4},
	{1, 2, 6, 5},
	{3, 0, 4, 7},
	{1, 0, 3, 2},
	{4, 5, 6, 7},
	{2, 3, 7, 6}};

int FT_PENTA[5][4] = {
	{0, 1, 4, 3},
	{1, 2, 5, 4},
	{2, 0, 3, 5},
	{2, 1, 0, 0},
	{3, 4, 5, 5}};

int FT_PYRA[5][4] = {
	{ 0, 1, 4, -1 },
	{ 1, 2, 4, -1 },
	{ 2, 3, 4, -1 },
	{ 3, 0, 4, -1 },
	{ 3, 2, 1, 0 }};

int FT_TET[4][4] = {
	{2, 1, 0, 0},
	{0, 1, 3, 3},
	{1, 2, 3, 3},
	{2, 0, 3, 3}};

int FT_HEX20[6][8] = {
	{0, 1, 5, 4,  8, 17, 12, 16},
	{1, 2, 6, 5,  9, 18, 13, 17},
	{2, 3, 7, 6, 10, 19, 14, 18},
	{3, 0, 4, 7, 11, 16, 15, 19},
	{3, 2, 1, 0, 10,  9,  8, 11},
	{4, 5, 6, 7, 12, 13, 14, 15}};

int FT_HEX27[6][9] = {
	{0, 1, 5, 4,  8, 17, 12, 16, 20},
	{1, 2, 6, 5,  9, 18, 13, 17, 21},
	{2, 3, 7, 6, 10, 19, 14, 18, 22},
	{3, 0, 4, 7, 11, 16, 15, 19, 23},
	{3, 2, 1, 0, 10,  9,  8, 11, 24},
	{4, 5, 6, 7, 12, 13, 14, 15, 25}};

int FT_TET10[4][6] = {
	{0, 1, 3, 4, 8, 7},
	{1, 2, 3, 5, 9, 8},
	{2, 0, 3, 6, 7, 9},
	{2, 1, 0, 5, 4, 6}};

int FT_TET15[4][7] = {
	{0, 1, 3, 4, 8, 7, 11},
	{1, 2, 3, 5, 9, 8, 12},
	{2, 0, 3, 6, 7, 9, 13},
	{2, 1, 0, 5, 4, 6, 10}};

int FT_TET20[4][10] = {
	{ 0, 1, 3, 4, 5, 12, 13, 10, 11, 16 },
	{ 1, 2, 3, 6, 7, 14, 15, 12, 13, 17 },
	{ 2, 0, 3, 9, 8, 10, 11, 14, 15, 18 },
	{ 2, 1, 0, 7, 6,  5,  4,  9,  8, 19 }};
	
int FT_PENTA15[5][8] = {
    {0, 1, 4, 3, 6, 13,  9, 12},
    {1, 2, 5, 4, 7, 14, 10, 13},
    {2, 0, 3, 5, 8, 12, 11, 14},
    {2, 1, 0, 7, 6,  8, -1, -1},
    {3, 4, 5, 9, 10, 11, -1, -1}};

const int ET_QUAD[4][2] = {
	{ 0, 1},
	{ 1, 2},
	{ 2, 3},
	{ 3, 0}};

const int ET_QUAD8[4][3] = {
    { 0, 1, 4},
    { 1, 2, 5},
    { 2, 3, 6},
    { 3, 0, 7}};

const int ET_TRI[3][2] = {
	{ 0, 1},
	{ 1, 2},
	{ 2, 3}};

const int ET_TRI6[3][3] = {
    { 0, 1, 3},
    { 1, 2, 4},
    { 2, 0, 5}};

//=============================================================================
// FEElementLibrary
//=============================================================================

vector<ElemTraits> FEElementLibrary::m_lib;

void FEElementLibrary::addElement(int ntype, int nshape, int nclass, int nodes, int faces, int edges)
{
	ElemTraits t = {ntype, nshape, nclass, nodes, faces, edges};
	m_lib.push_back(t);
}

void FEElementLibrary::InitLibrary()
{
	m_lib.clear();

	// NOTE: When adding new elements make sure to set the faces to zero for shells, and the edges to zero for solids.
	addElement(FE_LINE2  , ELEM_LINE , ELEM_BEAM ,  2, 0, 0);
	addElement(FE_LINE3  , ELEM_LINE , ELEM_BEAM ,  3, 0, 0);
	addElement(FE_TRI3   , ELEM_TRI  , ELEM_SHELL,  3, 0, 3);
	addElement(FE_TRI6   , ELEM_TRI  , ELEM_SHELL,  6, 0, 3);
	addElement(FE_QUAD4  , ELEM_QUAD , ELEM_SHELL,  4, 0, 4);
	addElement(FE_QUAD8  , ELEM_QUAD , ELEM_SHELL,  8, 0, 4);
	addElement(FE_QUAD9  , ELEM_QUAD , ELEM_SHELL,  9, 0, 4);
	addElement(FE_TET4   , ELEM_TET  , ELEM_SOLID,  4, 4, 0);
	addElement(FE_TET10  , ELEM_TET  , ELEM_SOLID, 10, 4, 0);
	addElement(FE_TET15  , ELEM_TET  , ELEM_SOLID, 15, 4, 0);
	addElement(FE_TET20  , ELEM_TET  , ELEM_SOLID, 20, 4, 0);
	addElement(FE_PENTA6 , ELEM_PENTA, ELEM_SOLID,  6, 5, 0);
	addElement(FE_PENTA15, ELEM_PENTA, ELEM_SOLID, 15, 5, 0);
	addElement(FE_HEX8   , ELEM_HEX  , ELEM_SOLID,  8, 6, 0);
	addElement(FE_HEX20  , ELEM_HEX  , ELEM_SOLID, 20, 6, 0);
	addElement(FE_HEX27  , ELEM_HEX  , ELEM_SOLID, 27, 6, 0);
	addElement(FE_PYRA5  , ELEM_PYRA , ELEM_SOLID,  5, 5, 0);
	addElement(FE_TET5   , ELEM_TET  , ELEM_SOLID,  5, 4, 0);
}

const ElemTraits* FEElementLibrary::GetTraits(FEElemType type)
{
	int ntype = (int) type;
	if ((ntype >= 0) && (ntype < m_lib.size()))
	{
		return &m_lib[ntype];
	}
	else
	{
		assert(false);
		return 0;
	}
}

//=============================================================================
FEGenericElement::FEGenericElement()
{
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = -1; 
}

FEGenericElement::FEGenericElement(const FEGenericElement& e) : FEElement(e)
{
	m_traits = e.m_traits;
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

void FEGenericElement::operator = (const FEGenericElement& e)
{
	m_traits = e.m_traits;
	FEElement::operator = (e);
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

//=============================================================================
FELinearElement::FELinearElement()
{
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = -1; 
}

FELinearElement::FELinearElement(const FELinearElement& e) : FEElement(e)
{
	m_traits = e.m_traits;
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

void FELinearElement::operator = (const FELinearElement& e)
{
	m_traits = e.m_traits;
	FEElement::operator = (e);
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

//=============================================================================
// FEElement
//-----------------------------------------------------------------------------
FEElement::FEElement()
{
	m_pElem[0] = 0; 
	m_pElem[1] = 0; 
	m_pElem[2] = 0; 
	m_pElem[3] = 0; 
	m_pElem[4] = 0; 
	m_pElem[5] = 0; 
	m_MatID = 0; 
	m_tex = 0.0f;
	m_traits = 0;
}

//-----------------------------------------------------------------------------
void FEElement::SetType(FEElemType type)
{
	m_traits = FEElementLibrary::GetTraits(type);
	assert(m_traits);
}

//-----------------------------------------------------------------------------
bool FEElement::HasNode(int node) const
{ 
	bool ret = false;
	const int n = Nodes();
	for (int i=0; i<n; i++) ret |= !(m_node[i] ^ node);
	return ret;
}

//-----------------------------------------------------------------------------
// Return a face of the element
FEFace FEElement::GetFace(int i) const
{
	FEFace f;
	GetFace(i, f);
	return f;
}

//-----------------------------------------------------------------------------
// Return a face of the element
void FEElement::GetFace(int i, FEFace& f) const
{
	switch (Type())
	{
	case FE_HEX8:
		f.m_ntype = FACE_QUAD4;
		f.node[0] = m_node[FT_HEX[i][0]];
		f.node[1] = m_node[FT_HEX[i][1]];
		f.node[2] = m_node[FT_HEX[i][2]];
		f.node[3] = m_node[FT_HEX[i][3]];
		break;
	case FE_PENTA6:
		{
			const int ft[5] = {FACE_QUAD4, FACE_QUAD4, FACE_QUAD4, FACE_TRI3, FACE_TRI3};
			f.m_ntype = ft[i];
			f.node[0] = m_node[FT_PENTA[i][0]];
			f.node[1] = m_node[FT_PENTA[i][1]];
			f.node[2] = m_node[FT_PENTA[i][2]];
			f.node[3] = m_node[FT_PENTA[i][3]];
		}
		break;

	case FE_TET4:
	case FE_TET5:
		f.m_ntype = FACE_TRI3;
		f.node[0] = m_node[FT_TET[i][0]];
		f.node[1] = m_node[FT_TET[i][1]];
		f.node[2] = m_node[FT_TET[i][2]];
		f.node[3] = m_node[FT_TET[i][3]];
		break;

	case FE_HEX20:
		f.m_ntype = FACE_QUAD8;
		f.node[0] = m_node[FT_HEX20[i][0]];
		f.node[1] = m_node[FT_HEX20[i][1]];
		f.node[2] = m_node[FT_HEX20[i][2]];
		f.node[3] = m_node[FT_HEX20[i][3]];
		f.node[4] = m_node[FT_HEX20[i][4]];
		f.node[5] = m_node[FT_HEX20[i][5]];
		f.node[6] = m_node[FT_HEX20[i][6]];
		f.node[7] = m_node[FT_HEX20[i][7]];
		break;

	case FE_HEX27:
		f.m_ntype = FACE_QUAD9;
		f.node[0] = m_node[FT_HEX27[i][0]];
		f.node[1] = m_node[FT_HEX27[i][1]];
		f.node[2] = m_node[FT_HEX27[i][2]];
		f.node[3] = m_node[FT_HEX27[i][3]];
		f.node[4] = m_node[FT_HEX27[i][4]];
		f.node[5] = m_node[FT_HEX27[i][5]];
		f.node[6] = m_node[FT_HEX27[i][6]];
		f.node[7] = m_node[FT_HEX27[i][7]];
		f.node[8] = m_node[FT_HEX27[i][8]];
		break;

	case FE_TET10:
		f.m_ntype = FACE_TRI6;
		f.node[0] = m_node[FT_TET10[i][0]];
		f.node[1] = m_node[FT_TET10[i][1]];
		f.node[2] = m_node[FT_TET10[i][2]];
		f.node[3] = m_node[FT_TET10[i][3]];
		f.node[4] = m_node[FT_TET10[i][4]];
		f.node[5] = m_node[FT_TET10[i][5]];
		break;

	case FE_TET15:
		f.m_ntype = FACE_TRI7;
		f.node[0] = m_node[FT_TET15[i][0]];
		f.node[1] = m_node[FT_TET15[i][1]];
		f.node[2] = m_node[FT_TET15[i][2]];
		f.node[3] = m_node[FT_TET15[i][3]];
		f.node[4] = m_node[FT_TET15[i][4]];
		f.node[5] = m_node[FT_TET15[i][5]];
		f.node[6] = m_node[FT_TET15[i][6]];
		break;

	case FE_TET20:
		f.m_ntype = FACE_TRI10;
		f.node[0] = m_node[FT_TET20[i][0]];
		f.node[1] = m_node[FT_TET20[i][1]];
		f.node[2] = m_node[FT_TET20[i][2]];
		f.node[3] = m_node[FT_TET20[i][3]];
		f.node[4] = m_node[FT_TET20[i][4]];
		f.node[5] = m_node[FT_TET20[i][5]];
		f.node[6] = m_node[FT_TET20[i][6]];
		f.node[7] = m_node[FT_TET20[i][7]];
		f.node[8] = m_node[FT_TET20[i][8]];
		f.node[9] = m_node[FT_TET20[i][9]];
		break;

	case FE_PYRA5:
		{
			const int ft[5] = { FACE_TRI3, FACE_TRI3, FACE_TRI3, FACE_TRI3, FACE_QUAD4 };
			f.m_ntype = ft[i];
			f.node[0] = m_node[FT_PYRA[i][0]];
			f.node[1] = m_node[FT_PYRA[i][1]];
			f.node[2] = m_node[FT_PYRA[i][2]];
			f.node[3] = m_node[FT_PYRA[i][3]];
		}
		break;
    
    case FE_PENTA15:
        {
            const int ft[5] = {FACE_QUAD8, FACE_QUAD8, FACE_QUAD8, FACE_TRI6, FACE_TRI6};
            f.m_ntype = ft[i];
            f.node[0] = m_node[FT_PENTA15[i][0]];
            f.node[1] = m_node[FT_PENTA15[i][1]];
            f.node[2] = m_node[FT_PENTA15[i][2]];
            f.node[3] = m_node[FT_PENTA15[i][3]];
            f.node[4] = m_node[FT_PENTA15[i][4]];
            f.node[5] = m_node[FT_PENTA15[i][5]];
            f.node[6] = m_node[FT_PENTA15[i][6]];
            f.node[7] = m_node[FT_PENTA15[i][7]];
        }
            break;
            
    };
}

//-----------------------------------------------------------------------------
// Return an edge of the element
FEEdge FEElement::GetEdge(int i) const
{
	FEEdge e;
	switch(Type())
	{
	case FE_QUAD4:
	case FE_TRI3:
		e.node[0] = m_node[ET_QUAD[i][0]];
		e.node[1] = m_node[ET_QUAD[i][1]];
		break;
    case FE_TRI6:
        e.node[0] = m_node[ET_TRI6[i][0]];
        e.node[1] = m_node[ET_TRI6[i][1]];
        e.node[2] = m_node[ET_TRI6[i][2]];
        break;
    case FE_QUAD8:
    case FE_QUAD9:
        e.node[0] = m_node[ET_QUAD8[i][0]];
        e.node[1] = m_node[ET_QUAD8[i][1]];
        e.node[2] = m_node[ET_QUAD8[i][2]];
        break;
	};
	return e;
}

//-----------------------------------------------------------------------------
// An element is exterior if it has at least one null neighbor or an invisible neighbor
bool FEElement::IsExterior() const
{
	// make sure the element is visible
	if (IsVisible() == false) return false;

	// get number of faces
	int NF = Faces();

	// a shell has 0 faces and is always exterior
	if (NF == 0) return true;

	// solid elements
	for (int i=0; i<NF; ++i)
	{
		FEElement* ei = m_pElem[i];
		if ((ei == 0) || (ei->IsVisible() == false)) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Check comparison between two elements
bool FEElement::operator != (FEElement& e)
{
	if (Type() != e.Type()) return true;
	assert(Shape() == e.Shape());
	switch (Shape())
	{
	case ELEM_HEX:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3]) ||
			(m_node[4] != e.m_node[4]) ||
			(m_node[5] != e.m_node[5]) ||
			(m_node[6] != e.m_node[6]) ||
			(m_node[7] != e.m_node[7])) return true;
		break;
	case ELEM_PENTA:
            if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3]) ||
			(m_node[4] != e.m_node[4]) ||
			(m_node[5] != e.m_node[5])) return true;
		break;
	case ELEM_PYRA:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3]) ||
			(m_node[4] != e.m_node[4])) return true;
		break;
	case ELEM_TET:
	case ELEM_QUAD:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3])) return true;
		break;
	case ELEM_TRI:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2])) return true;
		break;
	}

	return false;
}

//-----------------------------------------------------------------------------
//! Calculate the shape function values at the point (r,s,t)
void FEElement::shape(double *H, double r, double s, double t)
{
	switch (Type())
	{
	case FE_TET4   : TET4   ::shape(H, r, s, t); break;
	case FE_TET5   : TET5   ::shape(H, r, s, t); break;
	case FE_HEX8   : HEX8   ::shape(H, r, s, t); break;
	case FE_PENTA6 : PENTA6 ::shape(H, r, s, t); break;
	case FE_PYRA5  : PYRA5  ::shape(H, r, s, t); break;
	case FE_TET10  : TET10  ::shape(H, r, s, t); break;
	case FE_TET15  : TET15  ::shape(H, r, s, t); break;
	case FE_TET20  : TET20  ::shape(H, r, s, t); break;
	case FE_HEX20  : HEX20  ::shape(H, r, s, t); break;
	case FE_HEX27  : HEX27  ::shape(H, r, s, t); break;
	case FE_PENTA15: PENTA15::shape(H, r, s, t); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
double FEElement::eval(double* d, double r, double s, double t)
{
	double H[FEGenericElement::MAX_NODES];
	shape(H, r, s, t);
	double a = 0.0;
	for (int i=0; i<Nodes(); ++i) a += H[i]*d[i];
	return a;
}

//-----------------------------------------------------------------------------
float FEElement::eval(float* d, double r, double s, double t)
{
	double H[FEGenericElement::MAX_NODES];
	shape(H, r, s, t);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return (float) a;
}

//-----------------------------------------------------------------------------
vec3f FEElement::eval(vec3f* d, double r, double s, double t)
{
	double H[FEGenericElement::MAX_NODES];
	shape(H, r, s, t);
	vec3f a(0,0,0);
	for (int i=0; i<Nodes(); ++i) a += d[i]*((float)H[i]);
	return a;
}

//-----------------------------------------------------------------------------
void FEElement::shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
{
	switch (Type())
	{
	case FE_TET4   : TET4   ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_TET5   : TET5   ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_HEX8   : HEX8   ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_PENTA6 : PENTA6 ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_PYRA5  : PYRA5  ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_TET10  : TET10  ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_TET15  : TET15  ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_TET20  : TET20  ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_HEX20  : HEX20  ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_HEX27  : HEX27  ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
	case FE_PENTA15: PENTA15::shape_deriv(Hr, Hs, Ht, r, s, t); break;
    default:
		assert(false);
    }
}

//-----------------------------------------------------------------------------
void FEElement::iso_coord(int n, double q[3])
{
    // for n=-1 return isoparametric coordinates of element center
    
	assert((n>=-1)&&(n<Nodes()));
	switch (Type())
	{
	case FE_TET4   : TET4   ::iso_coord(n, q); break;
	case FE_TET5   : TET5   ::iso_coord(n, q); break;
	case FE_HEX8   : HEX8   ::iso_coord(n, q); break;
	case FE_PENTA6 : PENTA6 ::iso_coord(n, q); break;
	case FE_PYRA5  : PYRA5  ::iso_coord(n, q); break;
	case FE_TET10  : TET10  ::iso_coord(n, q); break;
	case FE_TET15  : TET15  ::iso_coord(n, q); break;
	case FE_TET20  : TET20  ::iso_coord(n, q); break;
	case FE_HEX20  : HEX20  ::iso_coord(n, q); break;
	case FE_HEX27  : HEX27  ::iso_coord(n, q); break;
	case FE_PENTA15: PENTA15::iso_coord(n, q); break;
	default:
		assert(false);
    }
}

