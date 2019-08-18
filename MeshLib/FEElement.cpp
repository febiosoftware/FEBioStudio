#include "FEElement.h"

//=============================================================================
// FEElement_
//-----------------------------------------------------------------------------
FEElement_::FEElement_()
{
	m_ntype = FE_INVALID_ELEMENT_TYPE;	// unknown type

	m_node = 0;
	m_nbr = 0;
	m_face = 0;
	m_h = 0;

	m_nodes = 0;
	m_nfaces = 0;
	m_nedges = 0;

	m_gid = 0;	// all elements need to be assigned to a partition

	m_Q.unit(); m_Qactive = false;
	m_a0 = 0;
}

//-----------------------------------------------------------------------------
FEFace FEElement_::GetFace(int i) const
{
	FEFace face;
	GetFace(face, i);
	return face;
}

//-----------------------------------------------------------------------------
void FEElement_::GetFace(FEFace& f, int i) const
{
	int* m = m_node;
	int* n = f.n;
	switch (m_ntype)
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
FEEdge FEElement_::GetEdge(int i)
{
	FEEdge e;

	switch (m_ntype)
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
void FEElement_::GetShellFace(FEFace& f) const
{
	switch (m_ntype)
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
//! Is this an exterior element (i.e. an element that is on the outside of the mesh).
//! shells and beams are alwasy exterior. Solids are exterior if they have at least one
//! null neighbor.
bool FEElement_::IsExterior()
{
	if (!IsSolid()) return true;
	int n = Faces();
	for (int i=0; i<n; ++i) if (m_nbr[i] < 0) return true;
	return false;
}

//-----------------------------------------------------------------------------
int FEElement_::FindNodeIndex(int nid)
{
	int n = Nodes();
	for (int i=0; i<n; ++i) 
		if (m_node[i] == nid) return i;
	return -1;
}

//-----------------------------------------------------------------------------
int FEElement_::FindFace(const FEFace& f)
{
	int nf = Faces();
	for (int i = 0; i<nf; ++i) {
		FEFace lf = GetFace(i);
		bool found = true;
		for (int j = 0; j<f.Nodes(); ++j) {
			found = lf.HasNode(f.n[j]);
			if (!found) break;
		}
		if (found) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
// TODO: This algorithm assumes that for 2nd order elements, interior nodes
// will only map to interior nodes of the other element. I'm not sure yet if that
// is an acceptable limitation.
bool FEElement_::is_equal(FEElement_& e)
{
	if (m_ntype != e.m_ntype) return false;
	int* n = m_node;
	int* m = e.m_node; 
	switch (m_ntype)
	{
	case FE_BEAM2:
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
void FEElement_::copy(const FEElement_& el)
{
	SetFEState(el.GetFEState());

	m_gid = el.m_gid;
	m_ntype = el.m_ntype;
	m_nodes = el.m_nodes;
	m_nfaces = el.m_nfaces;
	m_nedges = el.m_nedges;
	m_nid = el.m_nid;

	m_fiber = el.m_fiber;
	m_Q = el.m_Q;
	m_Qactive = el.m_Qactive;
	m_a0 = el.m_a0;
//	m_edata = el.m_edata;

	for (int i=0; i<m_nodes; ++i) m_node[i] = el.m_node[i];
}


//=============================================================================
// FEElement
//-----------------------------------------------------------------------------
FEElement::FEElement()
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
FEElement::FEElement(const FEElement& el)
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
FEElement& FEElement::operator = (const FEElement& el)
{
	copy(el);

	for (int i=0; i<6; ++i) { m_nbr[i] = el.m_nbr[i]; m_face[i] = el.m_face[i]; }
	for (int i=0; i<9; ++i) m_h[i] = el.m_h[i];

	return *this;
}

//-----------------------------------------------------------------------------
// Set the element type. This also sets some other type related data
void FEElement::SetType(int ntype)
{ 
	// set the type
	m_ntype = ntype; 
	assert(m_ntype != FE_INVALID_ELEMENT_TYPE);

	// set the number of nodes
	const int N[] = {0, 8,4,6,4,3,2,20,8,3,10,6,15,27,7,9,15,5,20,10, 5};
	m_nodes = N[m_ntype];

	// set the number of faces (shells have no faces)
	const int F[] = {0, 6,4,5,0,0,0,6,0,0,4,0,4,6,0,0,5,5,4,0, 4};
	m_nfaces = F[m_ntype];

	// set the number of edges (solids have no edges)
	const int E[] = {0, 0,0,0,4,3,1,0,4,1,0,3,0,0,3,4,0,0,0,3, 0};
	m_nedges = E[m_ntype];
}
