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
#include "GModifiedObject.h"
#include <FEMLib/FSModel.h>
#include <GLLib/GLMesh.h>

//-----------------------------------------------------------------------------
GModifiedObject::GModifiedObject(GObject* po) : GObject(GMODIFIED_OBJECT)
{
	m_po = 0;
	m_pStack = new GModifierStack(this);
	
	if (po) 
	{
		SetChildObject(po);
		Update();
	}
}

//-----------------------------------------------------------------------------
GModifiedObject::~GModifiedObject(void)
{
	// We have to delete the reference object ourselves
	delete m_po;

	// TODO: I think I can get rid of the ownership stuff
	if (m_pStack->GetOwner() == this) delete m_pStack;
	m_pStack = 0;
}

//-----------------------------------------------------------------------------
void GModifiedObject::SetChildObject(GObject *po, bool bclone)
{
	assert(m_po == 0);

	// set the reference object
	m_po = po;

	// copy ID's
	m_gid = po->GetID();
	m_lid = po->GetLocalID();

	// only copy this data if we are cloning the child-object
	// We usually clone, except when we load the object from file
	if (bclone)
	{
		// copy name
		SetName(po->GetName());

		// copy object transform data
		CopyTransform(po);

		// copy object appearance data
		SetColor(po->GetColor());

		// clone child data
		CloneChild();
	}

	Update(false);
}

//-----------------------------------------------------------------------------
void GModifiedObject::CloneChild()
{
	int i, N;

	// clear data
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	m_Part.clear();

	// copy object's nodal data
	N = m_po->Nodes(); for (i = 0; i<N; ++i) { GNode* node = new GNode(this); *node = *m_po->Node(i); m_Node.push_back(node); }
	N = m_po->Edges(); for (i = 0; i<N; ++i) { GEdge* edge = new GEdge(this); *edge = *m_po->Edge(i); m_Edge.push_back(edge); }
	N = m_po->Parts(); for (i = 0; i<N; ++i) { GPart* part = new GPart(this); *part = *m_po->Part(i); m_Part.push_back(part); }
	N = m_po->Faces(); for (i = 0; i<N; ++i) { GFace* face = new GFace(this); *face = *m_po->Face(i); m_Face.push_back(face); }
}

//-----------------------------------------------------------------------------
void GModifiedObject::AddModifier(GModifier* pmod)
{
	m_pStack->Add(pmod);
}

//-----------------------------------------------------------------------------
void GModifiedObject::DeleteModifier(GModifier* pmod)
{
	assert(m_pStack->Size()>0);
	m_pStack->Remove(pmod);
}

//-----------------------------------------------------------------------------
FSMesh* GModifiedObject::BuildMesh()
{
	delete GetFEMesh();
	SetFEMesh(nullptr);

	// ask the ref object to build a mesh
	FSMesh* newMesh = nullptr;
	FSMesh* pm = m_po->BuildMesh();
	if (pm)
	{
		newMesh = new FSMesh(*pm);
		newMesh->SetGObject(this);
	}
	
	// apply modifiers to FSMesh
	int N = m_pStack->Size();
	for (int i=0; i<N; ++i)
	{
		GModifier* pmod = m_pStack->Modifier(i);
		FSMesh* pm = pmod->BuildFEMesh(this);
		if (pm) SetFEMesh(pm);
	}

	// Make sure the normals and the bounding box are up to date.
	if (newMesh)
	{
		newMesh->UpdateMesh();
	}
	return newMesh;
}

//-----------------------------------------------------------------------------
void GModifiedObject::BuildGMesh()
{
	delete GetRenderMesh();
	m_po->BuildGMesh();
	GLMesh* pm = m_po->GetRenderMesh();
	GLMesh* gmesh = new GLMesh(*pm);
	int N = m_pStack->Size();
	for (int i=0; i<N; ++i)
	{
		m_pStack->Modifier(i)->BuildGMesh(this);
	}
	gmesh->Update();
	SetRenderMesh(gmesh);
}

//-----------------------------------------------------------------------------
bool GModifiedObject::Update(bool b)
{
	if (m_po->Update(b) == false) return false;

	// restore the original object
	CloneChild();

	// apply modifiers to GObject
	int N = m_pStack->Size();
	for (int i=0; i<N; ++i) m_pStack->Modifier(i)->Apply(this);

	// rebuild the GLMesh
	BuildGMesh();

	// if we have an FE mesh, we should rebuild that one as well
	if (m_po->GetFEMesh()) BuildMesh();

	return true;
}

