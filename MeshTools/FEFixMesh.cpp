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
#include "FEFixMesh.h"
#include <MeshLib/FSMeshBuilder.h>
using namespace std;

//-----------------------------------------------------------------------------
FEFixMesh::FEFixMesh() : FEModifier("Fix mesh")
{
	AddIntParam(0, "Task:", "Task:")->SetEnumNames("Remove duplicate edges\0Remove isolated vertices\0");
}

//-----------------------------------------------------------------------------
FSMesh* FEFixMesh::Apply(FSMesh* pm)
{
	// create a copy of the mesh
	FSMesh* pnew = new FSMesh(*pm);
	pnew->SetGObject(pm->GetGObject());

	// apply the task on this mesh
	int task = GetIntValue(0);
	bool ret = false;
	switch (task)
	{
	case 0: ret = RemoveDuplicateEdges  (pnew); break;
	case 1: ret = RemoveIsolatedVertices(pnew); break;
	}

	if (ret == false)
	{
		delete pnew;
		pnew = 0;
	}

	return pnew;
}

//-----------------------------------------------------------------------------
bool FEFixMesh::RemoveDuplicateEdges(FSMesh* pm)
{
	int NE = pm->Edges();
	int NN = pm->Nodes();

	vector<vector<int> > NET(NN);
	for (int i = 0; i < pm->Edges(); ++i)
	{
		FSEdge& ei = pm->Edge(i);
		NET[ei.n[0]].push_back(i);
		NET[ei.n[1]].push_back(i);
	}

	int duplicates = 0;
	pm->TagAllEdges(-1);
	for (int i = 0; i < pm->Edges(); ++i)
	{
		FSEdge& ei = pm->Edge(i);
		if (ei.m_ntag == -1)
		{
			vector<int>& net = NET[ei.n[0]];
			for (int j = 0; j < net.size(); ++j)
			{
				int nej = net[j];
				if (nej > i)
				{
					FSEdge& ej = pm->Edge(nej);
					if ((ej.m_ntag == -1) && (ej == ei))
					{
						ej.m_ntag = i;
						duplicates++;
					}
				}
			}
		}
	}
	SetError("%d duplicate edges found.", duplicates);
	if (duplicates == 0) return true;

	// re-index the edges
	vector<int> id(pm->Edges(), -1);
	int ne = 0;
	for (int i = 0; i < pm->Edges(); ++i)
	{
		FSEdge& ei = pm->Edge(i);
		if (ei.m_ntag == -1) id[i] = ne++;
	}

	// update edge neighbors
	for (int i = 0; i < pm->Edges(); ++i)
	{
		FSEdge& ei = pm->Edge(i);
		if (ei.m_ntag == -1)
		{
			if (ei.m_nbr[0] >= 0) ei.m_nbr[0] = id[ei.m_nbr[0]];
			if (ei.m_nbr[1] >= 0) ei.m_nbr[1] = id[ei.m_nbr[1]];
		}
	}

	// the faces could reference edges, so we will need to reindex them 
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& f = pm->Face(i);
		for (int j = 0; j < 4; ++j)
		{
			int nej = f.m_edge[j];
			if (nej >= 0)
			{
				FSEdge& ej = pm->Edge(nej);
				if (ej.m_ntag >= 0)
				{
					int nid = id[ej.m_ntag]; assert(nid >= 0);
					f.m_edge[j] = nid;
				}
			}
		}
	}

	// delete the duplicate edges
	ne = 0;
	for (int i = 0; i < pm->Edges(); ++i)
	{
		FSEdge& ei = pm->Edge(i);
		if (ei.m_ntag == -1)
		{
			assert(id[i] >= 0);
			if (i != ne)
			{
				FSEdge& en = pm->Edge(ne);
				en = ei;
			}
			ne++;
		}
	}

	// resize edges
	pm->ResizeEdges(ne);

	// TODO: It is possible that duplicate edges messed up
	// the node partitioning. Should I repartition nodes here? 

	//all done
	return true;
}

//-----------------------------------------------------------------------------
bool FEFixMesh::RemoveIsolatedVertices(FSMesh* pm)
{
	FSMeshBuilder mb(*pm);
	int n = mb.RemoveIsolatedNodes();
	SetError("%d vertices removed.", n);
	return true;
}
