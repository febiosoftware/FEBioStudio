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
#include <string.h>
#include "ClassDescriptor.h"
#include "FSObject.h"

ClassKernel* ClassKernel::m_pInst = 0;

//-----------------------------------------------------------------------------
ClassDescriptor::ClassDescriptor(Class_Type ntype, int cid, const char* szname, const char* szres, unsigned int flag)
{
	m_ntype = ntype;
	m_szname = szname;
	m_szres  = szres;
	m_ncount = 0;
	m_flag = flag;
	m_classId = cid;
}

//-----------------------------------------------------------------------------
ClassDescriptor::~ClassDescriptor()
{
}

//-----------------------------------------------------------------------------
FSObject* ClassDescriptor::Create()
{
	FSObject* po = CreateInstance();
	po->SetTypeString(GetName());
	return po;
}

//-----------------------------------------------------------------------------
ClassKernel* ClassKernel::GetInstance()
{
	if (m_pInst == 0) m_pInst = new ClassKernel;
	return m_pInst;
}

//-----------------------------------------------------------------------------
void ClassKernel::RegisterClassDescriptor(ClassDescriptor* pcd)
{
	ClassKernel* pCK = ClassKernel::GetInstance();
	pCK->m_CD.push_back(pcd);
}

//-----------------------------------------------------------------------------
Class_Iterator ClassKernel::FirstCD()
{
	ClassKernel* pCK = ClassKernel::GetInstance();
	return pCK->m_CD.begin();
}

//-----------------------------------------------------------------------------
Class_Iterator ClassKernel::LastCD()
{
	ClassKernel* pCK = ClassKernel::GetInstance();
	return pCK->m_CD.end();
}

ClassDescriptor* ClassKernel::FindClassDescriptor(Class_Type classType, const char* typeStr)
{
	Class_Iterator it;
	for (it = ClassKernel::FirstCD(); it != ClassKernel::LastCD(); ++it)
	{
		ClassDescriptor* pcd = *it;
		if ((pcd->GetType() == classType) && (strcmp(pcd->GetName(), typeStr) == 0))
		{
			return pcd;
		}
	}
	return nullptr;

}

FSObject* ClassKernel::CreateClass(Class_Type classType, const char* typeStr)
{
	ClassDescriptor* pcd = FindClassDescriptor(classType, typeStr);
	if (pcd)
	{
		return pcd->Create();
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
FSObject* ClassKernel::CreateClassFromID(Class_Type classType, int cid)
{
	Class_Iterator it;
	for (it = ClassKernel::FirstCD(); it != ClassKernel::LastCD(); ++it)
	{
		ClassDescriptor* pcd = *it;
		if ((pcd->GetType()    == classType) && 
			(pcd->GetClassId() == cid))
		{
			return pcd->Create();
		}
	}
	return nullptr;
}
