#include "stdafx.h"
#include "MeasureAreaTool.h"
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
CMeasureAreaTool::CMeasureAreaTool(CMainWindow* wnd) : CBasicTool(wnd, "Measure Area")
{
	addProperty("selected faces", CProperty::Int)->setFlags(CProperty::Visible);
	addProperty("area", CProperty::Float)->setFlags(CProperty::Visible);

	m_nsel = 0;
	m_area = 0.0;
}

//-----------------------------------------------------------------------------
QVariant CMeasureAreaTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_nsel; break;
	case 1: return m_area; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CMeasureAreaTool::SetPropertyValue(int i, const QVariant& v)
{
}

//-----------------------------------------------------------------------------
void CMeasureAreaTool::Update()
{
	m_nsel = 0;
	m_area = 0.0;
	FEMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	int NF = mesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& f = mesh->Face(i);
		if (f.IsSelected())
		{
			++m_nsel;
			m_area += mesh->FaceArea(f);
		}
	}
}
