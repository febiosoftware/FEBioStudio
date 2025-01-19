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

// FSGroup.cpp: implementation of the FSGroup class.
//
//////////////////////////////////////////////////////////////////////

#include "FSGroup.h"
#include <MeshLib/FEMesh.h>
#include <FEMLib/FSModel.h>
#include "GObject.h"
using namespace std;

//////////////////////////////////////////////////////////////////////
// FSGroup
//////////////////////////////////////////////////////////////////////

FSGroup::FSGroup(FSMesh* pm, int ntype, unsigned int flags) : FEItemListBuilder(ntype, flags)
{
	m_mesh = pm;
}

GObject* FSGroup::GetGObject()
{
	return (m_mesh ? m_mesh->GetGObject() : nullptr);
}

FSMesh* FSGroup::GetMesh()
{
	return m_mesh;
}

FSGroup::~FSGroup()
{

}

void FSGroup::Save(OArchive& ar)
{
	int N = (int)m_Item.size();
	ar.WriteChunk(ID, m_nID);
	ar.WriteChunk(NAME, GetName());
	ar.WriteChunk(SIZE, N);

	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int i=0; i<N; ++i, ++it)
	{
		ar.WriteChunk(ITEM, (*it));
	}
}

//-----------------------------------------------------------------------------
// Note that this function only reads the meshID. If this group is not managed
// by an FSMesh, then the owner of this group is responsible for setting the 
// correct mesh pointer (i.e. m_pMesh).
void FSGroup::Load(IArchive &ar)
{
	TRACE("FSGroup::Load");

	m_Item.clear();

	int N, n;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case ID: ar.read(n); SetID(n); break;
		case NAME: { char sz[256]; ar.read(sz); SetName(sz); } break;
		case SIZE: ar.read(N); break;
		case ITEM: ar.read(n); m_Item.push_back(n); break;
		case MESHID: ar.read(m_meshID); break;
		default:
			throw ReadError("unknown CID in FSGroup::Load");
		}
		ar.CloseChunk();
	}
	assert((int) m_Item.size() == N);
}

//////////////////////////////////////////////////////////////////////
// FSElemSet
//////////////////////////////////////////////////////////////////////

FSElemSet::FSElemSet(FSMesh* pm, const vector<int>& elset) : FSGroup(pm, FE_ELEMSET, FE_NODE_FLAG | FE_ELEM_FLAG)
{
	int nsel = (int) elset.size();
	if (nsel > 0)
	{
		for (int i=0; i<nsel; ++i) add(elset[i]);
	}
}

void FSElemSet::Copy(FSElemSet* pg)
{
	m_Item = pg->m_Item;
	SetName(pg->GetName());
}

FEItemListBuilder* FSElemSet::Copy()
{
	FSElemSet* pg = new FSElemSet(m_mesh);
	pg->m_Item = m_Item;
	return pg;
}

//-----------------------------------------------------------------------------
void FSElemSet::CreateFromMesh()
{
	Clear();
	FSMesh* m = GetMesh();
	if (m == nullptr) return;
	int NE = m->Elements();
	for (int i = 0; i < NE; ++i) add(i);
}

//-----------------------------------------------------------------------------
FEElement_* FSElemSet::GetElement(int n)
{
	FSMesh* m = GetMesh();
	if (m == nullptr) return nullptr;
	return m->ElementPtr(m_Item[n]);
}

//-----------------------------------------------------------------------------
FEElemList* FSElemSet::BuildElemList()
{
	FSMesh* pm = m_mesh;
	if (pm==0) return 0;

	FEElemList* pg = new FEElemList();

	FEItemListBuilder::Iterator it;
	for (it = m_Item.begin(); it != m_Item.end(); ++it)
	{
		FEElement_* pe = pm->ElementPtr(*it); assert(pe);
		pg->Add(pm, pe);
	}

	return pg;
}

//-----------------------------------------------------------------------------
FSNodeList* FSElemSet::BuildNodeList()
{
	int i, j;
	FSMesh* pm = m_mesh;
	int N = pm->Nodes();
	for (i=0; i<N; ++i) pm->Node(i).m_ntag = 0;

	FEItemListBuilder::Iterator it;
	for (it = m_Item.begin(); it != m_Item.end(); ++it)
	{
		FEElement_* pe = pm->ElementPtr(*it);
		for (j=0; j<pe->Nodes(); ++j) pm->Node(pe->m_node[j]).m_ntag = 1;
	}

	FSNodeList* pg = new FSNodeList();
	for (i=0; i<N; ++i)
	{
		FSNode& n = pm->Node(i);
		if (n.m_ntag == 1) pg->Add(pm, &n);
	}
	return pg;
}

