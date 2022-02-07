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
GMultiBox::GMultiBox(GObject *po) : GObject(GMULTI_BLOCK)
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
		g->SetMaterialID(po->Part(go.m_gid)->GetMaterialID());
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

	// remove the part
	int lid = pg->GetLocalID();
	assert(m_Part[lid] == pg);
	m_Part.erase(m_Part.begin() + lid);

	// adjust the part local IDs.
	for (int i = 0; i < m_Part.size(); ++i)
	{
		GPart* part = m_Part[i];
		part->SetLocalID(i);
	}

	// adjust the face's PID
	std::for_each(m_Face.begin(), m_Face.end(), [&lid](GFace* face) {

			// set the PID of the face to -1, if it attached to the deleted part
			// decreased the PID by one if it attached to a part with a higher local ID

			int* pid = face->m_nPID;
			if      (pid[0] == lid) pid[0] = -1;
			else if (pid[0]  > lid) pid[0] -= 1;

			if      (pid[1] == lid) pid[1] = -1;
			else if (pid[1] >  lid) pid[1] -= 1;

			// make sure the first pid is not -1.
			if (pid[0] == -1)
			{
				int tmp = pid[0]; pid[0] = pid[1]; pid[1] = tmp;

				// we also need to invert the edges and nodes
				int fn[4], fe[4], fw[4];
				for (int j = 0; j < 4; ++j) { 
					fn[j] = face->m_node[j]; 
					fe[j] = face->m_edge[j].nid; 
					fw[j] = face->m_edge[j].nwn;
				}
				for (int j = 0; j < 4; ++j)
				{
					face->m_node[j] = fn[(4 - j)%4];
					face->m_edge[j].nid = fe[3 - j];
					face->m_edge[j].nwn = -fw[3-j];
				}
			}
		});

	// remove any faces that are no longer used
	for (int i = 0; i < m_Face.size(); ++i)
	{
		GFace* face = m_Face[i];
		if ((face->m_nPID[0] == -1) && (face->m_nPID[1] == -1))
		{
			m_Face.erase(m_Face.begin() + i);
			i--;
		}
	}

	// adjust the face local IDs.
	for (int i = 0; i < m_Face.size(); ++i)
	{
		GFace* face = m_Face[i];
		face->SetLocalID(i);
	}

	// figure out if any edges need to be deleted
	// we'll count the edge reference
	for (GEdge* edge : m_Edge) edge->m_ntag = 0;
	for (GFace* face : m_Face) {
		for (int i = 0; i < face->m_edge.size(); ++i)
		{
			GEdge* edge = Edge(face->m_edge[i].nid);
			edge->m_ntag = 1;
		}
	}

	// update local ids
	// we do this first since we need to update the faces' edge list before we delete the edges
	int n = 0;
	for (GEdge* edge : m_Edge)
	{
		if (edge->m_ntag == 1) edge->SetLocalID(n++);
		else edge->SetLocalID(-1);
	}

	// now update the faces' edge list IDs
	for (GFace* face : m_Face)
	{
		for (int i = 0; i < face->m_edge.size(); ++i)
		{
			GEdge* edge = Edge(face->m_edge[i].nid);
			face->m_edge[i].nid = edge->GetLocalID(); assert(face->m_edge[i].nid >= 0);
		}
	}

	// now delete the unused edges
	for (int i = 0; i < m_Edge.size(); ++i)
	{
		GEdge* edge = m_Edge[i];
		if (edge->GetLocalID() == -1)
		{
			m_Edge.erase(m_Edge.begin() + i);
			--i;
		}
	}

	// figure out if any nodes need to be deleted
	// we'll count the nodes reference
	for (GNode* node : m_Node) node->m_ntag = 0;
	for (GEdge* edge : m_Edge) {
		GNode* node0 = Node(edge->m_node[0]);
		GNode* node1 = Node(edge->m_node[1]);
		node0->m_ntag = 1;
		node1->m_ntag = 1;

		// make sure we tag the center node, if it's used!
		if (edge->m_cnode >= 0) Node(edge->m_cnode)->m_ntag = 1;
	}

	// update local ids of nodes
	// we do this first since we need to update the edge and faces' node list before we delete the nodes
	n = 0;
	for (GNode* node : m_Node)
	{
		if (node->m_ntag == 1) node->SetLocalID(n++);
		else node->SetLocalID(-1);
	}

	// now update the faces' node list IDs
	for (GFace* face : m_Face)
	{
		for (int i = 0; i < face->m_node.size(); ++i)
		{
			GNode* node = Node(face->m_node[i]);
			face->m_node[i] = node->GetLocalID(); assert(face->m_node[i] >= 0);
		}
	}

	// update the edge's node list IDs
	for (GEdge* edge : m_Edge)
	{
		edge->m_node[0] = Node(edge->m_node[0])->GetLocalID();
		edge->m_node[1] = Node(edge->m_node[1])->GetLocalID();
		if (edge->m_cnode >= 0) edge->m_cnode = Node(edge->m_cnode)->GetLocalID();
	}

	// now delete the unused nodes
	for (int i = 0; i < m_Node.size(); ++i)
	{
		GNode* node = m_Node[i];
		if (node->GetLocalID() == -1)
		{
			m_Node.erase(m_Node.begin() + i);
			--i;
		}
	}

	// delete the existing mesh
	delete GetFEMesh();
	SetFEMesh(nullptr);

	// rebuild the MB
	// we need the multi block mesher to pull out the multiblock geometry
	FEMultiBlockMesher* mbMesher = dynamic_cast<FEMultiBlockMesher*>(GetFEMesher()); assert(mbMesher);

	// Get the multiblock mesh
	mbMesher->DeleteBlock(lid);

	// rebuild the GMesh
	BuildGMesh();

	return true;
}

GObject* GMultiBox::Clone()
{
	GMultiBox* clone = new GMultiBox(this);
	return clone;
}
