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

#include "GMultiPatch.h"
#include <MeshTools/FEMultiQuadMesh.h>
#include <MeshLib/FSMesh.h>
#include <algorithm>

//-----------------------------------------------------------------------------
//! Constructor
GMultiPatch::GMultiPatch() : GObject(GMULTI_PATCH)
{
	SetFEMesher(new FEMultiQuadMesher());
}

//-----------------------------------------------------------------------------
// This function creates a new GMultiBox from an existing GObject
// The new GMultiBox will have the same ID as the existing GObject
// This is used in the CCmdConvertObject command that converts a GPrimitve
// a GMultiBox
GMultiPatch::GMultiPatch(GObject* po) : GObject(GMULTI_PATCH)
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
	FEMultiQuadMesher* mbMesher = new FEMultiQuadMesher();
	SetFEMesher(mbMesher);

	// we need the multi block mesher to pull out the multiblock geometry
	FEMultiQuadMesh* mb = dynamic_cast<FEMultiQuadMesh*>(po->GetFEMesher()); assert(mb);

	// build the multi-block data
	bool b = mb->BuildMultiQuad(); assert(b);

	// build the object data from the multi-block
	BuildObject(*mb);

	// rebuild the render mesh
	SetRenderMesh(nullptr);
}

// This function builds the object data from the multiblock mesh
void GMultiPatch::BuildObject(FEMultiQuadMesh& mb)
{
	ClearAll();

	// next, we pull the geometry info from the multiblock 
	// --- Nodes ---
	int NN = mb.Nodes();
	m_Node.reserve(NN);
	for (int i = 0; i < NN; ++i)
	{
		MBNode& no = mb.GetMBNode(i);
		GNode* n = AddNode(no.m_r);
		n->SetType(no.m_type);
	}

	// --- Edges ---
	int NE = mb.Edges();
	m_Edge.reserve(NE);
	for (int i = 0; i < NE; ++i)
	{
		GEdge* e = AddEdge();
		MBEdge& eo = mb.GetEdge(i);
		e->m_node[0] = eo.m_node[0];
		e->m_node[1] = eo.m_node[1];
		if (eo.m_cnode >= 0) e->m_cnode.push_back(eo.m_cnode);
		e->m_ntype = eo.m_ntype;
		e->m_orient = eo.m_orient;
	}

	// --- Faces ---
	int NF = mb.Faces();
	m_Face.reserve(NF);
	for (int i = 0; i < NF; ++i)
	{
		GFace* f = AddFace();
		MBFace& fo = mb.GetFace(i);
		f->m_node.resize(4);
		f->m_edge.resize(4);
		for (int j = 0; j < 4; ++j)
		{
			f->m_node[j] = fo.m_node[j];
			f->m_edge[j].nid = fo.m_edge[j];
			f->m_edge[j].nwn = fo.m_edgeWinding[j];
		}
		f->m_nPID[0] = i;// fo.m_block[0];
		f->m_nPID[1] = -1;// fo.m_block[1];
		f->m_ntype = FACE_QUAD;	// TODO: Get this data from MBFace
		if (fo.m_isRevolve)
		{
			f->m_ntype = FACE_REVOLVE;
		}
	}

	// --- Parts (match faces) ---
	int NP = mb.Faces();
	m_Part.reserve(NP);
	for (int i = 0; i < NP; ++i)
	{
		GPart* g = AddPart();
		MBFace& go = mb.GetFace(i);

		assert(go.m_gid >= 0);
		g->SetLocalID(i);

		g->m_node.resize(4);
		for (int i = 0; i < 4; ++i) g->m_node[i] = go.m_node[i];

		g->m_edge.resize(4);
		for (int i = 0; i < 4; ++i) g->m_edge[i] = go.m_edge[i];

		g->m_face.resize(1);
		g->m_face[0] = i;
	}
}

FSMeshBase* GMultiPatch::GetEditableMesh()
{
	return GetFEMesh();
}

