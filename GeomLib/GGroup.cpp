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
#include "GGroup.h"
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>

GGroup::GGroup(GModel* ps, int ntype, unsigned int flags) : FEItemListBuilder(ntype, flags)
{
	m_ps = ps;
}

GGroup::~GGroup(void)
{
}

//-----------------------------------------------------------------------------
// GNodeList
//-----------------------------------------------------------------------------

FSNodeList* GNodeList::BuildNodeList()
{
	GModel& m = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	FSNodeList* ps = new FSNodeList();
	for (int i=0; i<N; ++i, ++it)
	{
		GNode* pn = m.FindNode(*it);
		if (pn)
		{
			GObject* po = dynamic_cast<GObject*>(pn->Object());
			FSMesh* pm = po->GetFEMesh();
			for (int j=0; j<pm->Nodes(); ++j)
			{
				FSNode& node = pm->Node(j);
				if (node.m_gid == pn->GetLocalID())
				{
					ps->Add(pm, &node);
					break;
				}
			}
		}
	}
	return ps;
}

//-----------------------------------------------------------------------------
GNodeList::GNodeList(GModel* ps, GNodeSelection* pg) : GGroup(ps, GO_NODE, FE_NODE_FLAG)
{
	int N = pg->Count();
	assert(N);
	GNodeSelection::Iterator it(pg);
	for (int i=0; i<N; ++i, ++it) add(it->GetID());
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GNodeList::Copy()
{
	GNodeList* pg = new GNodeList(m_ps);
	pg->m_Item = m_Item;
	return pg;
}

//-----------------------------------------------------------------------------
vector<GNode*> GNodeList::GetNodeList()
{
	vector<GNode*> nodeList;
	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GNode* pg = model.FindNode(*it);
		nodeList.push_back(pg);
	}

	return nodeList;
}

//-----------------------------------------------------------------------------
bool GNodeList::IsValid() const
{
	if (GGroup::IsValid() == false) return false;

	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::ConstIterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GNode* pg = model.FindNode(*it);
		if (pg == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// GEdgeList
//-----------------------------------------------------------------------------

GEdgeList::GEdgeList(GModel* ps, GEdgeSelection* pg) : GGroup(ps, GO_EDGE, FE_NODE_FLAG)
{
	int N = pg->Count();
	if (N > 0)
	{
		GEdgeSelection::Iterator it(pg);
		for (int i=0; i<N; ++i, ++it) add(it->GetID());
	}
}

//-----------------------------------------------------------------------------
FSNodeList* GEdgeList::BuildNodeList()
{
	GModel& model = *m_ps;
	FSNodeList* ps = new FSNodeList();
	int N = m_Item.size(), i, n;
	FEItemListBuilder::Iterator it = m_Item.begin();

	for (n=0; n<N; ++n, ++it)
	{
		GEdge *pe = model.FindEdge(*it);
		GObject* po = dynamic_cast<GObject*>(pe->Object());
		FSMesh& m = *po->GetFEMesh();
		for (i=0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;
	}

	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GEdge *pe = model.FindEdge(*it);
		int gid = pe->GetLocalID();
		GObject* po = dynamic_cast<GObject*>(pe->Object());
		FSMesh& m = *po->GetFEMesh();
		for (i=0; i<m.Edges(); ++i)
		{
			FSEdge& e = m.Edge(i);
			if (e.m_gid == gid)
			{
				m.Node(e.n[0]).m_ntag = 1;
				m.Node(e.n[1]).m_ntag = 1;
				if (e.n[2] != -1) m.Node(e.n[2]).m_ntag = 1;
			}
		}
	}
	
	// fill the list
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GEdge *pe = model.FindEdge(*it);
		GObject* po = dynamic_cast<GObject*>(pe->Object());
		FSMesh& m = *po->GetFEMesh();
		for (i = 0; i < m.Nodes(); ++i)
		{
			FSNode& node = m.Node(i);
			if (node.m_ntag == 1)
			{
				ps->Add(&m, m.NodePtr(i));

				// reset tag in case another edge references the same node
				node.m_ntag = 0;
			}
		}
	}
	
	return ps;
}

//-----------------------------------------------------------------------------
FEEdgeList* GEdgeList::BuildEdgeList()
{
	GModel& model = *m_ps;
	FEEdgeList* ps = new FEEdgeList();
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();

	for (int n = 0; n < N; ++n, ++it)
	{
		GEdge* pe = model.FindEdge(*it);
		int gid = pe->GetLocalID();
		GObject* po = dynamic_cast<GObject*>(pe->Object());
		FSMesh& m = *po->GetFEMesh();
		for (int i = 0; i < m.Edges(); ++i)
		{
			FSEdge& e = m.Edge(i);
			if (e.m_gid == gid)
			{
				ps->Add(&m, &e);
			}
		}
	}

	return ps;
}

//-----------------------------------------------------------------------------
vector<GEdge*> GEdgeList::GetEdgeList()
{
	vector<GEdge*> edgeList;
	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GEdge* pg = model.FindEdge(*it);
		edgeList.push_back(pg);
	}

	return edgeList;
}

