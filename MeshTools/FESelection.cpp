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

// FESelection.cpp: implementation of the FESelection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FESelection.h"
#include <GeomLib/GGroup.h>
#include <MeshLib/GMesh.h>
#include <MeshLib/FEMesh.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GMeshObject.h>

//////////////////////////////////////////////////////////////////////
// FESelection
//////////////////////////////////////////////////////////////////////

FESelection::FESelection(SelectionType ntype) : m_ntype(ntype)
{
	m_nsize = -1;
	m_movable = false;
}

FESelection::~FESelection()
{

}

bool FESelection::Supports(unsigned int itemFlag) const
{
	if (itemFlag == 0) return false;

	bool b = false;
	switch (Type())
	{
	case SELECT_NODES:
	case SELECT_FE_NODES:
		b = (itemFlag & FE_NODE_FLAG);
		break;
	case SELECT_SURFACES:
	case SELECT_FE_FACES:
		b = (itemFlag & (FE_FACE_FLAG | FE_NODE_FLAG));
		break;
	case SELECT_CURVES:
	case SELECT_FE_EDGES:
		b = (itemFlag & (FE_EDGE_FLAG | FE_NODE_FLAG));
		break;
	case SELECT_PARTS:
		b = (itemFlag & (FE_PART_FLAG | FE_FACE_FLAG | FE_NODE_FLAG));
		break;
	}
	return b;
}

//////////////////////////////////////////////////////////////////////
// GObjectSelection
//////////////////////////////////////////////////////////////////////

GObjectSelection::GObjectSelection(GModel* ps) : FESelection(SELECT_OBJECTS) 
{ 
	m_mdl = ps; Update(); 
	SetMovable(true);
}

int GObjectSelection::Count()
{
	return (int)m_item.size();
}

FEItemListBuilder* GObjectSelection::CreateItemList()
{
	GPartList* partList = new GPartList(m_mdl);
	GModel& m = *m_mdl;
	int N = m.Objects();
	for (int i = 0; i < N; ++i)
	{
		GObject* po = m.Object(i);
		if (po->IsSelected())
		{
			int parts = po->Parts();
			for (int j = 0; j < parts; ++j) partList->add(po->Part(j)->GetID());
		}
	}
	return partList;
}

void GObjectSelection::Invert()
{
	GModel& m = *m_mdl;
	int n = m.Objects();
	for (int i=0; i<n; ++i) 
	{
		GObject* po = m.Object(i);
		if (po->IsVisible())
		{
			if (po->IsSelected())
				po->UnSelect();
			else
				po->Select();
		}
	}

	// TODO: should we update the selection or should we 
	//       rely on the document to do the update
	// Update();
}

void GObjectSelection::Update()
{
	GModel& mdl = *m_mdl;
	int N = mdl.Objects();

	int m = 0;
	m_box = mdl.GetBoundingBox();

	m_item.clear();
	m_item.reserve(N);

	for (int i=0; i<N; ++i)
	{
		GObject* po = mdl.Object(i);
		if (po->IsSelected())
		{
			m_item.push_back(i);
			if (m==0) m_box = po->GetGlobalBox();
			else m_box += po->GetGlobalBox();
			++m;
		}
	}
}

void GObjectSelection::Translate(vec3d dr)
{
	GModel& m = *m_mdl;
	int N = (int)m_item.size();
	for (int i=0; i<N; ++i)
	{
		GObject* po = m.Object(m_item[i]);
		assert(po->IsSelected());
		po->GetTransform().Translate(dr);
	}
	Update();
}

void GObjectSelection::Rotate(quatd q, vec3d rc)
{
	GModel& m = *m_mdl;
	int N = (int)m_item.size();
	for (int i=0; i<N; ++i)
	{
		GObject* po = m.Object(m_item[i]);
		assert(po->IsSelected());
		po->GetTransform().Rotate(q, rc);
	}
	Update();
}

void GObjectSelection::Scale(double s, vec3d dr, vec3d c)
{
	GModel& m = *m_mdl;
	int N = (int)m_item.size();
	for (int i=0; i<N; ++i)
	{
		GObject* po = m.Object(m_item[i]);
		assert(po->IsSelected());
		po->GetTransform().Scale(s, dr, c);
	}
	Update();
}

quatd GObjectSelection::GetOrientation()
{
	GModel& m = *m_mdl;
	int N = (int)m_item.size();
	for (int i=0; i<N; ++i)
	{
		GObject* po = m.Object(m_item[i]);
		assert(po->IsSelected());
		return po->GetTransform().GetRotation();
	}

	return quatd(0,0,0,1);
}

vec3d GObjectSelection::GetPivot()
{
	GModel& m = *m_mdl;

	vec3d v;
	int N = (int)m_item.size();
	for (int i=0; i<N; ++i)
	{
		GObject* po = m.Object(m_item[i]);
		assert(po->IsSelected());
		v += po->GetTransform().LocalToGlobal(vec3d(0, 0, 0));
	}
	if (N != 0) v /= N;
	return v;
}

vec3d GObjectSelection::GetScale()
{
	GModel& m = *m_mdl;
	int N = (int) m_item.size();
	if (N == 1) return m.Object(m_item[0])->GetTransform().GetScale();
	return vec3d(1,1,1);
}

