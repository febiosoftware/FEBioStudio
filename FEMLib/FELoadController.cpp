#include "stdafx.h"
#include "FELoadController.h"

int FSLoadController::m_nref = 1;

FSLoadController::FSLoadController(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;

	// set unique ID
	m_nUID = m_nref++;
}

int FSLoadController::Type() const
{
	return m_ntype;
}

int FSLoadController::GetID() const { return m_nUID; }
void FSLoadController::SetID(int nid)
{
	m_nUID = nid;
	m_nref = (nid >= m_nref ? nid + 1 : m_nref);
}

void FSLoadController::ResetCounter() { m_nref = 1; }
void FSLoadController::SetCounter(int n) { m_nref = n; }
int FSLoadController::GetCounter() { return m_nref; }

LoadCurve* FSLoadController::CreateLoadCurve()
{
	return nullptr;
}

//=============================================================================
FEBioLoadController::FEBioLoadController(FSModel* fem) : FSLoadController(fem, FE_FEBIO_LOAD_CONTROLLER)
{
	m_plc = nullptr;
}

FEBioLoadController::~FEBioLoadController()
{
	if (m_plc) delete m_plc;
}

LoadCurve* FEBioLoadController::CreateLoadCurve()
{
	if (IsType("loadcurve"))
	{
		if (m_plc == nullptr) m_plc = new LoadCurve;
		LoadCurve& lc = *m_plc;

		lc.SetType(GetParam("interpolate")->GetIntValue());
		lc.SetExtend(GetParam("extend")->GetIntValue());

		Param* p = GetParam("points");
		std::vector<vec2d> v = p->GetVectorVec2dValue();
		lc.Clear();
		for (int i = 0; i < v.size(); ++i)
		{
			vec2d& pi = v[i];
			lc.Add(pi.x(), pi.y());
		}

		return m_plc;
	}
	else return nullptr;
}

bool FEBioLoadController::UpdateData(bool bsave)
{
	if (bsave && m_plc)
	{
		GetParam("interpolate")->SetIntValue(m_plc->GetType());
		GetParam("extend")->SetIntValue(m_plc->GetExtend());

		std::vector<vec2d> v;
		for (int i = 0; i < m_plc->Size(); ++i)
		{
			LOADPOINT& pi = m_plc->Item(i);
			v.push_back(vec2d(pi.time, pi.load));
		}

		GetParam("points")->SetVectorVec2dValue(v);
	}

	return false;
}
