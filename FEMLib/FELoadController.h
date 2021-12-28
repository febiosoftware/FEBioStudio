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

	void SetLoadCurve(LoadCurve* plc);

	LoadCurve* GetLoadCurve();

private:
	int	m_ntype;

	LoadCurve* m_plc;
};

class FEBioLoadController : public FSLoadController
{
public:
	FEBioLoadController(FSModel* fem = nullptr);
};