GObject* GObjectSelection::Object(int i)
{
	GModel& m = *m_mdl;
	if ((i<0) || (i>=(int) m_item.size())) return 0;
	return m.Object(m_item[i]);
}

string GObjectSelection::GetName()
{
	int N = Count();
	if (N == 1)
	{
		return Object(0)->GetName();
	}
	else return FESelection::GetName();
}

//////////////////////////////////////////////////////////////////////
// GPartSelection
//////////////////////////////////////////////////////////////////////

GPartSelection::Iterator::Iterator(GPartSelection* ps)
{
	m_npart = -1;
	m_pg = nullptr;
	m_ps = ps;
	if (ps && ps->Size())
	{
		m_npart = 0;
		m_pg = ps->Part(0);
	}
}

GPartSelection::Iterator& GPartSelection::Iterator::operator ++()
{
	if (m_npart < m_ps->Size() - 1)
	{
		m_npart++;
		m_pg = m_ps->Part(m_npart);
	}
	else
	{
		m_npart = m_ps->Size();
		m_pg = nullptr;
	}
	return (*this);
}

GPartSelection::GPartSelection(GModel* ps) : GSelection(ps, SELECT_PARTS)
{ 
	Update(); 
}

int GPartSelection::Count()
{
	return (int)m_partList.size();
}

void GPartSelection::Invert()
{
	GModel& m = *GetGModel();
	int N = m.Parts();
	for (int i=0; i<N; ++i)
	{
		GPart* pg = m.Part(i);
		if (pg->IsVisible() && pg->IsSelected()) pg->UnSelect(); else if (pg->IsVisible()) pg->Select();
	}
	Update();
}

void GPartSelection::Update()
{
	GModel& model = *GetGModel();

	m_partList.clear();
	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		for (int i=0; i<po->Parts(); ++i)
		{
			GPart* pg = po->Part(i); assert(pg);
			if (pg && pg->IsSelected())
			{
				m_partList.push_back(pg);
			}
		}
	}

	UpdateBoundingBox();
}

void GPartSelection::UpdateBoundingBox()
{
	BOX box;

	GModel& model = *GetGModel();
	for (int k = 0; k < model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		GMesh* pm = po->GetRenderMesh();
		int NF = pm->Faces();
		for (int i = 0; i < NF; ++i)
		{
			GMesh::FACE& f = pm->Face(i);
			GFace* pf = po->Face(f.pid); assert(pf);
			GPart* pg = (pf ? po->Part(pf->m_nPID[0]) : nullptr);
			assert(pg);
			if (pg && pg->IsSelected())
			{
				for (int j = 0; j < 3; ++j)
				{
					vec3d r = po->GetTransform().LocalToGlobal(to_vec3d(pm->Node(f.n[j]).r));
					box += r;
				}
			}
		}
	}

	m_box = box;
}

quatd GPartSelection::GetOrientation()
{
	return quatd(0,0,0);
}

FEItemListBuilder* GPartSelection::CreateItemList()
{
	return new GPartList(GetGModel(), this);
}

//////////////////////////////////////////////////////////////////////
// GFaceSelection
//////////////////////////////////////////////////////////////////////

GFaceSelection::Iterator::Iterator(GFaceSelection* ps)
{
	m_sel = ps;
	if (ps && ps->Size())
	{
		m_pf = ps->Face(0);
		m_nsurf = 0;
	}
	else
	{
		m_pf = nullptr;
		m_nsurf = -1;
	}
}

GFaceSelection::Iterator& GFaceSelection::Iterator::operator ++()
{
	assert(m_sel);
	if (m_nsurf < m_sel->Size() - 1)
	{
		m_nsurf++;
		m_pf = m_sel->Face(m_nsurf);
	}
	else
	{
		m_nsurf = m_sel->Size();
		m_pf = nullptr;
	}
	return (*this);
}

GFaceSelection::GFaceSelection(GModel* ps) : GSelection(ps, SELECT_SURFACES) 
{ 
	Update();

	// we only allow the selection to be moved if the owning object
	// has an object manipulator and has not been meshed yet.
	bool moveAble = true;
	for (int i = 0; i < Size(); ++i)
	{
		GFace* pf = Face(i);
		GObject* po = dynamic_cast<GObject*>(pf->Object());
		if ((po == nullptr) || (po->GetManipulator() == nullptr) || po->GetFEMesh())
		{
			moveAble = false;
			break;
		}
	}
	SetMovable(moveAble);
}

void GFaceSelection::Translate(vec3d dr)
{

}

int GFaceSelection::Count()
{
	return (int)m_faceList.size();
}

void GFaceSelection::Invert()
{
	GModel& m = *GetGModel();
	int N = m.Surfaces();
	for (int i=0; i<N; ++i)
	{
		GFace* pg = m.Surface(i);
		if (pg->IsVisible() && pg->IsSelected()) pg->UnSelect(); else if (pg->IsVisible()) pg->Select();
	}
	Update();
}

void GFaceSelection::Update()
{
	GModel& model = *GetGModel();

	m_faceList.clear();
	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		for (int i=0; i<po->Faces(); ++i)
		{
			GFace* pf = po->Face(i);
			if (pf && pf->IsSelected())
			{
				m_faceList.push_back(pf);
			}
		}
	}

	UpdateBoundingBox();
}