void FSElemSet::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FSCoreMesh& mesh = *GetMesh();
	int NN = mesh.Nodes();
	int NE = size();

	for (int i = 0; i < NN; ++i) mesh.Node(i).m_ntag = -1;

	int n = 0, nne = 0;
	std::vector<int> elemList = CopyItems();
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(elemList[i]);
		int ne = el.Nodes();
		nne += ne;
		for (int j = 0; j < ne; ++j)
		{
			if (mesh.Node(el.m_node[j]).m_ntag == -1) mesh.Node(el.m_node[j]).m_ntag = n++;
		}
	}

	node.resize(n);
	for (int i = 0; i < NN; ++i)
		if (mesh.Node(i).m_ntag >= 0) node[mesh.Node(i).m_ntag] = i;

	lnode.resize(nne); nne = 0;
	for (int i = 0; i < NE; ++i)
	{
		FEElement_& el = mesh.ElementRef(elemList[i]);
		int ne = el.Nodes();
		for (int j = 0; j < ne; ++j)
		{
			int lid = mesh.Node(el.m_node[j]).m_ntag; assert(lid >= 0);
			lnode[nne + j] = lid;
		}
		nne += ne;
	}
}

//////////////////////////////////////////////////////////////////////
// FSPartSet
//////////////////////////////////////////////////////////////////////

FEItemListBuilder* FSPartSet::Copy()
{
	FSPartSet* pg = new FSPartSet(m_mesh);
	pg->m_Item = m_Item;
	return pg;
}

void FSPartSet::Copy(FSPartSet* pg)
{
	m_Item = pg->m_Item;
	SetName(pg->GetName());
}

GPart* FSPartSet::GetPart(size_t n)
{
	if ((n < 0) || (n >= m_Item.size())) return nullptr;
	GObject* po = m_mesh->GetGObject(); assert(po);
	return (po ? po->Part(m_Item[n]) : nullptr);
}

std::vector<int> FSPartSet::BuildElementIndexList()
{
	FSMesh* mesh = GetMesh();
	assert(mesh);

	std::vector<int> elemList;
	int NE = mesh->Elements();
	for (int i = 0; i < size(); ++i)
	{
		int pid = m_Item[i];
		for (int j = 0; j < NE; ++j)
		{
			FSElement& el = mesh->Element(j);
			if (el.m_gid == pid)
			{
				elemList.push_back(j);
			}
		}
	}
	return elemList;
}

std::vector<int> FSPartSet::BuildElementIndexList(const std::vector<int>& partList)
{
	FSMesh* mesh = GetMesh();
	assert(mesh);

	std::vector<int> elemList;
	int NE = mesh->Elements();
	for (int i = 0; i < partList.size(); ++i)
	{
		// make sure this part is actually part of this set's part list
		int m = -1;
		for (int j = 0; j < m_Item.size(); ++j)
		{
			if (m_Item[j] == partList[i])
			{
				m = j;
				break;
			}
		}
		assert(m != -1);
		if (m != -1)
		{
			int pid = m_Item[m];
			for (int j = 0; j < NE; ++j)
			{
				FSElement& el = mesh->Element(j);
				if (el.m_gid == pid)
				{
					elemList.push_back(j);
				}
			}
		}
	}
	return elemList;
}

//////////////////////////////////////////////////////////////////////
// FSSurface
//////////////////////////////////////////////////////////////////////

FSSurface::FSSurface(FSMesh* pm, vector<int>& face) : FSGroup(pm, FE_SURFACE, FE_NODE_FLAG | FE_FACE_FLAG)
{
	int n = (int) face.size();
	if (n > 0)
	{
		for (int i=0; i<n; ++i) add(face[i]);
	}
}

void FSSurface::Copy(FSSurface* pg)
{
	m_Item = pg->m_Item;
	SetName(pg->GetName());
}

FEItemListBuilder* FSSurface::Copy()
{
	FSSurface* pg = new FSSurface(m_mesh);
	pg->m_Item = m_Item;
	return pg;
}

void FSSurface::GetNodeList(vector<int>& node, vector<int>& lnode)
{
	FSCoreMesh& mesh = *GetMesh();
	int NN = mesh.Nodes();
	int NF = size();

	for (int i = 0; i < NN; ++i) mesh.Node(i).m_ntag = -1;

	int n = 0, nnf = 0;
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = *GetFace(i);
		int nf = face.Nodes();
		nnf += nf;
		for (int j = 0; j < nf; ++j)
		{
			if (mesh.Node(face.n[j]).m_ntag == -1) mesh.Node(face.n[j]).m_ntag = n++;
		}
	}

	node.resize(n);
	for (int i = 0; i < NN; ++i)
		if (mesh.Node(i).m_ntag >= 0) node[mesh.Node(i).m_ntag] = i;

	lnode.resize(nnf); nnf = 0;
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = *GetFace(i);
		int nf = face.Nodes();
		for (int j = 0; j < nf; ++j)
		{
			int lid = mesh.Node(face.n[j]).m_ntag; assert(lid >= 0);
			lnode[nnf + j] = lid;
		}
		nnf += nf;
	}
}

FSFace* FSSurface::GetFace(int n)
{
	FSMesh* pm = m_mesh;
	if (pm == nullptr) return nullptr;
	return pm->FacePtr(m_Item[n]);
}

