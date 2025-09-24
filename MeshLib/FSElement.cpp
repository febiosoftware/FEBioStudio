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

#include "FSElement.h"
#include "FSElementLibrary.h"
#include "tet.h"
#include "penta.h"
#include "hex.h"
#include "pyra.h"
#include "tri3.h"
#include "quad4.h"

//=============================================================================
// FSElement_
//-----------------------------------------------------------------------------
FSElement_::FSElement_()
{
	m_traits = nullptr;

	m_node = 0;
	m_nbr = 0;
	m_face = 0;
	m_h = 0;

	m_gid = 0;	// all elements need to be assigned to a partition

	m_lid = 0;
	m_MatID = 0;
	m_tex = 0.0f;

	m_Q.unit(); m_Qactive = false;
	m_a0 = 0;
}

//-----------------------------------------------------------------------------
// Set the element type. This also sets some other type related data
void FSElement_::SetType(int ntype)
{
	m_traits = FSElementLibrary::GetTraits(ntype);
	assert(m_traits);
}

//-----------------------------------------------------------------------------
// Check comparison between two elements
bool FSElement_::operator != (FSElement_& e)
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
bool FSElement_::HasNode(int node) const
{
	bool ret = false;
	const int n = Nodes();
	for (int i = 0; i<n; i++) ret |= !(m_node[i] ^ node);
	return ret;
}

//-----------------------------------------------------------------------------
FSFace FSElement_::GetFace(int i) const
{
	FSFace face;
	GetFace(i, face);
	return face;
}

//-----------------------------------------------------------------------------
int FSElement_::GetLocalFaceIndices(int i, int* n) const
{
	int nodes = -1;
	switch (Type())
	{
	case FE_HEX8:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 5; n[3] = 4; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 6; n[3] = 5; break;
		case 2: n[0] = 2; n[1] = 3; n[2] = 7; n[3] = 6; break;
		case 3: n[0] = 3; n[1] = 0; n[2] = 4; n[3] = 7; break;
		case 4: n[0] = 3; n[1] = 2; n[2] = 1; n[3] = 0; break;
		case 5: n[0] = 4; n[1] = 5; n[2] = 6; n[3] = 7; break;
		}
		nodes = 4;
		break;
	case FE_HEX20:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 5; n[3] = 4; n[4] =  8; n[5] = 17; n[6] = 12; n[7] = 16; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 6; n[3] = 5; n[4] =  9; n[5] = 18; n[6] = 13; n[7] = 17; break;
		case 2: n[0] = 2; n[1] = 3; n[2] = 7; n[3] = 6; n[4] = 10; n[5] = 19; n[6] = 14; n[7] = 18; break;
		case 3: n[0] = 3; n[1] = 0; n[2] = 4; n[3] = 7; n[4] = 11; n[5] = 16; n[6] = 15; n[7] = 19; break;
		case 4: n[0] = 3; n[1] = 2; n[2] = 1; n[3] = 0; n[4] = 10; n[5] =  9; n[6] =  8; n[7] = 11; break;
		case 5: n[0] = 4; n[1] = 5; n[2] = 6; n[3] = 7; n[4] = 12; n[5] = 13; n[6] = 14; n[7] = 15; break;
		}
		nodes = 8;
		break;
	case FE_HEX27:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 5; n[3] = 4; n[4] =  8; n[5] = 17; n[6] = 12; n[7] = 16; n[8] = 20; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 6; n[3] = 5; n[4] =  9; n[5] = 18; n[6] = 13; n[7] = 17; n[8] = 21; break;
		case 2: n[0] = 2; n[1] = 3; n[2] = 7; n[3] = 6; n[4] = 10; n[5] = 19; n[6] = 14; n[7] = 18; n[8] = 22; break;
		case 3: n[0] = 3; n[1] = 0; n[2] = 4; n[3] = 7; n[4] = 11; n[5] = 16; n[6] = 15; n[7] = 19; n[8] = 23; break;
		case 4: n[0] = 3; n[1] = 2; n[2] = 1; n[3] = 0; n[4] = 10; n[5] =  9; n[6] =  8; n[7] = 11; n[8] = 24; break;
		case 5: n[0] = 4; n[1] = 5; n[2] = 6; n[3] = 7; n[4] = 12; n[5] = 13; n[6] = 14; n[7] = 15; n[8] = 25; break;
		}
		nodes = 9;
		break;
	case FE_PENTA6:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 4; n[3] = 3; nodes = 4; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 5; n[3] = 4; nodes = 4; break;
		case 2: n[0] = 0; n[1] = 3; n[2] = 5; n[3] = 2; nodes = 4; break;
		case 3: n[0] = 0; n[1] = 2; n[2] = 1; nodes = 3; break;
		case 4: n[0] = 3; n[1] = 4; n[2] = 5; nodes = 3; break;
		}
		break;
	case FE_PYRA5:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 4; nodes = 3; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 4; nodes = 3; break;
		case 2: n[0] = 2; n[1] = 3; n[2] = 4; nodes = 3; break;
		case 3: n[0] = 3; n[1] = 0; n[2] = 4; nodes = 3; break;
		case 4: n[0] = 3; n[1] = 2; n[2] = 1; n[3] = 0; nodes = 4; break;
		}
		break;
    case FE_PYRA13:
        switch (i)
        {
            case 0: n[0] = 0; n[1] = 1; n[2] = 4; n[3] = 5; n[4] = 10; n[5] =  9; nodes = 6; break;
            case 1: n[0] = 1; n[1] = 2; n[2] = 4; n[3] = 6; n[4] = 11; n[5] = 10; nodes = 6; break;
            case 2: n[0] = 2; n[1] = 3; n[2] = 4; n[3] = 7; n[4] = 12; n[5] = 11; nodes = 6; break;
            case 3: n[0] = 3; n[1] = 0; n[2] = 4; n[3] = 8; n[4] =  9; n[5] = 12; nodes = 6; break;
            case 4: n[0] = 3; n[1] = 2; n[2] = 1; n[3] = 0; n[4] =  7; n[5] =  6; n[6] =  5; n[7] =  8; nodes = 8; break;
        }
        break;
	case FE_PENTA15:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 4; n[3] = 3; n[4] =  6; n[5] = 13; n[6] =  9; n[7] = 12; nodes = 8; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 5; n[3] = 4; n[4] =  7; n[5] = 14; n[6] = 10; n[7] = 13; nodes = 8; break;
		case 2: n[0] = 0; n[1] = 3; n[2] = 5; n[3] = 2; n[4] = 12; n[5] = 11; n[6] = 14; n[7] =  8; nodes = 8; break;
		case 3: n[0] = 0; n[1] = 2; n[2] = 1; n[3] = 8; n[4] =  7; n[5] =  6; n[6] = n[7] = -1; nodes = 6; break;
		case 4: n[0] = 3; n[1] = 4; n[2] = 5; n[3] = 9; n[4] = 10; n[5] = 11; n[6] = n[7] = -1; nodes = 6; break;
		}
		break;
	case FE_TET4:
	case FE_TET5:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = n[3] = 3; n[4] = n[5] = n[6] = n[7] = -1; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = n[3] = 3; n[4] = n[5] = n[6] = n[7] = -1; break;
		case 2: n[0] = 0; n[1] = 3; n[2] = n[3] = 2; n[4] = n[5] = n[6] = n[7] = -1; break;
		case 3: n[0] = 0; n[1] = 2; n[2] = n[3] = 1; n[4] = n[5] = n[6] = n[7] = -1; break;
		}
		nodes = 3;
		break;
	case FE_TET10:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 3; n[3] = 4; n[4] = 8; n[5] = 7; n[6] = n[7] = -1; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 3; n[3] = 5; n[4] = 9; n[5] = 8; n[6] = n[7] = -1; break;
		case 2: n[0] = 2; n[1] = 0; n[2] = 3; n[3] = 6; n[4] = 7; n[5] = 9; n[6] = n[7] = -1; break;
		case 3: n[0] = 2; n[1] = 1; n[2] = 0; n[3] = 5; n[4] = 4; n[5] = 6; n[6] = n[7] = -1; break;
		}
		nodes = 6;
		break;
	case FE_TET15:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 3; n[3] = 4; n[4] = 8; n[5] = 7; n[6] = 11; n[7] = -1; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 3; n[3] = 5; n[4] = 9; n[5] = 8; n[6] = 12; n[7] = -1; break;
		case 2: n[0] = 2; n[1] = 0; n[2] = 3; n[3] = 6; n[4] = 7; n[5] = 9; n[6] = 13; n[7] = -1; break;
		case 3: n[0] = 2; n[1] = 1; n[2] = 0; n[3] = 5; n[4] = 4; n[5] = 6; n[6] = 10; n[7] = -1; break;
		}
		nodes = 7;
		break;
	case FE_TET20:
		switch (i)
		{
		case 0: n[0] = 0; n[1] = 1; n[2] = 3; n[3] = 4; n[4] = 5; n[5] = 12; n[6] = 13; n[7] = 10; n[8] = 11; n[9] = 16; break;
		case 1: n[0] = 1; n[1] = 2; n[2] = 3; n[3] = 6; n[4] = 7; n[5] = 14; n[6] = 15; n[7] = 12; n[8] = 13; n[9] = 17; break;
		case 2: n[0] = 2; n[1] = 0; n[2] = 3; n[3] = 9; n[4] = 8; n[5] = 10; n[6] = 11; n[7] = 14; n[8] = 15; n[9] = 18; break;
		case 3: n[0] = 2; n[1] = 1; n[2] = 0; n[3] = 7; n[4] = 6; n[5] =  5; n[6] =  4; n[7] =  9; n[8] =  8; n[9] = 19; break;
		}
		nodes = 10;
		break;
	case FE_TRI3:
		nodes = 3;
		n[0] = 0; n[1] = 1; n[2] = 2;
		break;
	case FE_TRI6:
		nodes = 6;
		n[0] = 0; n[1] = 1; n[2] = 2;
		n[3] = 3; n[4] = 4; n[5] = 5;
		break;
	case FE_TRI7:
		nodes = 7;
		n[0] = 0; n[1] = 1; n[2] = 2;
		n[3] = 3; n[4] = 4; n[5] = 5;
		n[6] = 6;
		break;
	case FE_TRI10:
		nodes = 10;
		n[0] = 0; n[1] = 1; n[2] = 2; n[3] = 3; n[4] = 4; 
		n[5] = 5; n[6] = 6; n[7] = 7; n[8] = 8; n[9] = 9;
		break;
	case FE_QUAD4:
		nodes = 4;
		n[0] = 0; n[1] = 1; n[2] = 2; n[3] = 3;
		break;
	case FE_QUAD8:
		nodes = 8;
		n[0] = 0; n[1] = 1; n[2] = 2; n[3] = 3;
		n[4] = 4; n[5] = 5; n[6] = 6; n[7] = 7;
		break;
	case FE_QUAD9:
		nodes = 9;
		n[0] = 0; n[1] = 1; n[2] = 2; n[3] = 3; n[4] = 4; n[5] = 5; n[6] = 6; n[7] = 7; n[8] = 8;
		break;
	}

	assert(nodes > 0);
	return nodes;
}

