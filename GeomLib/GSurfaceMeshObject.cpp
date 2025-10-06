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

#include "GSurfaceMeshObject.h"
#include <MeshTools/FETetGenMesher.h>
#include <MeshTools/FEShellMesher.h>
#include <MeshTools/FEModifier.h>
#include <MeshLib/FSCurveMesh.h>
#include <GLLib/GLMesh.h>
#include <MeshLib/FSNodeEdgeList.h>
#include <FSCore/ColorMap.h>
#include "GOCCObject.h"
#include <FSCore/ClassDescriptor.h>

GSurfaceMeshObject::GSurfaceMeshObject(FSSurfaceMesh* pm) : GObject(GSURFACEMESH_OBJECT), m_surfmesh(pm)
{
	SetFEMesher(new FETetGenMesher(*this));
	if (m_surfmesh)
	{
		m_surfmesh->SetGObject(this);
		Update();
	}
}

GSurfaceMeshObject::GSurfaceMeshObject(GObject* po) : GObject(GSURFACEMESH_OBJECT)
{
	SetFEMesher(new FETetGenMesher(*this));

	// next, we copy the geometry info
	// --- Nodes ---
	int NN = po->Nodes();
	m_Node.reserve(NN);
	for (int i = 0; i<NN; ++i)
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
	for (int i = 0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0];
		e->m_node[1] = eo.m_node[1];
		e->SetID(eo.GetID());
		e->SetLocalID(i);
		e->SetName(eo.GetName());
		assert(e->GetLocalID() == eo.GetLocalID());
		m_Edge.push_back(e);
	}

	// --- Faces ---
	int NF = po->Faces();
	m_Face.reserve(NF);
	for (int i = 0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		GFace& fo = *po->Face(i);
		f->m_node = fo.m_node;
		f->m_nPID[0] = fo.m_nPID[0];
		f->m_nPID[1] = fo.m_nPID[1];
		f->m_nPID[2] = fo.m_nPID[2];
		f->SetID(fo.GetID());
		f->SetLocalID(i);
		f->SetName(fo.GetName());
		assert(f->GetLocalID() == fo.GetLocalID());
		m_Face.push_back(f);
	}

	// --- Parts ---
	int NP = po->Parts();
	m_Part.reserve(NP);
	for (int i = 0; i<NP; ++i)
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

	// copy the surface mesh from the original object's mesh
	FSMeshBase* pm = po->GetEditableMesh(); assert(pm);

	FSSurfaceMesh* psm = dynamic_cast<FSSurfaceMesh*>(pm);
	if (psm)
	{
		m_surfmesh = new FSSurfaceMesh(*psm);
		m_surfmesh->SetGObject(this);
	}
	else
	{
		m_surfmesh = new FSSurfaceMesh;
		m_surfmesh->SetGObject(this);

		NN = pm->Nodes();
		for (int i = 0; i < NN; ++i) pm->Node(i).m_ntag = -1;
		NF = pm->Faces();
		for (int i = 0; i < NF; ++i)
		{
			FSFace& f = pm->Face(i);
			int nf = f.Nodes();
			for (int j = 0; j < nf; ++j) pm->Node(f.n[j]).m_ntag = 1;
		}

		int nodes = 0;
		for (int i = 0; i < NN; ++i)
		{
			FSNode& node = pm->Node(i);
			if (node.m_ntag == 1) node.m_ntag = nodes++;
		}

		// create new mesh
		m_surfmesh->Create(nodes, 0, NF);

		// copy nodes
		for (int i = 0; i < NN; ++i)
		{
			FSNode& node = pm->Node(i);
			if (node.m_ntag >= 0)
			{
				FSNode& snode = m_surfmesh->Node(node.m_ntag);
				snode = node;
			}
		}
		m_surfmesh->UpdateNodePartitions();

		// copy faces
		for (int i = 0; i < NF; ++i)
		{
			FSFace& f = m_surfmesh->Face(i);
			f = pm->Face(i);
			int nf = f.Nodes();
			for (int j = 0; j < nf; ++j) f.n[j] = pm->Node(f.n[j]).m_ntag;
		}
		m_surfmesh->UpdateFacePartitions();
		m_surfmesh->UpdateFaceNeighbors();
		m_surfmesh->UpdateNormals();
		m_surfmesh->UpdateBoundingBox();

		// copy edges
		// Build a node-edge tabel
		FSNodeEdgeList NEL(m_surfmesh);
		m_surfmesh->BuildEdges();
		for (int i = 0; i < pm->Edges(); ++i)
		{
			FSEdge& src = pm->Edge(i);
			int n0 = pm->Node(src.n[0]).m_ntag;
			int n1 = pm->Node(src.n[1]).m_ntag;

			FSEdge* pe = nullptr;
			const std::vector<NodeEdgeRef>& el = NEL.EdgeList(n0);
			for (int k = 0; k < el.size(); ++k)
			{
				FSEdge& e = *el[k].pe;
				if (((e.n[0] == n0) && (e.n[1] == n1)) ||
					((e.n[0] == n1) && (e.n[1] == n0)))
				{
					FSEdge* pe = &e;
					break;
				}
			}

			if (pe)
			{
				pe->m_gid = src.m_gid;
			}
		}
		m_surfmesh->UpdateEdgePartitions();
		m_surfmesh->UpdateEdgeNeighbors();
		m_surfmesh->AutoPartitionNodes();
	}

	// update the object
	Update();
}

