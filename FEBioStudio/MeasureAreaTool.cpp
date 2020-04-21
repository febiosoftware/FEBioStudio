#include "stdafx.h"
#include "MeasureAreaTool.h"
#include "PostDoc.h"
#include <PostLib/FEPostModel.h>
#include "MainWindow.h"
#include <PostGL/GLModel.h>
using namespace Post;

//-----------------------------------------------------------------------------
QVariant CMeasureAreaTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_bfilter; break;
	case 1: return m_minFilter; break;
	case 2: return m_maxFilter; break;
	case 3: return m_allSteps; break;
	case 4: return m_nsel; break;
	case 5: return m_area; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void CMeasureAreaTool::SetPropertyValue(int i, const QVariant& v)
{
	switch (i)
	{
	case 0: m_bfilter = v.toBool(); break;
	case 1: m_minFilter = v.toDouble(); break;
	case 2: m_maxFilter = v.toDouble(); break;
	case 3: m_allSteps = v.toBool(); break;
	}
}

//-----------------------------------------------------------------------------
CMeasureAreaTool::CMeasureAreaTool() : CBasicTool("Measure Area", CBasicTool::HAS_APPLY_BUTTON)
{
	addProperty("apply filter", CProperty::Bool);
	addProperty("min value", CProperty::Float);
	addProperty("max value", CProperty::Float);
	addProperty("all time steps", CProperty::Bool);
	addProperty("selected faces", CProperty::Int)->setFlags(CProperty::Visible);
	addProperty("area", CProperty::Float)->setFlags(CProperty::Visible);

	m_nsel = 0;
	m_area = 0.0;

	m_bfilter = false;
	m_minFilter = 0.0;
	m_maxFilter = 0.0;
	m_allSteps = false;
}

//-----------------------------------------------------------------------------
bool CMeasureAreaTool::OnApply()
{
	m_nsel = 0;
	m_area = 0.0;
	CPostDoc* doc = GetPostDoc();
	if (doc && doc->IsValid())
	{
		Post::FEPostModel& fem = *doc->GetFEModel();
		Post::CGLModel* mdl = doc->GetGLModel();
		Post::FEPostMesh& mesh = *mdl->GetActiveMesh();
		int index = mdl->CurrentTimeIndex();
		FEState* ps = fem.GetState(index);
		const vector<FEFace*> selectedFaces = doc->GetGLModel()->GetFaceSelection();
		m_nsel = (int)selectedFaces.size();

		Post::FEPostMesh* currentMesh = ps->GetFEMesh();

		if (m_allSteps)
		{
			int nstates = fem.GetStates();
			std::vector<double> area(nstates, 0.0);
			for (int i = 0; i < nstates; ++i)
			{
				FEState* state = fem.GetState(i);
				if (state->GetFEMesh() == currentMesh)
					area[i] = getValue(state, selectedFaces);
				else
					area[i] = 0.0;
			}

			m_area = area[index];

//			m_wnd->ShowData(area, "Area");
		}
		else m_area = getValue(ps, selectedFaces);
	}
	updateUi();
	return true;
}

double CMeasureAreaTool::getValue(FEState* state, const std::vector<FEFace*>& selectedFaces)
{
	double area = 0.0;
	int nsel = 0;
	Post::FEPostMesh& mesh = *state->GetFEMesh();
	int N = (int)selectedFaces.size();
	vector<vec3d> rt;
	for (int i = 0; i<N; ++i)
	{
		FEFace& f = *selectedFaces[i];

		float v = state->m_FACE[f.GetID() - 1].m_val;
		if ((m_bfilter == false) || ((v >= m_minFilter) && (v <= m_maxFilter)))
		{
			rt.resize(f.Nodes());
			for (int j = 0; j < f.Nodes(); ++j) rt[j] = state->m_NODE[f.n[j]].m_rt;
			area += mesh.FaceArea(rt, f.Nodes());
			++nsel;
		}
	}
	return area;
}
