#include "FEMKernel.h"

FEClassFactory::FEClassFactory(int module, int superID, int classID, const char* sztype, const char* helpURL)
{
	m_Module = module;
	m_SuperID = superID;
	m_ClassID = classID;
	m_szType = sztype;
	m_helpURL = helpURL;
}

FEClassFactory::~FEClassFactory()
{
}

//=================================================================================================

FEMKernel*	FEMKernel::m_This = 0;

FEMKernel* FEMKernel::Instance()
{
	if (m_This == 0) m_This = new FEMKernel;
	return m_This;
}

FSObject* FEMKernel::Create(FEModel* fem, int superID, int classID)
{
	for (size_t i=0; i<m_Class.size(); ++i)
	{
		FEClassFactory* fac = m_Class[i];
		if ((fac->GetSuperID() == superID) && (fac->GetClassID() == classID))
		{
			return fac->Create(fem);
		}
	}
	return 0;
}

void FEMKernel::RegisterClass(FEClassFactory* fac)
{
	m_Class.push_back(fac);
}

vector<FEClassFactory*> FEMKernel::FindAllClasses(int module, int superID)
{
	vector<FEClassFactory*> l;
	for (int i=0; i<m_This->m_Class.size(); ++i)
	{
		FEClassFactory* fac = m_This->m_Class[i];
		if ((fac->GetModule() & module) && (fac->GetSuperID() == superID))
		{
			l.push_back(fac);
		}
	}
	return l;
}

FEClassFactory* FEMKernel::FindClass(int module, int superID, int classID)
{
	for(FEClassFactory* fac : m_This->m_Class)
	{
		if((fac->GetModule() & module) && (fac->GetSuperID() == superID) && ((fac->GetClassID() == classID)))
		{
			return fac;
		}
	}

	return nullptr;
}
