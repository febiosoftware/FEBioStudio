#include "stdafx.h"
#include "FELoadController.h"

FSLoadController::FSLoadController(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
}

int FSLoadController::Type() const
{
	return m_ntype;
}

FEBioLoadController::FEBioLoadController(FSModel* fem) : FSLoadController(fem, FE_FEBIO_LOAD_CONTROLLER)
{

}