void GFaceSelection::UpdateBoundingBox()
{
	BOX box;
	for (int k = 0; k < m_faceList.size(); ++k)
	{
		GFace* pf = m_faceList[k];

		GObject* po = dynamic_cast<GObject*>(pf->Object());
		if (po)
		{
			GMesh* pm = po->GetRenderMesh();
			int NF = pm->Faces();
			for (int i = 0; i < NF; ++i)
			{
				GMesh::FACE& f = pm->Face(i);
				if (f.pid == pf->GetLocalID())
				{
					for (int j = 0; j < 3; ++j)
					{
						vec3d r_local = to_vec3d(pm->Node(f.n[j]).r);
						vec3d r = po->GetTransform().LocalToGlobal(r_local);
						box += r;
					}
				}
			}
		}
	}

	m_box = box;
}

FEItemListBuilder* GFaceSelection::CreateItemList()
{
	return new GFaceList(GetGModel(), this);
}

//////////////////////////////////////////////////////////////////////
// GEdgeSelection
//////////////////////////////////////////////////////////////////////

GEdgeSelection::Iterator::Iterator(GEdgeSelection* pg)
{
	m_ps = pg;
	m_nedge = -1;
	m_pe = nullptr;
	if (pg && pg->Size())
	{
		m_nedge = 0;
		m_pe = pg->Edge(0);
	}
}

GEdgeSelection::Iterator& GEdgeSelection::Iterator::operator ++()
{
	if (m_ps)
	{
		if (m_nedge < m_ps->Size() - 1)
		{
			m_nedge++;
			m_pe = m_ps->Edge(m_nedge);
		}
		else
		{
			m_nedge = m_ps->Size();
			m_pe = nullptr;
		}
	}
	return (*this);
}

GEdgeSelection::GEdgeSelection(GModel* ps) : GSelection(ps, SELECT_CURVES)
{ 
	Update(); 
}

int GEdgeSelection::Count()
{
	return (int)m_edgeList.size();
}

void GEdgeSelection::Invert()
{
	GModel& m = *GetGModel();
	int N = m.Edges();
	for (int i=0; i<N; ++i)
	{
		GEdge* pe = m.Edge(i);
		if (pe->IsSelected()) pe->UnSelect(); else pe->Select();
	}
	Update();
}

void GEdgeSelection::Update()
{
	GModel& model = *GetGModel();

	m_edgeList.clear();
	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		for (int i=0; i<po->Edges(); ++i)
		{
			GEdge* pe = po->Edge(i);
			if (pe && pe->IsSelected())
			{
				m_edgeList.push_back(pe);
			}
		}
	}

	UpdateBoundingBox();
}

void GEdgeSelection::UpdateBoundingBox()
{
	GModel& model = *GetGModel();

	BOX box;
	for (int k = 0; k < model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		GMesh* pm = po->GetRenderMesh();

		int NE = pm->Edges();
		for (int i = 0; i < NE; ++i)
		{
			GMesh::EDGE& e = pm->Edge(i);
			GEdge* pe = po->Edge(e.pid);
			assert(pe);
			if (pe && pe->IsSelected())
			{
				for (int j = 0; j < 2; ++j)
				{
					vec3d r_local = to_vec3d(pm->Node(e.n[j]).r);
					vec3d r = po->GetTransform().LocalToGlobal(r_local);

					box += r;
				}
			}
		}
	}

	m_box = box;
}

FEItemListBuilder* GEdgeSelection::CreateItemList()
{
	return new GEdgeList(GetGModel(), this);
}

//////////////////////////////////////////////////////////////////////
// GNodeSelection
//////////////////////////////////////////////////////////////////////

GNodeSelection::Iterator::Iterator(GNodeSelection* pg) : m_sel(pg)
{
	GModel& m = *pg->GetGModel();
	m_pn = nullptr;
	m_index = -1;
	if (pg->Size())
	{
		m_index = 0;
		m_pn = pg->Node(m_index);
	}
}

GNodeSelection::Iterator& GNodeSelection::Iterator::operator ++()
{
	assert(m_pn);
	if (m_index < m_sel->Size() - 1)
	{
		m_index++;
		m_pn = m_sel->Node(m_index);
	}
	else
	{
		m_index = m_sel->Size();
		m_pn = nullptr;
	}
	return (*this);
}

GNodeSelection::GNodeSelection(GModel* ps) : GSelection(ps, SELECT_NODES) 
{
	Update();

	// we only allow the selection to be moved if the owning object
	// has an object manipulator and has not been meshed yet.
	bool moveAble = true;
	for (int i = 0; i < Size(); ++i)
	{
		GNode* pn = Node(i);
		GObject* po = dynamic_cast<GObject*>(pn->Object());
		if ((po == nullptr) || (po->GetManipulator() == nullptr) || po->GetFEMesh())
		{
			moveAble = false;
			break;
		}
	}
	SetMovable(moveAble);
}

int GNodeSelection::Count()
{
	return (int)m_nodeList.size();
}

void GNodeSelection::Invert()
{
	GModel& m = *GetGModel();
	int N = m.Nodes();
	for (int i=0; i<N; ++i)
	{
		GNode* pn = m.Node(i);
		if (pn->IsSelected()) pn->UnSelect(); else pn->Select();
	}
	Update();
}

