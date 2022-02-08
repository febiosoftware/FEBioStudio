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
#include <MeshLib/FEMesh.h>
#include <algorithm>

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
GMultiBox::GMultiBox(GObject* po) : GObject(GMULTI_BLOCK)
{
	// copy to old object's ID
	SetID(po->GetID());

	// copy the name
	SetName(po->GetName());

	// copy the transform
	GetTransform() = po->GetTransform();

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
	mbMesher->CopyFrom(*mb);

	BuildObject();
}

void GMultiBox::BuildObject()
{
	ClearAll();

	FEMultiBlockMesh* mb = dynamic_cast<FEMultiBlockMesh*>(GetFEMesher()); assert(mb);

	// next, we pull the geometry info from the multiblock 
	// --- Nodes ---
	int NN = mb->Nodes();
	m_Node.reserve(NN);
	for (int i=0; i<NN; ++i)
	{
		MBNode& no = mb->GetMBNode(i);
		GNode* n = AddNode(no.m_r);
	}

	// --- Edges ---
	int NE = mb->Edges();
	m_Edge.reserve(NE);
	for (int i=0; i<NE; ++i)
	{
		GEdge* e = AddEdge();
		MBEdge& eo = mb->GetEdge(i);
		e->m_node[0] = eo.edge.m_node[0];
		e->m_node[1] = eo.edge.m_node[1];
		e->m_cnode = eo.edge.m_cnode;
		e->m_ntype = eo.edge.m_ntype;
		e->m_orient = eo.m_winding;
	}

	// --- Faces ---
	int NF = mb->Faces();
	m_Face.reserve(NF);
	for (int i=0; i<NF; ++i)
	{
		GFace* f = AddFace();
		MBFace& fo = mb->GetFace(i);
		f->m_node.resize(4);
		f->m_edge.resize(4);
		for (int j = 0; j < 4; ++j)
		{
			f->m_node[j] = fo.m_node[j];
			f->m_edge[j].nid = fo.m_edge[j];
			f->m_edge[j].nwn = fo.m_edgeWinding[j];
		}
		f->m_nPID[0] = fo.m_block[0];
		f->m_nPID[1] = fo.m_block[1];
		f->m_ntype = FACE_QUAD;	// TODO: Get this data from MBFace
		if (fo.m_isRevolve)
		{
			f->m_ntype = FACE_REVOLVE;
		}
	}

	// --- Parts ---
	int NP = mb->Blocks();
	m_Part.reserve(NP);
	for (int i=0; i<NP; ++i)
	{
		GPart* g = AddPart();
		MBBlock& go = mb->GetBlock(i);

		assert(go.m_gid >= 0);
		g->SetMaterialID(go.m_gid);
		g->SetLocalID(i);
	}

	// rebuild the GMesh
	BuildGMesh();
}

FEMeshBase* GMultiBox::GetEditableMesh()
{
	return GetFEMesh();
}

bool GMultiBox::DeletePart(GPart* pg)
{
	if (pg == nullptr) return false;
	if (pg->Object() != this) return false;

	// get the local ID of the part
	int lid = pg->GetLocalID();

	// delete the existing mesh
	delete GetFEMesh();
	SetFEMesh(nullptr);

	// rebuild the MB
	// we need the multi block mesher to pull out the multiblock geometry
	FEMultiBlockMesher* mbMesher = dynamic_cast<FEMultiBlockMesher*>(GetFEMesher()); assert(mbMesher);

	// Get the multiblock mesh
	mbMesher->DeleteBlock(lid);

	// rebuild the object
	BuildObject();

	// rebuild the GMesh
	BuildGMesh();

	return true;
}

GObject* GMultiBox::Clone()
{
	GMultiBox* clone = new GMultiBox(this);
	return clone;
}

bool GMultiBox::Merge(GMultiBox& mb)
{
	// delete the existing mesh
	delete GetFEMesh();
	SetFEMesh(nullptr);

	// get the mb's mesher
	FEMultiBlockMesher* mbMesher = dynamic_cast<FEMultiBlockMesher*>(mb.GetFEMesher()); assert(mbMesher);
	if (mbMesher == nullptr) return false;

	// we need to make a copy so that we can convert the nodes to this transform
	FEMultiBlockMesher copyMB(*mbMesher);

	// transform all nodes
	for (int i = 0; i < copyMB.Nodes(); ++i)
	{
		MBNode& node = copyMB.GetMBNode(i);
		vec3d q = mb.GetTransform().LocalToGlobal(node.m_r);
		node.m_r = GetTransform().GlobalToLocal(q);
	}

	// we need the multi block mesher
	FEMultiBlockMesher* thisMesher = dynamic_cast<FEMultiBlockMesher*>(GetFEMesher()); assert(thisMesher);
	if (thisMesher == nullptr) return false;

	// merge the multi-blocks
	thisMesher->MergeMultiBlock(copyMB);

	// rebuild the object
	BuildObject();

	// rebuild the GMesh
	BuildGMesh();

	return true;
}