//-----------------------------------------------------------------------------
void FSElement_::GetFace(int i, FSFace& f) const
{
	int* m = m_node;
	int* n = f.n;
	switch (Type())
	{
	case FE_HEX8:
		f.SetType(FE_FACE_QUAD4);
		switch (i)
		{
		case 0: n[0] = m[0]; n[1] = m[1]; n[2] = m[5]; n[3] = m[4]; break;
		case 1: n[0] = m[1]; n[1] = m[2]; n[2] = m[6]; n[3] = m[5]; break;
		case 2: n[0] = m[2]; n[1] = m[3]; n[2] = m[7]; n[3] = m[6]; break;
		case 3: n[0] = m[3]; n[1] = m[0]; n[2] = m[4]; n[3] = m[7]; break;
		case 4: n[0] = m[3]; n[1] = m[2]; n[2] = m[1]; n[3] = m[0]; break;
		case 5: n[0] = m[4]; n[1] = m[5]; n[2] = m[6]; n[3] = m[7]; break;
		}
		break;
	case FE_HEX20:
		f.SetType(FE_FACE_QUAD8);
		switch (i)
		{
		case 0: n[0] = m[0]; n[1] = m[1]; n[2] = m[5]; n[3] = m[4]; n[4] = m[ 8]; n[5] = m[17]; n[6] = m[12]; n[7] = m[16]; break;
		case 1: n[0] = m[1]; n[1] = m[2]; n[2] = m[6]; n[3] = m[5]; n[4] = m[ 9]; n[5] = m[18]; n[6] = m[13]; n[7] = m[17]; break;
		case 2: n[0] = m[2]; n[1] = m[3]; n[2] = m[7]; n[3] = m[6]; n[4] = m[10]; n[5] = m[19]; n[6] = m[14]; n[7] = m[18]; break;
		case 3: n[0] = m[3]; n[1] = m[0]; n[2] = m[4]; n[3] = m[7]; n[4] = m[11]; n[5] = m[16]; n[6] = m[15]; n[7] = m[19]; break;
		case 4: n[0] = m[3]; n[1] = m[2]; n[2] = m[1]; n[3] = m[0]; n[4] = m[10]; n[5] = m[ 9]; n[6] = m[ 8]; n[7] = m[11]; break;
		case 5: n[0] = m[4]; n[1] = m[5]; n[2] = m[6]; n[3] = m[7]; n[4] = m[12]; n[5] = m[13]; n[6] = m[14]; n[7] = m[15]; break;
		}
		break;
	case FE_HEX27:
		f.SetType(FE_FACE_QUAD9);
		switch (i)
		{
		case 0: n[0] = m[0]; n[1] = m[1]; n[2] = m[5]; n[3] = m[4]; n[4] = m[ 8]; n[5] = m[17]; n[6] = m[12]; n[7] = m[16]; n[8] = m[20]; break;
		case 1: n[0] = m[1]; n[1] = m[2]; n[2] = m[6]; n[3] = m[5]; n[4] = m[ 9]; n[5] = m[18]; n[6] = m[13]; n[7] = m[17]; n[8] = m[21]; break;
		case 2: n[0] = m[2]; n[1] = m[3]; n[2] = m[7]; n[3] = m[6]; n[4] = m[10]; n[5] = m[19]; n[6] = m[14]; n[7] = m[18]; n[8] = m[22]; break;
		case 3: n[0] = m[3]; n[1] = m[0]; n[2] = m[4]; n[3] = m[7]; n[4] = m[11]; n[5] = m[16]; n[6] = m[15]; n[7] = m[19]; n[8] = m[23]; break;
		case 4: n[0] = m[3]; n[1] = m[2]; n[2] = m[1]; n[3] = m[0]; n[4] = m[10]; n[5] = m[ 9]; n[6] = m[ 8]; n[7] = m[11]; n[8] = m[24]; break;
		case 5: n[0] = m[4]; n[1] = m[5]; n[2] = m[6]; n[3] = m[7]; n[4] = m[12]; n[5] = m[13]; n[6] = m[14]; n[7] = m[15]; n[8] = m[25]; break;
		}
		break;
	case FE_PENTA6:
		switch(i)
		{
		case 0: f.SetType(FE_FACE_QUAD4); n[0] = m[0]; n[1] = m[1]; n[2] = m[4]; n[3] = m[3]; break;
		case 1: f.SetType(FE_FACE_QUAD4); n[0] = m[1]; n[1] = m[2]; n[2] = m[5]; n[3] = m[4]; break;
		case 2: f.SetType(FE_FACE_QUAD4); n[0] = m[0]; n[1] = m[3]; n[2] = m[5]; n[3] = m[2]; break;
		case 3: f.SetType(FE_FACE_TRI3 ); n[0] = m[0]; n[1] = m[2]; n[2] = m[1]; break;
		case 4: f.SetType(FE_FACE_TRI3 ); n[0] = m[3]; n[1] = m[4]; n[2] = m[5]; break;
		}
		break;
	case FE_PYRA5:
		switch (i)
		{
		case 0: f.SetType(FE_FACE_TRI3 ); n[0] = m[0]; n[1] = m[1]; n[2] = m[4]; break;
		case 1: f.SetType(FE_FACE_TRI3 ); n[0] = m[1]; n[1] = m[2]; n[2] = m[4]; break;
		case 2: f.SetType(FE_FACE_TRI3 ); n[0] = m[2]; n[1] = m[3]; n[2] = m[4]; break;
		case 3: f.SetType(FE_FACE_TRI3 ); n[0] = m[3]; n[1] = m[0]; n[2] = m[4]; break;
		case 4: f.SetType(FE_FACE_QUAD4); n[0] = m[3]; n[1] = m[2]; n[2] = m[1]; n[3] = m[0]; break;
		}
		break;
    case FE_PYRA13:
        switch (i)
        {
            case 0: f.SetType(FE_FACE_TRI6 ); n[0] = m[0]; n[1] = m[1]; n[2] = m[4]; n[3] = m[5]; n[4] = m[10]; n[5] = m[9]; break;
            case 1: f.SetType(FE_FACE_TRI6 ); n[0] = m[1]; n[1] = m[2]; n[2] = m[4]; n[3] = m[6]; n[4] = m[11]; n[5] = m[10]; break;
            case 2: f.SetType(FE_FACE_TRI6 ); n[0] = m[2]; n[1] = m[3]; n[2] = m[4]; n[3] = m[7]; n[4] = m[12]; n[5] = m[11]; break;
            case 3: f.SetType(FE_FACE_TRI6 ); n[0] = m[3]; n[1] = m[0]; n[2] = m[4]; n[3] = m[8]; n[4] = m[ 9]; n[5] = m[12]; break;
            case 4: f.SetType(FE_FACE_QUAD8); n[0] = m[3]; n[1] = m[2]; n[2] = m[1]; n[3] = m[0]; n[4] = m[7]; n[5] = m[6]; n[6] = m[5]; n[7] = m[8]; break;
        }
        break;
	case FE_PENTA15:
		switch (i)
		{
		case 0: f.SetType(FE_FACE_QUAD8); n[0] = m[0]; n[1] = m[1]; n[2] = m[4]; n[3] = m[3]; n[4] = m[ 6]; n[5] = m[13]; n[6] = m[ 9]; n[7] = m[12]; break;
		case 1: f.SetType(FE_FACE_QUAD8); n[0] = m[1]; n[1] = m[2]; n[2] = m[5]; n[3] = m[4]; n[4] = m[ 7]; n[5] = m[14]; n[6] = m[10]; n[7] = m[13]; break;
		case 2: f.SetType(FE_FACE_QUAD8); n[0] = m[0]; n[1] = m[3]; n[2] = m[5]; n[3] = m[2]; n[4] = m[12]; n[5] = m[11]; n[6] = m[14]; n[7] = m[ 8]; break;
		case 3: f.SetType(FE_FACE_TRI6 ); n[0] = m[0]; n[1] = m[2]; n[2] = m[1]; n[3] = m[8]; n[4] = m[ 7]; n[5] = m[ 6]; n[6] = n[7] = -1; break;
		case 4: f.SetType(FE_FACE_TRI6 ); n[0] = m[3]; n[1] = m[4]; n[2] = m[5]; n[3] = m[9]; n[4] = m[10]; n[5] = m[11]; n[6] = n[7] = -1; break;
		}
	break;
	case FE_TET4:
	case FE_TET5:
		f.SetType(FE_FACE_TRI3);
		switch (i)
		{
		case 0: n[0] = m[0]; n[1] = m[1]; n[2] = n[3] = m[3]; n[4] = n[5] = n[6] = n[7] = -1; break;
		case 1: n[0] = m[1]; n[1] = m[2]; n[2] = n[3] = m[3]; n[4] = n[5] = n[6] = n[7] = -1; break;
		case 2: n[0] = m[0]; n[1] = m[3]; n[2] = n[3] = m[2]; n[4] = n[5] = n[6] = n[7] = -1; break;
		case 3: n[0] = m[0]; n[1] = m[2]; n[2] = n[3] = m[1]; n[4] = n[5] = n[6] = n[7] = -1; break;
		}
		break;
	case FE_TET10:
		f.SetType(FE_FACE_TRI6);
		switch (i)
		{
		case 0: n[0] = m[0]; n[1] = m[1]; n[2] = m[3]; n[3] = m[4]; n[4] = m[8]; n[5] = m[7]; n[6] = n[7] = -1; break;
		case 1: n[0] = m[1]; n[1] = m[2]; n[2] = m[3]; n[3] = m[5]; n[4] = m[9]; n[5] = m[8]; n[6] = n[7] = -1; break;
		case 2: n[0] = m[2]; n[1] = m[0]; n[2] = m[3]; n[3] = m[6]; n[4] = m[7]; n[5] = m[9]; n[6] = n[7] = -1; break;
		case 3: n[0] = m[2]; n[1] = m[1]; n[2] = m[0]; n[3] = m[5]; n[4] = m[4]; n[5] = m[6]; n[6] = n[7] = -1; break;
		}
		break;
	case FE_TET15:
		f.SetType(FE_FACE_TRI7);
		switch (i)
		{
		case 0: n[0] = m[0]; n[1] = m[1]; n[2] = m[3]; n[3] = m[4]; n[4] = m[8]; n[5] = m[7]; n[6] = m[11]; n[7] = -1; break;
		case 1: n[0] = m[1]; n[1] = m[2]; n[2] = m[3]; n[3] = m[5]; n[4] = m[9]; n[5] = m[8]; n[6] = m[12]; n[7] = -1; break;
		case 2: n[0] = m[2]; n[1] = m[0]; n[2] = m[3]; n[3] = m[6]; n[4] = m[7]; n[5] = m[9]; n[6] = m[13]; n[7] = -1; break;
		case 3: n[0] = m[2]; n[1] = m[1]; n[2] = m[0]; n[3] = m[5]; n[4] = m[4]; n[5] = m[6]; n[6] = m[10]; n[7] = -1; break;
		}
		break;
	case FE_TET20:
		f.SetType(FE_FACE_TRI10);
		switch(i)
		{
		case 0: n[0] = m[0]; n[1] = m[1]; n[2] = m[3]; n[3] = m[4]; n[4] = m[5]; n[5] = m[12]; n[6] = m[13]; n[7] = m[10]; n[8] = m[11]; n[9] = m[16]; break;
		case 1: n[0] = m[1]; n[1] = m[2]; n[2] = m[3]; n[3] = m[6]; n[4] = m[7]; n[5] = m[14]; n[6] = m[15]; n[7] = m[12]; n[8] = m[13]; n[9] = m[17]; break;
		case 2: n[0] = m[2]; n[1] = m[0]; n[2] = m[3]; n[3] = m[9]; n[4] = m[8]; n[5] = m[10]; n[6] = m[11]; n[7] = m[14]; n[8] = m[15]; n[9] = m[18]; break;
		case 3: n[0] = m[2]; n[1] = m[1]; n[2] = m[0]; n[3] = m[7]; n[4] = m[6]; n[5] = m[ 5]; n[6] = m[ 4]; n[7] = m[ 9]; n[8] = m[ 8]; n[9] = m[19]; break;
		}
		break;
	}
}

