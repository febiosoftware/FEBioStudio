#include "stdafx.h"
#include "FELoadController.h"

FSLoadController::FSLoadController(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
	m_plc = nullptr;
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


FEBioLoadController::FEBioLoadController(FSModel* fem) : FSLoadController(fem, FE_FEBIO_LOAD_CONTROLLER)
{

}
