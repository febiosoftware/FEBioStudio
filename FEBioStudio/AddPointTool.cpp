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
	UpdateNode();
}

//-----------------------------------------------------------------------------
CAddPointTool::CAddPointTool() : CBasicTool("Add Point")
{
	addProperty("X", CProperty::Float);
	addProperty("Y", CProperty::Float);
	addProperty("Z", CProperty::Float);

	m_deco = 0;
	m_pos = vec3f(0.f, 0.f, 0.f);
}

//-----------------------------------------------------------------------------
void CAddPointTool::activate(CMainWindow* wnd)
{
	CBasicTool::activate(wnd);

	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		if (m_deco)
		{
			doc->GetGLModel()->RemoveDecoration(m_deco);
			delete m_deco;
			m_deco = 0;
		}
		m_deco = new GPointDecoration;
		doc->GetGLModel()->AddDecoration(m_deco);
		UpdateNode();
	}
}

//-----------------------------------------------------------------------------
void CAddPointTool::deactivate()
{
	CBasicTool::deactivate();
	if (m_deco)
	{
		CPostDoc* doc = GetPostDoc();
		doc->GetGLModel()->RemoveDecoration(m_deco);
		delete m_deco;
		m_deco = 0;
	}
}

//-----------------------------------------------------------------------------
void CAddPointTool::UpdateNode()
{
	if (m_deco) m_deco->setVisible(false);
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		if (m_deco) 
		{
			m_deco->setPosition(m_pos);
			m_deco->setVisible(true);
		}
	}
	updateUi();
}