//-----------------------------------------------------------------------------
FSEdge FSElement_::GetEdge(int i) const
{
	FSEdge e;

	switch (Type())
	{
	case FE_QUAD4:
		e.SetType(FE_EDGE2);
		switch (i)
		{
		case 0: e.n[0] = m_node[0]; e.n[1] = m_node[1]; break;
		case 1: e.n[0] = m_node[1]; e.n[1] = m_node[2]; break;
		case 2: e.n[0] = m_node[2]; e.n[1] = m_node[3]; break;
		case 3: e.n[0] = m_node[3]; e.n[1] = m_node[0]; break;
		}
		break;
	case FE_TRI3:
		e.SetType(FE_EDGE2);
		switch (i)
		{
		case 0: e.n[0] = m_node[0]; e.n[1] = m_node[1]; break;
		case 1: e.n[0] = m_node[1]; e.n[1] = m_node[2]; break;
		case 2: e.n[0] = m_node[2]; e.n[1] = m_node[0]; break;
		}
		break;
    case FE_TRI6:
		e.SetType(FE_EDGE3);
		switch (i)
        {
        case 0: e.n[0] = m_node[0]; e.n[1] = m_node[1]; e.n[2] = m_node[3]; break;
        case 1: e.n[0] = m_node[1]; e.n[1] = m_node[2]; e.n[2] = m_node[4]; break;
        case 2: e.n[0] = m_node[2]; e.n[1] = m_node[0]; e.n[2] = m_node[5]; break;
        }
        break;
	case FE_QUAD8:
	case FE_QUAD9:
		e.SetType(FE_EDGE3);
		switch (i)
		{
		case 0: e.n[0] = m_node[0]; e.n[1] = m_node[1]; e.n[2] = m_node[4]; break;
		case 1: e.n[0] = m_node[1]; e.n[1] = m_node[2]; e.n[2] = m_node[5]; break;
		case 2: e.n[0] = m_node[2]; e.n[1] = m_node[3]; e.n[2] = m_node[6]; break;
		case 3: e.n[0] = m_node[3]; e.n[1] = m_node[0]; e.n[2] = m_node[7]; break;
		}
		break;
	default:
		assert(false);
	}
	assert(e.Type() != FE_EDGE_INVALID);
	return e;
}

