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

// get the name
const std::string& CGLObject::GetName() const { return m_name; }

// set the name
void CGLObject::SetName(const std::string& name) { m_name = name; }