//-----------------------------------------------------------------------------
GEdge* GEdgeList::GetEdge(int n)
{
	GModel& model = *m_ps;
	int N = m_Item.size();
	if ((n < 0) || (n >= N)) return nullptr;

	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int i = 0; i < n; ++i) ++it;
	GEdge* pg = model.FindEdge(*it);
	return pg;
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GEdgeList::Copy()
{
	GEdgeList* pg = new GEdgeList(m_ps);
	pg->m_Item = m_Item;
	return pg;
}

//-----------------------------------------------------------------------------
bool GEdgeList::IsValid() const
{
	if (GGroup::IsValid() == false) return false;

	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::ConstIterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GEdge* pg = model.FindEdge(*it);
		if (pg == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// GFaceList
//-----------------------------------------------------------------------------

GFaceList::GFaceList(GModel* ps, GFaceSelection* pg) : GGroup(ps, GO_FACE, FE_NODE_FLAG | FE_FACE_FLAG)
{
	int N = pg->Count();
	if (N > 0)
	{
		GFaceSelection::Iterator it(pg);
		for (int i=0; i<N; ++i, ++it) add(it->GetID());
	}
}

//-----------------------------------------------------------------------------
GFaceList::GFaceList(GModel* ps, GFace* pf) : GGroup(ps, GO_FACE, FE_NODE_FLAG | FE_FACE_FLAG)
{
	if (pf) add(pf->GetID());
}

//-----------------------------------------------------------------------------
FSNodeList* GFaceList::BuildNodeList()
{
	GModel& model = *m_ps;
	int N = m_Item.size(), n, i;
	FEItemListBuilder::Iterator it = m_Item.begin();

	// first we need to clear all tags
	for (n=0; n<N; ++n, ++it)
	{
		GFace *pf = model.FindSurface(*it);
		if (pf == 0) return 0;
		GObject* po = dynamic_cast<GObject*>(pf->Object());
		FSMesh& m = *po->GetFEMesh();

		// clear the tags
		for (i=0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;
	}

	// then we need to identify which nodes are part of the nodelist
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GFace *pf = model.FindSurface(*it);
		GObject* po = dynamic_cast<GObject*>(pf->Object());
		FSMesh& m = *po->GetFEMesh();
		int gid = pf->GetLocalID();

		// tag the nodes to be added to the list
		for (i=0; i<m.Faces(); ++i)
		{
			FSFace& f = m.Face(i);
			if (f.m_gid == gid)
			{
				int l = f.Nodes();
				for (int j=0; j<l; ++j) m.Node(f.n[j]).m_ntag = 1;
			}
		}
	}

	// create a new node list
	FSNodeList* ps = new FSNodeList();

	// next we add all the nodes
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GFace *pf = model.FindSurface(*it);
		GObject* po = dynamic_cast<GObject*>(pf->Object());
		FSMesh& m = *po->GetFEMesh();
		for (i = 0; i < m.Nodes(); ++i)
		{
			FSNode& node = m.Node(i);
			if (node.m_ntag == 1)
			{
				ps->Add(&m, m.NodePtr(i));

				// reset tag in case another surface in this list references
				// the same node
				node.m_ntag = 0;
			}
		}
	}

	return ps;
}

//-----------------------------------------------------------------------------
FEFaceList* GFaceList::BuildFaceList()
{
	GModel& m = *m_ps;
	FEFaceList* ps = new FEFaceList();
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n=0; n<N; ++n, ++it)
	{
		GFace* pf = m.FindSurface(*it);
//		assert(pf);
		if (pf)
		{
			GObject* po = dynamic_cast<GObject*>(pf->Object());
			FSMesh& m = *po->GetFEMesh();
			int gid = pf->GetLocalID();

			for (int i=0; i<m.Faces(); ++i) if (m.Face(i).m_gid == gid) ps->Add(&m, m.FacePtr(i));
		}
	}

	return ps;	
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GFaceList::Copy()
{
	GFaceList* pg = new GFaceList(m_ps);
	pg->m_Item = m_Item;
	return pg;
}

//-----------------------------------------------------------------------------
vector<GFace*> GFaceList::GetFaceList()
{
	vector<GFace*> surfList;
	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GFace *pg = model.FindSurface(*it);
		surfList.push_back(pg);
	}

	return surfList;
}

