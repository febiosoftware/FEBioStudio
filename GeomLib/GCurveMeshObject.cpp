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

#include "GCurveMeshObject.h"
#include <MeshLib/FSCurveMesh.h>
#include <MeshLib/FSMesh.h>

GCurveMeshObject::GCurveMeshObject(FSCurveMesh* pm) : m_curve(pm), GObject(GCURVEMESH_OBJECT)
{
	if (m_curve) 
	{
		pm->SetGObject(this);
		Update();
	}
}

// return the curve mesh
FSCurveMesh* GCurveMeshObject::GetCurveMesh()
{
	return m_curve;
}

FSLineMesh* GCurveMeshObject::GetEditableLineMesh()
{ 
	return GetCurveMesh(); 
}

void GCurveMeshObject::ClearMesh()
{
	m_curve->Clear();
	Update();
}

void GCurveMeshObject::Update()
{
	if (m_curve == nullptr)
	{
		m_Node.clear();
		m_Edge.clear();
		SetRenderMesh(nullptr);
		return;
	}

	// get the end points
	std::vector<int> endPoints = m_curve->EndPointList();
	if (endPoints.size() < m_Node.size())
	{
		// shrink the node list
		ResizeNodes(endPoints.size());
	}
	else if (endPoints.size() > m_Node.size())
	{
		// add new nodes
		for (int i = m_Node.size(); i < endPoints.size(); ++i)
		{
			GNode* newNode = new GNode(this);
			newNode->SetType(NODE_VERTEX);
			AddNode(newNode);
		}
	}

	// update nodal positions
	for (int i=0; i<endPoints.size(); ++i)
	{
		m_Node[i]->LocalPosition() = m_curve->Node(endPoints[i]).r;
	}

	// count segments
	int segments = m_curve->Segments();
	if (segments < m_Edge.size())
	{
		// shrink edge list
		ResizeCurves(segments);
	}
	else if (segments > m_Edge.size())
	{
		for (int i = m_Edge.size(); i<segments; ++i)
		{
			// add new edges
			GEdge* e = new GEdge(this);
			e->m_node[0] = -1;
			e->m_node[1] = -1;
			e->m_ntype = EDGE_MESH;
			AddEdge(e);
		}
	}

	// update edges
	for (int i=0; i<Edges(); ++i) 
	{
		Edge(i)->m_node[0] = -1;
		Edge(i)->m_node[1] = -1;
	}

	for (int i = 0; i<m_curve->Edges(); ++i)
	{
		FSEdge& ei = m_curve->Edge(i);
		int eid = ei.m_gid;
		assert((eid >= 0) && (eid < m_Edge.size()));

		GEdge& edge = *m_Edge[eid];

		if (ei.m_nbr[0] == -1)
		{
			int n0 = ei.n[0];

			if      (edge.m_node[0] == -1) edge.m_node[0] = m_curve->Node(n0).m_gid;
			else if (edge.m_node[1] == -1) edge.m_node[1] = m_curve->Node(n0).m_gid;
			else assert(false);
		}

		if (ei.m_nbr[1] == -1)
		{
			int n1 = ei.n[1];

			if      (edge.m_node[0] == -1) edge.m_node[0] = m_curve->Node(n1).m_gid;
			else if (edge.m_node[1] == -1) edge.m_node[1] = m_curve->Node(n1).m_gid;
			else assert(false);
		}
	}

	SetRenderMesh(nullptr);
}

//-----------------------------------------------------------------------------
// Return a curve mesh for edge with ID edgeId
FSCurveMesh* GCurveMeshObject::GetFECurveMesh(int edgeId)
{
	if (m_curve == 0) return 0;

	m_curve->TagAllNodes(-1);
	int NC = m_curve->Edges();
	int ne = 0;
	for (int i = 0; i<NC; ++i)
	{
		FSEdge& e = m_curve->Edge(i);
		if (e.m_gid == edgeId)
		{
			m_curve->Node(e.n[0]).m_ntag = 0;
			m_curve->Node(e.n[1]).m_ntag = 0;
			ne++;
		}
	}

	FSCurveMesh* curve = new FSCurveMesh;

	int NN = m_curve->Nodes();
	int nn = 0;
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = m_curve->Node(i);
		if (node.m_ntag != -1)
		{
			node.m_ntag = nn++;
			vec3d r = GetTransform().LocalToGlobal(node.r);
			curve->AddNode(r, false);
		}
	}

	for (int i = 0; i<NC; ++i)
	{
		FSEdge& sedge = m_curve->Edge(i);
		if (sedge.m_gid == edgeId)
		{
			int n0 = m_curve->Node(sedge.n[0]).m_ntag;
			int n1 = m_curve->Node(sedge.n[1]).m_ntag;
			curve->AddEdge(n0, n1);
		}
	}

	// update the curve
	curve->BuildMesh();

	return curve;
}