bool GMultiPatch::DeletePart(GPart* pg)
{
	if (pg == nullptr) return false;
	if (pg->Object() != this) return false;

	// get the local ID of the part
	int lid = pg->GetLocalID();

	// delete the existing mesh
	delete GetFEMesh();
	SetFEMesh(nullptr);

	// --------------------------------
	// tag new part indices
	int n = 0;
	for (int i = 0; i < m_Part.size(); ++i)
	{
		GPart& b = *m_Part[i];
		if (i == lid) b.m_ntag = -1;
		else b.m_ntag = n++;
	}
	assert(n == m_Part.size() - 1);

	// update face block ids.
	for (GFace* face : m_Face)
	{
		// update block IDs
		assert(face->m_nPID[0] >= 0);
		face->m_nPID[0] = m_Part[face->m_nPID[0]]->m_ntag;
		if (face->m_nPID[1] >= 0) {
			face->m_nPID[1] = m_Part[face->m_nPID[1]]->m_ntag;
		}

		// if the face 0 block is -1, then we need to invert it
		if ((face->m_nPID[0] == -1) && (face->m_nPID[1] != -1))
		{
			face->Invert();
		}
	}

	// remove the block
	delete m_Part[lid];
	m_Part.erase(m_Part.begin() + lid);
	for (int i = 0; i < m_Part.size(); ++i) m_Part[i]->SetLocalID(i);

	//-------------------------------
	// Mark faces for removal 
	n = 0;
	for (GFace* face : m_Face)
	{
		// if the face is free (not connected to a part), mark it for deletion (tag = -1)
		if ((face->m_nPID[0] == -1) && (face->m_nPID[1] == -1))
		{
			face->m_ntag = -1;
		}
		else face->m_ntag = n++;
	}

	// update block face IDs
	for (GPart* b : m_Part)
	{
		for (int j = 0; j < b->m_face.size(); ++j)
		{
			b->m_face[j] = m_Face[b->m_face[j]]->m_ntag;
			assert(b->m_face[j] >= 0);
		}
	}

	// now, delete the actual faces
	n = 0;
	for (int i = 0; i < m_Face.size(); ++i)
	{
		if (m_Face[i]->m_ntag == -1)
		{
			delete m_Face[i];
			m_Face.erase(m_Face.begin() + i);
			i--;
		}
		else m_Face[i]->SetLocalID(n++);
	}

	// figure out which edges can be deleted
	for (GEdge* edge : m_Edge) edge->m_ntag = 0;
	for (GFace* face : m_Face)
	{
		for (int j = 0; j < 4; ++j)
		{
			m_Edge[face->m_edge[j].nid]->m_ntag = 1;
		}
	}

	// assign new IDs (stored in tag)
	n = 0;
	for (GEdge* edge : m_Edge) {
		if (edge->m_ntag == 0) edge->m_ntag = -1;
		else edge->m_ntag = n++;
	};

	// update block edges
	for (GPart* b : m_Part)
	{
		for (int j = 0; j < b->m_edge.size(); ++j)
		{
			b->m_edge[j] = m_Edge[b->m_edge[j]]->m_ntag;
			assert(b->m_edge[j] >= 0);
		}
	}

	// update face edges
	for (GFace* f : m_Face)
	{
		for (int j = 0; j < 4; ++j)
		{
			f->m_edge[j].nid = m_Edge[f->m_edge[j].nid]->m_ntag;
			assert(f->m_edge[j].nid >= 0);
		}
	}

	// delete edges
	n = 0;
	for (int i = 0; i < m_Edge.size(); ++i)
	{
		if (m_Edge[i]->m_ntag == -1)
		{
			delete m_Edge[i];
			m_Edge.erase(m_Edge.begin() + i);
			--i;
		}
		else m_Edge[i]->SetLocalID(n++);
	}

	// see if any nodes can be deleted
	for (GNode* node : m_Node) node->m_ntag = 0;
	for (GEdge* edge : m_Edge)
	{
		m_Node[edge->m_node[0]]->m_ntag = 1;
		m_Node[edge->m_node[1]]->m_ntag = 1;
		if (!edge->m_cnode.empty())
		{
			m_Node[edge->m_cnode[0]]->m_ntag = 1;
		}
	}

	n = 0;
	for (GNode* node : m_Node)
	{
		if (node->m_ntag == 0) node->m_ntag = -1;
		else node->m_ntag = n++;
	}

	// update block nodes
	for (GPart* b : m_Part) {
		for (int j = 0; j < b->m_node.size(); ++j) {
			b->m_node[j] = m_Node[b->m_node[j]]->m_ntag;
			assert(b->m_node[j] >= 0);
		}
	}

	// update all face nodes
	for (GFace* f : m_Face) {
		for (int j = 0; j < 4; ++j) {
			f->m_node[j] = m_Node[f->m_node[j]]->m_ntag;
			assert(f->m_node[j] >= 0);
		}
	}

	// update all edge nodes
	for (GEdge* e : m_Edge) {
		e->m_node[0] = m_Node[e->m_node[0]]->m_ntag; assert(e->m_node[0] >= 0);
		e->m_node[1] = m_Node[e->m_node[1]]->m_ntag; assert(e->m_node[1] >= 0);
		if (!e->m_cnode.empty())
		{
			e->m_cnode[0] = m_Node[e->m_cnode[0]]->m_ntag;
			assert(e->m_cnode[0] >= 0);
		}
	}

	// delete nodes
	n = 0;
	for (int i = 0; i < m_Node.size(); ++i)
	{
		if (m_Node[i]->m_ntag == -1)
		{
			delete m_Node[i];
			m_Node.erase(m_Node.begin() + i);
			--i;
		}
		else m_Node[i]->SetLocalID(n++);
	}

	// rebuild the render mesh
	SetRenderMesh(nullptr);

	return true;
}

