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

#include "FEMaterialFactory.h"
#include "FECoreMaterial.h"
#include "FEUserMaterial.h"
using namespace std;

FEMatDescriptor::FEMatDescriptor(
	int module,
	int ntype,
	int nclass,
	const char* szname,
	unsigned int flags)
{
	m_nModule = module;
	m_nType = ntype; 
	m_nClass = nclass;
	m_szname = szname;
	m_flags = flags;
}

FEMaterialFactory* FEMaterialFactory::m_pFac = 0;

FEMaterialFactory::FEMaterialFactory(void)
{
}

FEMaterialFactory::~FEMaterialFactory(void)
{
	list<FEMatDescriptor*>::iterator pd = m_Desc.begin();
	while (pd != m_Desc.end()) { delete *pd; ++pd; }
}

void FEMaterialFactory::RegisterMaterial(FEMatDescriptor* pd)
{
	FEMaterialFactory* pFac = GetInstance();
	pFac->m_Desc.push_back(pd);
}

FEMatDescriptor* FEMaterialFactory::Find(int nid)
{
	list<FEMatDescriptor*>::iterator pd = m_Desc.begin();
	while (pd != m_Desc.end())
	{
		if ((*pd)->GetTypeID() == nid) return (*pd);
		++pd;
	}

	return 0;
}

FEMatDescriptor* FEMaterialFactory::Find(const char* szname, int classId)
{
	list<FEMatDescriptor*>::iterator pd = m_Desc.begin();
	while (pd != m_Desc.end())
	{
		if (strcmp((*pd)->GetTypeString(), szname) == 0)
		{
			int pid = (*pd)->GetClassID();

			// if we don't care about classID, return the first match
			if (classId == -1) return (*pd);

			// There is a difference between class Id's less than 0xFFFF
			// and larger so we need to handle those differently. 
			if (classId <= 0x0000FFFF)
			{
				if ((pid&0xFFFF)&classId) return (*pd);
			}
			else
			{
				if (pid == classId) return (*pd);
			}
		}
		++pd;
	}

	return 0;
}

FEMatDescriptor* FEMaterialFactory::AtIndex(int index)
{
	if (index < 0) return 0;

	if (index >= (int) m_Desc.size()) return 0;
	FEMatDescIter it = m_Desc.begin();
	for (int i=0; i<index; ++i) ++it;

	return (*it);
}

FSMaterial* FEMaterialFactory::Create(FSModel* fem, int nid)
{
	assert(m_pFac);
	FEMatDescriptor* pd = m_pFac->Find(nid);
	return (pd?pd->Create(fem):0);
}

FSMaterial* FEMaterialFactory::Create(FSModel* fem, const char *szname, int classId)
{
	assert(m_pFac);
	FEMatDescriptor* pd = m_pFac->Find(szname, classId);
	return (pd?pd->Create(fem):0);
}

const char* FEMaterialFactory::TypeStr(int nid)
{
    FEMatDescriptor* pd = m_pFac->Find(nid);
    return pd->GetTypeString();
}

const char* FEMaterialFactory::TypeStr(const FSMaterial *pm)
{
	if (pm == 0) return 0;

	// A user material will not be registered, but instead contains its own type str
	// TODO: Should I make TypeStr a member of each material?
	if (dynamic_cast<const FSUserMaterial*>(pm)) return (dynamic_cast<const FSUserMaterial*>(pm))->GetTypeString();

	FEMaterialFactory* pmf = GetInstance();

	list<FEMatDescriptor*>::iterator pd = pmf->m_Desc.begin();
	while (pd != pmf->m_Desc.end())
	{
		if ((*pd)->GetTypeID() == pm->Type()) return (*pd)->GetTypeString();
		++pd;
	}
	return 0;
}

int FEMaterialFactory::ClassID(FSMaterial *pm)
{
	FEMaterialFactory* pmf = GetInstance();

	list<FEMatDescriptor*>::iterator pd = pmf->m_Desc.begin();
	while (pd != pmf->m_Desc.end())
	{
		if ((*pd)->GetTypeID() == pm->Type()) return (*pd)->GetClassID();
		++pd;
	}
	return 0;
}

// return a list of material types for a given material class
list<FEMatDescriptor*> FEMaterialFactory::Enumerate(int matClass)
{
	FEMaterialFactory* pmf = GetInstance();

	list<FEMatDescriptor*> out;

	list<FEMatDescriptor*>::iterator pd = pmf->m_Desc.begin();
	while (pd != pmf->m_Desc.end())
	{
		if ((*pd)->GetClassID() == matClass) out.push_back(*pd);
		++pd;
	}
	return out;
}

void FEMaterialFactory::AddCategory(const std::string& name, int module, int catID)
{
	FEMaterialFactory* pmf = GetInstance();
	pmf->m_Cat.push_back(FEMatCategory(name, module, catID));
}

int FEMaterialFactory::Categories()
{
	FEMaterialFactory* pmf = GetInstance();
	return (int)pmf->m_Cat.size();
}

FEMatCategory& FEMaterialFactory::GetCategory(int i)
{
	FEMaterialFactory* pmf = GetInstance();
	return pmf->m_Cat[i];
}