void GNodeSelection::Translate(vec3d dr)
{
	for (int i = 0; i < Size(); ++i)
	{
		GNode* pn = Node(i);
		GObject* po = dynamic_cast<GObject*>(pn->Object());
		if (po && po->GetManipulator())
		{
			Transform T;
			T.SetPosition(dr);
			po->GetManipulator()->TransformNode(pn, T);
		}
	}
	UpdateBoundingBox();
}

void GNodeSelection::Update()
{
	GModel& model = *GetGModel();

	m_nodeList.clear();
	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);

		int N = po->Nodes();
		for (int i=0; i<N; ++i)
		{
			GNode* pn = po->Node(i);
			if (pn && pn->IsSelected())
			{
				m_nodeList.push_back(pn);
			}
		}
	}

	UpdateBoundingBox();
}

void GNodeSelection::UpdateBoundingBox()
{
	BOX box;
	for (int k = 0; k < Size(); ++k)
	{
		GNode* pn = Node(k); assert(pn && pn->IsSelected());
		box += pn->Position();
	}
	m_box = box;
}

FEItemListBuilder* GNodeSelection::CreateItemList()
{
	return new GNodeList(GetGModel(), this);
}

//////////////////////////////////////////////////////////////////////
// GDiscreteSelection
//////////////////////////////////////////////////////////////////////

GDiscreteSelection::Iterator::Iterator(GDiscreteSelection* pg)
{
	m_ps = pg->GetGModel();
	GModel& m = *m_ps;
	m_item = -1;
	m_comp = 0;
	m_pn = 0;
	int N = m.DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* pi = m.DiscreteObject(i);
		if (pi->IsSelected())
		{
			m_pn = pi;
			m_item = i;
			break;
		}
		GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(pi);
		if (pds)
		{
			for (int j = 0; j < pds->size(); ++j)
			{
				if (pds->element(j).IsSelected())
				{
					m_pn = pds;
					m_item = i;
					m_comp = j;
					break;
				}
			}
		}
	}
}

GDiscreteSelection::Iterator& GDiscreteSelection::Iterator::operator ++()
{
	assert(m_pn);
	GModel& m = *m_ps;
	int N = m.DiscreteObjects();
	for (int i = m_item + 1; i<N; ++i)
	{
		GDiscreteObject* pn = m.DiscreteObject(i);
		if (pn->IsSelected())
		{
			m_pn = pn;
			m_item = i;
			break;
		}
	}

	return (*this);
}

GDiscreteSelection::GDiscreteSelection(GModel* ps) : FESelection(SELECT_DISCRETE_OBJECT) 
{ 
	m_ps = ps; 
	m_count = 0;
	Update(); 
}

int GDiscreteSelection::Count()
{
	return m_count;
}

void GDiscreteSelection::Invert()
{
	if (m_ps == 0) return;
	GModel& m = *m_ps;
	int N = m.DiscreteObjects();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* pn = m.DiscreteObject(i);
		if (pn->IsSelected()) pn->UnSelect(); else pn->Select();
	}
}

void GDiscreteSelection::Update()
{
	if (m_ps == 0) return;
	GModel& model = *m_ps;

	m_count = 0;

	const double LARGE = 1e20;

	BOX box;

	int N = model.DiscreteObjects();
	vec3d r;
	for (int i = 0; i<N; ++i)
	{
		GDiscreteObject* pn = model.DiscreteObject(i);
		if (pn && pn->IsSelected())
		{
			GLinearSpring* pls = dynamic_cast<GLinearSpring*>(pn);
			if (pls)
			{
				for (int j=0; j<2; ++j)
				{
					GNode& nj = *model.FindNode(pls->m_node[j]);
					r = nj.Position();
					box += r;
				}
				++m_count;
			}
			GGeneralSpring* pgs = dynamic_cast<GGeneralSpring*>(pn);
			if (pgs)
			{
				for (int j = 0; j<2; ++j)
				{
					GNode& nj = *model.FindNode(pls->m_node[j]);
					r = nj.Position();
					box += r;
				}
				++m_count;
			}
		}
		if (dynamic_cast<GDiscreteSpringSet*>(pn))
		{
			GDiscreteSpringSet* ps = dynamic_cast<GDiscreteSpringSet*>(pn);
			for (int i = 0; i < ps->size(); ++i)
			{
				GDiscreteElement& d = ps->element(i);
				if (d.IsSelected())
				{
					GNode& n0 = *model.FindNode(d.Node(0));
					GNode& n1 = *model.FindNode(d.Node(1));

					box += n0.Position();
					box += n1.Position();

					++m_count;
				}
			}
		}
	}

	m_box = box;
}


//////////////////////////////////////////////////////////////////////
// FEElementSelection
//////////////////////////////////////////////////////////////////////


FEElementSelection::Iterator::Iterator(FEElementSelection* sel)
{
	m_sel = sel;
	m_n = 0;
	m_pelem = nullptr;
	if (m_sel)
	{
		size_t n = m_sel->Count();
		if (n > 0)
		{
			m_pelem = m_sel->Element(0);
		}
	}
}

void FEElementSelection::Iterator::operator ++()
{
	if (m_sel == nullptr) return;
	size_t n = m_sel->Count();
	if (m_n < n) m_n++;
	if (m_n < n) m_pelem = m_sel->Element(m_n);
	else m_pelem = nullptr;
}

