#include "stdafx.h"
#include "GGroup.h"
#include "FEModel.h"
#include "GModel.h"
#include <GeomLib/GObject.h>

GGroup::GGroup(FEModel* ps, int ntype) : FEItemListBuilder(ntype)
{
	m_ps = ps;
}

GGroup::~GGroup(void)
{
}

//-----------------------------------------------------------------------------
// GNodeList
//-----------------------------------------------------------------------------

FENodeList* GNodeList::BuildNodeList()
{
	FEModel* pfem = dynamic_cast<FEModel*>(m_ps);
	GModel& m = pfem->GetModel();
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	FENodeList* ps = new FENodeList();
	for (int i=0; i<N; ++i, ++it)
	{
		GNode* pn = m.FindNode(*it);
		if (pn)
		{
			GObject* po = dynamic_cast<GObject*>(pn->Object());
			FEMesh* pm = po->GetFEMesh();
			for (int j=0; j<pm->Nodes(); ++j)
			{
				FENode& node = pm->Node(j);
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
GNodeList::GNodeList(FEModel* ps, GNodeSelection* pg) : GGroup(ps, GO_NODE)
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
	GModel& model = m_ps->GetModel();
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

	GModel& model = m_ps->GetModel();
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

GEdgeList::GEdgeList(FEModel* ps, GEdgeSelection* pg) : GGroup(ps, GO_EDGE)
{
	int N = pg->Count();
	if (N > 0)
	{
		GEdgeSelection::Iterator it(pg);
		for (int i=0; i<N; ++i, ++it) add(it->GetID());
	}
}

//-----------------------------------------------------------------------------
FENodeList* GEdgeList::BuildNodeList()
{
	GModel& model = dynamic_cast<FEModel*>(m_ps)->GetModel();
	FENodeList* ps = new FENodeList();
	int N = m_Item.size(), i, n;
	FEItemListBuilder::Iterator it = m_Item.begin();

	for (n=0; n<N; ++n, ++it)
	{
		GEdge *pe = model.FindEdge(*it);
		GObject* po = dynamic_cast<GObject*>(pe->Object());
		FEMesh& m = *po->GetFEMesh();
		for (i=0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;
	}

	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GEdge *pe = model.FindEdge(*it);
		int gid = pe->GetLocalID();
		GObject* po = dynamic_cast<GObject*>(pe->Object());
		FEMesh& m = *po->GetFEMesh();
		for (i=0; i<m.Edges(); ++i)
		{
			FEEdge& e = m.Edge(i);
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
		FEMesh& m = *po->GetFEMesh();
		for (i = 0; i < m.Nodes(); ++i)
		{
			FENode& node = m.Node(i);
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
vector<GEdge*> GEdgeList::GetEdgeList()
{
	vector<GEdge*> edgeList;
	GModel& model = m_ps->GetModel();
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
	GModel& model = m_ps->GetModel();
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

	GModel& model = m_ps->GetModel();
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

GFaceList::GFaceList(FEModel* ps, GFaceSelection* pg) : GGroup(ps, GO_FACE)
{
	int N = pg->Count();
	if (N > 0)
	{
		GFaceSelection::Iterator it(pg);
		for (int i=0; i<N; ++i, ++it) add(it->GetID());
	}
}

//-----------------------------------------------------------------------------
FENodeList* GFaceList::BuildNodeList()
{
	GModel& model = dynamic_cast<FEModel*>(m_ps)->GetModel();
	int N = m_Item.size(), n, i;
	FEItemListBuilder::Iterator it = m_Item.begin();

	// first we need to clear all tags
	for (n=0; n<N; ++n, ++it)
	{
		GFace *pf = model.FindSurface(*it);
		if (pf == 0) return 0;
		GObject* po = dynamic_cast<GObject*>(pf->Object());
		FEMesh& m = *po->GetFEMesh();

		// clear the tags
		for (i=0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;
	}

	// then we need to identify which nodes are part of the nodelist
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GFace *pf = model.FindSurface(*it);
		GObject* po = dynamic_cast<GObject*>(pf->Object());
		FEMesh& m = *po->GetFEMesh();
		int gid = pf->GetLocalID();

		// tag the nodes to be added to the list
		for (i=0; i<m.Faces(); ++i)
		{
			FEFace& f = m.Face(i);
			if (f.m_gid == gid)
			{
				int l = f.Nodes();
				for (int j=0; j<l; ++j) m.Node(f.n[j]).m_ntag = 1;
			}
		}
	}

	// create a new node list
	FENodeList* ps = new FENodeList();

	// next we add all the nodes
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GFace *pf = model.FindSurface(*it);
		GObject* po = dynamic_cast<GObject*>(pf->Object());
		FEMesh& m = *po->GetFEMesh();
		for (i = 0; i < m.Nodes(); ++i)
		{
			FENode& node = m.Node(i);
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
	GModel& m = dynamic_cast<FEModel*>(m_ps)->GetModel();
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
			FEMesh& m = *po->GetFEMesh();
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
	GModel& model = m_ps->GetModel();
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

	GModel& model = m_ps->GetModel();
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

FEModel* GPartList::m_model = nullptr;

GPartList* GPartList::CreateNew()
{
	assert(m_model);
	return new GPartList(m_model);
}

void GPartList::SetModel(FEModel* mdl)
{
	m_model = mdl;
}

GPartList::GPartList(FEModel* ps, GPartSelection* pg) : GGroup(ps, GO_PART)
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
	GModel& model = dynamic_cast<FEModel*>(m_ps)->GetModel();
	FEElemList* ps = new FEElemList();
	int N = m_Item.size();
	FEItemListBuilder::Iterator it = m_Item.begin();
	for (int n=0; n<N; ++n, ++it)
	{
		GPart *pg = model.FindPart(*it);
		GObject* po = dynamic_cast<GObject*>(pg->Object());
		FEMesh& m = *po->GetFEMesh();
		int gid = pg->GetLocalID();
		for (int i=0; i<m.Elements(); ++i)
		{
			FEElement& e = m.Element(i);
			if (e.m_gid == gid) ps->Add(&m, &e, i);
		}
	}
	return ps;
}

//-----------------------------------------------------------------------------
FENodeList* GPartList::BuildNodeList()
{
	GModel& model = dynamic_cast<FEModel*>(m_ps)->GetModel();
	int N = m_Item.size(), n, i, j;
	FEItemListBuilder::Iterator it = m_Item.begin();

	// first we need to clear all tags
	for (n=0; n<N; ++n, ++it)
	{
        GPart *pg = model.FindPart(*it);
        if (pg) {
            GObject* po = dynamic_cast<GObject*>(pg->Object());
            FEMesh& m = *po->GetFEMesh();
            
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
            FEMesh& m = *po->GetFEMesh();
            int gid = pg->GetLocalID();
            
            // tag the nodes to be added to the list
            for (i=0; i<m.Elements(); ++i)
            {
                FEElement& e = m.Element(i);
                if (e.m_gid == gid)
                {
                    for (j=0; j<e.Nodes(); ++j) m.Node(e.m_node[j]).m_ntag = 1;
                }
            }
        }
	}

	// next we add all the nodes
	FENodeList* ps = new FENodeList();
	it = m_Item.begin();
	for (n=0; n<N; ++n, ++it)
	{
		GPart *pg = model.FindPart(*it);
        if (pg) {
            GObject* po = dynamic_cast<GObject*>(pg->Object());
            FEMesh& m = *po->GetFEMesh();
			for (i = 0; i < m.Nodes(); ++i)
			{
				FENode& node = m.Node(i);
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
	GModel& model = m_ps->GetModel();
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

	GModel& model = m_ps->GetModel();
	int N = m_Item.size();
	FEItemListBuilder::ConstIterator it = m_Item.begin();
	for (int n = 0; n<N; ++n, ++it)
	{
		GPart* pg = model.FindPart(*it);
		if (pg == 0) return false;
	}

	return true;
}
