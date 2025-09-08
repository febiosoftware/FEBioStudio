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
#include <algorithm>
#include <sstream>

// comparison of two strings, ignoring case
static bool iequals(const std::string& a, const std::string& b) {
	return a.size() == b.size() &&
		std::equal(a.begin(), a.end(), b.begin(),
			[](unsigned char c1, unsigned char c2) {
				return std::tolower(c1) == std::tolower(c2);
			});
}

AbaqusModel::ASSEMBLY::ASSEMBLY()
{
	m_currentInstance = nullptr;
}

AbaqusModel::ASSEMBLY::~ASSEMBLY()
{
	list<INSTANCE*>::iterator ii;
	for (ii = m_Instance.begin(); ii != m_Instance.end(); ++ii) delete (*ii);
	m_Instance.clear();
	m_currentInstance = nullptr;
}

// add an instance
AbaqusModel::INSTANCE* AbaqusModel::ASSEMBLY::AddInstance()
{
	// create a new instance
	m_currentInstance = new AbaqusModel::INSTANCE;
	m_Instance.push_back(m_currentInstance);
	return m_currentInstance;
}

void AbaqusModel::ASSEMBLY::ClearCurrentInstance()
{
	m_currentInstance = nullptr;
}

//=============================================================================
AbaqusModel::AbaqusModel()
{
	m_Assembly = nullptr;
}

AbaqusModel::~AbaqusModel()
{
	// delete the assembly
	delete m_Assembly;
	m_Assembly = nullptr;

	// delete all parts
	list<PART*>::iterator pi;
	for (pi = m_Part.begin(); pi != m_Part.end(); ++pi) delete (*pi);
	m_Part.clear();
}

// add an assembly
AbaqusModel::ASSEMBLY* AbaqusModel::CreateAssembly()
{
	assert(m_Assembly == nullptr);
	if (m_Assembly == nullptr) m_Assembly = new ASSEMBLY;
	return m_Assembly;
}

AbaqusModel::INSTANCE* AbaqusModel::FindInstance(const string& name)
{
	if (m_Assembly == nullptr) return nullptr;
	list<INSTANCE*>::iterator pg;
	for (pg = m_Assembly->m_Instance.begin(); pg != m_Assembly->m_Instance.end(); ++pg)
	{
		INSTANCE* pi = *pg;
		if (iequals(pi->GetName(), name)) return pi;
	}
	return nullptr;
}

AbaqusModel::PART* AbaqusModel::CreatePart(const string& name)
{
	PART* pg = new PART;
	if (name.empty())
	{
		std::stringstream ss;
		ss << "Part" << (m_Part.size() + 1);
		pg->m_name = ss.str();
	}
	else pg->m_name = name;
	m_Part.push_back(pg);
	return pg;
}

AbaqusModel::PART* AbaqusModel::FindPart(const string& name)
{
	list<PART*>::iterator pg;
	for (pg = m_Part.begin(); pg != m_Part.end(); ++pg)
	{
		PART* part = *pg;
		if (iequals(part->m_name, name)) return part;
	}
	return 0;
}

// find a node set based on a name
AbaqusModel::NODE_SET* AbaqusModel::FindNodeSet(const string& name)
{
	// see if there is a dot
	size_t npos = name.find('.');
	if (npos != string::npos)
	{
		string partName = name.substr(0, npos);
		string setName = name.substr(npos + 1);
		INSTANCE* pi = FindInstance(partName);
		if (pi)
		{
			AbaqusModel::PART* pg = pi->GetPart();
			if (pg)
			{
				NODE_SET* pns = pg->FindNodeSet(setName);
				return pns;
			}
		}
	}
	else
	{
		NODE_SET* ns = m_globalPart.FindNodeSet(name);
		if (ns) return ns;

		list<AbaqusModel::PART*>::iterator it;
		for (it = m_Part.begin(); it != m_Part.end(); ++it)
		{
			AbaqusModel::PART& part = *(*it);
			NODE_SET* ns = part.FindNodeSet(name);
			if (ns != nullptr) return ns;
		}
	}

	return nullptr;
}

// find a part with a particular element set
AbaqusModel::ELEMENT_SET* AbaqusModel::FindElementSet(const string& name)
{
	size_t n = name.find('.');
	if (n != string::npos)
	{
		string partName = name.substr(0, n);
		AbaqusModel::INSTANCE* inst = FindInstance(partName);
		if (inst == nullptr) return nullptr;

		AbaqusModel::PART* pg = inst->GetPart();
		string setName = name.substr(n + 1);
		ELEMENT_SET* ps = pg->FindElementSet(setName);
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
			ELEMENT_SET* ps = part.FindElementSet(name);
			if (ps != nullptr)
			{
				return ps;
			}
		}
	}

	return 0;
}

