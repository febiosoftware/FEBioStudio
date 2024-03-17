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

//////////////////////////////////////////////////////////////////////
// FESelection
//////////////////////////////////////////////////////////////////////

FESelection::FESelection(int ntype) : m_ntype(ntype)
{
	m_nsize = -1;
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

int GObjectSelection::Count()
{
	return (int)m_item.size();
}

int GObjectSelection::Next()
{
	int n = -1;
	GModel& m = *m_mdl;
	int N = m.Objects();
	for (int i=0; i<N; ++i) if (m.Object(i)->IsSelected()) { n = (i+1)%N; break; }
	return n;
}

int GObjectSelection::Prev()
{
	int n = -1;
	GModel& m = *m_mdl;
	int N = m.Objects();
	for (int i=0; i<N; ++i) if (m.Object(i)->IsSelected()) { n = (i==0?N-1:i-1); break; }
	return n;
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
	m_pg = 0;
	m_ps = ps->GetGModel();
	GModel& m = *m_ps;
	int N = m.Parts();
	for (int i=0; i<N; ++i)
	{
		GPart* pg = m.Part(i);
		if (pg->IsVisible() && pg->IsSelected())
		{
			m_pg = pg;
			m_npart = i;
			break;
		}
	}
}

GPartSelection::Iterator& GPartSelection::Iterator::operator ++()
{
	assert(m_pg);
	GModel& m = *m_ps;
	int N = m.Parts();
	for (int i=m_npart+1; i<N; ++i)
	{
		GPart* pg = m.Part(i);
		if (pg->IsVisible() && pg->IsSelected())
		{
			m_pg = pg;
			m_npart = i;
			break;
		}
	}

	return (*this);
}

int GPartSelection::Count()
{
	if (m_mdl == 0) return 0;
	GModel& m = *m_mdl;
	int n = 0;
	int N = m.Parts();
	for (int i=0; i<N; ++i) if (m.Part(i)->IsVisible() && m.Part(i)->IsSelected()) ++n;
	return n;
}

int GPartSelection::Next()
{
	GModel& m = *m_mdl;
	int n = -1;
	int N = m.Parts();
	for (int i=0; i<N; ++i) if (m.Part(i)->IsVisible() && m.Part(i)->IsSelected()) { n = m.Part((i+1)%N)->GetID(); break; }
	return n;
}

int GPartSelection::Prev()
{
	GModel& m = *m_mdl;
	int n = -1;
	int N = m.Parts();
	for (int i=0; i<N; ++i) if (m.Part(i)->IsVisible() && m.Part(i)->IsSelected()) { n = m.Part((i==0?N-1:i-1))->GetID(); break; }
	return n;
}

void GPartSelection::Invert()
{
	if (m_mdl == 0) return;
	GModel& m = *m_mdl;
	int N = m.Parts();
	for (int i=0; i<N; ++i)
	{
		GPart* pg = m.Part(i);
		if (pg->IsVisible() && pg->IsSelected()) pg->UnSelect(); else if (pg->IsVisible()) pg->Select();
	}
}

void GPartSelection::Update()
{
	if (m_mdl == 0) return;
	GModel& model = *m_mdl;

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		GMesh* pm = po->GetRenderMesh();
		int NF = pm->Faces();
		for (int i=0; i<NF; ++i)
		{
			GMesh::FACE& f = pm->Face(i);
			GFace* pf = po->Face(f.pid); assert(pf);
			GPart* pg = (pf ? po->Part(pf->m_nPID[0]) : nullptr);
			assert(pg);
			if (pg && pg->IsSelected())
			{
				for (int j=0; j<3; ++j)
				{
					vec3d r = po->GetTransform().LocalToGlobal(pm->Node(f.n[j]).r);

					if (r.x < m_box.x0) m_box.x0 = r.x;
					if (r.y < m_box.y0) m_box.y0 = r.y;
					if (r.z < m_box.z0) m_box.z0 = r.z;

					if (r.x > m_box.x1) m_box.x1 = r.x;
					if (r.y > m_box.y1) m_box.y1 = r.y;
					if (r.z > m_box.z1) m_box.z1 = r.z;
				}

				++m;
			}
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
}

quatd GPartSelection::GetOrientation()
{
	return quatd(0,0,0);
}

FEItemListBuilder* GPartSelection::CreateItemList()
{
	return new GPartList(m_mdl, this);
}

//////////////////////////////////////////////////////////////////////
// GFaceSelection
//////////////////////////////////////////////////////////////////////

GFaceSelection::Iterator::Iterator(GFaceSelection* ps)
{
	m_ps = ps->GetGModel();
	GModel& m = *m_ps;
	m_nsurf = -1;
	m_pf = 0;
	int N = m.Surfaces();
	for (int i=0; i<N; ++i)
	{
		GFace* pf = m.Surface(i);
		if (pf->IsVisible() && pf->IsSelected())
		{
			m_pf = pf;
			m_nsurf = i;
			break;
		}
	}
}

GFaceSelection::Iterator& GFaceSelection::Iterator::operator ++()
{
	assert(m_pf);
	GModel& m = *m_ps;
	int N = m.Surfaces();
	for (int i=m_nsurf+1; i<N; ++i)
	{
		GFace* pf = m.Surface(i);
		if (pf->IsVisible() && pf->IsSelected())
		{
			m_pf = pf;
			m_nsurf = i;
			break;
		}
	}

	return (*this);
}

int GFaceSelection::Count()
{
	if (m_ps == 0) return 0;
	GModel& m = *m_ps;
	int n = 0;
	int N = m.Surfaces();
	for (int i=0; i<N; ++i) if (m.Surface(i)->IsVisible() && m.Surface(i)->IsSelected()) ++n;
	return n;
}

int GFaceSelection::Next()
{
	int n = -1;
	GModel& m = *m_ps;
	int N = m.Surfaces();
	for (int i=0; i<N; ++i) 
	{
		GFace& f = *m.Surface(i);
		if (f.IsVisible() && f.IsExternal() && f.IsSelected())
		{
			n = f.GetID();
			for (int j=1; j<N; ++j)
			{
				GFace& f = *m.Surface((i+j)%N);
				if (f.IsExternal()) return f.GetID();
			}
			break;
		}
	}
	return n;
}

int GFaceSelection::Prev()
{
	int n = -1, m;
	GModel& model = *m_ps;

	int N = model.Surfaces();
	for (int i=0; i<N; ++i) 
	{
		GFace& f = *model.Surface(i);
		if (f.IsVisible() && f.IsExternal() && f.IsSelected())
		{
			n = f.GetID();
			for (int j=1; j<N; ++j)
			{
				m = (i-j < 0 ? (i-j+N) : i-j);
				GFace& f = *model.Surface(m);
				if (f.IsExternal()) return f.GetID();
			}
			break;
		}
	}
	return n;
}

void GFaceSelection::Invert()
{
	if (m_ps == 0) return;
	GModel& m = *m_ps;
	int N = m.Surfaces();
	for (int i=0; i<N; ++i)
	{
		GFace* pg = m.Surface(i);
		if (pg->IsVisible() && pg->IsSelected()) pg->UnSelect(); else if (pg->IsVisible()) pg->Select();
	}
}

void GFaceSelection::Update()
{
	if (m_ps == 0) return;
	GModel& model = *m_ps;

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		GMesh* pm = po->GetRenderMesh();

		int NF = pm->Faces();
		for (int i=0; i<NF; ++i)
		{
			GMesh::FACE& f = pm->Face(i);
			GFace* pf = po->Face(f.pid);
			assert(pf);
			if (pf && pf->IsSelected())
			{
				for (int j=0; j<3; ++j)
				{
					vec3d r = po->GetTransform().LocalToGlobal(pm->Node(f.n[j]).r);

					if (r.x < m_box.x0) m_box.x0 = r.x;
					if (r.y < m_box.y0) m_box.y0 = r.y;
					if (r.z < m_box.z0) m_box.z0 = r.z;

					if (r.x > m_box.x1) m_box.x1 = r.x;
					if (r.y > m_box.y1) m_box.y1 = r.y;
					if (r.z > m_box.z1) m_box.z1 = r.z;
				}

				++m;
			}
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
}


FEItemListBuilder* GFaceSelection::CreateItemList()
{
	return new GFaceList(m_ps, this);
}

//////////////////////////////////////////////////////////////////////
// GEdgeSelection
//////////////////////////////////////////////////////////////////////

GEdgeSelection::Iterator::Iterator(GEdgeSelection* pg)
{
	m_ps = pg->GetGModel();
	GModel& m = *m_ps;
	m_nedge = -1;
	m_pe = 0;
	int N = m.Edges();
	for (int i=0; i<N; ++i)
	{
		GEdge* pe = m.Edge(i);
		if (pe->IsSelected())
		{
			m_pe = pe;
			m_nedge = i;
			break;
		}
	}
}

GEdgeSelection::Iterator& GEdgeSelection::Iterator::operator ++()
{
	assert(m_pe);
	GModel& m = *m_ps;
	int N = m.Edges();
	for (int i=m_nedge+1; i<N; ++i)
	{
		GEdge* pe = m.Edge(i);
		if (pe->IsSelected())
		{
			m_pe = pe;
			m_nedge = i;
			break;
		}
	}

	return (*this);
}

int GEdgeSelection::Count()
{
	if (m_ps == 0) return 0;
	GModel& m = *m_ps;
	int n = 0;
	int N = m.Edges();
	for (int i=0; i<N; ++i) if (m.Edge(i)->IsSelected()) ++n;
	return n;
}

int GEdgeSelection::Next()
{
	int n = -1;
	GModel& m = *m_ps;
	int N = m.Edges();
	for (int i=0; i<N; ++i) if (m.Edge(i)->IsSelected()) { n = m.Edge((i+1)%N)->GetID(); break; }
	return n;
}

int GEdgeSelection::Prev()
{
	int n = -1;
	GModel& m = *m_ps;
	int N = m.Edges();
	for (int i=0; i<N; ++i) if (m.Edge(i)->IsSelected()) { n = m.Edge((i==0?N-1:i-1))->GetID(); break; }
	return n;
}

void GEdgeSelection::Invert()
{
	if (m_ps == 0) return;
	GModel& m = *m_ps;
	int N = m.Edges();
	for (int i=0; i<N; ++i)
	{
		GEdge* pe = m.Edge(i);
		if (pe->IsSelected()) pe->UnSelect(); else pe->Select();
	}
}

void GEdgeSelection::Update()
{
	if (m_ps == 0) return;
	GModel& model = *m_ps;

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);
		GMesh* pm = po->GetRenderMesh();

		int NE = pm->Edges();
		for (int i=0; i<NE; ++i)
		{
			GMesh::EDGE& e = pm->Edge(i);
			GEdge* pe = po->Edge(e.pid);
			assert(pe);
			if (pe && pe->IsSelected())
			{
				for (int j=0; j<2; ++j)
				{
					vec3d r = po->GetTransform().LocalToGlobal(pm->Node(e.n[j]).r);

					if (r.x < m_box.x0) m_box.x0 = r.x;
					if (r.y < m_box.y0) m_box.y0 = r.y;
					if (r.z < m_box.z0) m_box.z0 = r.z;

					if (r.x > m_box.x1) m_box.x1 = r.x;
					if (r.y > m_box.y1) m_box.y1 = r.y;
					if (r.z > m_box.z1) m_box.z1 = r.z;
				}

				++m;
			}
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
}


FEItemListBuilder* GEdgeSelection::CreateItemList()
{
	return new GEdgeList(m_ps, this);
}

//////////////////////////////////////////////////////////////////////
// GNodeSelection
//////////////////////////////////////////////////////////////////////

GNodeSelection::Iterator::Iterator(GNodeSelection* pg)
{
	GModel& m = *pg->GetGModel();
	m_pn = nullptr;
	m_node = -1;
	int N = m.Nodes();
	for (int i=0; i<N; ++i)
	{
		GNode* pn = m.Node(i);
		if (pn->IsSelected())
		{
			m_sel.push_back(pn);
		}
	}
	if (m_sel.empty() == false)
	{
		m_pn = m_sel[0];
		m_node = 0;
	}
}

GNodeSelection::Iterator& GNodeSelection::Iterator::operator ++()
{
	assert(m_pn);
	if (m_node < m_sel.size() - 1)
	{
		m_node++;
		m_pn = m_sel[m_node];
	}
	else
	{
		m_node = m_sel.size();
		m_pn = nullptr;
	}
	return (*this);
}

int GNodeSelection::Count()
{
	if (m_ps == 0) return 0;
	GModel& m = *m_ps;
	int n = 0;
	int N = m.Nodes();
	for (int i=0; i<N; ++i) 
	{
		GNode* pn = m.Node(i);
		assert(pn);
		if (pn->IsSelected()) ++n;
	}
	return n;
}

int GNodeSelection::Next()
{
	int n = -1;
	GModel& m = *m_ps;
	int N = m.Nodes();
	for (int i=0; i<N; ++i) if (m.Node(i)->IsSelected()) 
	{ 
		do { n = m.Node((++i)%N)->GetID(); } while (n == -1);
		break; 
	}
	return n;
}

int GNodeSelection::Prev()
{
	int n = -1;
	GModel& m = *m_ps;
	int N = m.Nodes();
	for (int i=0; i<N; ++i) if (m.Node(i)->IsSelected())
	{ 
		do {
			i = (i==0?N-1:i-1);
			n = m.Node(i)->GetID();
		}
		while (n == -1);
		break; 
	}
	return n;
}

void GNodeSelection::Invert()
{
	if (m_ps == 0) return;
	GModel& m = *m_ps;
	int N = m.Nodes();
	for (int i=0; i<N; ++i)
	{
		GNode* pn = m.Node(i);
		if (pn->IsSelected()) pn->UnSelect(); else pn->Select();
	}
}

void GNodeSelection::Translate(vec3d dr)
{
	// TODO: don't know an application for this yet
}

void GNodeSelection::Update()
{
	if (m_ps == 0) return;
	GModel& model = *m_ps;

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	for (int k=0; k<model.Objects(); ++k)
	{
		GObject* po = model.Object(k);

		int N = po->Nodes();
		vec3d r;
		for (int i=0; i<N; ++i)
		{
			GNode* pn = po->Node(i);
			if (pn && pn->IsSelected())
			{
				r = pn->Position();

				if (r.x < m_box.x0) m_box.x0 = r.x;
				if (r.y < m_box.y0) m_box.y0 = r.y;
				if (r.z < m_box.z0) m_box.z0 = r.z;

				if (r.x > m_box.x1) m_box.x1 = r.x;
				if (r.y > m_box.y1) m_box.y1 = r.y;
				if (r.z > m_box.z1) m_box.z1 = r.z;
				++m;
			}
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
}


FEItemListBuilder* GNodeSelection::CreateItemList()
{
	return new GNodeList(m_ps, this);
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

int GDiscreteSelection::Next()
{
	int n = -1;
	GModel& m = *m_ps;
	int N = m.DiscreteObjects();
	for (int i = 0; i<N; ++i) if (m.DiscreteObject(i)->IsSelected())
	{
		n = (++i) % N;
		break;
	}
	return n;
}

int GDiscreteSelection::Prev()
{
	int n = -1;
	GModel& m = *m_ps;
	int N = m.DiscreteObjects();
	for (int i = 0; i<N; ++i) if (m.DiscreteObject(i)->IsSelected())
	{
		i = (i == 0 ? N - 1 : i - 1);
		n = i;
		break;
	}
	return n;
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


FEElementSelection::Iterator::Iterator(FSMesh* pm)
{
	m_pm = pm;
	m_n = 0;
	if (m_pm == 0) return;
	do { m_pelem = m_pm->ElementPtr(m_n++); } while (m_pelem && !m_pelem->IsSelected());
}

void FEElementSelection::Iterator::operator ++()
{
	if (m_pm == 0) return;
	do { m_pelem = m_pm->ElementPtr(m_n++); } while (m_pelem && !m_pelem->IsSelected());
}

int FEElementSelection::Count()
{
	return (int)m_item.size();
}

void FEElementSelection::Invert()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Elements(); 
	for (int i = 0; i < N; ++i)
	{
		FEElement_* pe = m_pMesh->ElementPtr(i);

		if (pe->IsVisible())
		{
			if (pe->IsSelected()) pe->Unselect();
			else pe->Select();
		}
	}
}

void FEElementSelection::Update()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Elements();
	FSNode* pn = m_pMesh->NodePtr();

	GObject* po = m_pMesh->GetGObject();

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	m_item.clear();
	m_item.reserve(N);

	int* n, l, i, j;
	vec3d r;
	for (i=0; i<N; ++i)
	{
		FEElement_* pe = m_pMesh->ElementPtr(i);
		if (pe->IsSelected())
		{
			FEElement_& e = *pe;
			n = e.m_node;
			l = e.Nodes();
			m_item.push_back(i);

			for (j=0; j<l; ++j)
			{
				r = po->GetTransform().LocalToGlobal(pn[n[j]].r);

				if (r.x < m_box.x0) m_box.x0 = r.x;
				if (r.y < m_box.y0) m_box.y0 = r.y;
				if (r.z < m_box.z0) m_box.z0 = r.z;

				if (r.x > m_box.x1) m_box.x1 = r.x;
				if (r.y > m_box.y1) m_box.y1 = r.y;
				if (r.z > m_box.z1) m_box.z1 = r.z;
			}

			++m;
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
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
	vector<int> elset;
	for (int i=0; i<pm->Elements(); ++i) 
		if (pm->Element(i).IsSelected()) elset.push_back(i);
	return new FSElemSet(po, elset);
}

FEElement_* FEElementSelection::Element(int i)
{
	if ((i<0) || (i>=(int) m_item.size())) return 0;
	return m_pMesh->ElementPtr(m_item[i]);
}

int FEElementSelection::ElementID(int i)
{
    if ((i<0) || (i>=(int) m_item.size())) return -1;
    return m_item[i];
}

//////////////////////////////////////////////////////////////////////
// FEFaceSelection
//////////////////////////////////////////////////////////////////////

int FEFaceSelection::Count()
{
	if (m_pMesh == 0) return 0;
	int N = 0;
	FSFace* pf = m_pMesh->FacePtr();
	for (int i=0; i<m_pMesh->Faces(); ++i, ++pf)
		if (pf->IsSelected()) ++N;

	return N;
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
}

void FEFaceSelection::Update()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Faces();
	FSNode* pn = m_pMesh->NodePtr();
	FSFace* pf = m_pMesh->FacePtr();
	GObject* po = m_pMesh->GetGObject();

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	int* n, i, j;
	vec3d r;
	for (i=0; i<N; ++i, ++pf)
	{
		if (pf->IsSelected())
		{
			FSFace& f = *pf;
			n = f.n;
			for (j=0; j<pf->Nodes(); ++j)
			{
				r = po->GetTransform().LocalToGlobal(pn[n[j]].r);

				if (r.x < m_box.x0) m_box.x0 = r.x;
				if (r.y < m_box.y0) m_box.y0 = r.y;
				if (r.z < m_box.z0) m_box.z0 = r.z;

				if (r.x > m_box.x1) m_box.x1 = r.x;
				if (r.y > m_box.y1) m_box.y1 = r.y;
				if (r.z > m_box.z1) m_box.z1 = r.z;
			}

			++m;
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
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
	Iterator pf(m_pMesh);
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
	Iterator pf(m_pMesh);
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
	Iterator pf(m_pMesh);
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

FEFaceSelection::Iterator::Iterator(FSMeshBase* pm)
{ 
	m_pm = pm;
	m_n = 0;
	if (m_pm == 0) return;
	do { m_pface = m_pm->FacePtr(m_n++); } while (m_pface && !m_pface->IsSelected());
}

void FEFaceSelection::Iterator::operator ++()
{
	if (m_pm == 0) return;
	do { m_pface = m_pm->FacePtr(m_n++); } while (m_pface && !m_pface->IsSelected());
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
	return Iterator(m_pMesh);
}

//////////////////////////////////////////////////////////////////////
// FEEdgeSelection
//////////////////////////////////////////////////////////////////////

int FEEdgeSelection::Count()
{
	if (m_pMesh == 0) return 0;
	int N = 0;
	FSEdge* pe = m_pMesh->EdgePtr();
	for (int i=0; i<m_pMesh->Edges(); ++i, ++pe)
		if (pe->IsSelected()) ++N;

	return N;
}

void FEEdgeSelection::Invert()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Edges(); 
	FSEdge* pe = m_pMesh->EdgePtr();
	for (int i=0; i<N; ++i, ++pe)
		if (pe->IsVisible())
		{
			if (pe->IsSelected()) pe->Unselect(); 
			else pe->Select();
		}
}

void FEEdgeSelection::Update()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Edges();
	FSNode* pn = m_pMesh->NodePtr();
	FSEdge* pe = m_pMesh->EdgePtr();

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	int* n, i, j;
	vec3d r;
	for (i=0; i<N; ++i, ++pe)
	{
		if (pe->IsSelected())
		{
			FSEdge& e = *pe;
			n = e.n;
			for (j=0; j<e.Nodes(); ++j)
			{
				r = m_pMesh->NodePosition(n[j]);

				if (r.x < m_box.x0) m_box.x0 = r.x;
				if (r.y < m_box.y0) m_box.y0 = r.y;
				if (r.z < m_box.z0) m_box.z0 = r.z;

				if (r.x > m_box.x1) m_box.x1 = r.x;
				if (r.y > m_box.y1) m_box.y1 = r.y;
				if (r.z > m_box.z1) m_box.z1 = r.z;
			}

			++m;
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
}

void FEEdgeSelection::Translate(vec3d dr)
{
	if (m_pMesh == 0) return;
	int i, j;
	GObject* po = m_pMesh->GetGObject();

	int N = m_pMesh->Nodes();
	// clear the tags
	FSNode* pn = m_pMesh->NodePtr();

	for (i=0; i<N; ++i)  pn[i].m_ntag = 0;

	// loop over all selected edges
	Iterator pe(m_pMesh);
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
	Iterator pe(m_pMesh);
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
	Iterator pe(m_pMesh);
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

FEEdgeSelection::Iterator::Iterator(FSLineMesh* pm)
{ 
	m_pm = pm;
	m_n = 0;
	if (m_pm == 0) return;
	do { m_pedge = m_pm->EdgePtr(m_n++); } while (m_pedge && !m_pedge->IsSelected());
}

void FEEdgeSelection::Iterator::operator ++()
{
	if (m_pm == 0) return;
	do { m_pedge = m_pm->EdgePtr(m_n++); } while (m_pedge && !m_pedge->IsSelected());
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

int FENodeSelection::Count()
{
	if (m_pMesh == 0) return 0;
	int N = 0;
	FSNode* pn = m_pMesh->NodePtr();
	for (int i=0; i<m_pMesh->Nodes(); ++i, ++pn)
		if (pn->IsSelected()) ++N;

	return N;
}

FENodeSelection::Iterator FENodeSelection::First()
{
	return Iterator(m_pMesh);
}

void FENodeSelection::Invert()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Nodes(); 
	FSNode* pn = m_pMesh->NodePtr();
	for (int i=0; i<N; ++i, ++pn)
		if (pn->IsVisible())
		{
			if (pn->IsSelected()) pn->Unselect(); 
			else pn->Select();
		}
}

void FENodeSelection::Update()
{
	if (m_pMesh == 0) return;
	int N = m_pMesh->Nodes();
	FSNode* pn = m_pMesh->NodePtr();
	GObject* po = m_pMesh->GetGObject();

	int m = 0;

	const double LARGE = 1e20;

	m_box = BOX(LARGE, LARGE, LARGE, -LARGE, -LARGE, -LARGE);

	vec3d r;
	for (int i=0; i<N; ++i, ++pn)
	{
		if (pn->IsSelected())
		{
			r = po->GetTransform().LocalToGlobal(pn->r);

			if (r.x < m_box.x0) m_box.x0 = r.x;
			if (r.y < m_box.y0) m_box.y0 = r.y;
			if (r.z < m_box.z0) m_box.z0 = r.z;

			if (r.x > m_box.x1) m_box.x1 = r.x;
			if (r.y > m_box.y1) m_box.y1 = r.y;
			if (r.z > m_box.z1) m_box.z1 = r.z;
			++m;
		}
	}

	if (m==0) m_box = BOX(0,0,0,0,0,0);
}

void FENodeSelection::Translate(vec3d dr)
{
	if (m_pMesh == 0) return;
	GObject* po = m_pMesh->GetGObject();

	vec3d drf = (vec3d) dr;
	// loop over all selected nodes
	Iterator pn(m_pMesh);
	vec3d r;
	while (pn)
	{
		r = po->GetTransform().LocalToGlobal(pn->r);
		r += dr;
		pn->r = po->GetTransform().GlobalToLocal(r);
		++pn;
	}

	m_pMesh->UpdateMesh();

	// update the box
	m_box.x0 += drf.x;
	m_box.y0 += drf.y;
	m_box.z0 += drf.z;

	m_box.x1 += drf.x;
	m_box.y1 += drf.y;
	m_box.z1 += drf.z;

	// update the geometry nodes
	po->UpdateGNodes();
}

void FENodeSelection::Rotate(quatd q, vec3d rc)
{
	if (m_pMesh == 0) return;
	GObject* po = m_pMesh->GetGObject();

	vec3d dr;

	q.MakeUnit();

	Iterator pn(m_pMesh);
	vec3d r;
	while (pn)
	{
		r = po->GetTransform().LocalToGlobal(pn->r);
		dr = r - rc;
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

	Iterator pn(m_pMesh);
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

FENodeSelection::Iterator::Iterator(FSLineMesh* pm)
{ 
	m_pm = pm;
	m_n = 0;
	if (m_pm == 0) return;
	do { m_pnode = m_pm->NodePtr(m_n++); } while (m_pnode && !m_pnode->IsSelected());
}

void FENodeSelection::Iterator::operator ++()
{
	if (m_pm == 0) return;
	do { m_pnode = m_pm->NodePtr(m_n++); } while (m_pnode && !m_pnode->IsSelected());
}

FEItemListBuilder* FENodeSelection::CreateItemList()
{
	FSMesh* pm = dynamic_cast<FSMesh*>(GetMesh());
	if (pm == nullptr) return nullptr;

	GObject* po = pm->GetGObject();
	vector<int> ns;
	for (int i=0; i<pm->Nodes(); ++i)
		if (pm->Node(i).IsSelected()) ns.push_back(i);
	return new FSNodeSet(po, ns);
}
