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

#include "AbaqusModel.h"

// in AbaqusImport.cpp
bool szicmp(const char* sz1, const char* sz2);

//-----------------------------------------------------------------------------
AbaqusModel::AbaqusModel()
{
	m_pPart = 0;
	m_pInst = 0;
	m_pStep = 0;
}

//-----------------------------------------------------------------------------
AbaqusModel::~AbaqusModel()
{
	// delete all instances
	list<INSTANCE*>::iterator ii;
	for (ii = m_Inst.begin(); ii != m_Inst.end(); ++ii) delete (*ii);
	m_Inst.clear();
	m_pInst = 0;

	// delete all parts
	list<PART*>::iterator pi;
	for (pi = m_Part.begin(); pi != m_Part.end(); ++pi) delete (*pi);
	m_Part.clear();
	m_pPart = 0;
}

//-----------------------------------------------------------------------------
AbaqusModel::PART* AbaqusModel::CreatePart(const char* sz)
{
	char szname[256] = { 0 };
	if (sz == 0)
	{
		sprintf(szname, "Part%02d", (int)m_Part.size() + 1);
	}
	else strcpy(szname, sz);
	PART* pg = new PART;
	pg->SetName(szname);
	m_Part.push_back(pg);
	return pg;
}

//-----------------------------------------------------------------------------
AbaqusModel::PART* AbaqusModel::FindPart(const char* sz)
{
	list<PART*>::iterator pg;
	for (pg = m_Part.begin(); pg != m_Part.end(); ++pg)
	{
		PART* part = *pg;
		if (szicmp(part->GetName(), sz)) return part;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// add an instance
AbaqusModel::INSTANCE* AbaqusModel::AddInstance()
{
	// create a new instance
	m_pInst = new AbaqusModel::INSTANCE;
	m_Inst.push_back(m_pInst);

	return m_pInst;
}

//-----------------------------------------------------------------------------
AbaqusModel::INSTANCE* AbaqusModel::FindInstance(const char* sz)
{
	list<INSTANCE*>::iterator pg;
	for (pg = m_Inst.begin(); pg != m_Inst.end(); ++pg)
	{
		INSTANCE* pi = *pg;
		if (szicmp(pi->GetName(), sz)) return pi;
	}
	return 0;
}

// find a node set based on a name
AbaqusModel::NODE_SET* AbaqusModel::FindNodeSet(const char* sznset)
{
	list<AbaqusModel::PART*>::iterator it;
	for (it = m_Part.begin(); it != m_Part.end(); ++it)
	{
		AbaqusModel::PART& part = *(*it);
		map<string, NODE_SET>::iterator ns = part.FindNodeSet(sznset);
		if (ns != part.m_NSet.end())
		{
			return &((*ns).second);
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// find a part with a particular element set
AbaqusModel::ELEMENT_SET* AbaqusModel::FindElementSet(const char* szelemset)
{
	list<AbaqusModel::PART*>::iterator it;
	for (it = m_Part.begin(); it != m_Part.end(); ++it)
	{
		AbaqusModel::PART& part = *(*it);
		list<AbaqusModel::ELEMENT_SET>::iterator ps = part.FindElementSet(szelemset);
		if (ps != part.m_ElSet.end())
		{
			return &(*ps);
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
AbaqusModel::SURFACE* AbaqusModel::FindSurface(const char* szname)
{
	list<AbaqusModel::PART*>::iterator it;
	for (it = m_Part.begin(); it != m_Part.end(); ++it)
	{
		AbaqusModel::PART& part = *(*it);
		list<SURFACE>::iterator ps = part.FindSurface(szname);
		if (ps != part.m_Surf.end())
		{
			return &(*ps);
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
AbaqusModel::PART* AbaqusModel::GetActivePart(bool bcreate)
{
	if ((m_pPart == 0) && bcreate) m_pPart = CreatePart();
	return m_pPart;
}

//-----------------------------------------------------------------------------
void AbaqusModel::ClearCurrentInstance()
{
	m_pPart = 0;
	m_pInst = 0;
}

// Add a material
AbaqusModel::MATERIAL* AbaqusModel::AddMaterial(const char* szname)
{
	MATERIAL newmat;
	m_Mat.push_back(newmat);
	AbaqusModel::MATERIAL& pm = m_Mat.back();
	return &pm;
}

// add a pressure load
AbaqusModel::DSLOAD*	AbaqusModel::AddPressureLoad(AbaqusModel::DSLOAD& p)
{
	m_SLoads.push_back(p);
	return &(m_SLoads.back());
}

// add a boundary condition
AbaqusModel::BOUNDARY* AbaqusModel::AddBoundaryCondition(AbaqusModel::BOUNDARY& p)
{
	m_Boundary.push_back(p);
	return &(m_Boundary.back());
}

// add a step
AbaqusModel::STEP* AbaqusModel::AddStep(const char* szname)
{
	STEP step;
	if (szname)
		strcpy(step.szname, szname);
	else
		sprintf(step.szname, "Step-%d", (int)m_Step.size() + 1);

	step.dt0 = 0.1;
	step.time = 1.0;

	m_Step.push_back(step);
	m_pStep = &(m_Step.back());
	return m_pStep;
}

// set the current step
void AbaqusModel::SetCurrentStep(STEP* p)
{
	m_pStep = p;
}

//=============================================================================

//-----------------------------------------------------------------------------
void AbaqusModel::PART::SetName(const char* sz)
{
	strcpy(m_szname, sz);
}

//-----------------------------------------------------------------------------
AbaqusModel::Tnode_itr AbaqusModel::PART::AddNode(AbaqusModel::NODE& n)
{
	if (m_Node.size() == 0)
	{
		m_Node.push_back(n);
		return m_Node.begin();
	}
	else
	{
		Tnode_itr p1 = m_Node.begin();
		Tnode_itr p2 = --m_Node.end();

		if (n.id < p1->id)
		{
			m_Node.insert(m_Node.begin(), n);
			return m_Node.begin();
		}
		else if (n.id > p2->id)
		{
			m_Node.push_back(n);
			return --m_Node.end();
		}
		else
		{
			if (n.id - p1->id < p2->id - n.id)
			{
				do { ++p1; } while (p1->id < n.id);
				return m_Node.insert(p1, n);
			}
			else
			{
				do { --p2; } while (p2->id > n.id); ++p2;
				return m_Node.insert(p2, n);
			}
		}
	}

	return m_Node.end();
}

//-----------------------------------------------------------------------------
AbaqusModel::Tnode_itr AbaqusModel::PART::FindNode(int id)
{
	assert(!m_NLT.empty());
	return m_NLT[id - m_ioff];
}

list<AbaqusModel::SPRING>::iterator AbaqusModel::PART::AddSpring(AbaqusModel::SPRING& s)
{
	m_Spring.push_back(s);
	return --m_Spring.end();
}

//-----------------------------------------------------------------------------

void AbaqusModel::PART::AddElement(AbaqusModel::ELEMENT& newElem)
{
	int nid = newElem.id;
	if (nid >= (int)m_Elem.size())
	{
		int oldSize = (int)m_Elem.size();
		int newSize = nid + 1000;
		m_Elem.resize(newSize);
		for (int i = oldSize; i < newSize; ++i) m_Elem[i].id = -1;
	}

	m_Elem[nid] = newElem;
}

//-----------------------------------------------------------------------------

vector<AbaqusModel::ELEMENT>::iterator AbaqusModel::PART::FindElement(int id)
{
	return m_Elem.begin() + id;
}

//-----------------------------------------------------------------------------

list<AbaqusModel::ELEMENT_SET>::iterator AbaqusModel::PART::FindElementSet(const char* szname)
{
	size_t n = m_ElSet.size();
	list<ELEMENT_SET>::iterator pe = m_ElSet.begin();
	for (size_t i = 0; i<n; ++i, ++pe) if (strcmp(pe->szname, szname) == 0) return pe;
	return m_ElSet.end();
}

//-----------------------------------------------------------------------------

list<AbaqusModel::ELEMENT_SET>::iterator AbaqusModel::PART::AddElementSet(const char* szname)
{
	ELEMENT_SET es;
	strcpy(es.szname, szname);
	m_ElSet.push_back(es);
	return --m_ElSet.end();
}

//-----------------------------------------------------------------------------

map<string, AbaqusModel::NODE_SET>::iterator AbaqusModel::PART::FindNodeSet(const char* szname)
{
	map<string, AbaqusModel::NODE_SET>::iterator it = m_NSet.find(szname);
	return it;
}

//-----------------------------------------------------------------------------

map<string, AbaqusModel::NODE_SET>::iterator AbaqusModel::PART::AddNodeSet(const char* szname)
{
	NODE_SET ns;
	strcpy(ns.szname, szname);
	m_NSet[szname] = ns;
	return m_NSet.find(szname);
}

//-----------------------------------------------------------------------------

list<AbaqusModel::SURFACE>::iterator AbaqusModel::PART::FindSurface(const char* szname)
{
	size_t n = m_Surf.size();
	list<SURFACE>::iterator ps = m_Surf.begin();
	for (size_t i = 0; i<n; ++i, ++ps) if (strcmp(ps->szname, szname) == 0) return ps;
	return m_Surf.end();
}

//-----------------------------------------------------------------------------
// add a solid section
list<AbaqusModel::SOLID_SECTION>::iterator AbaqusModel::PART::AddSolidSection(const char* szset, const char* szmat, const char* szorient)
{
	SOLID_SECTION ss;
	strcpy(ss.szelset, szset);
	if (szmat) strcpy(ss.szmat, szmat); else ss.szmat[0] = 0;
	if (szorient) strcpy(ss.szorient, szorient); else ss.szorient[0] = 0;
	ss.part = this;
	m_Solid.push_back(ss);
	return --m_Solid.end();
}

//-----------------------------------------------------------------------------

list<AbaqusModel::SURFACE>::iterator AbaqusModel::PART::AddSurface(const char* szname)
{
	SURFACE surf;
	strcpy(surf.szname, szname);
	m_Surf.push_back(surf);
	return --m_Surf.end();
}


//-----------------------------------------------------------------------------
bool AbaqusModel::PART::BuildNLT()
{
	// make sure the NLT is empty
	if (!m_NLT.empty()) m_NLT.clear();
	m_ioff = 0;

	if (m_Node.empty())
	{
		assert(false);
		return false;
	}

	// find the lowest and highest node index
	int imin, imax;
	Tnode_itr it = m_Node.begin();
	imin = imax = it->id;
	for (++it; it != m_Node.end(); ++it)
	{
		if (it->id < imin) imin = it->id;
		if (it->id > imax) imax = it->id;
	}

	// allocate NLT
	m_NLT.assign(imax - imin + 1, m_Node.end());

	// fill the NLT
	for (it = m_Node.begin(); it != m_Node.end(); ++it)
	{
		m_NLT[it->id - imin] = it;
	}
	m_ioff = imin;

	return true;
}

//-----------------------------------------------------------------------------
void AbaqusModel::PART::AddOrientation(const char* szname, const char* szdist)
{
	Orientation o;
	strcpy(o.szname, szname);
	strcpy(o.szdist, szdist);
	m_Orient.push_back(o);
}

//-----------------------------------------------------------------------------
AbaqusModel::Orientation* AbaqusModel::PART::FindOrientation(const char* szname)
{
	if (szname == 0) return 0;
	list<Orientation>::iterator it;
	for (it = m_Orient.begin(); it != m_Orient.end(); ++it)
	{
		if (strcmp(it->szname, szname) == 0) return &(*it);
	}
	return 0;
}

//-----------------------------------------------------------------------------
AbaqusModel::Distribution* AbaqusModel::PART::FindDistribution(const char* szname)
{
	if (szname == 0) return 0;
	list<Distribution>::iterator it;
	for (it = m_Distr.begin(); it != m_Distr.end(); ++it)
	{
		if (strcmp(it->m_szname, szname) == 0) return &(*it);
	}
	return 0;
}


//=============================================================================
AbaqusModel::INSTANCE::INSTANCE()
{
	m_szname[0] = 0;
	m_pPart = 0;
	m_trans[0] = m_trans[1] = m_trans[2] = 0.0;

	m_rot[0] = 0; m_rot[1] = 0; m_rot[2] = 0;
	m_rot[3] = 0; m_rot[4] = 0; m_rot[5] = 0;
	m_rot[6] = 0;
}

//-----------------------------------------------------------------------------
void AbaqusModel::INSTANCE::SetName(const char* sz)
{
	strcpy(m_szname, sz);
}

//-----------------------------------------------------------------------------
void AbaqusModel::INSTANCE::SetTranslation(double t[3])
{
	m_trans[0] = t[0];
	m_trans[1] = t[1];
	m_trans[2] = t[2];
}

//-----------------------------------------------------------------------------
void AbaqusModel::INSTANCE::GetTranslation(double t[3])
{
	t[0] = m_trans[0];
	t[1] = m_trans[1];
	t[2] = m_trans[2];
}

//-----------------------------------------------------------------------------
void AbaqusModel::INSTANCE::SetRotation(double t[7])
{
	m_rot[0] = t[0]; m_rot[1] = t[1]; m_rot[2] = t[2];
	m_rot[3] = t[3]; m_rot[4] = t[4]; m_rot[5] = t[5];
	m_rot[6] = t[6];
}

//-----------------------------------------------------------------------------
void AbaqusModel::INSTANCE::GetRotation(double t[7])
{
	t[0] = m_rot[0]; t[1] = m_rot[1]; t[2] = m_rot[2];
	t[3] = m_rot[3]; t[4] = m_rot[4]; t[5] = m_rot[5];
	t[6] = m_rot[6];
}