// default mesher
FEMesher* GSurfaceMeshObject::CreateDefaultMesher()
{
	return new FETetGenMesher(*this);
}

FSMesh* GSurfaceMeshObject::BuildMesh()
{
	// make sure that the surface is triangular
	int NF = m_surfmesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = m_surfmesh->Face(i);

		// Only triangles, quads are accepted
		int nf = face.Nodes();
		if ((nf != 3) && (nf != 4)) return 0;
	}

	// get the mesher
	FEMesher* mesher = GetFEMesher();

	// keep a pointer to the old mesh
	FSMesh* pold = GetFEMesh();

	// create a new mesh
	FSMesh* pmesh = mesher->BuildMesh();
	SetFEMesh(pmesh);

	// now it is safe to delete the old mesh
	if (pold) delete pold;

	return pmesh;
}

void GSurfaceMeshObject::Update()
{
	// create one part
	if (Parts() == 0) AddSolidPart();

	// Add surfaces
	UpdateSurfaces();

	// update nodes
	UpdateNodes();

	// update edges
	UpdateEdges();

	// rebuild the render mesh
	SetRenderMesh(nullptr);
}

//-----------------------------------------------------------------------------
// clone function
GObject* GSurfaceMeshObject::Clone()
{
	GSurfaceMeshObject* po = new GSurfaceMeshObject(this);
	return po;
}

//-----------------------------------------------------------------------------
void GSurfaceMeshObject::UpdateNodes()
{
	// get the mesh
	FSSurfaceMesh& m = *GetSurfaceMesh();

	// count the node partitions
	int ng = m.CountNodePartitions();

	// create the Nodes
	if (ng < (int) m_Node.size()) ResizeNodes(ng);
	else if (ng >(int) m_Node.size())
	{
		for (int i= (int)m_Node.size(); i<ng; ++i)
		{
			GNode* node = new GNode(this);
			node->SetType(NODE_VERTEX);
			AddNode(node);
		}
	}

	// reset nodes
	int NN = m.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = m.Node(i);
		if (node.m_gid >= 0)
		{
			GNode& n = *m_Node[node.m_gid];
			n.LocalPosition() = node.r;
		}
	}
}

//-----------------------------------------------------------------------------
void GSurfaceMeshObject::UpdateEdges()
{
	// get the mesh
	FSSurfaceMesh& m = *GetSurfaceMesh();
	int NE = m.Edges();

	int ng = m.CountEdgePartitions();
	for (int i = 0; i<NE; ++i)

	// create the Edge
	if (ng < (int)m_Edge.size()) ResizeCurves(ng);
	else if (ng >(int) m_Edge.size())
	{
		for (int i = (int)m_Edge.size(); i<ng; ++i)
		{
			GEdge* e = new GEdge(this);
			e->m_node[0] = -1;
			e->m_node[1] = -1;
			AddEdge(e);
		}
	}

	// reset edge nodes
	for (int i = 0; i<ng; ++i)
	{
		GEdge& e = *m_Edge[i];
		e.m_node[0] = -1;
		e.m_node[1] = -1;
	}

	// set the nodes for the GEdge
	const int NN = Nodes();
	for (int i = 0; i<NE; ++i)
	{
		FSEdge& e = m.Edge(i);
		if (e.m_gid >= 0)
		{
			for (int j = 0; j<2; ++j)
			{
				if (e.m_nbr[j] == -1)
				{
					GEdge& ge = *m_Edge[e.m_gid];
					if (ge.m_node[0] == -1)
					{
						FSNode& nj = m.Node(e.n[j]);
						if ((nj.m_gid >= 0) && (nj.m_gid < NN))
						{
							ge.m_node[0] = m_Node[nj.m_gid]->GetLocalID();
						}
//						else assert(false);
						
					}
					else if (ge.m_node[1] == -1)
					{
						FSNode& nj = m.Node(e.n[j]);
						if ((nj.m_gid >= 0) && (nj.m_gid < NN))
						{
							ge.m_node[1] = m_Node[nj.m_gid]->GetLocalID();
						}
//						else assert(false);						
					}
				}
			}
		}
	}
}

