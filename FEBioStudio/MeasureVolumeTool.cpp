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
	case 0: return m_nsel; break;
	case 1: return m_vol; break;
	case 2: return m_nformula; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CMeasureVolumeTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 2) m_nformula = v.toInt();
}

//-----------------------------------------------------------------------------
CMeasureVolumeTool::CMeasureVolumeTool() : CBasicTool("Measure Volume", CBasicTool::HAS_APPLY_BUTTON)
{
	addProperty("selected faces", CProperty::Int)->setFlags(CProperty::Visible);
	addProperty("volume", CProperty::Float)->setFlags(CProperty::Visible);
	addProperty("symmetry", CProperty::Enum)->setEnumValues(QStringList() << "(None)" << "X" << "Y" << "Z");

	m_nsel = 0;
	m_vol = 0.0;
	m_nformula = 0;
}

//-----------------------------------------------------------------------------
bool CMeasureVolumeTool::OnApply()
{
	m_nsel = 0;
	m_vol = 0.0;
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		Post::FEPostModel& fem = *doc->GetFEModel();
		Post::CGLModel* mdl = doc->GetGLModel();
		Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
		int ntime = fem.CurrentTime();
		const vector<FEFace*> selectedFaces = doc->GetGLModel()->GetFaceSelection();
		int N = (int)selectedFaces.size();
		for (int i = 0; i<N; ++i)
		{
			FEFace& f = *selectedFaces[i];

			// get the average position, area and normal
			vec3d r = mesh.FaceCenter(f);
			double area = mesh.FaceArea(f);
			vec3d N = f.m_fn;

			switch (m_nformula)
			{
			case 0: m_vol += area*(N*r) / 3.f; break;
			case 1: m_vol += 2.f*area*(r.x*N.x); break;
			case 2: m_vol += 2.f*area*(r.y*N.y); break;
			case 3: m_vol += 2.f*area*(r.z*N.z); break;
			}
			++m_nsel;
		}

		m_vol = fabs(m_vol);
	}
	updateUi();
	return true;
}