//-----------------------------------------------------------------------------
bool GFaceList::IsValid() const
{
	if (GGroup::IsValid() == false) return false;

	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::ConstIterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GFace *pg = model.FindSurface(*it);
		if (pg == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// GPartList
//-----------------------------------------------------------------------------

// TODO: Temporarily removing face flag, since generating surfaces from parts is causing
//       some issues on export. 
//GPartList::GPartList(GModel* ps) : GGroup(ps, GO_PART, FE_NODE_FLAG | FE_FACE_FLAG | FE_ELEM_FLAG | FE_PART_FLAG) {}
GPartList::GPartList(GModel* ps) : GGroup(ps, GO_PART, FE_NODE_FLAG | FE_ELEM_FLAG | FE_PART_FLAG) {}

GPartList::GPartList(GModel* ps, GPartSelection* pg) : GGroup(ps, GO_PART, FE_NODE_FLAG | FE_ELEM_FLAG | FE_PART_FLAG)
//GPartList::GPartList(GModel* ps, GPartSelection* pg) : GGroup(ps, GO_PART, FE_NODE_FLAG | FE_FACE_FLAG | FE_ELEM_FLAG | FE_PART_FLAG)
{
	int N = pg->Count();
	GPartSelection::Iterator it(pg);
	for (int i=0; i<N; ++i, ++it) add(it->GetID());
}

//-----------------------------------------------------------------------------
void GPartList::Create(GObject* po)
{
	if (po == nullptr) return;
	int N = po->Parts();
	for (int i = 0; i < N; ++i) add(po->Part(i)->GetID());
}

//-----------------------------------------------------------------------------
FEElemList* GPartList::BuildElemList()
{
	GModel& model = *m_ps;
	FEElemList* ps = new FEElemList();
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n=0; n<N; ++n, ++it)
	{
		GPart *pg = model.FindPart(*it);
		GObject* po = dynamic_cast<GObject*>(pg->Object());
		FSMesh& m = *po->GetFEMesh();
		int gid = pg->GetLocalID();
		for (int i=0; i<m.Elements(); ++i)
		{
			FSElement& e = m.Element(i);
			if (e.m_gid == gid) ps->Add(&m, &e, i);
		}
	}
	return ps;
}

//-----------------------------------------------------------------------------
FSNodeList* GPartList::BuildNodeList()
{
	GModel& model = *m_ps;
	int N = m_Item.size(), n, i, j;
	FEItemListBuilder::Iterator it = m_Item.begin();

	// first we need to clear all tags
	for (n=0; n<N; ++n, ++it)
	{
        GPart *pg = model.FindPart(*it);
        if (pg) {
            GObject* po = dynamic_cast<GObject*>(pg->Object());
            FSMesh& m = *po->GetFEMesh();
            
            // clear the tags
            for (i=0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;
        }
	}

	// then we need to identify which nodes are part of the nodelist
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GPart *pg = model.FindPart(*it);
        if (pg) {
            GObject* po = dynamic_cast<GObject*>(pg->Object());
            FSMesh& m = *po->GetFEMesh();
            int gid = pg->GetLocalID();
            
            // tag the nodes to be added to the list
            for (i=0; i<m.Elements(); ++i)
            {
                FSElement& e = m.Element(i);
                if (e.m_gid == gid)
                {
                    for (j=0; j<e.Nodes(); ++j) m.Node(e.m_node[j]).m_ntag = 1;
                }
            }
        }
	}

	// next we add all the nodes
	FSNodeList* ps = new FSNodeList();
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GPart *pg = model.FindPart(*it);
        if (pg) {
            GObject* po = dynamic_cast<GObject*>(pg->Object());
            FSMesh& m = *po->GetFEMesh();
			for (i = 0; i < m.Nodes(); ++i)
			{
				FSNode& node = m.Node(i);
				if (node.m_ntag == 1)
				{
					ps->Add(&m, m.NodePtr(i));

					// reset tag in case another part references this same node
					node.m_ntag = 0;
				}
			}
        }
	}

	return ps;
}

