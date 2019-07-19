#include "stdafx.h"
#include "ClassDescriptor.h"

ClassKernel* ClassKernel::m_pInst = 0;

//-----------------------------------------------------------------------------
ClassDescriptor::ClassDescriptor(Class_Type ntype, const char* szname, const char* szres, unsigned int flag)
{
	m_ntype = ntype;
	m_szname = szname;
	m_szres  = szres;
	m_ncount = 0;
	m_flag = flag;
}

//-----------------------------------------------------------------------------
ClassDescriptor::~ClassDescriptor()
{
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
