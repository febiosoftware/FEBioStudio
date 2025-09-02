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
using namespace std;

#ifdef LINUX // same for Linux and Mac OS X
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

//-----------------------------------------------------------------------------
AbaqusModel::ASSEMBLY::ASSEMBLY()
{
	m_currentInstance = nullptr;
}

//-----------------------------------------------------------------------------
AbaqusModel::ASSEMBLY::~ASSEMBLY()
{
	list<INSTANCE*>::iterator ii;
	for (ii = m_Instance.begin(); ii != m_Instance.end(); ++ii) delete (*ii);
	m_Instance.clear();
	m_currentInstance = nullptr;
}

//-----------------------------------------------------------------------------
// add an instance
AbaqusModel::INSTANCE* AbaqusModel::ASSEMBLY::AddInstance()
{
	// create a new instance
	m_currentInstance = new AbaqusModel::INSTANCE;
	m_Instance.push_back(m_currentInstance);
	return m_currentInstance;
}

//-----------------------------------------------------------------------------
void AbaqusModel::ASSEMBLY::ClearCurrentInstance()
{
	m_currentInstance = nullptr;
}

//=============================================================================
AbaqusModel::AbaqusModel()
{
	m_Assembly = nullptr;
	m_currentPart = nullptr;
	m_currentStep = nullptr;
	m_currentAssembly = nullptr;
	m_fem = nullptr;
}

//-----------------------------------------------------------------------------
AbaqusModel::~AbaqusModel()
{
	// delete the assembly
	delete m_Assembly;
	m_Assembly = nullptr;

	// delete all parts
	list<PART*>::iterator pi;
	for (pi = m_Part.begin(); pi != m_Part.end(); ++pi) delete (*pi);
	m_Part.clear();
	m_currentPart = nullptr;
}

//-----------------------------------------------------------------------------
// add an assembly
AbaqusModel::ASSEMBLY* AbaqusModel::CreateAssembly()
{
	assert(m_Assembly == nullptr);
	if (m_Assembly == nullptr) m_Assembly = new ASSEMBLY;
	return m_Assembly;
}

