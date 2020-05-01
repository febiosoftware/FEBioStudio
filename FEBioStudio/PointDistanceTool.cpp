#include "stdafx.h"
#include "PointDistanceTool.h"
#include <GLLib/GDecoration.h>
#include <MeshLib/FEMesh.h>

QVariant CPointDistanceTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_node1; break;
	case 1: return m_node2; break;
	case 2: return fabs(m_d.x); break;
	case 3: return fabs(m_d.y); break;
	case 4: return fabs(m_d.z); break;
	case 5: return m_d.Length(); break;
	}
	return QVariant();
}

void CPointDistanceTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i==0) m_node1 = v.toInt();
	if (i==1) m_node2 = v.toInt();
}

CPointDistanceTool::CPointDistanceTool(CMainWindow* wnd) : CBasicTool(wnd, "Point Distance")
{ 
	m_node1 = 0; 
	m_node2 = 0; 
	m_d = vec3f(0,0,0); 

	addProperty("node 1", CProperty::Int);
	addProperty("node 2", CProperty::Int);
	addProperty("Dx", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Dy", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Dz", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("Length", CProperty::Float)->setFlags(CProperty::Visible);
}

void CPointDistanceTool::Activate()
{
	CBasicTool::Activate();
	Update();
}

void CPointDistanceTool::Update()
{
	SetDecoration(nullptr);
	m_d = vec3f(0.f, 0.f, 0.f);

	FEMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	int nsel = 0;
	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		FENode& node = mesh->Node(i);
		int nid = i + 1;
		if (node.IsSelected())
		{
			nsel++;
			if      (m_node1 == 0) m_node1 = nid;
			else if (m_node2 == 0) m_node2 = nid;
			else
			{
				m_node1 = m_node2;
				m_node2 = nid;
			}
		}
	}

	if (nsel == 0)
	{
		m_node1 = m_node2 = 0;
	}
	else if ((m_node1 > 0) && (m_node2 > 0))
	{
		vec3d a = mesh->Node(m_node1 - 1).pos();
		vec3d b = mesh->Node(m_node2 - 1).pos();
		m_d = b - a;

		GCompositeDecoration* deco = new GCompositeDecoration;
		deco->AddDecoration(new GPointDecoration(to_vec3f(a)));
		deco->AddDecoration(new GPointDecoration(to_vec3f(b)));
		deco->AddDecoration(new GLineDecoration(to_vec3f(a), to_vec3f(b)));
		SetDecoration(deco);
	}
}