void GCurveMeshObject::Load(IArchive& ar)
{
	TRACE("GCurveMeshObject::Load");

	int nparts = -1, nedges = -1, nnodes = -1;

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
			// object name
		case CID_OBJ_NAME:
		{
			std::string name;
			ar.read(name);
			SetName(name);
		}
		break;
		case CID_FEOBJ_INFO:
		{
			std::string info;
			ar.read(info);
			SetInfo(info);
		}
		break;
		// header
		case CID_OBJ_HEADER:
		{
			vec3d pos, scl;
			quatd rot;
			GLMaterial mat;
			mat.type = GLMaterial::PLASTIC;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				int oid;
				switch (nid)
				{
				case CID_OBJ_ID: ar.read(oid); SetID(oid); break;
				case CID_OBJ_POS: ar.read(pos); break;
				case CID_OBJ_ROT: ar.read(rot); break;
				case CID_OBJ_SCALE: ar.read(scl); break;
				case CID_OBJ_COLOR: { ar.read(mat.diffuse); mat.ambient = mat.diffuse; } break;
				case CID_OBJ_PARTS: ar.read(nparts); break;
				case CID_OBJ_EDGES: ar.read(nedges); break;
				case CID_OBJ_NODES: ar.read(nnodes); break;
				case CID_MAT_AMBIENT   : ar.read(mat.ambient); break;
				case CID_MAT_DIFFUSE   : ar.read(mat.diffuse); break;
				case CID_MAT_SPECULAR  : ar.read(mat.specular); break;
				case CID_MAT_EMISSION  : ar.read(mat.emission); break;
				case CID_MAT_SHININESS : ar.read(mat.shininess); break;
				case CID_MAT_OPACITY   : ar.read(mat.opacity); break;
				case CID_MAT_REFLECTION: ar.read(mat.reflection); break;
				}
				ar.CloseChunk();
			}

			SetMaterial(mat);

			Transform& transform = GetTransform();
			transform.SetPosition(pos);
			transform.SetRotation(rot);
			transform.SetScale(scl);
		}
		break;
		// object parameters
		case CID_OBJ_PARAMS:
			ParamContainer::Load(ar);
			break;
			// object parts
		case CID_OBJ_PART_LIST:
		{
			assert(nparts > 0);
			m_Part.reserve(nparts);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_LIST");

				GPart* p = new GPart(this);
				p->Load(ar);
				p->SetLocalID(n++);
				m_Part.push_back(p);

				ar.CloseChunk();
			}
			assert((int)m_Part.size() == nparts);
		}
		break;
		// object edges
		case CID_OBJ_EDGE_LIST:
		{
			m_Edge.clear();
			if (nedges > 0) m_Edge.reserve(nedges);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_LIST");

				GEdge* e = new GEdge(this);
				e->Load(ar);
				e->SetLocalID(n++);
				m_Edge.push_back(e);

				ar.CloseChunk();
			}
			assert((int)m_Edge.size() == nedges);
		}
		break;
		// object nodes
		case CID_OBJ_NODE_LIST:
		{
			m_Node.clear();
			if (nnodes > 0)
			{
				m_Node.reserve(nnodes);
				int m = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_LIST");

					GNode* n = new GNode(this);
					n->Load(ar);
					n->SetLocalID(m++);
					m_Node.push_back(n);

					ar.CloseChunk();
				}
				assert((int)m_Node.size() == nnodes);
			}
		}
		break;
		// the mesh object
		case CID_MESH:
			if (GetFEMesh()) delete GetFEMesh();
			SetFEMesh(new FSMesh);
			GetFEMesh()->Load(ar);
			break;
		case CID_CURVE_MESH:
			if (m_curve) delete m_curve;
			m_curve = new FSCurveMesh;
			m_curve->SetGObject(this);
			m_curve->Load(ar);
			break;
		}
		ar.CloseChunk();
	}

	SetRenderMesh(nullptr);
}
