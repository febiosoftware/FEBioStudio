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
