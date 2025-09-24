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

#include "stdafx.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSFaceEdgeList.h>
#include "FEModifier.h"


//-----------------------------------------------------------------------------
FETet4ToHex8::FETet4ToHex8(bool bsmooth) : FEModifier("Tet4-to-Hex8")
{
	m_bsmooth = bsmooth;
}

//-----------------------------------------------------------------------------
FSMesh* FETet4ToHex8::Apply(FSMesh* pm)
{
	// before we get started, let's make sure this is a tet4 mesh
	if (pm->IsType(FE_TET4) == false) return 0;

	// convert to a Tet15 mesh
	FETet4ToTet15 tet4to15;
	tet4to15.SetSmoothing(m_bsmooth);
	FSMesh* tet15 = tet4to15.Apply(pm);

	// create a new mesh
	int nodes = tet15->Nodes();
	int elems = tet15->Elements();
	int faces = tet15->Faces();
	int edges = tet15->Edges();
	FSMesh* pnew = new FSMesh;
	pnew->Create(nodes, 4*elems, 3*faces, 2*edges);

	// copy the nodes from the tet15 mesh
	for (int i = 0; i<nodes; ++i)
	{
		FSNode& n0 = pnew->Node(i);
		FSNode& n1 = tet15->Node(i);
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
		FSElement& e0 = tet15->Element(i);

		for (int j = 0; j < 4; ++j)
		{
			FSElement& e1 = pnew->Element(ne++);

			e1.SetType(FE_HEX8);
			e1.m_gid = e0.m_gid;
			for (int k=0; k<8; ++k) e1.m_node[k] = e0.m_node[ NLT[j][k] ];
		}
	}

	// create the new faces
	const int FLT[3][4] = {
		{ 0, 3, 6, 5 },
		{ 3, 1, 4, 6 },
		{ 2, 5, 6, 4 }
	};

	int nf = 0;
	for (int i = 0; i < faces; ++i)
	{
		FSFace& f0 = tet15->Face(i);

		for (int j = 0; j < 3; ++j)
		{
			FSFace& f1 = pnew->Face(nf++);

			f1.SetType(FE_FACE_QUAD4);
			f1.m_gid = f0.m_gid;
			f1.m_sid = f0.m_sid;

			for (int k = 0; k < 4; ++k) f1.n[k] = f0.n[FLT[j][k]];
		}
	}

	// create new edges
	const int ELT[2][2] = { {0,2}, {2,1} };
	int nc = 0;
	for (int i = 0; i < edges; ++i)
	{
		FSEdge& c0 = tet15->Edge(i);
		for (int j = 0; j < 2; ++j)
		{
			FSEdge& c1 = pnew->Edge(nc++);
			c1.SetType(FE_EDGE2);
			c1.m_gid = c0.m_gid;

			c1.n[0] = c0.n[ELT[j][0]];
			c1.n[1] = c0.n[ELT[j][1]];
		}
	}

	// build the other mesh structures
	pnew->BuildMesh();

	// don't forget to clean up
	delete tet15;

	return pnew;
}