FEElementSelection::FEElementSelection(FSMesh* pm) : FEMeshSelection(SELECT_FE_ELEMS)
{ 
	m_pMesh = pm; 
	if (pm && pm->IsEditable()) SetMovable(true);
	Update();
}

int FEElementSelection::Count()
{
	return (int)m_item.size();
}

void FEElementSelection::Invert()
{
	if (m_pMesh == nullptr) return;
	int N = m_pMesh->Elements(); 
	for (int i = 0; i < N; ++i)
	{
		FSElement& el = m_pMesh->Element(i);
		if (el.IsVisible())
		{
			if (el.IsSelected()) el.Unselect();
			else el.Select();
		}
	}
	Update();
}

void FEElementSelection::Update()
{
	m_item.clear();
	if (m_pMesh == nullptr) return;
	FSMesh& mesh = *m_pMesh;

	GObject* po = mesh.GetGObject(); assert(po);
	const Transform& T = po->GetTransform();

	int N = mesh.Elements();
	m_item.reserve(N);

	BOX box;
	for (int i=0; i<N; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.IsSelected())
		{
			int* n = el.m_node;
			int l = el.Nodes();
			m_item.push_back(i);
			for (int j=0; j<l; ++j)
			{
				box += mesh.Node(n[j]).r;
			}
		}
	}

	m_box = LocalToGlobalBox(box, T);
}

void FEElementSelection::Translate(vec3d dr)
{
	if (m_pMesh == 0) return;
	int i, j, l;

	GObject* po = m_pMesh->GetGObject();

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected elements
	int NE = (int)m_item.size();
	for (i=0; i<NE; ++i)
	{
		FEElement_* pe = m_pMesh->ElementPtr(m_item[i]);
		assert(pe->IsSelected());
		l = pe->Nodes();
		for (j=0; j<l; ++j)
			pn[pe->m_node[j]].m_ntag = 1;
	}

	// move all tagged nodes
	vec3d r;
	for (i=0; i<N; ++i) 
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r) + dr;
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}

	m_pMesh->UpdateMesh();

	// update the box
	m_box.x0 += dr.x;
	m_box.y0 += dr.y;
	m_box.z0 += dr.z;

	m_box.x1 += dr.x;
	m_box.y1 += dr.y;
	m_box.z1 += dr.z;

	// update the geometry nodes
	po->UpdateGNodes();
}

void FEElementSelection::Rotate(quatd q, vec3d rc)
{
	if (m_pMesh == 0) return;
	int i, j, l;

	GObject* po = m_pMesh->GetGObject();

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected elements
	int NE = (int)m_item.size();
	for (i=0; i<NE; ++i)
	{
		FEElement_* pe = m_pMesh->ElementPtr(m_item[i]);
		l = pe->Nodes();
		for (j=0; j<l; ++j) pn[pe->m_node[j]].m_ntag = 1;

		++pe;
	}

	// move all tagged nodes
	vec3d dr, r;
	for (i=0; i<N; ++i) 
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			dr = r - rc;
			r = rc + q*dr;
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}

	m_pMesh->UpdateMesh();

	Update();
	// update the geometry nodes
	po->UpdateGNodes();
}

void FEElementSelection::Scale(double s, vec3d dr, vec3d c)
{
	if (m_pMesh == 0) return;
	int i, j, l;
	GObject* po = m_pMesh->GetGObject();

	double a = s - 1;
	dr.x = 1 + a*fabs(dr.x);
	dr.y = 1 + a*fabs(dr.y);
	dr.z = 1 + a*fabs(dr.z);

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected elements
	int NE = (int)m_item.size();
	for (i=0; i<NE; ++i)
	{
		FEElement_* pe = m_pMesh->ElementPtr(m_item[i]);
		assert(pe->IsSelected());
		l = pe->Nodes();
		for (j=0; j<l; ++j) pn[pe->m_node[j]].m_ntag = 1;

		++pe;
	}

	// scale all tagged nodes
	vec3d r;
	for (i=0; i<N; ++i) 
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			r.x = c.x + dr.x*(r.x - c.x);
			r.y = c.y + dr.y*(r.y - c.y);
			r.z = c.z + dr.z*(r.z - c.z);
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}

	m_pMesh->UpdateMesh();

	Update();

	// update the geometry nodes
	po->UpdateGNodes();

}

quatd FEElementSelection::GetOrientation()
{
	if (m_pMesh == 0) return quatd(0, 0, 0, 1); else return m_pMesh->GetGObject()->GetTransform().GetRotation();
}

FEItemListBuilder* FEElementSelection::CreateItemList()
{
	FSMesh* pm = GetMesh();
	GObject* po = pm->GetGObject();
	return new FSElemSet(po, m_item);
}

FEElement_* FEElementSelection::Element(size_t i)
{
	if ((i<0) || (i>= m_item.size())) return nullptr;
	return m_pMesh->ElementPtr(m_item[i]);
}

int FEElementSelection::ElementIndex(size_t i) const
{
	if ((i<0) || (i>= m_item.size())) return -1;
	return m_item[i];
}

//////////////////////////////////////////////////////////////////////
// FEFaceSelection
//////////////////////////////////////////////////////////////////////

