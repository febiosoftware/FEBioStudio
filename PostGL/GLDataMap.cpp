#include "stdafx.h"
#include "GLDataMap.h"
using namespace Post;

CGLDataMap::CGLDataMap(CGLModel* po)
{
	m_pgl = po;
}

CGLDataMap::~CGLDataMap(void)
{
	m_pgl = 0;
}

CGLModel* CGLDataMap::GetModel()
{
	return m_pgl;
}
