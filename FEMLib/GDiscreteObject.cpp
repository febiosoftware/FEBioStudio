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
#include "GDiscreteObject.h"
#include <FSCore/Archive.h>
#include <GeomLib/GModel.h>
#include <sstream>

GDiscreteObject::GDiscreteObject(GModel* gm, int ntype)
{
	m_gm = gm;
	m_ntype = ntype;
	m_state = (GEO_VISIBLE | GEO_ACTIVE);
	m_col = GLColor(0, 255, 0);
}

GDiscreteObject::~GDiscreteObject(void)
{
}

GLColor GDiscreteObject::GetColor() const { return m_col; }
void GDiscreteObject::SetColor(const GLColor& c) { m_col = c; }

const GModel* GDiscreteObject::GetModel() const { return m_gm; }
GModel* GDiscreteObject::GetModel() { return m_gm; }

//-----------------------------------------------------------------------------

GLinearSpring::GLinearSpring(GModel* gm) : GDiscreteObject(gm, FE_DISCRETE_SPRING)
{
	m_node[0] = m_node[1] = -1;
	AddDoubleParam(0, "E", "spring constant");
}

GLinearSpring::GLinearSpring(GModel* gm, int n1, int n2) : GDiscreteObject(gm, FE_DISCRETE_SPRING)
{
	m_node[0] = n1;
	m_node[1] = n2;
	AddDoubleParam(0, "E", "spring constant");
}

void GLinearSpring::Save(OArchive& ar)
{
	double E = GetFloatValue(MP_E);
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(1, m_node[0]);
	ar.WriteChunk(2, m_node[1]);
	ar.WriteChunk(3, E);
}

void GLinearSpring::Load(IArchive& ar)
{
	TRACE("GLinearSpring::Load");

	char sz[256] = {0};
	double E;

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0: ar.read(sz); SetName(sz); break;
		case 1: ar.read(m_node[0]); break;
		case 2: ar.read(m_node[1]); break;
		case 3: ar.read(E); SetFloatValue(MP_E, E); break;
		}

		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------

GGeneralSpring::GGeneralSpring(GModel* gm) : GDiscreteObject(gm, FE_GENERAL_SPRING)
{
	m_node[0] = m_node[1] = -1;
	AddDoubleParam(1, "force", "spring force");

	// create an initial linear ramp
//	LOADPOINT p0(0,0), p1(1,1);
//	GetParamLC(MP_F)->Add(p0);
//	GetParamLC(MP_F)->Add(p1);
}

GGeneralSpring::GGeneralSpring(GModel* gm, int n1, int n2) : GDiscreteObject(gm, FE_GENERAL_SPRING)
{
	m_node[0] = n1;
	m_node[1] = n2;
	AddDoubleParam(1, "force", "spring force");

	// create an initial linear ramp
//	LOADPOINT p0(0,0), p1(1,1);
//	GetParamLC(MP_F)->Add(p0);
//	GetParamLC(MP_F)->Add(p1);
}