AbaqusModel::SURFACE* AbaqusModel::FindSurface(const string& name)
{
	size_t n = name.find('.');
	if (n != string::npos)
	{
		string partName = name.substr(0, n);
		AbaqusModel::INSTANCE* inst = FindInstance(partName);
		if (inst == nullptr) return nullptr;

		AbaqusModel::PART* pg = inst->GetPart();
		string setName = name.substr(n + 1);
		SURFACE* ps = pg->FindSurface(setName);
		return ps;
	}
	else
	{
		SURFACE* ps = m_globalPart.FindSurface(name);
		if (ps) return ps;

		list<AbaqusModel::PART*>::iterator it;
		for (it = m_Part.begin(); it != m_Part.end(); ++it)
		{
			AbaqusModel::PART& part = *(*it);
			SURFACE* ps = part.FindSurface(name);
			if (ps != nullptr)
			{
				return ps;
			}
		}
	}
	return nullptr;
}

AbaqusModel::SpringSet* AbaqusModel::FindSpringSet(const string& name)
{
	list<AbaqusModel::PART*>::iterator it;
	for (it = m_Part.begin(); it != m_Part.end(); ++it)
	{
		AbaqusModel::PART& part = *(*it);
		AbaqusModel::SpringSet* spring = part.FindSpringSet(name);
		if (spring) return spring;
	}
	return nullptr;
}

// Add a material
AbaqusModel::MATERIAL* AbaqusModel::AddMaterial(const string& name)
{
	MATERIAL newmat;
	newmat.name = name;
	m_Mat.push_back(newmat);
	AbaqusModel::MATERIAL& pm = m_Mat.back();
	return &pm;
}

// add a pressure load
AbaqusModel::DSLOAD* AbaqusModel::STEP::AddPressureLoad(AbaqusModel::DSLOAD& p)
{
	m_SLoads.push_back(p);
	return &(m_SLoads.back());
}

// add a boundary condition
AbaqusModel::BOUNDARY* AbaqusModel::STEP::AddBoundaryCondition(AbaqusModel::BOUNDARY& p)
{
	m_Boundary.push_back(p);
	return &(m_Boundary.back());
}

// add a step
AbaqusModel::STEP* AbaqusModel::AddStep(const string& name)
{
	STEP step;
	if (!name.empty()) step.name = name;
	else
	{
		std::stringstream ss;
		ss << "Step-" << (m_Step.size() + 1);
		step.name = ss.str();
	}

	step.dt0 = 0.1;
	step.time = 1.0;

	m_Step.push_back(step);
	return &m_Step.back();
}

//=============================================================================
AbaqusModel::PART::PART() 
{ 
	m_ioff = 0;
}

AbaqusModel::PART::~PART()
{
	m_NSet.clear();
	m_ESet.clear();
	m_Surf.clear();
	m_Node.clear();
	m_Elem.clear();
}

AbaqusModel::PART* AbaqusModel::PART::Clone()
{
	// use copy constructor to do most of the work
	AbaqusModel::PART* pg = new PART(*this);

	// update part pointers
	for (auto& s : pg->m_NSet ) s.part = pg;
	for (auto& s : pg->m_ESet ) s.part = pg;
	for (auto& s : pg->m_Surf ) s.part = pg;
	for (auto& s : pg->m_Solid) s.part = pg;
	for (auto& s : pg->m_Shell) s.part = pg;

	// clear the NLT
	pg->m_NLT.clear();

	// reset instance
	pg->m_instance = nullptr;

	return pg;
}

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

AbaqusModel::Tnode_itr AbaqusModel::PART::FindNode(int id)
{
	if (m_NLT.empty()) BuildNLT();
	assert(!m_NLT.empty());
	int m = id - m_ioff;
	if ((m < 0) || (m >= m_NLT.size())) return m_Node.end();
	return m_NLT[m];
}

void AbaqusModel::PART::AddSpring(AbaqusModel::SPRING& s)
{
	m_Spring.push_back(s);
}

void AbaqusModel::PART::AddSpringSet(AbaqusModel::SpringSet& s)
{
	m_SpringSet.push_back(s);
}

AbaqusModel::SpringSet* AbaqusModel::PART::FindSpringSet(const std::string& s)
{
	for (auto& spring : m_SpringSet)
	{
		if (iequals(s, spring.name)) return &spring;
	}
	return nullptr;
}

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

vector<AbaqusModel::ELEMENT>::iterator AbaqusModel::PART::FindElement(int id)
{
	if ((id < 0) || (id >= m_Elem.size())) return m_Elem.end();
	return m_Elem.begin() + id;
}

AbaqusModel::ELEMENT_SET* AbaqusModel::PART::FindElementSet(const string& name)
{
	for (auto& eset : m_ESet)
	{
		if (iequals(name, eset.name)) return &eset;
	}
	return nullptr;
}

vector<AbaqusModel::ELEMENT_SET*> AbaqusModel::PART::FindElementSets(const string & name)
{
	vector<AbaqusModel::ELEMENT_SET*> sets;
	for (auto& eset : m_ESet)
	{
		if (eset.name == name) sets.push_back(&eset);
	}
	return sets;
}

