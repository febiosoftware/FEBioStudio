#pragma once
#include "FEModelComponent.h"

enum LoadControllerType
{
	FE_FEBIO_LOAD_CONTROLLER = 1
};

class FSLoadController : public FSModelComponent
{
public:
	FSLoadController(FSModel* fem, int ntype);

	int Type() const;

private:
	int	m_ntype;
};

class FEBioLoadController : public FSLoadController
{
public:
	FEBioLoadController(FSModel* fem = nullptr);
};