//-----------------------------------------------------------------------------
void GModifiedObject::Save(OArchive &ar)
{
	// save the name
	ar.WriteChunk(CID_OBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// save the transform stuff
	ar.BeginChunk(CID_OBJ_HEADER);
	{
		int nid = GetID();
		ar.WriteChunk(CID_OBJ_ID, nid);
		ar.WriteChunk(CID_OBJ_POS, GetTransform().GetPosition());
		ar.WriteChunk(CID_OBJ_ROT, GetTransform().GetRotation());
		ar.WriteChunk(CID_OBJ_SCALE, GetTransform().GetScale());
		ar.WriteChunk(CID_OBJ_COLOR, GetColor());

		int nparts = Parts();
		int nfaces = Faces();
		int nedges = Edges();
		int nnodes = Nodes();

		ar.WriteChunk(CID_OBJ_PARTS, nparts);
		ar.WriteChunk(CID_OBJ_FACES, nfaces);
		ar.WriteChunk(CID_OBJ_EDGES, nedges);
		ar.WriteChunk(CID_OBJ_NODES, nnodes);
	}
	ar.EndChunk();

	// save the parts
	ar.BeginChunk(CID_OBJ_PART_LIST);
	{
		for (int i=0; i<Parts(); ++i)
		{
			ar.BeginChunk(CID_OBJ_PART);
			{
				GPart& p = *Part(i);
				int nid = p.GetID();
				int mid = p.GetMaterialID();
				ar.WriteChunk(CID_OBJ_PART_ID, nid);
				ar.WriteChunk(CID_OBJ_PART_MAT, mid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the surfaces
	ar.BeginChunk(CID_OBJ_FACE_LIST);
	{
		for (int i=0; i<Faces(); ++i)
		{
			ar.BeginChunk(CID_OBJ_FACE);
			{
				GFace& f = *Face(i);
				int nid = f.GetID();
				ar.WriteChunk(CID_OBJ_FACE_ID, nid);
				ar.WriteChunk(CID_OBJ_FACE_PID0, f.m_nPID[0]);
				ar.WriteChunk(CID_OBJ_FACE_PID1, f.m_nPID[1]);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the edges
	ar.BeginChunk(CID_OBJ_EDGE_LIST);
	{
		for (int i=0; i<Edges(); ++i)
		{
			ar.BeginChunk(CID_OBJ_EDGE);
			{
				GEdge& e = *Edge(i);
				int nid = e.GetID();
				ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the nodes
	// note that it is possible that an object doesn't have any nodes
	// for instance, a shell disc
	if (Nodes()>0)
	{
		ar.BeginChunk(CID_OBJ_NODE_LIST);
		{
			for (int i=0; i<Nodes(); ++i)
			{	
				ar.BeginChunk(CID_OBJ_NODE);
				{
					GNode& v = *Node(i);
					int nid = v.GetID();
					ar.WriteChunk(CID_OBJ_NODE_ID, nid);
					ar.WriteChunk(CID_OBJ_NODE_POS, v.LocalPosition());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the child object
	ar.BeginChunk(CID_OBJ_GOBJECTS);
	{
		int ntype = m_po->GetType();
		ar.BeginChunk(ntype);
		{
			m_po->Save(ar);
		}
		ar.EndChunk();
	}
	ar.EndChunk();

	// save the modifiers
	if (m_pStack->Size() > 0)
	{
		ar.BeginChunk(CID_MODIFIER_STACK);
		{
			m_pStack->Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
extern GObject* BuildObject(int);

//-----------------------------------------------------------------------------
void GModifiedObject::Load(IArchive &ar)
{
	TRACE("GModifiedObject::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;
	GObject* po = 0;
	while (ar.OpenChunk() == IArchive::IO_OK)
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
		// object parts
		case CID_OBJ_PART_LIST:
			{
				assert(nparts > 0);
				m_Part.reserve(nparts);
				int n = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_LIST (GModifiedObject::Load)");

					GPart* p = new GPart(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid, mid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_PART_ID : ar.read(nid); p->SetID(nid); break;
						case CID_OBJ_PART_MAT: ar.read(mid); p->SetMaterialID(mid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					p->SetLocalID(n++);

					m_Part.push_back(p);
				}
				assert((int) m_Part.size() == nparts);
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
					if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_LIST (GModifiedObject::Load)");

					GFace* f = new GFace(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_FACE_ID  : ar.read(nid); f->SetID(nid); break;
						case CID_OBJ_FACE_PID0: ar.read(nid); f->m_nPID[0] = nid; break;
						case CID_OBJ_FACE_PID1: ar.read(nid); f->m_nPID[1] = nid; break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					f->SetLocalID(n++);

					m_Face.push_back(f);
				}
				assert((int) m_Face.size() == nfaces);
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
					if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_LIST (GModifiedObject::Load)");

					GEdge* e = new GEdge(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_EDGE_ID: ar.read(nid); e->SetID(nid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					e->SetLocalID(n++);

					m_Edge.push_back(e);
				}
				assert((int) m_Edge.size() == nedges);
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
						if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_LIST (GModifiedObject::Load)");

						GNode* n = new GNode(this);
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int nid;
							switch (ar.GetChunkID())
							{
							case CID_OBJ_NODE_ID: ar.read(nid); n->SetID(nid); break;
							case CID_OBJ_NODE_POS: ar.read(n->LocalPosition()); break;
							}
							ar.CloseChunk();
						}
						ar.CloseChunk();

						n->SetLocalID(m++);

						m_Node.push_back(n);
					}
					assert((int) m_Node.size() == nnodes);
				}
			}
			break;
		// child object
		case CID_OBJ_GOBJECTS:
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();
					assert(m_po == 0);
					po = BuildObject(ntype);
					if (po == 0) throw ReadError("error parsing CID_OBJ_GOBJECTS (GModifiedObject::Load)");
					po->Load(ar);
					ar.CloseChunk();
				}
			}
			break;
		// modifier stack
		case CID_MODIFIER_STACK:
			{
				m_pStack->Load(ar);
			}
			break;
		}
		ar.CloseChunk();
	}
	assert(po);
	SetChildObject(po, false);
}

//-----------------------------------------------------------------------------
FEMesher* GModifiedObject::GetFEMesher()
{
	if (GObject::GetFEMesher() == 0) return m_po->GetFEMesher();
	return GObject::GetFEMesher();
}

//-----------------------------------------------------------------------------
GObject* GModifiedObject::Clone()
{
	// first, we clone the child object
	GObject* pc = m_po->Clone();
	if (pc == 0) return 0;

	// next, we create a new modifier object
	GModifiedObject* po = new GModifiedObject(pc);

	// copy transform
	po->CopyTransform(this);

	// copy color
	po->SetColor(GetColor());

	// copy the modifier stack
	po->m_pStack->Copy(m_pStack);

	// update the new object
	po->Update();

	return po;
}