GObject* GMultiPatch::Clone()
{
	GMultiPatch* clone = new GMultiPatch();

	// copy nodes
	for (int i = 0; i < Nodes(); ++i)
	{
		GNode& ni = *Node(i);
		GNode* newNode = clone->AddNode(ni.LocalPosition(), ni.Type());
		newNode->SetMeshWeight(ni.GetMeshWeight());
	}

	// copy edges
	for (int i = 0; i < Edges(); ++i)
	{
		GEdge& ei = *Edge(i);
		GEdge* ec = clone->AddEdge();

		ec->m_node[0] = ei.m_node[0];
		ec->m_node[1] = ei.m_node[1];
		ec->m_cnode = ei.m_cnode;
		ec->m_ntype = ei.m_ntype;
		ec->m_orient = ei.m_orient;
		ec->SetMeshWeight(ei.GetMeshWeight());
	}

	// copy faces
	for (int i = 0; i < Faces(); ++i)
	{
		GFace& fi = *Face(i);
		GFace* fc = clone->AddFace();
		fc->m_ntype = fi.m_ntype;
		fc->m_nPID[0] = fi.m_nPID[0];
		fc->m_nPID[1] = fi.m_nPID[1];
		fc->m_nPID[2] = fi.m_nPID[2];
		fc->m_node = fi.m_node;
		fc->m_edge = fi.m_edge;
		fc->SetMeshWeight(fi.GetMeshWeight());
	}

	// copy parts
	for (int i = 0; i < Parts(); ++i)
	{
		GPart& pi = *Part(i);
		GPart* pc = clone->AddPart();
		pc->m_node = pi.m_node;
		pc->m_edge = pi.m_edge;
		pc->m_face = pi.m_face;
		pc->SetMeshWeight(pi.GetMeshWeight());
	}

	clone->Update();

	return clone;
}

