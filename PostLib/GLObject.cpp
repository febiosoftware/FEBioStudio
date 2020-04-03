#include "stdafx.h"
#include "GLObject.h"
#include <string.h>
using namespace Post;

CGLObject::CGLObject(CGLModel* mdl) : m_pModel(mdl)
{
	m_bactive = true;
}

CGLObject::~CGLObject(void)
{

}

void CGLObject::ChangeName(const std::string& name)
{
	FSObject::SetName(name);
}
