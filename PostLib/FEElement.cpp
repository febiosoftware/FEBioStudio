#include "stdafx.h"
#include "FEElement.h"
#include <MeshLib/tet4.h>
#include <MeshLib/penta6.h>
#include <MeshLib/penta15.h>
#include <MeshLib/hex8.h>
#include <MeshLib/pyra5.h>
#include <MeshLib/tet10.h>
#include <MeshLib/tet15.h>
#include <MeshLib/tet20.h>
#include <MeshLib/hex20.h>
#include <MeshLib/hex27.h>
#include <MeshLib/MeshMetrics.h>

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

vector<Post::ElemTraits> Post::FEElementLibrary::m_lib;

void Post::FEElementLibrary::addElement(int ntype, int nshape, int nclass, int nodes, int faces, int edges)
{
	Post::ElemTraits t = {ntype, nshape, nclass, nodes, faces, edges};
	m_lib.push_back(t);
}

void Post::FEElementLibrary::InitLibrary()
{
	m_lib.clear();

	// NOTE: When adding new elements make sure to set the faces to zero for shells, and the edges to zero for solids.
	addElement(FE_INVALID_ELEMENT_TYPE, 0, 0, 0, 0, 0);
	addElement(FE_HEX8   , Post::ELEM_HEX  , Post::ELEM_SOLID,  8, 6, 0);
	addElement(FE_TET4   , Post::ELEM_TET  , Post::ELEM_SOLID,  4, 4, 0);
	addElement(FE_PENTA6 , Post::ELEM_PENTA, Post::ELEM_SOLID,  6, 5, 0);
	addElement(FE_QUAD4  , Post::ELEM_QUAD , Post::ELEM_SHELL,  4, 0, 4);
	addElement(FE_TRI3   , Post::ELEM_TRI  , Post::ELEM_SHELL,  3, 0, 3);
	addElement(FE_BEAM2  , Post::ELEM_LINE , Post::ELEM_BEAM ,  2, 0, 0);
	addElement(FE_HEX20  , Post::ELEM_HEX  , Post::ELEM_SOLID, 20, 6, 0);
	addElement(FE_QUAD8  , Post::ELEM_QUAD , Post::ELEM_SHELL,  8, 0, 4);
	addElement(FE_BEAM3  , Post::ELEM_LINE , Post::ELEM_BEAM ,  3, 0, 0);
	addElement(FE_TET10  , Post::ELEM_TET  , Post::ELEM_SOLID, 10, 4, 0);
	addElement(FE_TRI6   , Post::ELEM_TRI  , Post::ELEM_SHELL,  6, 0, 3);
	addElement(FE_TET15  , Post::ELEM_TET  , Post::ELEM_SOLID, 15, 4, 0);
	addElement(FE_HEX27  , Post::ELEM_HEX  , Post::ELEM_SOLID, 27, 6, 0);
	addElement(FE_TRI7   , Post::ELEM_TRI  , Post::ELEM_SHELL,  7, 0, 3);
	addElement(FE_QUAD9  , Post::ELEM_QUAD , Post::ELEM_SHELL,  9, 0, 4);
	addElement(FE_PENTA15, Post::ELEM_PENTA, Post::ELEM_SOLID, 15, 5, 0);
	addElement(FE_PYRA5  , Post::ELEM_PYRA , Post::ELEM_SOLID,  5, 5, 0);
	addElement(FE_TET20  , Post::ELEM_TET  , Post::ELEM_SOLID, 20, 4, 0);
	addElement(FE_TRI10  , Post::ELEM_TRI  , Post::ELEM_SHELL, 10, 0, 3);
	addElement(FE_TET5   , Post::ELEM_TET  , Post::ELEM_SOLID,  5, 4, 0);
}

const Post::ElemTraits* Post::FEElementLibrary::GetTraits(FEElementType type)
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
Post::FEGenericElement::FEGenericElement()
{
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = -1; 
}

