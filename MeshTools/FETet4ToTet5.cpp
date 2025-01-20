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
#include "FEModifier.h"
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
FETet4ToTet5::FETet4ToTet5() : FEModifier("Tet4-to-Tet5")
{

}

//-----------------------------------------------------------------------------
FSMesh* FETet4ToTet5::Apply(FSMesh* pm)
{
	// before we get started, let's make sure this is a tet4 mesh
	if (pm->IsType(FE_TET4) == false) return 0;

	int NN = pm->Nodes();
	int NF = pm->Faces();
	int NT = pm->Elements();
	int NC = pm->Edges();

	// a new node is added for each element
	int NN1 = NN + NT;

	// allocate a new mesh
	FSMesh* pnew = new FSMesh;
	pnew->Create(NN1, NT, NF, NC);

	// copy the old nodes
	for (int i = 0; i<NN; ++i)
	{
		FSNode& n0 = pm->Node(i);
		FSNode& n1 = pnew->Node(i);
		n1.r = n0.r;
		n1.m_gid = n0.m_gid;
	}

	// create the new nodes
	for (int i = 0; i<NT; ++i)
	{
		FSElement& el = pm->Element(i);
		vec3d& r0 = pm->Node(el.m_node[0]).r;
		vec3d& r1 = pm->Node(el.m_node[1]).r;
		vec3d& r2 = pm->Node(el.m_node[2]).r;
		vec3d& r3 = pm->Node(el.m_node[3]).r;

		FSNode& n1 = pnew->Node(i + NN);
		n1.r = (r0 + r1 + r2 + r3)*0.25;
		n1.SetExterior(false);
	}

	// create the elements
	for (int i = 0; i<NT; ++i)
	{
		FSElement& e0 = pm->Element(i);
		FSElement& e1 = pnew->Element(i);
		e1 = e0;

		e1.m_gid = e0.m_gid;

		e1.SetType(FE_TET5);
		e1.m_node[0] = e0.m_node[0];
		e1.m_node[1] = e0.m_node[1];
		e1.m_node[2] = e0.m_node[2];
		e1.m_node[3] = e0.m_node[3];
		e1.m_node[4] = NN + i;
	}

	// create the new faces
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f0 = pm->Face(i);
		FSFace& f1 = pnew->Face(i);
		f1 = f0;
	}

	// create the new edges
	for (int i = 0; i<NC; ++i)
	{
		FSEdge& e0 = pm->Edge(i);
		FSEdge& e1 = pnew->Edge(i);
		e1 = e0;
	}

	return pnew;
}

//-----------------------------------------------------------------------------
FETet5ToTet4::FETet5ToTet4() : FEModifier("Tet5-to-Tet4")
{

}

//-----------------------------------------------------------------------------
FSMesh* FETet5ToTet4::Apply(FSMesh* pm)
{
	// before we get started, let's make sure this is a tet4 mesh
	if (pm->IsType(FE_TET5) == false) return 0;

	int NN = pm->Nodes();
	int NF = pm->Faces();
	int NT = pm->Elements();
	int NC = pm->Edges();

	// figure out how many nodes we actually need
	vector<int> nodeTag(NN, -1);
	for (int i = 0; i < NT; ++i)
	{
		FSElement& el = pm->Element(i);
		for (int j = 0; j < 4; ++j) nodeTag[el.m_node[j]] = 1;
	}
	int NN1 = 0;
	for (int i = 0; i < NN; ++i)
	{
		if (nodeTag[i] != -1) nodeTag[i] = NN1++;
	}

	// allocate a new mesh
	FSMesh* pnew = new FSMesh;
	pnew->Create(NN1, NT, NF, NC);

	// create the new nodes
	for (int i = 0; i<NN; ++i)
	{
		if (nodeTag[i] >= 0)
		{
			FSNode& nd = pnew->Node(nodeTag[i]);
			FSNode& ns = pm->Node(i);
			nd = ns;
		}
	}

	// create the elements
	for (int i = 0; i<NT; ++i)
	{
		FSElement& e0 = pm->Element(i);
		FSElement& e1 = pnew->Element(i);
		e1 = e0;

		e1.m_gid = e0.m_gid;

		e1.SetType(FE_TET4);
		e1.m_node[0] = e0.m_node[0];
		e1.m_node[1] = e0.m_node[1];
		e1.m_node[2] = e0.m_node[2];
		e1.m_node[3] = e0.m_node[3];
	}

	// create the new faces
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f0 = pm->Face(i);
		FSFace& f1 = pnew->Face(i);
		f1 = f0;
	}

	// create the new edges
	for (int i = 0; i<NC; ++i)
	{
		FSEdge& e0 = pm->Edge(i);
		FSEdge& e1 = pnew->Edge(i);
		e1 = e0;
	}

	return pnew;
}
