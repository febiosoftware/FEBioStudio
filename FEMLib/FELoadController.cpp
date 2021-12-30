#include "stdafx.h"
#include "FELoadController.h"

int FSLoadController::m_nref = 1;

FSLoadController::FSLoadController(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
	m_plc = nullptr;

	// set unique ID
	m_nUID = m_nref++;
}

int FSLoadController::Type() const
{
	return m_ntype;
}

void FSLoadController::SetLoadCurve(LoadCurve* plc)
{
	m_plc = plc;
}

LoadCurve* FSLoadController::GetLoadCurve()
{
	return m_plc;
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

//=============================================================================
FEBioLoadController::FEBioLoadController(FSModel* fem) : FSLoadController(fem, FE_FEBIO_LOAD_CONTROLLER)
{

}