Post::FEGenericElement::FEGenericElement(const Post::FEGenericElement& e) : FEElement(e)
{
	m_traits = e.m_traits;
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

void Post::FEGenericElement::operator = (const Post::FEGenericElement& e)
{
	m_traits = e.m_traits;
	FEElement::operator = (e);
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

//=============================================================================
Post::FELinearElement::FELinearElement()
{
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = -1; 
}

Post::FELinearElement::FELinearElement(const Post::FELinearElement& e) : Post::FEElement(e)
{
	m_traits = e.m_traits;
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

void Post::FELinearElement::operator = (const Post::FELinearElement& e)
{
	m_traits = e.m_traits;
	FEElement::operator = (e);
	m_node = _node;
	for (int i=0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i]; 
}

//=============================================================================
// FEElement
//-----------------------------------------------------------------------------
Post::FEElement::FEElement()
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
void Post::FEElement::SetType(FEElementType type)
{
	m_traits = Post::FEElementLibrary::GetTraits(type);
	assert(m_traits);
}

//-----------------------------------------------------------------------------
bool Post::FEElement::HasNode(int node) const
{ 
	bool ret = false;
	const int n = Nodes();
	for (int i=0; i<n; i++) ret |= !(m_node[i] ^ node);
	return ret;
}

//-----------------------------------------------------------------------------
// Return a face of the element
Post::FEFace Post::FEElement::GetFace(int i) const
{
	Post::FEFace f;
	GetFace(i, f);
	return f;
}

//-----------------------------------------------------------------------------
// Return a face of the element
void Post::FEElement::GetFace(int i, Post::FEFace& f) const
{
	switch (Type())
	{
	case FE_HEX8:
		f.m_ntype = FE_FACE_QUAD4;
		f.node[0] = m_node[FTHEX8[i][0]];
		f.node[1] = m_node[FTHEX8[i][1]];
		f.node[2] = m_node[FTHEX8[i][2]];
		f.node[3] = m_node[FTHEX8[i][3]];
		break;
	case FE_PENTA6:
		{
			const int ft[5] = { FE_FACE_QUAD4, FE_FACE_QUAD4, FE_FACE_QUAD4, FE_FACE_TRI3, FE_FACE_TRI3};
			f.m_ntype = ft[i];
			f.node[0] = m_node[FTPENTA[i][0]];
			f.node[1] = m_node[FTPENTA[i][1]];
			f.node[2] = m_node[FTPENTA[i][2]];
			f.node[3] = m_node[FTPENTA[i][3]];
		}
		break;

	case FE_TET4:
	case FE_TET5:
		f.m_ntype = FE_FACE_TRI3;
		f.node[0] = m_node[FTTET[i][0]];
		f.node[1] = m_node[FTTET[i][1]];
		f.node[2] = m_node[FTTET[i][2]];
		f.node[3] = m_node[FTTET[i][3]];
		break;

	case FE_HEX20:
		f.m_ntype = FE_FACE_QUAD8;
		f.node[0] = m_node[FTHEX20[i][0]];
		f.node[1] = m_node[FTHEX20[i][1]];
		f.node[2] = m_node[FTHEX20[i][2]];
		f.node[3] = m_node[FTHEX20[i][3]];
		f.node[4] = m_node[FTHEX20[i][4]];
		f.node[5] = m_node[FTHEX20[i][5]];
		f.node[6] = m_node[FTHEX20[i][6]];
		f.node[7] = m_node[FTHEX20[i][7]];
		break;

	case FE_HEX27:
		f.m_ntype = FE_FACE_QUAD9;
		f.node[0] = m_node[FTHEX27[i][0]];
		f.node[1] = m_node[FTHEX27[i][1]];
		f.node[2] = m_node[FTHEX27[i][2]];
		f.node[3] = m_node[FTHEX27[i][3]];
		f.node[4] = m_node[FTHEX27[i][4]];
		f.node[5] = m_node[FTHEX27[i][5]];
		f.node[6] = m_node[FTHEX27[i][6]];
		f.node[7] = m_node[FTHEX27[i][7]];
		f.node[8] = m_node[FTHEX27[i][8]];
		break;

	case FE_TET10:
		f.m_ntype = FE_FACE_TRI6;
		f.node[0] = m_node[FTTET10[i][0]];
		f.node[1] = m_node[FTTET10[i][1]];
		f.node[2] = m_node[FTTET10[i][2]];
		f.node[3] = m_node[FTTET10[i][3]];
		f.node[4] = m_node[FTTET10[i][4]];
		f.node[5] = m_node[FTTET10[i][5]];
		break;

	case FE_TET15:
		f.m_ntype = FE_FACE_TRI7;
		f.node[0] = m_node[FTTET15[i][0]];
		f.node[1] = m_node[FTTET15[i][1]];
		f.node[2] = m_node[FTTET15[i][2]];
		f.node[3] = m_node[FTTET15[i][3]];
		f.node[4] = m_node[FTTET15[i][4]];
		f.node[5] = m_node[FTTET15[i][5]];
		f.node[6] = m_node[FTTET15[i][6]];
		break;

	case FE_TET20:
		f.m_ntype = FE_FACE_TRI10;
		f.node[0] = m_node[FTTET20[i][0]];
		f.node[1] = m_node[FTTET20[i][1]];
		f.node[2] = m_node[FTTET20[i][2]];
		f.node[3] = m_node[FTTET20[i][3]];
		f.node[4] = m_node[FTTET20[i][4]];
		f.node[5] = m_node[FTTET20[i][5]];
		f.node[6] = m_node[FTTET20[i][6]];
		f.node[7] = m_node[FTTET20[i][7]];
		f.node[8] = m_node[FTTET20[i][8]];
		f.node[9] = m_node[FTTET20[i][9]];
		break;

	case FE_PYRA5:
		{
			const int ft[5] = { FE_FACE_TRI3, FE_FACE_TRI3, FE_FACE_TRI3, FE_FACE_TRI3, FE_FACE_QUAD4 };
			f.m_ntype = ft[i];
			f.node[0] = m_node[FTPYRA5[i][0]];
			f.node[1] = m_node[FTPYRA5[i][1]];
			f.node[2] = m_node[FTPYRA5[i][2]];
			f.node[3] = m_node[FTPYRA5[i][3]];
		}
		break;
    
    case FE_PENTA15:
        {
            const int ft[5] = { FE_FACE_QUAD8, FE_FACE_QUAD8, FE_FACE_QUAD8, FE_FACE_TRI6, FE_FACE_TRI6};
            f.m_ntype = ft[i];
            f.node[0] = m_node[FTPENTA15[i][0]];
            f.node[1] = m_node[FTPENTA15[i][1]];
            f.node[2] = m_node[FTPENTA15[i][2]];
            f.node[3] = m_node[FTPENTA15[i][3]];
            f.node[4] = m_node[FTPENTA15[i][4]];
            f.node[5] = m_node[FTPENTA15[i][5]];
            f.node[6] = m_node[FTPENTA15[i][6]];
            f.node[7] = m_node[FTPENTA15[i][7]];
        }
		break;
    };
}

//-----------------------------------------------------------------------------
// Return an edge of the element
Post::FEEdge Post::FEElement::GetEdge(int i) const
{
	Post::FEEdge e;
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
bool Post::FEElement::IsExterior() const
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
bool Post::FEElement::operator != (Post::FEElement& e)
{
	if (Type() != e.Type()) return true;
	assert(Shape() == e.Shape());
	switch (Shape())
	{
	case Post::ELEM_HEX:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3]) ||
			(m_node[4] != e.m_node[4]) ||
			(m_node[5] != e.m_node[5]) ||
			(m_node[6] != e.m_node[6]) ||
			(m_node[7] != e.m_node[7])) return true;
		break;
	case Post::ELEM_PENTA:
            if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3]) ||
			(m_node[4] != e.m_node[4]) ||
			(m_node[5] != e.m_node[5])) return true;
		break;
	case Post::ELEM_PYRA:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3]) ||
			(m_node[4] != e.m_node[4])) return true;
		break;
	case Post::ELEM_TET:
	case Post::ELEM_QUAD:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2]) ||
			(m_node[3] != e.m_node[3])) return true;
		break;
	case Post::ELEM_TRI:
		if ((m_node[0] != e.m_node[0]) ||
			(m_node[1] != e.m_node[1]) ||
			(m_node[2] != e.m_node[2])) return true;
		break;
	}

	return false;
}

//-----------------------------------------------------------------------------
//! Calculate the shape function values at the point (r,s,t)
void Post::FEElement::shape(double *H, double r, double s, double t)
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
double Post::FEElement::eval(double* d, double r, double s, double t)
{
	double H[Post::FEGenericElement::MAX_NODES];
	shape(H, r, s, t);
	double a = 0.0;
	for (int i=0; i<Nodes(); ++i) a += H[i]*d[i];
	return a;
}

//-----------------------------------------------------------------------------
float Post::FEElement::eval(float* d, double r, double s, double t)
{
	double H[Post::FEGenericElement::MAX_NODES];
	shape(H, r, s, t);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return (float) a;
}

//-----------------------------------------------------------------------------
vec3f Post::FEElement::eval(vec3f* d, double r, double s, double t)
{
	double H[Post::FEGenericElement::MAX_NODES];
	shape(H, r, s, t);
	vec3f a(0,0,0);
	for (int i=0; i<Nodes(); ++i) a += d[i]*((float)H[i]);
	return a;
}

//-----------------------------------------------------------------------------
void Post::FEElement::shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
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
void Post::FEElement::iso_coord(int n, double q[3])
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

