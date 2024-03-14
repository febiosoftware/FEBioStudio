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
	int NP = po->Parts();
	m_Part.reserve(NP);
	for (int i = 0; i < NP; ++i)
	{
		GPart* g = new GPart(this);
		GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i);
		g->SetName(go.GetName());
		assert(g->GetLocalID() == go.GetLocalID());
		m_Part.push_back(g);
	}

	// update the object
	Update();
}

GObject* GCurveObject::Clone()
{
	GCurveObject* po = new GCurveObject(this);
	return po;
}