FEFaceSelection::FEFaceSelection(FSMeshBase* pm) : FEMeshSelection(SELECT_FE_FACES)
{ 
	m_pMesh = pm; 
	if (pm && pm->IsEditable()) SetMovable(true);
	Update(); 
}

FSFace* FEFaceSelection::Face(size_t n)
{
	if (m_pMesh) return m_pMesh->FacePtr(m_item[n]);
	else return nullptr;
}

int FEFaceSelection::Count()
{
	return (int)m_item.size();
}

void FEFaceSelection::Invert()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Faces(); 
	FSFace* pf = m_pMesh->FacePtr();
	for (int i=0; i<N; ++i, ++pf)
		if (pf->IsVisible())
		{
			if (pf->IsSelected()) pf->Unselect(); 
			else pf->Select();
		}
	Update();
}

void FEFaceSelection::Update()
{
	m_item.clear();
	if (m_pMesh == nullptr) return;

	int N = m_pMesh->Faces();
	FSNode* pn = m_pMesh->NodePtr();
	FSFace* pf = m_pMesh->FacePtr();
	GObject* po = m_pMesh->GetGObject();
	const Transform& T = po->GetTransform();

	BOX box;
	for (int i=0; i<N; ++i, ++pf)
	{
		if (pf->IsSelected())
		{
			FSFace& f = *pf;
			int* n = f.n;
			for (int j=0; j<pf->Nodes(); ++j)
			{
				box += pn[n[j]].r;
			}
			m_item.push_back(i);
		}
	}

	// convert local box to global box
	m_box = LocalToGlobalBox(box, T);
}

void FEFaceSelection::Translate(vec3d dr)
{
	if (m_pMesh == 0) return;
	int i, j;
	GObject* po = m_pMesh->GetGObject();

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected faces
	Iterator pf(this);
	while (pf)
	{
		for (j=0; j<pf->Nodes(); ++j) pn[pf->n[j]].m_ntag = 1;

		++pf;
	}

	// move all tagged nodes
	vec3d r;
	for (i=0; i<N; ++i)
	{
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			r += dr;
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}
	}

	m_pMesh->UpdateMesh();

	// update the box
	m_box.x0 += dr.x;
	m_box.y0 += dr.y;
	m_box.z0 += dr.z;

	m_box.x1 += dr.x;
	m_box.y1 += dr.y;
	m_box.z1 += dr.z;

	// update the geometry nodes
	po->UpdateGNodes();

}

void FEFaceSelection::Rotate(quatd q, vec3d rc)
{
	if (m_pMesh == 0) return;
	int i, j;
	GObject* po = m_pMesh->GetGObject();

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected faces
	Iterator pf(this);
	while (pf)
	{
		for (j=0; j<pf->Nodes(); ++j) pn[pf->n[j]].m_ntag = 1;
		++pf;
	}

	// move all tagged nodes
	vec3d dr, r;
	for (i=0; i<N; ++i) 
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			dr = r - rc;
			r = rc + q*dr;
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}

	m_pMesh->UpdateMesh();

	Update();

	// update the geometry nodes
	po->UpdateGNodes();
}

void FEFaceSelection::Scale(double s, vec3d dr, vec3d c)
{
	if (m_pMesh == 0) return;
	int i, j;
	GObject* po = m_pMesh->GetGObject();

	double a = s - 1;
	dr.x = 1 + a*fabs(dr.x);
	dr.y = 1 + a*fabs(dr.y);
	dr.z = 1 + a*fabs(dr.z);

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected faces
	Iterator pf(this);
	while (pf)
	{
		for (j=0; j<pf->Nodes(); ++j) pn[pf->n[j]].m_ntag = 1;

		++pf;
	}

	// move all tagged nodes
	vec3d r;
	for (i=0; i<N; ++i) 
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			r.x = c.x + dr.x*(r.x - c.x);
			r.y = c.y + dr.y*(r.y - c.y);
			r.z = c.z + dr.z*(r.z - c.z);
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}

	m_pMesh->UpdateMesh();

	Update();

	// update the geometry nodes
	po->UpdateGNodes();
}

quatd FEFaceSelection::GetOrientation()
{
	if (m_pMesh == 0) return quatd(0, 0, 0, 1); else return m_pMesh->GetGObject()->GetTransform().GetRotation();
}

FEFaceSelection::Iterator::Iterator(FEFaceSelection* psel)
{ 
	m_psel = psel;
	m_n = 0;
	m_pface = nullptr;
	if (m_psel && m_psel->Count())
	{
		m_pface = m_psel->Face(0);
	}
}

void FEFaceSelection::Iterator::operator ++()
{
	if (m_psel)
	{
		int N = m_psel->Count();
		if (m_n < N) m_n++;
		if (m_n < N) m_pface = m_psel->Face(m_n);
		else m_pface = nullptr;
	}
}

FEItemListBuilder* FEFaceSelection::CreateItemList()
{
	FSMesh* pm = dynamic_cast<FSMesh*>(GetMesh());
	if (pm == nullptr) return nullptr;

	GObject* po = pm->GetGObject();
	vector<int> fs;
	for (int i=0; i<pm->Faces(); ++i)
		if (pm->Face(i).IsSelected()) fs.push_back(i);
	return new FSSurface(po, fs);
}