//-----------------------------------------------------------------------------
AbaqusModel::INSTANCE* AbaqusModel::FindInstance(const char* sz)
{
	if (m_Assembly == nullptr) return nullptr;
	list<INSTANCE*>::iterator pg;
	for (pg = m_Assembly->m_Instance.begin(); pg != m_Assembly->m_Instance.end(); ++pg)
	{
		INSTANCE* pi = *pg;
		if (stricmp(pi->GetName(), sz) == 0) return pi;
	}
	return nullptr;
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
		if (stricmp(part->GetName(), sz) == 0) return part;
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
		NODE_SET* ns = part.FindNodeSet(sznset);
		if (ns != nullptr) return ns;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// find a part with a particular element set
AbaqusModel::ELEMENT_SET* AbaqusModel::FindElementSet(const char* szelemset)
{
	char szbuf[256] = { 0 };
	const char* ch = strchr(szelemset, '.');
	if (ch)
	{
		strncpy(szbuf, szelemset, (int)(ch - szelemset));
		AbaqusModel::INSTANCE* inst = FindInstance(szbuf);
		if (inst == nullptr) return nullptr;

		AbaqusModel::PART* pg = inst->GetPart();
		ELEMENT_SET* ps = pg->FindElementSet(ch+1);
		if (ps != nullptr)
		{
			return ps;
		}
	}
	else
	{
		list<AbaqusModel::PART*>::iterator it;
		for (it = m_Part.begin(); it != m_Part.end(); ++it)
		{
			AbaqusModel::PART& part = *(*it);
			ELEMENT_SET* ps = part.FindElementSet(szelemset);
			if (ps != nullptr)
			{
				return ps;
			}
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
		SURFACE* ps = part.FindSurface(szname);
		if (ps != nullptr)
		{
			return ps;
		}
	}
	return 0;
}

AbaqusModel::SpringSet* AbaqusModel::FindSpringSet(const char* szname)
{
	list<AbaqusModel::PART*>::iterator it;
	for (it = m_Part.begin(); it != m_Part.end(); ++it)
	{
		AbaqusModel::PART& part = *(*it);
		auto it = part.m_SpringSet.find(szname);
		if (it != part.m_SpringSet.end())
		{
			return &(it->second);
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
AbaqusModel::PART* AbaqusModel::GetActivePart()
{
	return (m_currentPart ? m_currentPart : &m_globalPart);
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
	m_currentStep = &(m_Step.back());
	return m_currentStep;
}

// set the current step
void AbaqusModel::SetCurrentStep(STEP* p)
{
	m_currentStep = p;
}

//=============================================================================

AbaqusModel::PART::PART() 
{ 
	m_po = nullptr; 
	m_szname[0] = 0;
	m_ioff = 0;
}

AbaqusModel::PART::~PART()
{
	for (auto it : m_NSet) delete it.second; m_NSet.clear();
	for (auto it : m_ESet) delete it.second; m_ESet.clear();
	for (auto it : m_Surf) delete it.second; m_Surf.clear();
}

//-----------------------------------------------------------------------------
void AbaqusModel::PART::SetName(const char* sz)
{
	strcpy(m_szname, sz);
}

//-----------------------------------------------------------------------------
AbaqusModel::Tnode_itr AbaqusModel::PART::AddNode(AbaqusModel::NODE& n)
{
	m_NLT.clear(); // invalidate the node lookup table
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
	if (m_NLT.empty()) BuildNLT();
	assert(!m_NLT.empty());
	return m_NLT[id - m_ioff];
}

list<AbaqusModel::SPRING_ELEMENT>::iterator AbaqusModel::PART::AddSpring(AbaqusModel::SPRING_ELEMENT& s)
{
	m_SpringElem.push_back(s);
	return --m_SpringElem.end();
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
	if ((id < 0) || (id >= m_Elem.size())) return m_Elem.end();
	return m_Elem.begin() + id;
}

//-----------------------------------------------------------------------------
AbaqusModel::ELEMENT_SET* AbaqusModel::PART::FindElementSet(const char* szname)
{
	auto it = m_ESet.find(szname);
	if (it != m_ESet.end()) return it->second;
	return nullptr;
}

//-----------------------------------------------------------------------------
AbaqusModel::ELEMENT_SET* AbaqusModel::PART::AddElementSet(const char* szname)
{
	ELEMENT_SET* es = new ELEMENT_SET;
	es->part = this;
	strcpy(es->szname, szname);
	m_ESet[szname] = es;
	return es;
}

//-----------------------------------------------------------------------------

AbaqusModel::NODE_SET* AbaqusModel::PART::FindNodeSet(const char* szname)
{
	for (auto& it : m_NSet)
	{
		AbaqusModel::NODE_SET* nset = it.second;
		if (nset)
		{
			if (stricmp(nset->szname, szname) == 0) return it.second;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------

AbaqusModel::NODE_SET* AbaqusModel::PART::AddNodeSet(const char* szname)
{
	NODE_SET* ns = new NODE_SET;
	ns->part = this;
	strcpy(ns->szname, szname);
	m_NSet[szname] = ns;
	return ns;
}

//-----------------------------------------------------------------------------

AbaqusModel::SURFACE* AbaqusModel::PART::FindSurface(const char* szname)
{
	auto it = m_Surf.find(szname);
	if (it != m_Surf.end()) return it->second;
	return nullptr;
}

//-----------------------------------------------------------------------------
// add a solid section
void AbaqusModel::PART::AddSolidSection(const char* szset, const char* szmat, const char* szorient)
{
	SOLID_SECTION& ss = m_Solid[szset];
	strcpy(ss.szelset, szset);
	if (szmat) strcpy(ss.szmat, szmat); else ss.szmat[0] = 0;
	if (szorient) strcpy(ss.szorient, szorient); else ss.szorient[0] = 0;
	ss.part = this;
}

//-----------------------------------------------------------------------------
// add a shell section
AbaqusModel::SHELL_SECTION& AbaqusModel::PART::AddShellSection(const char* szset, const char* szmat, const char* szorient)
{
	SHELL_SECTION& ss = m_Shell[szset];
	strcpy(ss.szelset, szset);
	if (szmat) strcpy(ss.szmat, szmat); else ss.szmat[0] = 0;
	if (szorient) strcpy(ss.szorient, szorient); else ss.szorient[0] = 0;
	ss.part = this;
	return ss;
}

//-----------------------------------------------------------------------------
AbaqusModel::SURFACE* AbaqusModel::PART::AddSurface(const char* szname)
{
	SURFACE* surf = new SURFACE;
	surf->part = this;
	strcpy(surf->szname, szname);
	m_Surf[szname] = surf;
	return surf;
}

//-----------------------------------------------------------------------------
bool AbaqusModel::PART::BuildNLT()
{
	// make sure the NLT is empty
	if (!m_NLT.empty()) m_NLT.clear();
	m_ioff = 0;

	if (m_Node.empty()) return false;

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
		if (stricmp(it->szname, szname) == 0) return &(*it);
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
		if (stricmp(it->m_szname, szname) == 0) return &(*it);
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

//-----------------------------------------------------------------------------
void AbaqusModel::AddAmplitude(const AbaqusModel::Amplitude& a)
{
	m_Amp.push_back(a);
}

int AbaqusModel::Amplitudes() const
{
	return (int)m_Amp.size();
}

const AbaqusModel::Amplitude& AbaqusModel::GetAmplitude(int n) const
{
	return m_Amp[n];
}

int AbaqusModel::FindAmplitude(const char* szname) const
{
	for (int i = 0; i < m_Amp.size(); ++i)
	{
		if (stricmp(szname, m_Amp[i].m_name.c_str()) == 0)
		{
			return i;
		}
	}
	return -1;
}

void AbaqusModel::AddContactPair(CONTACT_PAIR& cp)
{
	m_ContactPair.push_back(cp);
}

int AbaqusModel::ContactPairs() const
{
	return (int)m_ContactPair.size();
}

const AbaqusModel::CONTACT_PAIR& AbaqusModel::GetContactPair(int n) const
{
	return m_ContactPair[n];
}
