#include "stdafx.h"
#include <MeshLib/FEMesh.h>
#include <MeshLib/FEFaceEdgeList.h>
#include "FEModifier.h"
using namespace std;

//-----------------------------------------------------------------------------
FETet4ToHex8::FETet4ToHex8() : FEModifier("Tet4-to-Hex8")
{
	m_bsmooth = false;
}

//-----------------------------------------------------------------------------
FEMesh* FETet4ToHex8::Apply(FEMesh* pm)
{
	// before we get started, let's make sure this is a tet4 mesh
	if (pm->IsType(FE_TET4) == false) return 0;

	// convert to a Tet15 mesh
	FETet4ToTet15 tet4to15;
	tet4to15.SetSmoothing(m_bsmooth);
	FEMesh* tet15 = tet4to15.Apply(pm);

	// create a new mesh
	int nodes = tet15->Nodes();
	int elems = tet15->Elements();
	FEMesh* pnew = new FEMesh;
	pnew->Create(nodes, 4*elems);

	// copy the nodes from the tet15 mesh
	for (int i = 0; i<nodes; ++i)
	{
		FENode& n0 = pnew->Node(i);
		FENode& n1 = tet15->Node(i);
		n0.r = n1.r;
		n0.m_gid = n1.m_gid;
	}

	// node lookup table
	const int NLT[4][8] = { 
		{ 0, 4, 10, 6, 7, 11, 14, 13 },
		{ 4, 1, 5, 10, 11, 8, 12, 14 },
		{ 5, 2, 6, 10, 12, 9, 13, 14 },
		{ 7, 3, 8, 11, 13, 9, 12, 14 }
	};

	// create the new elements
	int ne = 0;
	for (int i = 0; i<elems; ++i)
	{
		FEElement& e0 = tet15->Element(i);

		for (int j = 0; j < 4; ++j)
		{
			FEElement& e1 = pnew->Element(ne++);

			e1.SetType(FE_HEX8);
			e1.m_gid = e0.m_gid;
			for (int k=0; k<8; ++k) e1.m_node[k] = e0.m_node[ NLT[j][k] ];
		}
	}

	// build the other mesh structures
	pnew->RebuildMesh();

	// don't forget to clean up
	delete tet15;

	return pnew;
}
