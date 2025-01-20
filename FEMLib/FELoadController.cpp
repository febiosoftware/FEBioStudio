#include "stdafx.h"
#include "FELoadController.h"
#include <FECore/fecore_enum.h>
using namespace std;

int FSLoadController::m_nref = 1;

FSLoadController::FSLoadController(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	SetSuperClassID(FELOADCONTROLLER_ID);
	m_ntype = ntype;
	m_count = 0;

	// set unique ID
	m_nUID = m_nref++;
}

int FSLoadController::Type() const
{
	return m_ntype;
}

void FSLoadController::Save(OArchive& ar)
{
	int nid = GetID();
	ar.WriteChunk(CID_LOAD_CONTROLLER_ID, nid);
	string name = GetName();
	if (name.empty() == false)
	{
		ar.WriteChunk(CID_LOAD_CONTROLLER_NAME, name);
	}
	string info = GetInfo();
	if (info.empty() == false)
	{
		ar.WriteChunk(CID_LOAD_CONTROLLER_INFO, info);
	}
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_LOAD_CONTROLLER_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}
}

void FSLoadController::Load(IArchive& ar)
{
	TRACE("FSLoadController::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_LOAD_CONTROLLER_ID  : { int lcid = -1; ar.read(lcid); SetID(lcid); } break;
		case CID_LOAD_CONTROLLER_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_LOAD_CONTROLLER_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_LOAD_CONTROLLER_PARAMS: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
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

		lc.SetInterpolator(GetParam("interpolate")->GetIntValue());
		lc.SetExtendMode(GetParam("extend")->GetIntValue());

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
		GetParam("interpolate")->SetIntValue(m_plc->GetInterpolator());
		GetParam("extend")->SetIntValue(m_plc->GetExtendMode());

		std::vector<vec2d> v;
		for (int i = 0; i < m_plc->Points(); ++i)
		{
			vec2d pi = m_plc->Point(i);
			v.push_back(pi);
		}

		GetParam("points")->SetVectorVec2dValue(v);
	}

	return false;
}

void FEBioLoadController::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSLoadController::Save(ar);
	}
	ar.EndChunk();
}

void FEBioLoadController::Load(IArchive& ar)
{
	TRACE("FEBioLoadController::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSLoadController::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

//=============================================================================

int FSFunction1D::m_nref = 1;

FSFunction1D::FSFunction1D(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;

	// set unique ID
	m_nUID = m_nref++;
}

int FSFunction1D::Type() const
{
	return m_ntype;
}

int FSFunction1D::GetID() const { return m_nUID; }
void FSFunction1D::SetID(int nid)
{
	m_nUID = nid;
	m_nref = (nid >= m_nref ? nid + 1 : m_nref);
}

void FSFunction1D::ResetCounter() { m_nref = 1; }
void FSFunction1D::SetCounter(int n) { m_nref = n; }
int FSFunction1D::GetCounter() { return m_nref; }

LoadCurve* FSFunction1D::CreateLoadCurve()
{
	return nullptr;
}

//=============================================================================
FEBioFunction1D::FEBioFunction1D(FSModel* fem) : FSFunction1D(fem, FE_FEBIO_FUNCTION1D)
{
	m_plc = nullptr;
}

FEBioFunction1D::~FEBioFunction1D()
{
	if (m_plc) delete m_plc;
}

LoadCurve* FEBioFunction1D::CreateLoadCurve()
{
	if (IsType("point"))
	{
		if (m_plc == nullptr) m_plc = new LoadCurve;
		LoadCurve& lc = *m_plc;

		lc.SetInterpolator(GetParam("interpolate")->GetIntValue());
		lc.SetExtendMode(GetParam("extend")->GetIntValue());

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

bool FEBioFunction1D::UpdateData(bool bsave)
{
	if (bsave && m_plc)
	{
		GetParam("interpolate")->SetIntValue(m_plc->GetInterpolator());
		GetParam("extend")->SetIntValue(m_plc->GetExtendMode());

		std::vector<vec2d> v;
		for (int i = 0; i < m_plc->Points(); ++i)
		{
			vec2d pi = m_plc->Point(i);
			v.push_back(pi);
		}

		GetParam("points")->SetVectorVec2dValue(v);
	}

	return false;
}

void FEBioFunction1D::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSFunction1D::Save(ar);
	}
	ar.EndChunk();
}

void FEBioFunction1D::Load(IArchive& ar)
{
	TRACE("FEBioFunction1D::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSFunction1D::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
