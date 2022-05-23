#pragma once
#include "FEDomainComponent.h"

enum MeshAdaptorType {
	FE_FEBIO_MESH_ADAPTOR = 1
};

class FSMeshAdaptor : public FSDomainComponent
{
public:
	FSMeshAdaptor(FSModel* fem, int ntype);
};

class FEBioMeshAdaptor : public FSMeshAdaptor
{
public:
	FEBioMeshAdaptor(FSModel* fem);

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;
};
