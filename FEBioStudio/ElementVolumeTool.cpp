#include "stdafx.h"
#include "ElementVolumeTool.h"
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
CElementVolumeTool::CElementVolumeTool(CMainWindow* wnd) : CBasicTool(wnd, "Element Volume")
{
	addProperty("Elements", CProperty::Int)->setFlags(CProperty::Visible);
	addProperty("volume", CProperty::Float)->setFlags(CProperty::Visible);

	m_nsel = 0;
	m_vol = 0.0;
}

//-----------------------------------------------------------------------------
QVariant CElementVolumeTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_nsel; break;
	case 1: return m_vol; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CElementVolumeTool::SetPropertyValue(int i, const QVariant& v)
{
}

//-----------------------------------------------------------------------------
void CElementVolumeTool::Update()
{
	m_nsel = 0;
	m_vol = 0.0;
	FEMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	int NE = mesh->Elements();
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = mesh->Element(i);
		if (el.IsSelected())
		{
			++m_nsel;
			m_vol += mesh->ElementVolume(el);
		}
	}
}