FEFaceSelection::Iterator FEFaceSelection::begin()
{
	return Iterator(this);
}

//////////////////////////////////////////////////////////////////////
// FEEdgeSelection
//////////////////////////////////////////////////////////////////////

FEEdgeSelection::FEEdgeSelection(FSLineMesh* pm) : FEMeshSelection(SELECT_FE_EDGES)
{ 
	m_pMesh = pm; 
	if (pm && pm->IsEditable()) SetMovable(true);
	Update(); 
}

int FEEdgeSelection::Count()
{
	return (int)m_items.size();
}

void FEEdgeSelection::Invert()
{
	if (m_pMesh == nullptr) return;
	int N = m_pMesh->Edges(); 
	FSEdge* pe = m_pMesh->EdgePtr();
	for (int i=0; i<N; ++i, ++pe)
		if (pe->IsVisible())
		{
			if (pe->IsSelected()) pe->Unselect(); 
			else pe->Select();
		}
	Update();
}

void FEEdgeSelection::Update()
{
	m_items.clear();
	if (m_pMesh == nullptr) return;
	FSEdge* pe = m_pMesh->EdgePtr();
	BOX box;
	int NE = m_pMesh->Edges();
	for (int i=0; i<NE; ++i, ++pe)
	{
		if (pe->IsSelected())
		{
			FSEdge& e = *pe;
			for (int j=0; j<e.Nodes(); ++j)
			{
				vec3d r = m_pMesh->Node(e.n[j]).r;
				box += r;
			}
			m_items.push_back(i);
		}
	}
	m_box = LocalToGlobalBox(box, m_pMesh->GetGObject()->GetTransform());
}

void FEEdgeSelection::Translate(vec3d dr)
{
	if (m_pMesh == nullptr) return;
	int i, j;
	GObject* po = m_pMesh->GetGObject();

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected edges
	Iterator pe(this);
	while (pe)
	{
		for (j=0; j<pe->Nodes(); ++j) pn[pe->n[j]].m_ntag = 1;

		++pe;
	}

	// move all tagged nodes
	vec3d r;
	for (i=0; i<N; ++i)
	{
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			r += dr;
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}
	}

	m_pMesh->UpdateMesh();

	// update the box
	m_box.x0 += dr.x;
	m_box.y0 += dr.y;
	m_box.z0 += dr.z;

	m_box.x1 += dr.x;
	m_box.y1 += dr.y;
	m_box.z1 += dr.z;

	// update the geometry nodes
	po->UpdateGNodes();
}

void FEEdgeSelection::Rotate(quatd q, vec3d rc)
{
	if (m_pMesh == 0) return;
	int i, j;
	GObject* po = m_pMesh->GetGObject();

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected faces
	Iterator pe(this);
	while (pe)
	{
		for (j=0; j<pe->Nodes(); ++j) pn[pe->n[j]].m_ntag = 1;

		++pe;
	}

	// move all tagged nodes
	vec3d dr, r;
	for (i=0; i<N; ++i) 
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			dr = r - rc;
			r = rc + q*dr;
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}

	m_pMesh->UpdateMesh();

	Update();

	// update the geometry nodes
	po->UpdateGNodes();
}

void FEEdgeSelection::Scale(double s, vec3d dr, vec3d c)
{
	if (m_pMesh == 0) return;
	int i, j;
	GObject* po = m_pMesh->GetGObject();

	double a = s - 1;
	dr.x = 1 + a*fabs(dr.x);
	dr.y = 1 + a*fabs(dr.y);
	dr.z = 1 + a*fabs(dr.z);

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected faces
	Iterator pe(this);
	while (pe)
	{
		for (j=0; j<pe->Nodes(); ++j) pn[pe->n[j]].m_ntag = 1;
		++pe;
	}

	// move all tagged nodes
	vec3d r;
	for (i=0; i<N; ++i) 
		if (pn[i].m_ntag) 
		{
			r = po->GetTransform().LocalToGlobal(pn[i].r);
			r.x = c.x + dr.x*(r.x - c.x);
			r.y = c.y + dr.y*(r.y - c.y);
			r.z = c.z + dr.z*(r.z - c.z);
			pn[i].r = po->GetTransform().GlobalToLocal(r);
		}

	m_pMesh->UpdateMesh();

	Update();

	// update the geometry nodes
	po->UpdateGNodes();
}

quatd FEEdgeSelection::GetOrientation()
{
	if (m_pMesh == 0) return quatd(0, 0, 0, 1); else return m_pMesh->GetGObject()->GetTransform().GetRotation();
}

FEEdgeSelection::Iterator::Iterator(FEEdgeSelection* sel)
{ 
	m_sel = sel;
	m_n = 0;
	m_pedge = nullptr;
	if (m_sel)
	{
		m_pedge = sel->Edge(0);
	}
}

void FEEdgeSelection::Iterator::operator ++()
{
	if (m_sel)
	{
		int n = m_sel->Count();
		if (m_n < n) m_n++;
		if (m_n < n) m_pedge = m_sel->Edge(m_n);
		else m_pedge = nullptr;
	}
}