//-----------------------------------------------------------------------------
//! Find the edge index of a shell
int FSElement_::FindEdge(const FSEdge& edge) const
{
	assert(IsShell());
	int nbre = Edges();
	for (int l = 0; l < nbre; l++)
	{
		if (edge == GetEdge(l))
		{
			return l;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
void FSElement_::GetShellFace(FSFace& f) const
{
	switch (Type())
	{
	case FE_QUAD4: f.SetType(FE_FACE_QUAD4); f.n[0] = m_node[0]; f.n[1] = m_node[1]; f.n[2] = m_node[2]; f.n[3] = m_node[3]; break;
	case FE_TRI3 : f.SetType(FE_FACE_TRI3 ); f.n[0] = m_node[0]; f.n[1] = m_node[1]; f.n[2] = m_node[2]; f.n[3] = m_node[2]; break;
	case FE_QUAD8:
		f.SetType(FE_FACE_QUAD8);
		f.n[0] = m_node[0]; f.n[1] = m_node[1]; f.n[2] = m_node[2]; f.n[3] = m_node[3]; 
		f.n[4] = m_node[4]; f.n[5] = m_node[5]; f.n[6] = m_node[6]; f.n[7] = m_node[7]; 
		break;
	case FE_QUAD9: 
		f.SetType(FE_FACE_QUAD9);
		f.n[0] = m_node[0]; f.n[1] = m_node[1]; f.n[2] = m_node[2]; f.n[3] = m_node[3];
		f.n[4] = m_node[4]; f.n[5] = m_node[5]; f.n[6] = m_node[6]; f.n[7] = m_node[7]; 
		f.n[8] = m_node[8];
		break;
    case FE_TRI6 :
		f.SetType(FE_FACE_TRI6);
        f.n[0] = m_node[0]; f.n[1] = m_node[1]; f.n[2] = m_node[2];
        f.n[3] = m_node[3]; f.n[4] = m_node[4]; f.n[5] = m_node[5];
        break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
int FSElement_::FindNodeIndex(int nid)
{
	int n = Nodes();
	for (int i=0; i<n; ++i) 
		if (m_node[i] == nid) return i;
	return -1;
}

//-----------------------------------------------------------------------------
int FSElement_::FindFace(const FSFace& f)
{
	FSFace tmp;
	int nf = Faces();
	for (int i = 0; i<nf; ++i) {
		GetFace(i, tmp);
		if (tmp == f)
		{
			return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
// TODO: This algorithm assumes that for 2nd order elements, interior nodes
// will only map to interior nodes of the other element. I'm not sure yet if that
// is an acceptable limitation.
bool FSElement_::is_equal(FSElement_& e)
{
	if (Type() != e.Type()) return false;
	int* n = m_node;
	int* m = e.m_node; 
	switch (Type())
	{
	case FE_BEAM2:
		if ((n[0] != m[0]) && (n[0] != m[1]) && (n[0] != m[2])) return false;
		if ((n[1] != m[0]) && (n[1] != m[1]) && (n[1] != m[2])) return false;
		break;
	case FE_TRI3:
    case FE_TRI6:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])) return false;
		break;
	case FE_QUAD4:
	case FE_QUAD8:
	case FE_QUAD9:
	case FE_TET4:
	case FE_TET5:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])&&(n[0]!=m[3])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])&&(n[1]!=m[3])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])&&(n[2]!=m[3])) return false;
		if ((n[3]!=m[0])&&(n[3]!=m[1])&&(n[3]!=m[2])&&(n[3]!=m[3])) return false;
		break;
	case FE_PENTA6:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])&&(n[0]!=m[3])&&(n[0]!=m[4])&&(n[0]!=m[5])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])&&(n[1]!=m[3])&&(n[1]!=m[4])&&(n[1]!=m[5])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])&&(n[2]!=m[3])&&(n[2]!=m[4])&&(n[2]!=m[5])) return false;
		if ((n[3]!=m[0])&&(n[3]!=m[1])&&(n[3]!=m[2])&&(n[3]!=m[3])&&(n[3]!=m[4])&&(n[3]!=m[5])) return false;
		if ((n[4]!=m[0])&&(n[4]!=m[1])&&(n[4]!=m[2])&&(n[4]!=m[3])&&(n[4]!=m[4])&&(n[4]!=m[5])) return false;
		if ((n[5]!=m[0])&&(n[5]!=m[1])&&(n[5]!=m[2])&&(n[5]!=m[3])&&(n[5]!=m[4])&&(n[5]!=m[5])) return false;
		break;
	case FE_PENTA15:
		if ((n[0] != m[0]) && (n[0] != m[1]) && (n[0] != m[2]) && (n[0] != m[3]) && (n[0] != m[4]) && (n[0] != m[5])) return false;
		if ((n[1] != m[0]) && (n[1] != m[1]) && (n[1] != m[2]) && (n[1] != m[3]) && (n[1] != m[4]) && (n[1] != m[5])) return false;
		if ((n[2] != m[0]) && (n[2] != m[1]) && (n[2] != m[2]) && (n[2] != m[3]) && (n[2] != m[4]) && (n[2] != m[5])) return false;
		if ((n[3] != m[0]) && (n[3] != m[1]) && (n[3] != m[2]) && (n[3] != m[3]) && (n[3] != m[4]) && (n[3] != m[5])) return false;
		if ((n[4] != m[0]) && (n[4] != m[1]) && (n[4] != m[2]) && (n[4] != m[3]) && (n[4] != m[4]) && (n[4] != m[5])) return false;
		if ((n[5] != m[0]) && (n[5] != m[1]) && (n[5] != m[2]) && (n[5] != m[3]) && (n[5] != m[4]) && (n[5] != m[5])) return false;

		if ((n[6] != m[6]) && (n[6] != m[7]) && (n[6] != m[8]) && (n[6] != m[9]) && (n[6] != m[10]) && (n[6] != m[11]) && (n[6] != m[12]) && (n[6] != m[13]) && (n[6] != m[14])) return false;
		if ((n[7] != m[6]) && (n[7] != m[7]) && (n[7] != m[8]) && (n[7] != m[9]) && (n[7] != m[10]) && (n[7] != m[11]) && (n[7] != m[12]) && (n[7] != m[13]) && (n[7] != m[14])) return false;
		if ((n[8] != m[6]) && (n[8] != m[7]) && (n[8] != m[8]) && (n[8] != m[9]) && (n[8] != m[10]) && (n[8] != m[11]) && (n[8] != m[12]) && (n[8] != m[13]) && (n[8] != m[14])) return false;
		if ((n[9] != m[6]) && (n[9] != m[7]) && (n[9] != m[8]) && (n[9] != m[9]) && (n[9] != m[10]) && (n[9] != m[11]) && (n[9] != m[12]) && (n[9] != m[13]) && (n[9] != m[14])) return false;
		if ((n[10] != m[6]) && (n[10] != m[7]) && (n[10] != m[8]) && (n[10] != m[9]) && (n[10] != m[10]) && (n[10] != m[11]) && (n[10] != m[12]) && (n[10] != m[13]) && (n[10] != m[14])) return false;
		if ((n[11] != m[6]) && (n[11] != m[7]) && (n[11] != m[8]) && (n[11] != m[9]) && (n[11] != m[10]) && (n[11] != m[11]) && (n[11] != m[12]) && (n[11] != m[13]) && (n[11] != m[14])) return false;
		if ((n[12] != m[6]) && (n[12] != m[7]) && (n[12] != m[8]) && (n[12] != m[9]) && (n[12] != m[10]) && (n[12] != m[11]) && (n[12] != m[12]) && (n[12] != m[13]) && (n[12] != m[14])) return false;
		if ((n[13] != m[6]) && (n[13] != m[7]) && (n[13] != m[8]) && (n[13] != m[9]) && (n[13] != m[10]) && (n[13] != m[11]) && (n[13] != m[12]) && (n[13] != m[13]) && (n[13] != m[14])) return false;
		if ((n[14] != m[6]) && (n[14] != m[7]) && (n[14] != m[8]) && (n[14] != m[9]) && (n[14] != m[10]) && (n[14] != m[11]) && (n[14] != m[12]) && (n[14] != m[13]) && (n[14] != m[14])) return false;
		break;

	case FE_PYRA5:
		if ((n[0] != m[0]) && (n[0] != m[1]) && (n[0] != m[2]) && (n[0] != m[3]) && (n[0] != m[4])) return false;
		if ((n[1] != m[0]) && (n[1] != m[1]) && (n[1] != m[2]) && (n[1] != m[3]) && (n[1] != m[4])) return false;
		if ((n[2] != m[0]) && (n[2] != m[1]) && (n[2] != m[2]) && (n[2] != m[3]) && (n[2] != m[4])) return false;
		if ((n[3] != m[0]) && (n[3] != m[1]) && (n[3] != m[2]) && (n[3] != m[3]) && (n[3] != m[4])) return false;
		if ((n[4] != m[0]) && (n[4] != m[1]) && (n[4] != m[2]) && (n[4] != m[3]) && (n[4] != m[4])) return false;
		break;
    case FE_PYRA13:
        if ((n[0] != m[0]) && (n[0] != m[1]) && (n[0] != m[2]) && (n[0] != m[3]) && (n[0] != m[4])) return false;
        if ((n[1] != m[0]) && (n[1] != m[1]) && (n[1] != m[2]) && (n[1] != m[3]) && (n[1] != m[4])) return false;
        if ((n[2] != m[0]) && (n[2] != m[1]) && (n[2] != m[2]) && (n[2] != m[3]) && (n[2] != m[4])) return false;
        if ((n[3] != m[0]) && (n[3] != m[1]) && (n[3] != m[2]) && (n[3] != m[3]) && (n[3] != m[4])) return false;
        if ((n[4] != m[0]) && (n[4] != m[1]) && (n[4] != m[2]) && (n[4] != m[3]) && (n[4] != m[4])) return false;
        break;
            
	case FE_HEX8:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])&&(n[0]!=m[3])&&(n[0]!=m[4])&&(n[0]!=m[5])&&(n[0]!=m[6])&&(n[0]!=m[7])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])&&(n[1]!=m[3])&&(n[1]!=m[4])&&(n[1]!=m[5])&&(n[1]!=m[6])&&(n[1]!=m[7])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])&&(n[2]!=m[3])&&(n[2]!=m[4])&&(n[2]!=m[5])&&(n[2]!=m[6])&&(n[2]!=m[7])) return false;
		if ((n[3]!=m[0])&&(n[3]!=m[1])&&(n[3]!=m[2])&&(n[3]!=m[3])&&(n[3]!=m[4])&&(n[3]!=m[5])&&(n[3]!=m[6])&&(n[3]!=m[7])) return false;
		if ((n[4]!=m[0])&&(n[4]!=m[1])&&(n[4]!=m[2])&&(n[4]!=m[3])&&(n[4]!=m[4])&&(n[4]!=m[5])&&(n[4]!=m[6])&&(n[4]!=m[7])) return false;
		if ((n[5]!=m[0])&&(n[5]!=m[1])&&(n[5]!=m[2])&&(n[5]!=m[3])&&(n[5]!=m[4])&&(n[5]!=m[5])&&(n[5]!=m[6])&&(n[5]!=m[7])) return false;
		if ((n[6]!=m[0])&&(n[6]!=m[1])&&(n[6]!=m[2])&&(n[6]!=m[3])&&(n[6]!=m[4])&&(n[6]!=m[5])&&(n[6]!=m[6])&&(n[6]!=m[7])) return false;
		if ((n[7]!=m[0])&&(n[7]!=m[1])&&(n[7]!=m[2])&&(n[7]!=m[3])&&(n[7]!=m[4])&&(n[7]!=m[5])&&(n[7]!=m[6])&&(n[7]!=m[7])) return false;
		break;
	case FE_HEX20:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])&&(n[0]!=m[3])&&(n[0]!=m[4])&&(n[0]!=m[5])&&(n[0]!=m[6])&&(n[0]!=m[7])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])&&(n[1]!=m[3])&&(n[1]!=m[4])&&(n[1]!=m[5])&&(n[1]!=m[6])&&(n[1]!=m[7])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])&&(n[2]!=m[3])&&(n[2]!=m[4])&&(n[2]!=m[5])&&(n[2]!=m[6])&&(n[2]!=m[7])) return false;
		if ((n[3]!=m[0])&&(n[3]!=m[1])&&(n[3]!=m[2])&&(n[3]!=m[3])&&(n[3]!=m[4])&&(n[3]!=m[5])&&(n[3]!=m[6])&&(n[3]!=m[7])) return false;
		if ((n[4]!=m[0])&&(n[4]!=m[1])&&(n[4]!=m[2])&&(n[4]!=m[3])&&(n[4]!=m[4])&&(n[4]!=m[5])&&(n[4]!=m[6])&&(n[4]!=m[7])) return false;
		if ((n[5]!=m[0])&&(n[5]!=m[1])&&(n[5]!=m[2])&&(n[5]!=m[3])&&(n[5]!=m[4])&&(n[5]!=m[5])&&(n[5]!=m[6])&&(n[5]!=m[7])) return false;
		if ((n[6]!=m[0])&&(n[6]!=m[1])&&(n[6]!=m[2])&&(n[6]!=m[3])&&(n[6]!=m[4])&&(n[6]!=m[5])&&(n[6]!=m[6])&&(n[6]!=m[7])) return false;
		if ((n[7]!=m[0])&&(n[7]!=m[1])&&(n[7]!=m[2])&&(n[7]!=m[3])&&(n[7]!=m[4])&&(n[7]!=m[5])&&(n[7]!=m[6])&&(n[7]!=m[7])) return false;

		if ((n[ 8]!=m[8])&&(n[ 8]!=m[9])&&(n[ 8]!=m[10])&&(n[ 8]!=m[11])&&(n[ 8]!=m[12])&&(n[ 8]!=m[13])&&(n[ 8]!=m[14])&&(n[ 8]!=m[15])&&(n[ 8]!=m[16])&&(n[ 8]!=m[17])&&(n[ 8]!=m[18])&&(n[ 8]!=m[19])) return false;
		if ((n[ 9]!=m[8])&&(n[ 9]!=m[9])&&(n[ 9]!=m[10])&&(n[ 9]!=m[11])&&(n[ 9]!=m[12])&&(n[ 9]!=m[13])&&(n[ 9]!=m[14])&&(n[ 9]!=m[15])&&(n[ 9]!=m[16])&&(n[ 9]!=m[17])&&(n[ 9]!=m[18])&&(n[ 9]!=m[19])) return false;
		if ((n[10]!=m[8])&&(n[10]!=m[9])&&(n[10]!=m[10])&&(n[10]!=m[11])&&(n[10]!=m[12])&&(n[10]!=m[13])&&(n[10]!=m[14])&&(n[10]!=m[15])&&(n[10]!=m[16])&&(n[10]!=m[17])&&(n[10]!=m[18])&&(n[10]!=m[19])) return false;
		if ((n[11]!=m[8])&&(n[11]!=m[9])&&(n[11]!=m[10])&&(n[11]!=m[11])&&(n[11]!=m[12])&&(n[11]!=m[13])&&(n[11]!=m[14])&&(n[11]!=m[15])&&(n[11]!=m[16])&&(n[11]!=m[17])&&(n[11]!=m[18])&&(n[11]!=m[19])) return false;
		if ((n[12]!=m[8])&&(n[12]!=m[9])&&(n[12]!=m[10])&&(n[12]!=m[11])&&(n[12]!=m[12])&&(n[12]!=m[13])&&(n[12]!=m[14])&&(n[12]!=m[15])&&(n[12]!=m[16])&&(n[12]!=m[17])&&(n[12]!=m[18])&&(n[12]!=m[19])) return false;
		if ((n[13]!=m[8])&&(n[13]!=m[9])&&(n[13]!=m[10])&&(n[13]!=m[11])&&(n[13]!=m[12])&&(n[13]!=m[13])&&(n[13]!=m[14])&&(n[13]!=m[15])&&(n[13]!=m[16])&&(n[13]!=m[17])&&(n[13]!=m[18])&&(n[13]!=m[19])) return false;
		if ((n[14]!=m[8])&&(n[14]!=m[9])&&(n[14]!=m[10])&&(n[14]!=m[11])&&(n[14]!=m[12])&&(n[14]!=m[13])&&(n[14]!=m[14])&&(n[14]!=m[15])&&(n[14]!=m[16])&&(n[14]!=m[17])&&(n[14]!=m[18])&&(n[14]!=m[19])) return false;
		if ((n[15]!=m[8])&&(n[15]!=m[9])&&(n[15]!=m[10])&&(n[15]!=m[11])&&(n[15]!=m[12])&&(n[15]!=m[13])&&(n[15]!=m[14])&&(n[15]!=m[15])&&(n[15]!=m[16])&&(n[15]!=m[17])&&(n[15]!=m[18])&&(n[15]!=m[19])) return false;
		if ((n[16]!=m[8])&&(n[16]!=m[9])&&(n[16]!=m[10])&&(n[16]!=m[11])&&(n[16]!=m[12])&&(n[16]!=m[13])&&(n[16]!=m[14])&&(n[16]!=m[15])&&(n[16]!=m[16])&&(n[16]!=m[17])&&(n[16]!=m[18])&&(n[16]!=m[19])) return false;
		if ((n[17]!=m[8])&&(n[17]!=m[9])&&(n[17]!=m[10])&&(n[17]!=m[11])&&(n[17]!=m[12])&&(n[17]!=m[13])&&(n[17]!=m[14])&&(n[17]!=m[15])&&(n[17]!=m[16])&&(n[17]!=m[17])&&(n[17]!=m[18])&&(n[17]!=m[19])) return false;
		if ((n[18]!=m[8])&&(n[18]!=m[9])&&(n[18]!=m[10])&&(n[18]!=m[11])&&(n[18]!=m[12])&&(n[18]!=m[13])&&(n[18]!=m[14])&&(n[18]!=m[15])&&(n[18]!=m[16])&&(n[18]!=m[17])&&(n[18]!=m[18])&&(n[18]!=m[19])) return false;
		if ((n[19]!=m[8])&&(n[19]!=m[9])&&(n[19]!=m[10])&&(n[19]!=m[11])&&(n[19]!=m[12])&&(n[19]!=m[13])&&(n[19]!=m[14])&&(n[19]!=m[15])&&(n[19]!=m[16])&&(n[19]!=m[17])&&(n[19]!=m[18])&&(n[19]!=m[19])) return false;
		break;
	case FE_HEX27:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])&&(n[0]!=m[3])&&(n[0]!=m[4])&&(n[0]!=m[5])&&(n[0]!=m[6])&&(n[0]!=m[7])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])&&(n[1]!=m[3])&&(n[1]!=m[4])&&(n[1]!=m[5])&&(n[1]!=m[6])&&(n[1]!=m[7])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])&&(n[2]!=m[3])&&(n[2]!=m[4])&&(n[2]!=m[5])&&(n[2]!=m[6])&&(n[2]!=m[7])) return false;
		if ((n[3]!=m[0])&&(n[3]!=m[1])&&(n[3]!=m[2])&&(n[3]!=m[3])&&(n[3]!=m[4])&&(n[3]!=m[5])&&(n[3]!=m[6])&&(n[3]!=m[7])) return false;
		if ((n[4]!=m[0])&&(n[4]!=m[1])&&(n[4]!=m[2])&&(n[4]!=m[3])&&(n[4]!=m[4])&&(n[4]!=m[5])&&(n[4]!=m[6])&&(n[4]!=m[7])) return false;
		if ((n[5]!=m[0])&&(n[5]!=m[1])&&(n[5]!=m[2])&&(n[5]!=m[3])&&(n[5]!=m[4])&&(n[5]!=m[5])&&(n[5]!=m[6])&&(n[5]!=m[7])) return false;
		if ((n[6]!=m[0])&&(n[6]!=m[1])&&(n[6]!=m[2])&&(n[6]!=m[3])&&(n[6]!=m[4])&&(n[6]!=m[5])&&(n[6]!=m[6])&&(n[6]!=m[7])) return false;
		if ((n[7]!=m[0])&&(n[7]!=m[1])&&(n[7]!=m[2])&&(n[7]!=m[3])&&(n[7]!=m[4])&&(n[7]!=m[5])&&(n[7]!=m[6])&&(n[7]!=m[7])) return false;

		if ((n[ 8]!=m[8])&&(n[ 8]!=m[9])&&(n[ 8]!=m[10])&&(n[ 8]!=m[11])&&(n[ 8]!=m[12])&&(n[ 8]!=m[13])&&(n[ 8]!=m[14])&&(n[ 8]!=m[15])&&(n[ 8]!=m[16])&&(n[ 8]!=m[17])&&(n[ 8]!=m[18])&&(n[ 8]!=m[19])) return false;
		if ((n[ 9]!=m[8])&&(n[ 9]!=m[9])&&(n[ 9]!=m[10])&&(n[ 9]!=m[11])&&(n[ 9]!=m[12])&&(n[ 9]!=m[13])&&(n[ 9]!=m[14])&&(n[ 9]!=m[15])&&(n[ 9]!=m[16])&&(n[ 9]!=m[17])&&(n[ 9]!=m[18])&&(n[ 9]!=m[19])) return false;
		if ((n[10]!=m[8])&&(n[10]!=m[9])&&(n[10]!=m[10])&&(n[10]!=m[11])&&(n[10]!=m[12])&&(n[10]!=m[13])&&(n[10]!=m[14])&&(n[10]!=m[15])&&(n[10]!=m[16])&&(n[10]!=m[17])&&(n[10]!=m[18])&&(n[10]!=m[19])) return false;
		if ((n[11]!=m[8])&&(n[11]!=m[9])&&(n[11]!=m[10])&&(n[11]!=m[11])&&(n[11]!=m[12])&&(n[11]!=m[13])&&(n[11]!=m[14])&&(n[11]!=m[15])&&(n[11]!=m[16])&&(n[11]!=m[17])&&(n[11]!=m[18])&&(n[11]!=m[19])) return false;
		if ((n[12]!=m[8])&&(n[12]!=m[9])&&(n[12]!=m[10])&&(n[12]!=m[11])&&(n[12]!=m[12])&&(n[12]!=m[13])&&(n[12]!=m[14])&&(n[12]!=m[15])&&(n[12]!=m[16])&&(n[12]!=m[17])&&(n[12]!=m[18])&&(n[12]!=m[19])) return false;
		if ((n[13]!=m[8])&&(n[13]!=m[9])&&(n[13]!=m[10])&&(n[13]!=m[11])&&(n[13]!=m[12])&&(n[13]!=m[13])&&(n[13]!=m[14])&&(n[13]!=m[15])&&(n[13]!=m[16])&&(n[13]!=m[17])&&(n[13]!=m[18])&&(n[13]!=m[19])) return false;
		if ((n[14]!=m[8])&&(n[14]!=m[9])&&(n[14]!=m[10])&&(n[14]!=m[11])&&(n[14]!=m[12])&&(n[14]!=m[13])&&(n[14]!=m[14])&&(n[14]!=m[15])&&(n[14]!=m[16])&&(n[14]!=m[17])&&(n[14]!=m[18])&&(n[14]!=m[19])) return false;
		if ((n[15]!=m[8])&&(n[15]!=m[9])&&(n[15]!=m[10])&&(n[15]!=m[11])&&(n[15]!=m[12])&&(n[15]!=m[13])&&(n[15]!=m[14])&&(n[15]!=m[15])&&(n[15]!=m[16])&&(n[15]!=m[17])&&(n[15]!=m[18])&&(n[15]!=m[19])) return false;
		if ((n[16]!=m[8])&&(n[16]!=m[9])&&(n[16]!=m[10])&&(n[16]!=m[11])&&(n[16]!=m[12])&&(n[16]!=m[13])&&(n[16]!=m[14])&&(n[16]!=m[15])&&(n[16]!=m[16])&&(n[16]!=m[17])&&(n[16]!=m[18])&&(n[16]!=m[19])) return false;
		if ((n[17]!=m[8])&&(n[17]!=m[9])&&(n[17]!=m[10])&&(n[17]!=m[11])&&(n[17]!=m[12])&&(n[17]!=m[13])&&(n[17]!=m[14])&&(n[17]!=m[15])&&(n[17]!=m[16])&&(n[17]!=m[17])&&(n[17]!=m[18])&&(n[17]!=m[19])) return false;
		if ((n[18]!=m[8])&&(n[18]!=m[9])&&(n[18]!=m[10])&&(n[18]!=m[11])&&(n[18]!=m[12])&&(n[18]!=m[13])&&(n[18]!=m[14])&&(n[18]!=m[15])&&(n[18]!=m[16])&&(n[18]!=m[17])&&(n[18]!=m[18])&&(n[18]!=m[19])) return false;
		if ((n[19]!=m[8])&&(n[19]!=m[9])&&(n[19]!=m[10])&&(n[19]!=m[11])&&(n[19]!=m[12])&&(n[19]!=m[13])&&(n[19]!=m[14])&&(n[19]!=m[15])&&(n[19]!=m[16])&&(n[19]!=m[17])&&(n[19]!=m[18])&&(n[19]!=m[19])) return false;

		if ((n[20]!=m[20])&&(n[20]!=m[21])&&(n[20]!=m[22])&&(n[20]!=m[23])&&(n[20]!=m[24])&&(n[20]!=m[25])) return false;
		if ((n[21]!=m[20])&&(n[21]!=m[21])&&(n[21]!=m[22])&&(n[21]!=m[23])&&(n[21]!=m[24])&&(n[21]!=m[25])) return false;
		if ((n[22]!=m[20])&&(n[22]!=m[21])&&(n[22]!=m[22])&&(n[22]!=m[23])&&(n[22]!=m[24])&&(n[22]!=m[25])) return false;
		if ((n[23]!=m[20])&&(n[23]!=m[21])&&(n[23]!=m[22])&&(n[23]!=m[23])&&(n[23]!=m[24])&&(n[23]!=m[25])) return false;
		if ((n[24]!=m[20])&&(n[24]!=m[21])&&(n[24]!=m[22])&&(n[24]!=m[23])&&(n[24]!=m[24])&&(n[24]!=m[25])) return false;
		if ((n[25]!=m[20])&&(n[25]!=m[21])&&(n[25]!=m[22])&&(n[25]!=m[23])&&(n[25]!=m[24])&&(n[25]!=m[25])) return false;

		if (n[26]!=m[26]) return false;
		break;
	case FE_TET10:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])&&(n[0]!=m[3])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])&&(n[1]!=m[3])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])&&(n[2]!=m[3])) return false;
		if ((n[3]!=m[0])&&(n[3]!=m[1])&&(n[3]!=m[2])&&(n[3]!=m[3])) return false;

		if ((n[4]!=m[4])&&(n[4]!=m[5])&&(n[4]!=m[6])&&(n[4]!=m[7])&&(n[4]!=m[8])&&(n[4]!=m[9])) return false;
		if ((n[5]!=m[4])&&(n[5]!=m[5])&&(n[5]!=m[6])&&(n[5]!=m[7])&&(n[5]!=m[8])&&(n[5]!=m[9])) return false;
		if ((n[6]!=m[4])&&(n[6]!=m[5])&&(n[6]!=m[6])&&(n[6]!=m[7])&&(n[6]!=m[8])&&(n[6]!=m[9])) return false;
		if ((n[7]!=m[4])&&(n[7]!=m[5])&&(n[7]!=m[6])&&(n[7]!=m[7])&&(n[7]!=m[8])&&(n[7]!=m[9])) return false;
		if ((n[8]!=m[4])&&(n[8]!=m[5])&&(n[8]!=m[6])&&(n[8]!=m[7])&&(n[8]!=m[8])&&(n[8]!=m[9])) return false;
		if ((n[9]!=m[4])&&(n[9]!=m[5])&&(n[9]!=m[6])&&(n[9]!=m[7])&&(n[9]!=m[8])&&(n[9]!=m[9])) return false;
		break;
	case FE_TET15:
		if ((n[0]!=m[0])&&(n[0]!=m[1])&&(n[0]!=m[2])&&(n[0]!=m[3])) return false;
		if ((n[1]!=m[0])&&(n[1]!=m[1])&&(n[1]!=m[2])&&(n[1]!=m[3])) return false;
		if ((n[2]!=m[0])&&(n[2]!=m[1])&&(n[2]!=m[2])&&(n[2]!=m[3])) return false;
		if ((n[3]!=m[0])&&(n[3]!=m[1])&&(n[3]!=m[2])&&(n[3]!=m[3])) return false;

		if ((n[4]!=m[4])&&(n[4]!=m[5])&&(n[4]!=m[6])&&(n[4]!=m[7])&&(n[4]!=m[8])&&(n[4]!=m[9])) return false;
		if ((n[5]!=m[4])&&(n[5]!=m[5])&&(n[5]!=m[6])&&(n[5]!=m[7])&&(n[5]!=m[8])&&(n[5]!=m[9])) return false;
		if ((n[6]!=m[4])&&(n[6]!=m[5])&&(n[6]!=m[6])&&(n[6]!=m[7])&&(n[6]!=m[8])&&(n[6]!=m[9])) return false;
		if ((n[7]!=m[4])&&(n[7]!=m[5])&&(n[7]!=m[6])&&(n[7]!=m[7])&&(n[7]!=m[8])&&(n[7]!=m[9])) return false;
		if ((n[8]!=m[4])&&(n[8]!=m[5])&&(n[8]!=m[6])&&(n[8]!=m[7])&&(n[8]!=m[8])&&(n[8]!=m[9])) return false;
		if ((n[9]!=m[4])&&(n[9]!=m[5])&&(n[9]!=m[6])&&(n[9]!=m[7])&&(n[9]!=m[8])&&(n[9]!=m[9])) return false;

		if ((n[10]!=m[10])&&(n[10]!=m[11])&&(n[10]!=m[12])&&(n[10]!=m[13])) return false;
		if ((n[11]!=m[10])&&(n[11]!=m[11])&&(n[11]!=m[12])&&(n[11]!=m[13])) return false;
		if ((n[12]!=m[10])&&(n[12]!=m[11])&&(n[12]!=m[12])&&(n[12]!=m[13])) return false;
		if ((n[13]!=m[10])&&(n[13]!=m[11])&&(n[13]!=m[12])&&(n[13]!=m[13])) return false;

		if (n[14]!=m[14]) return false;

		break;
	case FE_TET20:
		// compare corner nodes first
		if ((n[0] != m[0]) && (n[0] != m[1]) && (n[0] != m[2]) && (n[0] != m[3])) return false;
		if ((n[1] != m[0]) && (n[1] != m[1]) && (n[1] != m[2]) && (n[1] != m[3])) return false;
		if ((n[2] != m[0]) && (n[2] != m[1]) && (n[2] != m[2]) && (n[2] != m[3])) return false;
		if ((n[3] != m[0]) && (n[3] != m[1]) && (n[3] != m[2]) && (n[3] != m[3])) return false;

		// TODO: Do I really need to check the other nodes?
		break;
	default:
		assert(false);
	}

	return true;
}