//-----------------------------------------------------------------------------
FEFaceList*	GPartList::BuildFaceList()
{
	GModel& m = *m_ps;
	FEFaceList* ps = new FEFaceList();
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n = 0; n < N; ++n, ++it)
	{
		GPart* pg = m.FindPart(*it);
		if (pg)
		{
			int partId = pg->GetLocalID();

			GObject* po = dynamic_cast<GObject*>(pg->Object());
			FSMesh& m = *po->GetFEMesh();
			for (int i = 0; i < m.Faces(); ++i)
			{
				FSFace& f = m.Face(i);
				if (f.IsExterior())
				{
					int faceId = f.m_gid;
					GFace* surface = po->Face(faceId);
					if (surface && (surface->m_nPID[0] == partId))
					{
						ps->Add(&m, m.FacePtr(i));
					}
				}
			}
		}
	}

	return ps;
}

//-----------------------------------------------------------------------------
FSPartSet* GPartList::BuildPartSet()
{
	GModel& model = *m_ps;

	// first, make sure all parts belong to the same object
	GObject* po = nullptr;
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n = 0; n < N; ++n, ++it)
	{
		GPart* pg = model.FindPart(*it);
		GObject* pon = dynamic_cast<GObject*>(pg->Object());
		if (po == nullptr) po = pon;
		else if (po != pon) return nullptr;
	}
	if (po == nullptr) return nullptr;

	// build the part set
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return nullptr;

	FSPartSet* partSet = new FSPartSet(pm);
	it = m_Item.begin();
	for (int n = 0; n < N; ++n, ++it)
	{
		GPart* pg = model.FindPart(*it);
		partSet->add(pg->GetLocalID());
	}

	return partSet;
}

//-----------------------------------------------------------------------------
FEItemListBuilder* GPartList::Copy()
{
	GPartList* pg = new GPartList(m_ps);
	pg->m_Item = m_Item;
	return pg;
}

//-----------------------------------------------------------------------------
vector<GPart*> GPartList::GetPartList()
{
	vector<GPart*> partList;
	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GPart *pg = model.FindPart(*it);
		partList.push_back(pg);
	}

	return partList;
}

//-----------------------------------------------------------------------------
bool GPartList::IsValid() const
{
	if (GGroup::IsValid() == false) return false;

	GModel& model = *m_ps;
	int N = m_Item.size();
	FEItemListBuilder::ConstIterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GPart* pg = model.FindPart(*it);
		if (pg == 0) return false;
	}

	return true;
}
