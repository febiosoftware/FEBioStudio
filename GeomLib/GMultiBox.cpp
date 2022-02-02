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

#include "GMultiBox.h"
#include <MeshTools/FEMultiBlockMesh.h>

//-----------------------------------------------------------------------------
//! Constructor
GMultiBox::GMultiBox() : GObject(GMULTI_BLOCK)
{
	SetFEMesher(new FEMultiBlockMesher(this));
}

//-----------------------------------------------------------------------------
// This function creates a new GMultiBox from an existing GObject
// The new GMultiBox will have the same ID as the existing GObject
// This is used in the CCmdConvertObject command that converts a GPrimitve
// a GMultiBox
GMultiBox::GMultiBox(GObject *po) : GObject(GMULTI_BLOCK)
{
	// copy to old object's ID
	SetID(po->GetID());

	// creating a new object has increased the object counter
	// so we need to decrease it again
	GItem_T<GBaseObject>::DecreaseCounter();

	// define the default mesher
	FEMultiBlockMesher* mbMesher = new FEMultiBlockMesher(this);
	SetFEMesher(mbMesher);

	// we need the multi block mesher to pull out the multiblock geometry
	FEMultiBlockMesh* mb = dynamic_cast<FEMultiBlockMesh*>(po->GetFEMesher()); assert(mb);

	// build the multi-block data
	bool b = mb->BuildMultiBlock(); assert(b);

	// copy it to the multiblock mesh
	mbMesher->SetMultiBlockMesh(*mb);

	// next, we pull the geometry info from the multiblock 
	// --- Nodes ---
	int NN = mb->Nodes();
	m_Node.reserve(NN);
	for (int i=0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		MBNode& no = mb->GetMBNode(i);
		n->LocalPosition() = no.m_r; // TODO: check if this is local coordinates?
		n->SetID(no.m_gid);
		n->SetLocalID(i);
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE = mb->Edges();
	m_Edge.reserve(NE);
	for (int i=0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		MBEdge& eo = mb->GetEdge(i);
		e->m_node[0] = eo.edge.m_node[0];
		e->m_node[1] = eo.edge.m_node[1];
		e->SetID(eo.m_gid);
		e->SetLocalID(i);
		m_Edge.push_back(e);
	}

	// --- Faces ---
	int NF = mb->Faces();
	m_Face.reserve(NF);
	for (int i=0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		MBFace& fo = mb->GetFace(i);
		f->m_node.resize(4);
		f->m_edge.resize(4);
		for (int j = 0; j < 4; ++j)
		{
			f->m_node[j] = fo.m_node[j];
			f->m_edge[j].nid = fo.m_edge[j];
		}
		f->m_nPID[0] = fo.m_block[0];
		f->m_nPID[1] = fo.m_block[1];
		f->SetID(fo.m_gid);
		f->SetLocalID(i);
		f->m_ntype = FACE_QUAD;	// TODO: Get this data from MBFace
		m_Face.push_back(f);
	}

	// --- Parts ---
	int NP = mb->Blocks();
	m_Part.reserve(NP);
	for (int i=0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		MBBlock& go = mb->GetBlock(i);

		g->SetMaterialID(po->Part(go.m_gid)->GetMaterialID());
		g->SetID(go.m_gid);
		g->SetLocalID(i);
		m_Part.push_back(g);
	}

	// rebuild the GMesh
	BuildGMesh();
}
