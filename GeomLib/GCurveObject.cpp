/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "GCurveObject.h"
#include <MeshTools/FSCurveObjectMesher.h>
#include <MeshTools/FESelection.h>
#include <MeshLib/FEMesh.h>
#include <map>

GCurveObject::GCurveObject() : GObject(GCURVE_OBJECT)
{
	SetFEMesher(new FSCurveObjectMesher(this));
}

GCurveObject::GCurveObject(GCurveObject* po) : GObject(GCURVE_OBJECT)
{
	SetFEMesher(new FSCurveObjectMesher(this));

	// copy transform
	GetTransform() = po->GetTransform();

	// next, we copy the geometry info
	// --- Nodes ---
	int NN = po->Nodes();
	m_Node.reserve(NN);
	for (int i = 0; i < NN; ++i)
	{
		GNode* n = new GNode(this);
		GNode& no = *po->Node(i);
		n->LocalPosition() = no.LocalPosition();
		n->SetID(no.GetID());
		n->SetLocalID(i);
		n->SetType(no.Type());
		assert(n->GetLocalID() == no.GetLocalID());
		n->SetName(no.GetName());
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE = po->Edges();
	m_Edge.reserve(NE);
	for (int i = 0; i < NE; ++i)
	{
		GEdge* e = new GEdge(this);
		GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0];
		e->m_node[1] = eo.m_node[1];
		e->SetID(eo.GetID());
		e->SetLocalID(i);
		e->SetName(eo.GetName());
		e->m_ntype = eo.Type();
		assert(e->GetLocalID() == eo.GetLocalID());
		m_Edge.push_back(e);
	}

	// --- Parts ---
	AddBeamPart();

	// update the object
	Update();
}

GObject* GCurveObject::Clone()
{
	GCurveObject* po = new GCurveObject(this);
	return po;
}

void GCurveObject::Merge(GCurveObject* po)
{
	// TODO: Should we merge the mesh as well?
	// delete the existing mesh
	delete GetFEMesh();
	SetFEMesh(nullptr);

	Transform& T = GetTransform();

	std::map<int, int> nodemap;

	for (int i = 0; i < po->Nodes(); ++i)
	{
		GNode* pold = po->Node(i);
		vec3d r = T.GlobalToLocal(pold->Position());
		GNode* pn = AddNode(r);
		nodemap[pold->GetLocalID()] = pn->GetLocalID();
	}

	for (int i = 0; i < po->Edges(); ++i)
	{
		GEdge* eold = po->Edge(i);
		int n0 = nodemap[eold->m_node[0]];
		int n1 = nodemap[eold->m_node[1]];
		AddLine(n0, n1);
	}
	Update();
}

void GCurveObject::MergeNodes(GNodeSelection* sel)
{
	double eps = 1e-6;
	for (int i = 0; i < Nodes(); ++i) Node(i)->m_ntag = -1;
	GNodeSelection::Iterator it1(sel);
	int N = sel->Count();
	for (int i = 0; i < N; ++i, ++it1)
	{
		vec3d r1 = it1->LocalPosition();
		GNodeSelection::Iterator it2(sel);
		double dmin = 0;
		int nmin = -1;
		for (int j = 0; j < N; ++j, ++it2)
		{
			if ((j != i) && (it2->m_ntag == -1))
			{
				vec3d r2 = it2->LocalPosition();
				double D = (r2 - r1).Length();
				if (D < eps)
				{
					if ((nmin == -1) || (D < dmin))
					{
						nmin = it2->GetLocalID();
						dmin = D;
					}
				}
			}
		}
		it1->m_ntag = nmin;
	}

	for (int i = 0; i < Edges(); ++i)
	{
		GEdge* pe = Edge(i);
		GNode* n0 = Node(pe->m_node[0]);
		if (n0->m_ntag != -1) pe->m_node[0] = n0->m_ntag;
		GNode* n1 = Node(pe->m_node[1]);
		if (n1->m_ntag != -1) pe->m_node[1] = n1->m_ntag;
	}

	// reindex nodes
	int nc = 0;
	for (int i = 0; i < Nodes(); ++i)
	{
		GNode* pn = Node(i);
		if (pn->m_ntag == -1) pn->m_ntag = nc++;
		else pn->m_ntag = -1;
	}

	// reindex edges
	for (int i = 0; i < Edges(); ++i)
	{
		GEdge* pe = Edge(i);
		GNode* n0 = Node(pe->m_node[0]);
		pe->m_node[0] = n0->m_ntag;
		GNode* n1 = Node(pe->m_node[1]);
		pe->m_node[1] = n1->m_ntag;
	}

	// remove the tagged nodes
	vector<GNode*> newNodes;
	for (int i = 0; i < Nodes(); ++i)
	{
		GNode* pn = Node(i);
		if (pn->m_ntag != -1) newNodes.push_back(pn);
		else delete pn;
	}
	assert(newNodes.size() == nc);
	m_Node = newNodes;
	Update();
}