bool GMultiPatch::Merge(GMultiPatch& mb)
{
	// delete the existing mesh
	delete GetFEMesh();
	SetFEMesh(nullptr);

	const double tol = 1e-12;

	// The tag will be set to the node on this that it corresponds to. 
	// new nodes may be added
	for (int i = 0; i < mb.Nodes(); ++i)
	{
		GNode& ni = *mb.Node(i);

		// get the global position of the node
		vec3d r0 = ni.Position();

		// transform to this coordinate system
		vec3d ri = GetTransform().GlobalToLocal(r0);

		// see if we already have this node
		int jmin = -1;
		double D2min = 0.0;
		for (int j = 0; j < Nodes(); ++j)
		{
			GNode& nj = *Node(j);
			vec3d rj = nj.LocalPosition();

			double D2 = (ri - rj).SqrLength();
			if ((jmin == -1) || (D2 < D2min))
			{
				jmin = j;
				D2min = D2;
			}
		}
		assert(jmin != -1);

		if (D2min < tol)
		{
			ni.m_ntag = jmin;
		}
		else
		{
			GNode& newNode = *AddNode(ri);
			ni.m_ntag = Nodes() - 1;
			newNode.SetMeshWeight(ni.GetMeshWeight());
		}
	}

	// add all edges
	for (int i = 0; i < mb.Edges(); ++i)
	{
		GEdge& ei = *mb.Edge(i);
		int n0 = mb.Node(ei.m_node[0])->m_ntag; assert(n0 >= 0);
		int n1 = mb.Node(ei.m_node[1])->m_ntag; assert(n1 >= 0);
		int n2 = (ei.m_cnode.empty() ? -1 : mb.Node(ei.m_cnode[0])->m_ntag);
		assert((ei.m_cnode.empty()) || (n2 >= 0));

		// see if we already have this edge
		ei.m_ntag = -1;
		for (int j = 0; j < Edges(); ++j)
		{
			GEdge& ej = *Edge(j);
			int m0 = ej.m_node[0];
			int m1 = ej.m_node[1];

			if ((n0 == m0) && (n1 == m1))
			{
				assert(ei.m_ntype == ej.m_ntype);
				ei.m_ntag = j;
				break;
			}
			else if ((n0 == m1) && (n1 == m0))
			{
				assert(ei.m_ntype == ej.m_ntype);
				ei.m_ntag = j;
				break;
			}
		}

		if (ei.m_ntag == -1)
		{
			ei.m_ntag = m_Edge.size();
			GEdge* newEdge = AddEdge();
			newEdge->m_node[0] = n0;
			newEdge->m_node[1] = n1;
			if (n2 >=0) newEdge->m_cnode.push_back(n2);
			newEdge->m_ntype = ei.m_ntype;
			newEdge->m_orient = ei.m_orient;
			newEdge->SetMeshWeight(ei.GetMeshWeight());
		}
	}

	// add all faces
	for (int i = 0; i < mb.Faces(); ++i)
	{
		GFace& fi = *mb.Face(i);
		int n[4];
		for (int l = 0; l < 4; ++l) {
			n[l] = mb.Node(fi.m_node[l])->m_ntag; assert(n[l] >= 0);
		}

		// see if we already have this face
		fi.m_ntag = -1;
		for (int j = 0; j < Faces(); ++j)
		{
			GFace& fj = *Face(j);
			int m[4];
			for (int l = 0; l < 4; ++l) m[l] = fj.m_node[l];

			if (IsSameFace(n, m))
			{
				fi.m_ntag = j;

				assert(fi.m_nPID[1] == -1);
				assert(fj.m_nPID[1] == -1);

				fj.m_nPID[1] = fi.m_nPID[0] + Parts();
				break;
			}
		}

		if (fi.m_ntag == -1)
		{
			fi.m_ntag = m_Face.size();
			GFace& newFace = *AddFace();
			newFace.m_ntype = fi.m_ntype;
			newFace.SetMeshWeight(fi.GetMeshWeight());
			newFace.m_node.resize(4);
			newFace.m_edge.resize(4);
			for (int l = 0; l < 4; ++l) newFace.m_node[l] = n[l];
			for (int l = 0; l < 4; ++l)
			{
				int n0 = newFace.m_node[l];
				int n1 = newFace.m_node[(l + 1) % 4];
				newFace.m_edge[l].nid = mb.Edge(fi.m_edge[l].nid)->m_ntag;

				GEdge& edge = *Edge(newFace.m_edge[l].nid);
				if ((edge.m_node[0] == n0) && (edge.m_node[1] == n1)) newFace.m_edge[l].nwn = 1;
				else if ((edge.m_node[0] == n1) && (edge.m_node[1] == n0)) newFace.m_edge[l].nwn = -1;
				else assert(false);
			}
			if (fi.m_nPID[0] >= 0) newFace.m_nPID[0] = fi.m_nPID[0] + Parts();
			if (fi.m_nPID[1] >= 0) newFace.m_nPID[1] = fi.m_nPID[1] + Parts();
		}
	}

	// add new parts
	for (int i = 0; i < mb.Parts(); ++i)
	{
		GPart& bi = *mb.Part(i);
		GPart& newPart = *AddPart();
		newPart.SetMeshWeight(bi.GetMeshWeight());
		newPart.m_node.resize(4);
		newPart.m_edge.resize(4);
		newPart.m_face.resize(1);
		for (int l = 0; l < 4; ++l) newPart.m_node[l] = mb.Node(bi.m_node[l])->m_ntag;
		for (int l = 0; l < 4; ++l) newPart.m_edge[l] = mb.Edge(bi.m_edge[l])->m_ntag;
		for (int l = 0; l < 1; ++l) newPart.m_face[l] = mb.Face(bi.m_face[l])->m_ntag;
	}

	// rebuild the render mesh
	SetRenderMesh(nullptr);

	return true;
}