void GGeneralSpring::Save(OArchive& ar)
{
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(1, m_node[0]);
	ar.WriteChunk(2, m_node[1]);
	ar.BeginChunk(3);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void GGeneralSpring::Load(IArchive& ar)
{
	TRACE("GGeneralSpring::Load");

	char sz[256] = {0};

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0: ar.read(sz); SetName(sz); break;
		case 1: ar.read(m_node[0]); break;
		case 2: ar.read(m_node[1]); break;
		case 3: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
int GDiscreteElement::m_ncount = 1;

GDiscreteElement::GDiscreteElement(GModel* gm) : GDiscreteObject(gm, FE_DISCRETE_ELEMENT)
{ 
	m_node[0] = m_node[1] = -1; m_state = 0; 
	m_nid = m_ncount++;
}

GDiscreteElement::GDiscreteElement(GModel* gm, int n0, int n1) : GDiscreteObject(gm, FE_DISCRETE_ELEMENT)
{ 
	m_node[0] = n0; 
	m_node[1] = n1; 
	m_state = 0; 
	m_nid = m_ncount++;
}

GDiscreteElement::GDiscreteElement(const GDiscreteElement& el) : GDiscreteObject(el.m_gm, FE_DISCRETE_ELEMENT)
{
	m_node[0] = el.m_node[0]; 
	m_node[1] = el.m_node[1]; 
	m_state = el.m_state; 
	m_nid = el.m_nid;
}

void GDiscreteElement::operator = (const GDiscreteElement& el)
{
	m_node[0] = el.m_node[0]; 
	m_node[1] = el.m_node[1]; 
	m_state = el.m_state; 
	m_nid = el.m_nid;
}

void GDiscreteElement::SetNodes(int n0, int n1)
{
	m_node[0] = n0;
	m_node[1] = n1;
}

//=================================================================================================

GDiscreteElementSet::GDiscreteElementSet(GModel* gm, int ntype) : GDiscreteObject(gm, ntype)
{
}

//-----------------------------------------------------------------------------
GDiscreteElementSet::~GDiscreteElementSet()
{
	Clear();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::Clear()
{
	for (size_t i = 0; i<m_elem.size(); ++i) delete m_elem[i];
	m_elem.clear();
}

//-----------------------------------------------------------------------------
int GDiscreteElementSet::size() const 
{ 
	return (int)m_elem.size(); 
}

//-----------------------------------------------------------------------------
GDiscreteElement& GDiscreteElementSet::element(int i)
{ 
	return *m_elem[i]; 
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::AddElement(int n0, int n1)
{
	// create new discrete element
	GDiscreteElement* el = new GDiscreteElement(GetModel(), n0, n1);

	// set default name
	char szbuf[32] = {0};
	sprintf(szbuf, "spring%d", (int)m_elem.size()+1);
	el->SetName(szbuf);

	// add it to the pile
	m_elem.push_back(el);

	GModel* gm = GetModel();
	GNode* pn0 = gm->FindNode(n0); assert(pn0);
	GNode* pn1 = gm->FindNode(n1); assert(pn1);
	if (pn0) pn0->MakeRequired();
	if (pn1) pn1->MakeRequired();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::AddElement(GNode* node0, GNode* node1)
{
	assert(node0);
	assert(node1);
	if ((node0 == nullptr) || (node1 == nullptr)) return;

	int n0 = node0->GetID();
	int n1 = node1->GetID();

	// create new discrete element
	GDiscreteElement* el = new GDiscreteElement(GetModel(), n0, n1);

	// set default name
	char szbuf[32] = { 0 };
	sprintf(szbuf, "spring%d", (int)m_elem.size() + 1);
	el->SetName(szbuf);

	// add it to the pile
	m_elem.push_back(el);

	if (node0) node0->MakeRequired();
	if (node1) node1->MakeRequired();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::AddElement(const GDiscreteElement& el)
{
	GDiscreteElement* newElem = new GDiscreteElement(el);
	for (int i=0; i<size(); ++i) 
		if (newElem->GetID() < m_elem[i]->GetID())
		{
			m_elem.insert(m_elem.begin()+i, newElem);
			return;
		}
	m_elem.push_back(newElem);
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::UnSelect()
{
	for (int i=0; i<(int) m_elem.size(); ++i) m_elem[i]->UnSelect();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::Select()
{
	for (int i=0; i<(int) m_elem.size(); ++i) m_elem[i]->Select();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::SelectComponent(int n)
{
	m_elem[n]->Select();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::UnselectComponent(int n)
{
	m_elem[n]->UnSelect();
}

//-----------------------------------------------------------------------------
int GDiscreteElementSet::FindElement(const GDiscreteElement& el) const
{
	for (int i=0; i<(int)m_elem.size(); ++i)
	{
		const GDiscreteElement& eli = *m_elem[i];
		if ((eli.Node(0) == el.Node(0))&&
			(eli.Node(1) == el.Node(1))) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
int GDiscreteElementSet::FindElement(int id) const
{
	for (int i=0; i<(int)m_elem.size(); ++i)
	{
		const GDiscreteElement& eli = *m_elem[i];
		if (eli.GetID() == id) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::RemoveElement(int index)
{
	m_elem.erase(m_elem.begin() + index);
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::Save(OArchive& ar)
{
	int N = m_elem.size();
	for (int i=0; i<N; ++i)
	{
		ar.BeginChunk(0);
		{
			GDiscreteElement& ei = *m_elem[i];
			int n0 = ei.Node(0);
			int n1 = ei.Node(1);
			ar.WriteChunk(0, n0);
			ar.WriteChunk(1, n1);
		}
		ar.EndChunk();
	}
}

void GDiscreteElementSet::Load(IArchive& ar)
{
	TRACE("GSpringSet::Load");
	
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == 0)
		{
			int n0, n1;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				if      (nid == 0) ar.read(n0);
				else if (nid == 1) ar.read(n1);
				ar.CloseChunk();
			}
			AddElement(n0, n1);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
GDiscreteSpringSet::GDiscreteSpringSet(GModel* gm) : GDiscreteElementSet(gm, FE_DISCRETE_SPRING_SET)
{
	m_mat = nullptr;
}

//-----------------------------------------------------------------------------
void GDiscreteSpringSet::CopyDiscreteElementSet(GDiscreteElementSet* ds)
{
	m_elem.clear();
	int N = ds->size();
	m_elem.reserve(N);
	for (int i = 0; i < N; ++i)
	{
		GDiscreteElement& de = ds->element(i);
		GDiscreteElement* pe = new GDiscreteElement(de);
		std::string name = de.GetName();
		if (name.empty())
		{
			std::stringstream ss; 
			ss << "spring" << i + 1;
			name = ss.str();
		}
		pe->SetName(de.GetName());
		m_elem.push_back(pe);
	}
}

//-----------------------------------------------------------------------------
void GDiscreteSpringSet::SetMaterial(FSDiscreteMaterial* mat)
{
	m_mat = mat;
}

//-----------------------------------------------------------------------------
FSDiscreteMaterial* GDiscreteSpringSet::GetMaterial()
{
	return m_mat;
}

//-----------------------------------------------------------------------------
void GDiscreteSpringSet::Save(OArchive& ar)
{
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	if (m_elem.size() > 0)
	{
		ar.BeginChunk(1);
		{
			GDiscreteElementSet::Save(ar);
		}
		ar.EndChunk();
	}
	ar.BeginChunk(2);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
	ar.WriteChunk(3, GetColor());

	if (m_mat)
	{
		ar.BeginChunk(4);
		{
			ar.BeginChunk(m_mat->Type());
			{
				m_mat->Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void GDiscreteSpringSet::Load(IArchive& ar)
{
	TRACE("GDiscreteSpringSet::Load");

	if (m_mat) delete m_mat;
	m_mat = nullptr;

	FSModel* fem = GetModel()->GetFSModel();

	string s;
	GLColor col = GetColor();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case 0: ar.read(s); SetName(s); break;
		case 1: GDiscreteElementSet::Load(ar); break;
		case 2: ParamContainer::Load(ar); break;
		case 3: ar.read(col); break;
		case CID_FEOBJ_INFO: ar.read(s); SetInfo(s); break;
		case 4:
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int mid = ar.GetChunkID();
					switch (mid)
					{
					case FE_DISCRETE_LINEAR_SPRING   : m_mat = new FSLinearSpringMaterial(fem); break;
					case FE_DISCRETE_NONLINEAR_SPRING: m_mat = new FSNonLinearSpringMaterial(fem); break;
					case FE_DISCRETE_HILL            : m_mat = new FSHillContractileMaterial(fem); break;
					case FE_DISCRETE_FEBIO_MATERIAL  : m_mat = new FEBioDiscreteMaterial(fem); break;
					default:
						assert(false);
						throw ReadError("Unknown discrete material");
					}
					m_mat->Load(ar);

					ar.CloseChunk();
				}
			}
		}
		ar.CloseChunk();
	}

	SetColor(col);
}

//=============================================================================
GLinearSpringSet::GLinearSpringSet(GModel* gm) : GDiscreteElementSet(gm, FE_LINEAR_SPRING_SET)
{
	AddDoubleParam(1, "E", "spring constant");
}

//-----------------------------------------------------------------------------
void GLinearSpringSet::Save(OArchive& ar)
{
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	if (m_elem.size() > 0)
	{
		ar.BeginChunk(1);
		{
			GDiscreteElementSet::Save(ar);
		}
		ar.EndChunk();
	}
	ar.BeginChunk(2);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
	ar.WriteChunk(3, GetColor());
}

//-----------------------------------------------------------------------------
void GLinearSpringSet::Load(IArchive& ar)
{
	TRACE("GDiscreteSpringSet::Load");

	string s;
	GLColor col = GetColor();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case 0: ar.read(s); SetName(s); break;
		case 1: GDiscreteElementSet::Load(ar); break;
		case 2: ParamContainer::Load(ar); break;
		case 3: ar.read(col); break;
		case CID_FEOBJ_INFO: ar.read(s); SetInfo(s); break;
		}
		ar.CloseChunk();
	}

	SetColor(col);
}

//-----------------------------------------------------------------------------
GNonlinearSpringSet::GNonlinearSpringSet(GModel* gm) : GDiscreteElementSet(gm, FE_NONLINEAR_SPRING_SET)
{
	AddDoubleParam(1, "force", "spring force");

	// create an initial linear ramp
//	LOADPOINT p0(0,0), p1(1,1);
//	GetParamLC(MP_F)->Clear();
//	GetParamLC(MP_F)->Add(p0);
//	GetParamLC(MP_F)->Add(p1);
}


//-----------------------------------------------------------------------------
void GNonlinearSpringSet::Save(OArchive& ar)
{
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	if (m_elem.size() > 0)
	{
		ar.BeginChunk(1);
		{
			GDiscreteElementSet::Save(ar);
		}
		ar.EndChunk();
	}
	ar.BeginChunk(2);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
	ar.WriteChunk(3, GetColor());
}

//-----------------------------------------------------------------------------
void GNonlinearSpringSet::Load(IArchive& ar)
{
	TRACE("GDiscreteSpringSet::Load");

	string s;
	GLColor col = GetColor();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case 0: ar.read(s); SetName(s); break;
		case 1: GDiscreteElementSet::Load(ar); break;
		case 2: ParamContainer::Load(ar); break;
		case 3: ar.read(col); break;
		case CID_FEOBJ_INFO: ar.read(s); SetInfo(s); break;
		}
		ar.CloseChunk();
	}

	SetColor(col);
}

//-----------------------------------------------------------------------------
GDeformableSpring::GDeformableSpring(GModel* gm) : GDiscreteObject(gm, FE_DEFORMABLE_SPRING)
{
	AddDoubleParam(1, "E", "spring constant");
	AddIntParam(1, "divs", "Divisions");

	m_node[0] = m_node[1] = -1;
}

//-----------------------------------------------------------------------------
GDeformableSpring::GDeformableSpring(GModel* gm, int n0, int n1) : GDiscreteObject(gm, FE_DEFORMABLE_SPRING)
{
	AddDoubleParam(1, "E", "spring constant");
	AddIntParam(1, "divs", "Divisions")->SetState(Param_EDITABLE | Param_PERSISTENT);

	m_node[0] = n0;
	m_node[1] = n1;
}