//-----------------------------------------------------------------------------
// This function is used by derived copy constructors and assignment operators
// to simplify copying the common element data.
void FSElement_::copy(const FSElement_& el)
{
	SetState(el.GetState());

	m_gid = el.m_gid;
	m_traits = el.m_traits;
	m_nid = el.m_nid;
	m_ntag = el.m_ntag;
	m_MatID = el.m_MatID;

	m_fiber = el.m_fiber;
	m_Q = el.m_Q;
	m_Qactive = el.m_Qactive;
	m_a0 = el.m_a0;
//	m_edata = el.m_edata;

	for (int i=0; i<Nodes(); ++i) m_node[i] = el.m_node[i];
}

//-----------------------------------------------------------------------------
//! Calculate the shape function values at the point (r,s,t)
void FSElement_::shape(double *H, double r, double s, double t)
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
    case FE_PYRA13 : PYRA13 ::shape(H, r, s, t); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
//! Calculate the shape function values at the point (r,s)
void FSElement_::shape_2d(double* H, double r, double s)
{
	switch (Type())
	{
	case FE_TRI3 : TRI3 ::shape(H, r, s); break;
	case FE_QUAD4: QUAD4::shape(H, r, s); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
double FSElement_::eval(double* d, double r, double s, double t)
{
	double H[FSElement::MAX_NODES];
	shape(H, r, s, t);
	double a = 0.0;
	for (int i=0; i<Nodes(); ++i) a += H[i]*d[i];
	return a;
}

//-----------------------------------------------------------------------------
float FSElement_::eval(float* d, double r, double s, double t)
{
	double H[FSElement::MAX_NODES];
	shape(H, r, s, t);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return (float) a;
}

//-----------------------------------------------------------------------------
vec3f FSElement_::eval(vec3f* d, double r, double s, double t)
{
	double H[FSElement::MAX_NODES];
	shape(H, r, s, t);
	vec3f a(0,0,0);
	for (int i=0; i<Nodes(); ++i) a += d[i]*((float)H[i]);
	return a;
}

//-----------------------------------------------------------------------------
vec3f FSElement_::eval(vec3f* d, double r, double s)
{
	double H[FSElement::MAX_NODES];
	shape_2d(H, r, s);
	vec3f a(0, 0, 0);
	for (int i = 0; i < Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}

//-----------------------------------------------------------------------------
void FSElement_::shape_deriv(double* Hr, double* Hs, double* Ht, double r, double s, double t)
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
    case FE_PYRA13 : PYRA13 ::shape_deriv(Hr, Hs, Ht, r, s, t); break;
    default:
		assert(false);
    }
}

//-----------------------------------------------------------------------------
void FSElement_::shape_deriv_2d(double* Hr, double* Hs, double r, double s)
{
	switch (Type())
	{
	case FE_TRI3 : TRI3 ::shape_deriv(Hr, Hs, r, s); break;
	case FE_QUAD4: QUAD4::shape_deriv(Hr, Hs, r, s); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FSElement_::iso_coord(int n, double q[3])
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
    case FE_PYRA13 : PYRA13 ::iso_coord(n, q); break;
	default:
		assert(false);
    }
}

//-----------------------------------------------------------------------------
void FSElement_::iso_coord_2d(int n, double q[2])
{
	// for n=-1 return isoparametric coordinates of element center
	assert((n >= -1) && (n < Nodes()));
	switch (Type())
	{
	case FE_TRI3 : TRI3::iso_coord(n, q); break;
	case FE_QUAD4: QUAD4::iso_coord(n, q); break;
	default:
		assert(false);
	}
}

void FSElement_::setAxes(const vec3d& a, const vec3d& d)
{
	vec3d e1(a); e1.Normalize();
	vec3d e3 = e1 ^ d; e3.Normalize();
	vec3d e2 = e3 ^ e1; e2.Normalize();
	m_Q = mat3d(
		e1.x, e2.x, e3.x,
		e1.y, e2.y, e3.y,
		e1.z, e2.z, e3.z);
	m_Qactive = true;
}

//=============================================================================
// FSElement
//-----------------------------------------------------------------------------
FSElement::FSElement()
{ 
	m_node = _node;
	m_nbr = _nbr;
	m_face = _face;
	m_h	= _h;
	m_nid = -1;

	for (int i=0; i<MAX_NODES; ++i) m_node[i] = -1;
	for (int i=0; i<6; ++i) { m_nbr[i] = m_face[i] = -1; }
	for (int i=0; i<9; ++i) m_h[i] = 0.0;
}

//-----------------------------------------------------------------------------
FSElement::FSElement(const FSElement& el)
{
	m_node = _node;
	m_nbr = _nbr;
	m_face = _face;
	m_h	= _h;

	copy(el);

	for (int i=0; i<6; ++i) { m_nbr[i] = el.m_nbr[i]; m_face[i] = el.m_face[i]; }
	for (int i=0; i<9; ++i) m_h[i] = el.m_h[i];
}

//-----------------------------------------------------------------------------
FSElement& FSElement::operator = (const FSElement& el)
{
	copy(el);

	for (int i=0; i<6; ++i) { m_nbr[i] = el.m_nbr[i]; m_face[i] = el.m_face[i]; }
	for (int i=0; i<9; ++i) m_h[i] = el.m_h[i];

	return *this;
}

int ET_LINE[1][2] = {
	{ 0, 1 }};

int ET_QUAD[4][2] = {
	{ 0, 1 },
	{ 1, 2 },
	{ 2, 3 },
	{ 3, 0 } };

int ET_QUAD8[4][3] = {
	{ 0, 1, 4 },
	{ 1, 2, 5 },
	{ 2, 3, 6 },
	{ 3, 0, 7 } };

int ET_TRI[3][2] = {
	{ 0, 1 },
	{ 1, 2 },
	{ 2, 0 } };

int ET_TRI6[3][3] = {
	{ 0, 1, 3 },
	{ 1, 2, 4 },
	{ 2, 0, 5 } };

//=============================================================================
FSLinearElement::FSLinearElement()
{
	m_node = _node;
	m_nbr  = _nbr;
	m_face = _face;
	m_h    = _h;
	for (int i = 0; i<MAX_NODES; ++i) m_node[i] = -1;
}

FSLinearElement::FSLinearElement(const FSLinearElement& e) : FSElement_(e)
{
	m_traits = e.m_traits;
	m_node = _node;
	m_nbr  = _nbr;
	m_face = _face;
	m_h    = _h;
	for (int i = 0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i];
}

void FSLinearElement::operator = (const FSLinearElement& e)
{
	m_traits = e.m_traits;
	FSElement_::operator = (e);
	for (int i = 0; i<MAX_NODES; ++i) m_node[i] = e.m_node[i];
}
