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
#include "FEModifier.h"

//-----------------------------------------------------------------------------
//! Convert a tet10 to a tet4 mesh by eliminating all the edge nodes
FSMesh* FETet15ToTet4::Apply(FSMesh* pm)
{
	// make sure the mesh is a tet10 mesh
	if (pm->IsType(FE_TET15) == false) return 0;

	// get the number of items
	int NN = pm->Nodes();
	int NE = pm->Elements();
	int NF = pm->Faces();
	int NC = pm->Edges();

	// count the number of corner nodes
	for (int i=0; i<NN; ++i) pm->Node(i).m_ntag = -1;
	for (int i=0; i<NE; ++i)
	{
		FSElement& el = pm->Element(i);
		for (int j=0; j<4; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
	}
	int nn = 0;
	for (int i=0; i<NN; ++i)
	{
		FSNode& ni = pm->Node(i);
		if (ni.m_ntag == 1) 
		{
			ni.m_ntag = nn++;
		}
	}

	// allocate a new mesh
	FSMesh* pnew = new FSMesh;
	pnew->Create(nn, NE, NF, NC);

	// create the nodes
	nn = 0;
	for (int i=0; i<NN; ++i)
	{
		FSNode& n0 = pm->Node(i);
		if (n0.m_ntag >= 0)
		{
			FSNode& n1 = pnew->Node(nn++);
			n1.r = n0.r;
			n1.m_gid = n0.m_gid;
		}
	}

	// create the elements
	for (int i=0; i<NE; ++i)
	{
		FSElement& e0 = pm->Element(i);
		FSElement& e1 = pnew->Element(i);
		e1 = e0;

		e1.m_gid = e0.m_gid;

		e1.SetType(FE_TET4);
		e1.m_node[0] = pm->Node(e0.m_node[0]).m_ntag;
		e1.m_node[1] = pm->Node(e0.m_node[1]).m_ntag;
		e1.m_node[2] = pm->Node(e0.m_node[2]).m_ntag;
		e1.m_node[3] = pm->Node(e0.m_node[3]).m_ntag;
	}

	// create the new faces
	for (int i=0; i<NF; ++i)
	{
		FSFace& f0 = pm->Face(i);
		FSFace& f1 = pnew->Face(i);

		f1.SetType(FE_FACE_TRI3);
		f1.m_gid = f0.m_gid;
		f1.m_sid = f0.m_sid;
		f1.n[0] = pm->Node(f0.n[0]).m_ntag;
		f1.n[1] = pm->Node(f0.n[1]).m_ntag;
		f1.n[2] = pm->Node(f0.n[2]).m_ntag;
		f1.m_elem[0] = f0.m_elem[0];
		f1.m_elem[1] = f0.m_elem[1];
		f1.m_elem[2] = f0.m_elem[2];
		f1.m_nbr[0] = f0.m_nbr[0];
		f1.m_nbr[1] = f0.m_nbr[1];
		f1.m_nbr[2] = f0.m_nbr[2];
	}

	// create the new edges
	for (int i=0; i<NC; ++i)
	{
		FSEdge& e0 = pm->Edge(i);
		FSEdge& e1 = pnew->Edge(i);

		e1.SetType(FE_EDGE2);
		e1.n[0] = pm->Node(e0.n[0]).m_ntag;
		e1.n[1] = pm->Node(e0.n[1]).m_ntag;
		e1.n[2] = -1;
		e1.m_nid = e0.m_nid;
		e1.m_gid = e0.m_gid;
		e1.m_nbr[0] = e0.m_nbr[0];
		e1.m_nbr[1] = e0.m_nbr[1];
		e1.m_elem = e0.m_elem;
		e1.SetExterior(e0.IsExterior());
	}

	pnew->UpdateMesh();

	return pnew;
}