void GSurfaceMeshObject::UpdateSurfaces()
{
	FSSurfaceMesh& m = *GetSurfaceMesh();
	int NF = m.Faces();

	int ng = m.CountFacePartitions();
	if (ng > Faces())
	{
		for (int i=Faces(); i<ng; ++i)
		{
			GFace* face = new GFace(this);
			face->m_nPID[0] = 0;
			AddFace(face);
		}
	}
	else if (ng < Faces()) ResizeSurfaces(ng);
}


void GSurfaceMeshObject::BuildGMesh()
{
	// allocate new GL mesh
	GLMesh* gmesh = new GLMesh();

	// we'll extract the data from the FE mesh
	FSSurfaceMesh* pm = m_surfmesh;

	// create nodes
	for (int i = 0; i<pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		gmesh->AddNode(to_vec3f(node.r), node.m_gid);
	}

	// create edges
	int max_gid = -1;
	for (int i = 0; i<pm->Edges(); ++i)
	{
		FSEdge& es = pm->Edge(i);
		if (es.m_gid >= 0)
		{
			if (es.m_gid > max_gid) max_gid = es.m_gid;
			gmesh->AddEdge(es.n, es.Nodes(), es.m_gid);
		}
	}
	max_gid++;

	// create face data
	for (int i = 0; i < pm->Faces(); ++i) pm->Face(i).m_ntag = i;
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FSFace& fs = pm->Face(i);
		gmesh->AddFace(fs.n, fs.Nodes(), fs.m_gid, fs.m_sid, true, i);

		// add additional edges for rendering meshlines
		int ne = fs.Edges();
		for (int j = 0; j < ne; ++j)
		{
			FSFace* pf = pm->FacePtr(fs.m_nbr[j]);
			if ((pf == nullptr) || !pf->IsVisible() || (fs.m_ntag < pf->m_ntag))
			{
				FSEdge e = fs.GetEdge(j);
				gmesh->AddEdge(e.n, e.Nodes(), max_gid);
			}
		}
	}

	gmesh->AutoPartition();
	gmesh->Update();

	// The update sorted the edges, so the edges for rendering meshlines will
	// be at the back. However, they still have a pid set which will causes these
	// edges to render as feature edges. So, we need to set them to -1
	for (int i = 0; i < gmesh->Edges(); ++i)
	{
		GLMesh::EDGE& edge = gmesh->Edge(i);
		if (edge.pid == max_gid) edge.pid = -1;
	}

	SetRenderMesh(gmesh);
}

FSSurfaceMesh* GSurfaceMeshObject::GetSurfaceMesh()
{
	return m_surfmesh;
}

const FSSurfaceMesh* GSurfaceMeshObject::GetSurfaceMesh() const
{
	return m_surfmesh;
}

void GSurfaceMeshObject::ReplaceSurfaceMesh(FSSurfaceMesh* newMesh)
{
	m_surfmesh = newMesh;
	m_surfmesh->SetGObject(this);
	Update();
}

// get the mesh of an edge curve
FSCurveMesh* GSurfaceMeshObject::GetFECurveMesh(int edgeId)
{
	FSSurfaceMesh* mesh = GetSurfaceMesh();
	if (mesh == 0) return 0;

	mesh->TagAllNodes(-1);
	int NC = mesh->Edges();
	int ne = 0;
	for (int i = 0; i<NC; ++i)
	{
		FSEdge& e = mesh->Edge(i);
		if (e.m_gid == edgeId)
		{
			mesh->Node(e.n[0]).m_ntag = 0;
			mesh->Node(e.n[1]).m_ntag = 0;
			ne++;
		}
	}

	FSCurveMesh* curve = new FSCurveMesh;

	int NN = mesh->Nodes();
	int nn = 0;
	for (int i = 0; i<NN; ++i)
	{
		FSNode& node = mesh->Node(i);
		if (node.m_ntag != -1)
		{
			node.m_ntag = nn++;
			vec3d r = GetTransform().LocalToGlobal(node.r);
			curve->AddNode(r, false);
		}
	}

	for (int i = 0; i<NC; ++i)
	{
		FSEdge& sedge = mesh->Edge(i);
		if (sedge.m_gid == edgeId)
		{
			int n0 = mesh->Node(sedge.n[0]).m_ntag;
			int n1 = mesh->Node(sedge.n[1]).m_ntag;
			curve->AddEdge(n0, n1);
		}
	}

	curve->BuildMesh();

	return curve;
}

