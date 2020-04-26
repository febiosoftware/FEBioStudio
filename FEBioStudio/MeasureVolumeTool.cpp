#include "stdafx.h"
#include "MeasureVolumeTool.h"
#include "PostDoc.h"
#include <PostLib/FEPostModel.h>
#include <PostGL/GLModel.h>
using namespace Post;

//-----------------------------------------------------------------------------
QVariant CMeasureVolumeTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_nformula; break;
	case 1: return m_vol; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CMeasureVolumeTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 2) m_nformula = v.toInt();
}

//-----------------------------------------------------------------------------
CMeasureVolumeTool::CMeasureVolumeTool(CMainWindow* wnd) : CBasicTool(wnd, "Surface Volume", CBasicTool::HAS_APPLY_BUTTON)
{
	addProperty("symmetry", CProperty::Enum)->setEnumValues(QStringList() << "(None)" << "X" << "Y" << "Z");
	addProperty("volume", CProperty::Float)->setFlags(CProperty::Visible);

	m_vol = 0.0;
	m_nformula = 0;
}

//-----------------------------------------------------------------------------
bool CMeasureVolumeTool::OnApply()
{
	m_vol = 0.0;

	FEMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return false;

	int NF = mesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& f = mesh->Face(i);

		// get the average position, area and normal
		vec3d r = mesh->FaceCenter(f);
		double area = mesh->FaceArea(f);
		vec3d N = f.m_fn;

		switch (m_nformula)
		{
		case 0: m_vol += area*(N*r) / 3.f; break;
		case 1: m_vol += 2.f*area*(r.x*N.x); break;
		case 2: m_vol += 2.f*area*(r.y*N.y); break;
		case 3: m_vol += 2.f*area*(r.z*N.z); break;
		}
	}

	m_vol = fabs(m_vol);
	return true;
}