AbaqusModel::ELEMENT_SET* AbaqusModel::PART::AddElementSet(const string& name)
{
	ELEMENT_SET es;
	es.name = name;
	es.part = this;
	m_ESet.push_back(es);
	return &(m_ESet.back());
}

AbaqusModel::NODE_SET* AbaqusModel::PART::FindNodeSet(const string& name)
{
	for (auto& nset : m_NSet)
	{
		if (iequals(nset.name, name)) return &nset;
	}
	return nullptr;
}

AbaqusModel::NODE_SET* AbaqusModel::PART::AddNodeSet(const string& name)
{
	NODE_SET ns;
	ns.name = name;
	ns.part = this;
	m_NSet.push_back(ns);
	return &m_NSet.back();
}

AbaqusModel::SURFACE* AbaqusModel::PART::FindSurface(const string& name)
{
	for (auto& surf : m_Surf)
	{
		if (iequals(surf.name, name)) return &surf;
	}
	return nullptr;
}

// add a solid section
void AbaqusModel::PART::AddSolidSection(const string& elset, const string& mat, const string& orient)
{
	SOLID_SECTION ss;
	ss.elset = elset;
	ss.mat = mat;
	ss.orient = orient;
	ss.part = this;
	m_Solid.push_back(ss);
}

// add a shell section
AbaqusModel::SHELL_SECTION& AbaqusModel::PART::AddShellSection(const string& elset, const string& mat, const string& orient)
{
	SHELL_SECTION ss;
	ss.elset = elset;
	ss.mat = mat;
	ss.orient = orient;
	ss.part = this;
	m_Shell.push_back(ss);
	return m_Shell.back();
}

AbaqusModel::SURFACE* AbaqusModel::PART::AddSurface(const string& name)
{
	SURFACE surf;
	surf.name = name;
	surf.part = this;
	m_Surf.push_back(surf);
	return &m_Surf.back();
}

int AbaqusModel::PART::CountElements()
{
	int n = 0;
	for (int i = 0; i < m_Elem.size(); ++i) if (m_Elem[i].id != -1) n++;
	return n;
}

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

void AbaqusModel::PART::AddOrientation(const string& name, const string& dist)
{
	Orientation o = { name, dist };
	m_Orient.push_back(o);
}

AbaqusModel::Orientation* AbaqusModel::PART::FindOrientation(const string& name)
{
	for (auto it = m_Orient.begin(); it != m_Orient.end(); ++it)
	{
		if (iequals(it->name, name)) return &(*it);
	}
	return 0;
}

AbaqusModel::Distribution* AbaqusModel::PART::FindDistribution(const string& name)
{
	for (auto it = m_Distr.begin(); it != m_Distr.end(); ++it)
	{
		if (iequals(it->name, name)) return &(*it);
	}
	return 0;
}

//=============================================================================
AbaqusModel::INSTANCE::INSTANCE()
{
	m_part = nullptr;
	m_trans[0] = m_trans[1] = m_trans[2] = 0.0;

	m_rot[0] = 0; m_rot[1] = 0; m_rot[2] = 0;
	m_rot[3] = 0; m_rot[4] = 0; m_rot[5] = 0;
	m_rot[6] = 0;
}

void AbaqusModel::INSTANCE::SetPart(AbaqusModel::PART* pg) 
{ 
	assert(pg->m_instance == nullptr);
	m_part = pg; 
	pg->m_instance = this;
}

void AbaqusModel::INSTANCE::SetTranslation(double t[3])
{
	m_trans[0] = t[0];
	m_trans[1] = t[1];
	m_trans[2] = t[2];
}

void AbaqusModel::INSTANCE::GetTranslation(double t[3])
{
	t[0] = m_trans[0];
	t[1] = m_trans[1];
	t[2] = m_trans[2];
}

void AbaqusModel::INSTANCE::SetRotation(double t[7])
{
	m_rot[0] = t[0]; m_rot[1] = t[1]; m_rot[2] = t[2];
	m_rot[3] = t[3]; m_rot[4] = t[4]; m_rot[5] = t[5];
	m_rot[6] = t[6];
}

void AbaqusModel::INSTANCE::GetRotation(double t[7])
{
	t[0] = m_rot[0]; t[1] = m_rot[1]; t[2] = m_rot[2];
	t[3] = m_rot[3]; t[4] = m_rot[4]; t[5] = m_rot[5];
	t[6] = m_rot[6];
}

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

int AbaqusModel::FindAmplitude(const string& name) const
{
	for (int i = 0; i < m_Amp.size(); ++i)
	{
		if (iequals(name, m_Amp[i].m_name))
		{
			return i;
		}
	}
	return -1;
}

void AbaqusModel::STEP::AddContactPair(CONTACT_PAIR& cp)
{
	m_ContactPair.push_back(cp);
}

int AbaqusModel::STEP::ContactPairs() const
{
	return (int)m_ContactPair.size();
}

const AbaqusModel::CONTACT_PAIR& AbaqusModel::STEP::GetContactPair(int n) const
{
	return m_ContactPair[n];
}