// serialization
void GSurfaceMeshObject::Save(OArchive& ar)
{
	// Save base class data
	GObject::Save(ar);

	// save the surface mesh
	if (m_surfmesh)
	{
		ar.BeginChunk(CID_SURFACE_MESH);
		{
			m_surfmesh->Save(ar);
		}
		ar.EndChunk();
	}
}

void GSurfaceMeshObject::Load(IArchive& ar)
{
	TRACE("GSurfaceMeshObject::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;

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
		// object info
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
			GLColor col;
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
				case CID_OBJ_COLOR: ar.read(col); break;
				case CID_OBJ_PARTS: ar.read(nparts); break;
				case CID_OBJ_FACES: ar.read(nfaces); break;
				case CID_OBJ_EDGES: ar.read(nedges); break;
				case CID_OBJ_NODES: ar.read(nnodes); break;
				}
				ar.CloseChunk();
			}

			SetColor(col);

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
		// object surfaces
		case CID_OBJ_FACE_LIST:
		{
			assert(nfaces > 0);
			m_Face.reserve(nfaces);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_LIST");

				GFace* f = new GFace(this);
				f->Load(ar);
				f->SetLocalID(n++);
				m_Face.push_back(f);

				ar.CloseChunk();
			}
			assert((int)m_Face.size() == nfaces);
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
		// mesher object
		case CID_OBJ_FEMESHER:
		{
			if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
			else
			{
				int ntype = ar.GetChunkID();
				FEMesher* mesher = FSCore::CreateClassFromID<FEMesher>(CLASS_MESHER, ntype);
				if (mesher)
				{
					SetFEMesher(mesher);
					mesher->Load(ar);
				}
			}
			ar.CloseChunk();
			if (ar.OpenChunk() != IArchive::IO_END) throw ReadError("error parsing CID_OBJ_FEMESHER (GSurfaceMeshObjects::Load)");
		}
		break;
		// the mesh object
		case CID_MESH:
			if (GetFEMesh()) delete GetFEMesh();
			SetFEMesh(new FSMesh);
			GetFEMesh()->Load(ar);
			break;
		case CID_SURFACE_MESH:
			if (m_surfmesh) delete m_surfmesh;
			m_surfmesh = new FSSurfaceMesh;
			m_surfmesh->SetGObject(this);
			m_surfmesh->Load(ar);
			break;
		}
		ar.CloseChunk();
	}

	UpdateFEElementMatIDs();
	UpdateItemVisibility();

	// rebuild the render meshes
	SetRenderMesh(nullptr);
	SetFERenderMesh(nullptr);
}

void GSurfaceMeshObject::Attach(const GSurfaceMeshObject* po, bool weld, double weldTolerance)
{
	// Add the nodes
	int NN0 = Nodes();
	int NN = po->Nodes();
	for (int i = 0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		const GNode& no = *po->Node(i);
		n->LocalPosition() = GetTransform().GlobalToLocal(po->GetTransform().LocalToGlobal(no.LocalPosition()));
		n->SetID(no.GetID());
		n->SetLocalID(i + NN0);
		n->SetType(no.Type());
		n->SetName(no.GetName());
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE0 = Edges();
	int NE = po->Edges();
	for (int i = 0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		const GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0] + NN0;
		e->m_node[1] = eo.m_node[1] + NN0;
		if (!eo.m_cnode.empty())
			e->m_cnode.push_back(eo.m_cnode[0] + NN0);
		e->SetID(eo.GetID());
		e->SetLocalID(i + NE0);
		e->SetName(eo.GetName());
		m_Edge.push_back(e);
	}

	// --- Parts ---
	int NP0 = Parts();
	int NP = po->Parts();
	for (int i = 0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		const GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i + NP0);
		g->SetName(go.GetName());
		m_Part.push_back(g);
	}

	// --- Faces ---
	int NF0 = Faces();
	int NF = po->Faces();
	for (int i = 0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		const GFace& fo = *po->Face(i);

		f->m_node = fo.m_node;
		for (int j = 0; j<(int)fo.m_node.size(); ++j)
		{
			f->m_node[j] = fo.m_node[j] + NN0;
		}

		f->m_nPID[0] = (fo.m_nPID[0] >= 0 ? fo.m_nPID[0] + NP0 : -1);
		f->m_nPID[1] = (fo.m_nPID[1] >= 0 ? fo.m_nPID[1] + NP0 : -1);
		f->m_nPID[2] = (fo.m_nPID[2] >= 0 ? fo.m_nPID[2] + NP0 : -1);
		f->SetID(fo.GetID());
		f->SetLocalID(i + NF0);
		f->SetName(fo.GetName());
		m_Face.push_back(f);
	}

	// attach to the new mesh
	const FSSurfaceMesh* oldMesh = po->GetSurfaceMesh();
	FSSurfaceMesh* newMesh = GetSurfaceMesh();
	if (weld)
	{
		newMesh->AttachAndWeld(*oldMesh, weldTolerance);
		Update();
	}
	else
	{
		newMesh->Attach(*oldMesh);
	}

	newMesh->UpdateMesh();

	// rebuild the render mesh
	SetRenderMesh(nullptr);
}

