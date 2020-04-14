#include "FEMaterialFactory.h"
#include <FEMLib/FECoreMaterial.h>
#include "FEUserMaterial.h"

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

FEMatDescriptor* FEMaterialFactory::Find(const char* szname)
{
	list<FEMatDescriptor*>::iterator pd = m_Desc.begin();
	while (pd != m_Desc.end())
	{
		if (strcmp((*pd)->GetTypeString(), szname) == 0) return (*pd);
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

FEMaterial* FEMaterialFactory::Create(int nid)
{
	assert(m_pFac);
	FEMatDescriptor* pd = m_pFac->Find(nid);
	return (pd?pd->Create():0);
}

FEMaterial* FEMaterialFactory::Create(const char *szname)
{
	assert(m_pFac);
	FEMatDescriptor* pd = m_pFac->Find(szname);
	return (pd?pd->Create():0);
}

const char* FEMaterialFactory::TypeStr(FEMaterial *pm)
{
	if (pm == 0) return 0;

	// A user material will not be registered, but instead contains its own type str
	// TODO: Should I make TypeStr a member of each material?
	if (dynamic_cast<FEUserMaterial*>(pm)) return (dynamic_cast<FEUserMaterial*>(pm))->GetTypeStr();

	FEMaterialFactory* pmf = GetInstance();

	list<FEMatDescriptor*>::iterator pd = pmf->m_Desc.begin();
	while (pd != pmf->m_Desc.end())
	{
		if ((*pd)->GetTypeID() == pm->Type()) return (*pd)->GetTypeString();
		++pd;
	}
	return 0;
}

int FEMaterialFactory::ClassID(FEMaterial *pm)
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
