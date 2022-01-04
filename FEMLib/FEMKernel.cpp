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

FSObject* FEMKernel::Create(FSModel* fem, int superID, int classID)
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

FSObject* FEMKernel::Create(FSModel* fem, int superID, const char* szTypeString)
{
	for (size_t i = 0; i < m_Class.size(); ++i)
	{
		FEClassFactory* fac = m_Class[i];
		if ((fac->GetSuperID() == superID) && (strcmp(fac->GetTypeStr(), szTypeString) == 0))
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

const char* FEMKernel::TypeStr(int superID, int classID)
{
    for (size_t i=0; i<m_Class.size(); ++i)
	{
		FEClassFactory* fac = m_Class[i];
		if ((fac->GetSuperID() == superID) && (fac->GetClassID() == classID))
		{
			return fac->GetTypeStr();
		}
	}
	return 0;
}