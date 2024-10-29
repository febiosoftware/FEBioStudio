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
#include "FEModifier.h"

FEDeleteElements::FEDeleteElements() : FEModifier("delete elements")
{

}

FSMesh* FEDeleteElements::Apply(FSMesh* pm)
{
	const int TAG = 1;
	int NE0 = pm->Elements();
	int NN0 = pm->Nodes();
	int nsel = 0;
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (!el.IsSelected()) {
			el.m_ntag = TAG; nsel++;
		}
		else el.m_ntag = 0;
	}
	if (nsel == 0)
	{
		SetError("No elements selected.");
		return nullptr;
	}

	pm->TagAllNodes(TAG);
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag != TAG)
		{
			int nn = el.Nodes();
			for (int j = 0; j < nn; ++j) pm->Node(el.m_node[j]).m_ntag = 0;
		}
	}
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == TAG)
		{
			int nn = el.Nodes();
			for (int j = 0; j < nn; ++j) pm->Node(el.m_node[j]).m_ntag = TAG;
		}
	}
	for (int i = 0; i < NN0; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.IsRequired()) node.m_ntag = TAG;
	}

	// count elements
	int NE1 = 0;
	for (int i = 0; i < NE0; ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == TAG) el.m_ntag = NE1++;
		else el.m_ntag = -1;
	}

	// allocate nodes
	int NN1 = 0;
	for (int i = 0; i < NN0; ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_ntag == TAG) node.m_ntag = NN1++;
		else node.m_ntag = -1;
	}

	// create new mesh
	FSMesh* newMesh = new FSMesh;
	newMesh->Create(NN1, NE1);

	for (int i = 0; i < NN0; ++i)
	{
		FSNode& ns = pm->Node(i);
		if (ns.m_ntag >= 0)
		{
			FSNode& nd = newMesh->Node(ns.m_ntag);
			nd = ns;
		}
	}

	for (int i = 0; i < NE0; ++i)
	{
		FSElement& es = pm->Element(i);
		if (es.m_ntag >= 0)
		{
			FSElement& ed = newMesh->Element(es.m_ntag);
			ed.SetType(es.Type());
			ed.m_nid = es.m_nid;
			ed.m_gid = es.m_gid;
			int nn = es.Nodes();
			for (int j = 0; j < nn; ++j)
			{
				ed.m_node[j] = pm->Node(es.m_node[j]).m_ntag;
				assert(ed.m_node[j] >= 0);
			}
		}
	}

	newMesh->RebuildMesh();

	return newMesh;
}
