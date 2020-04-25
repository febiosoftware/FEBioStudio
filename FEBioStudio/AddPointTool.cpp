#include "stdafx.h"
#include "AddPointTool.h"
#include "PostDoc.h"
#include <PostGL/GLModel.h>
using namespace Post;

//-----------------------------------------------------------------------------
QVariant CAddPointTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_pos.x; break;
	case 1: return m_pos.y; break;
	case 2: return m_pos.z; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CAddPointTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i==0) m_pos.x = v.toFloat();
	if (i==1) m_pos.y = v.toFloat();
	if (i==2) m_pos.z = v.toFloat();
	Update();
}

//-----------------------------------------------------------------------------
CAddPointTool::CAddPointTool(CMainWindow* wnd) : CBasicTool(wnd, "Add Point")
{
	addProperty("X", CProperty::Float);
	addProperty("Y", CProperty::Float);
	addProperty("Z", CProperty::Float);
	m_pos = vec3f(0.f, 0.f, 0.f);
}

//-----------------------------------------------------------------------------
void CAddPointTool::Update()
{
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
	}
}