FEItemListBuilder* FEEdgeSelection::CreateItemList()
{
	FSMesh* pm = dynamic_cast<FSMesh*>(GetMesh());
	if (pm == nullptr) return nullptr;
	GObject* po = pm->GetGObject();
	vector<int> es;
	for (int i=0; i<pm->Edges(); ++i)
		if (pm->Edge(i).IsSelected()) es.push_back(i);
	return new FSEdgeSet(po, es);
}

//////////////////////////////////////////////////////////////////////
// FENodeSelection
//////////////////////////////////////////////////////////////////////

FENodeSelection::FENodeSelection(FSLineMesh* pm) : FEMeshSelection(SELECT_FE_NODES)
{ 
	m_pMesh = pm; 
	if (pm && pm->IsEditable()) SetMovable(true);
	Update();
}

int FENodeSelection::Count()
{
	return (int)m_items.size();
}

FSNode* FENodeSelection::Node(size_t n)
{
	if (m_pMesh == nullptr) return nullptr;
	if (n >= m_items.size()) return nullptr;
	return m_pMesh->NodePtr(m_items[n]);
}

FENodeSelection::Iterator FENodeSelection::First()
{
	return Iterator(this);
}

void FENodeSelection::Invert()
{
	if (m_pMesh == nullptr) return;
	int N = m_pMesh->Nodes(); 
	FSNode* pn = m_pMesh->NodePtr();
	for (int i=0; i<N; ++i, ++pn)
		if (pn->IsVisible())
		{
			if (pn->IsSelected()) pn->Unselect(); 
			else pn->Select();
		}
	Update();
}

void FENodeSelection::Update()
{
	m_items.clear();
	if (m_pMesh == nullptr) return;
	int N = m_pMesh->Nodes();
	FSNode* pn = m_pMesh->NodePtr();
	BOX box;
	for (int i=0; i<N; ++i, ++pn)
	{
		if (pn->IsSelected())
		{
			vec3d r = pn->r;
			box += r;
			m_items.push_back(i);
		}
	}

	GObject* po = m_pMesh->GetGObject();
	m_box = LocalToGlobalBox(box, po->GetTransform());
}

void FENodeSelection::Translate(vec3d dr)
{
	if (m_pMesh == 0) return;
	GObject* po = m_pMesh->GetGObject();

	// loop over all selected nodes
	Iterator pn(this);
	while (pn)
	{
		vec3d r = po->GetTransform().LocalToGlobal(pn->r);
		r += dr;
		pn->r = po->GetTransform().GlobalToLocal(r);
		++pn;
	}

	m_pMesh->UpdateMesh();

	// update the box
	m_box.x0 += dr.x;
	m_box.y0 += dr.y;
	m_box.z0 += dr.z;

	m_box.x1 += dr.x;
	m_box.y1 += dr.y;
	m_box.z1 += dr.z;

	// update the geometry nodes
	po->UpdateGNodes();
}

void FENodeSelection::Rotate(quatd q, vec3d rc)
{
	if (m_pMesh == 0) return;
	GObject* po = m_pMesh->GetGObject();

	q.MakeUnit();

	Iterator pn(this);
	while (pn)
	{
		vec3d r = po->GetTransform().LocalToGlobal(pn->r);
		vec3d dr = r - rc;
		r = rc + q*dr;
		pn->r = po->GetTransform().GlobalToLocal(r);
		++pn;
	}

	m_pMesh->UpdateMesh();

	Update();

	// update the geometry nodes
	po->UpdateGNodes();
}

void FENodeSelection::Scale(double s, vec3d dr, vec3d c)
{
	if (m_pMesh == 0) return;
	GObject* po = m_pMesh->GetGObject();

	double a = s - 1;
	dr.x = 1 + a*fabs(dr.x);
	dr.y = 1 + a*fabs(dr.y);
	dr.z = 1 + a*fabs(dr.z);

	Iterator pn(this);
	vec3d r;
	while (pn)
	{
		r = po->GetTransform().LocalToGlobal(pn->r);
		r.x = c.x + dr.x*(r.x - c.x);
		r.y = c.y + dr.y*(r.y - c.y);
		r.z = c.z + dr.z*(r.z - c.z);
		pn->r = po->GetTransform().GlobalToLocal(r);
		++pn;
	}

	m_pMesh->UpdateMesh();

	Update();

	// update the geometry nodes
	po->UpdateGNodes();
}

quatd FENodeSelection::GetOrientation()
{
	if (m_pMesh == 0) return quatd(0, 0, 0, 1); else return m_pMesh->GetGObject()->GetTransform().GetRotation();
}

FENodeSelection::Iterator::Iterator(FENodeSelection* psel)
{ 
	m_psel = psel;
	m_n = 0;
	m_pnode = nullptr;
	if (psel && psel->Count())
	{
		m_pnode = psel->Node(m_n);
	}
}

void FENodeSelection::Iterator::operator ++()
{
	if (m_psel)
	{
		int n = m_psel->Count();
		if (m_n < n) m_n++;
		if (m_n < n) m_pnode = m_psel->Node(m_n);
		else m_pnode = nullptr;
	}
}

FEItemListBuilder* FENodeSelection::CreateItemList()
{
	FSMesh* pm = dynamic_cast<FSMesh*>(GetMesh());
	if (pm == nullptr) return nullptr;
	GObject* po = pm->GetGObject();
	return new FSNodeSet(po, m_items);
}