FEFaceList* FSSurface::BuildFaceList()
{
	FSMesh* pm = m_mesh;
	if (pm == 0) return 0;

	FEFaceList* ps = new FEFaceList();

	FEItemListBuilder::Iterator it = m_Item.begin();

	for (int i=0; i<size(); ++i, ++it) ps->Add(pm, pm->FacePtr(*it));
	return ps;
}

FSNodeList* FSSurface::BuildNodeList()
{	
	FSMesh* pm = m_mesh;
	if (pm == 0) return 0;

	FSNodeList* pg = new FSNodeList();
	
	// tag all nodes to be added
	int i, j, n;
	for (i=0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = 0;

	FEItemListBuilder::Iterator it = m_Item.begin();
	int N = (int)m_Item.size();
	for (i=0; i<N; ++i, ++it)
	{
		FSFace& f = pm->Face(*it);
		n = f.Nodes();
		for (j=0; j<n; ++j) pm->Node(f.n[j]).m_ntag = 1;
	}

	// add nodes to list
	for (i=0; i<pm->Nodes(); ++i)
	{
		FSNode* pn = pm->NodePtr(i);
		if (pn->m_ntag == 1) pg->Add(pm, pn);
	}

	return pg;
}


//////////////////////////////////////////////////////////////////////
// FSEdgeSet
//////////////////////////////////////////////////////////////////////

FSEdgeSet::FSEdgeSet(FSMesh* pm, vector<int>& edge) : FSGroup(pm, FE_EDGESET, FE_NODE_FLAG)
{
	int n = (int) edge.size();
	if (n > 0)
	{
		for (int i=0; i<n; ++i) add(edge[i]);
	}
}

void FSEdgeSet::Copy(FSEdgeSet* pg)
{
	m_Item = pg->m_Item;
	SetName(pg->GetName());
}

FEItemListBuilder* FSEdgeSet::Copy()
{
	FSEdgeSet* pg = new FSEdgeSet(m_mesh);
	pg->m_Item = m_Item;
	return pg;
}

FSEdge* FSEdgeSet::Edge(FEItemListBuilder::Iterator it)
{
	FSMesh* pm = m_mesh;
	if (pm == 0) return 0;
	FSEdge& e = pm->Edge(*it);
	return &e;
}

FSNodeList* FSEdgeSet::BuildNodeList()
{	
	FSMesh* pm = m_mesh;
	if (pm == 0) return 0;

	FSNodeList* pg = new FSNodeList();
	
	// tag all nodes to be added
	int i, j, n;
	for (i=0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = 0;

	FEItemListBuilder::Iterator it = m_Item.begin();
	int N = (int)m_Item.size();
	for (i=0; i<N; ++i, ++it)
	{
		FSEdge& e = pm->Edge(*it);
		n = e.Nodes();
		for (j=0; j<n; ++j) pm->Node(e.n[j]).m_ntag = 1;
	}

	// add nodes to list
	for (i=0; i<pm->Nodes(); ++i)
	{
		FSNode* pn = pm->NodePtr(i);
		if (pn->m_ntag == 1) pg->Add(pm, pn);
	}

	return pg;
}

FEEdgeList* FSEdgeSet::BuildEdgeList()
{
	FSMesh* pm = m_mesh;
	if (pm == 0) return 0;

	FEEdgeList* pg = new FEEdgeList();

	FEItemListBuilder::Iterator it = m_Item.begin();
	int N = (int)m_Item.size();
	for (int i = 0; i < N; ++i, ++it)
	{
		FSEdge& e = pm->Edge(*it);
		pg->Add(pm, &e);
	}

	return pg;
}

//////////////////////////////////////////////////////////////////////
// FSNodeSet
//////////////////////////////////////////////////////////////////////

FSNodeSet::FSNodeSet(FSMesh* pm, const vector<int>& node) : FSGroup(pm, FE_NODESET, FE_NODE_FLAG)
{
	int n = (int) node.size();
	if (n > 0)
	{
		for (int i=0; i<n; ++i) add(node[i]);
	}
}

void FSNodeSet::Copy(FSNodeSet* pg)
{
	m_Item = pg->m_Item;
	SetName(pg->GetName());
}

FEItemListBuilder* FSNodeSet::Copy()
{
	FSNodeSet* pg = new FSNodeSet(m_mesh);
	pg->m_Item = m_Item;
	return pg;
}

void FSNodeSet::CreateFromMesh()
{
	Clear();
	FSMesh* m = m_mesh;
	if (m == nullptr) return;
	int NN = m->Nodes();
	for (int i = 0; i < NN; ++i) add(i);
}

FSNodeList* FSNodeSet::BuildNodeList()
{
	FSMesh* pm = m_mesh;
	if (pm == 0) return 0;
	FSNodeList* ps = new FSNodeList();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int i=0; i<size(); ++i, ++it) ps->Add(pm, pm->NodePtr(*it));
	return ps;
}

FSNode* FSNodeSet::GetNode(size_t n)
{
	FSMesh* pm = m_mesh;
	if (pm == nullptr) return nullptr;
	return pm->NodePtr(m_Item[n]);
}
