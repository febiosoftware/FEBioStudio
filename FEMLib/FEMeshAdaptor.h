#pragma once
#include "FEStepComponent.h"

enum MeshAdaptorType {
	FE_FEBIO_MESH_ADAPTOR = 1
};

class FSMeshAdaptor : public FSStepComponent
{
public:
	FSMeshAdaptor(FSModel* fem, int ntype);

	int Type() const;

private:
	int	m_ntype;
};


class FEBioMeshAdaptor : public FSMeshAdaptor
{
public:
	FEBioMeshAdaptor(FSModel* fem);
};
