#include "stdafx.h"
#include "FEMeshAdaptor.h"

FSMeshAdaptor::FSMeshAdaptor(FSModel* fem, int ntype) : FSStepComponent(fem)
{
	m_ntype = ntype;
}

int FSMeshAdaptor::Type() const
{
	return m_ntype;
}

FEBioMeshAdaptor::FEBioMeshAdaptor(FSModel* fem) : FSMeshAdaptor(fem, FE_FEBIO_MESH_ADAPTOR)
{

}