FSSurfaceMesh* createSurfaceMesh(GLMesh* glmesh)
{
	if (glmesh == nullptr) return nullptr;

	int NN = glmesh->Nodes();
	int NF = glmesh->Faces();

	FSSurfaceMesh* pm = new FSSurfaceMesh;
	pm->Create(NN, 0, NF);

	for (int i = 0; i < NN; ++i)
	{
		FSNode& nodei = pm->Node(i);
		const GLMesh::NODE& glnode = glmesh->Node(i);
		nodei.r = to_vec3d(glnode.r);
	}

	for (int i = 0; i < NF; ++i)
	{
		FSFace& facei = pm->Face(i);
		const GLMesh::FACE& glface = glmesh->Face(i);
		facei.SetType(FE_FACE_TRI3);
		facei.m_gid = glface.pid;
		facei.n[0] = glface.n[0];
		facei.n[1] = glface.n[1];
		facei.n[2] = glface.n[2];
	}
	pm->BuildMesh();

	return pm;
}

// Helper function for converting an object to an editable surface.
// This assumes that the object has a mesh.
GSurfaceMeshObject* ConvertToEditableSurface(GObject* po)
{
	// make sure there is something to do
	if (po == nullptr) return nullptr;

	// If the object has a mesh, we'll use that 
	GSurfaceMeshObject* pnew = nullptr;
	if (po->GetFEMesh())
	{
		pnew = new GSurfaceMeshObject(po);
	}
	else if (dynamic_cast<GOCCObject*>(po))
	{
		// If this is an OCC, we'll use the render mesh
		GLMesh* glmesh = po->GetRenderMesh();
		if (glmesh == nullptr) return nullptr;

		// create FSSurfaceMesh from GLMesh
		FSSurfaceMesh* surfMesh = createSurfaceMesh(glmesh);

		// create the surface mesh object
		pnew = new GSurfaceMeshObject(surfMesh);
	}
	else return nullptr;

	// copy data 
	pnew->SetName(po->GetName());

	// copy to old object's ID
	pnew->SetID(po->GetID());

	// creating a new object has increased the object counter
	// so we need to decrease it again
	GItem_T<GBaseObject>::DecreaseCounter();

	// copy data
	pnew->CopyTransform(po);
	pnew->SetColor(po->GetColor());

	// copy the selection state
	if (po->IsSelected()) pnew->Select();

	return pnew;
}

void GSurfaceMeshObject::UpdateSurfaceMeshData()
{
	GLMesh* gmsh = GetRenderMesh();
	if (gmsh == nullptr) return;

	FSSurfaceMesh* pm = GetSurfaceMesh();
	if (pm == nullptr) return;

	Mesh_Data& data = pm->GetMeshData();
	if (!data.IsValid()) return;

	double vmin, vmax;
	data.GetValueRange(vmin, vmax);
	if (vmax == vmin) vmax++;

	int NN = pm->Nodes();
	vector<double> val(NN, 0);

	CColorMap map;
	map.SetRange((float)vmin, (float)vmax);

	int NF = gmsh->Faces();
	for (int i = 0; i < NF; ++i)
	{
		GLMesh::FACE& fi = gmsh->Face(i);
		int fid = fi.fid;
		FSFace* pf = pm->FacePtr(fid);
		if (pf)
		{
			FSFace& face = *pf;
			if (data.GetElementDataTag(fid) > 0)
			{
				int ne = face.Nodes();
				for (int j = 0; j < ne; ++j)
				{
					double vj = data.GetElementValue(fid, j);
					val[face.n[j]] = vj;
				}

				for (int j = 0; j < 3; ++j)
				{
					double vj = val[fi.n[j]];
					fi.c[j] = map.map(vj);
				}
			}
			else
			{
				GLColor col(212, 212, 212);
				for (int j = 0; j < 3; ++j) fi.c[j] = col;
			}
		}
	}
}
