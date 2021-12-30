#pragma once
#include "FEModelComponent.h"
#include <FSCore/LoadCurve.h>

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

public:
	int GetID() const;
	void SetID(int nid);

	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();

private:
	int	m_ntype;
	LoadCurve* m_plc;

private:
	int			m_nUID;	//!< unique ID
	static	int	m_nref;
};

class FEBioLoadController : public FSLoadController
{
public:
	FEBioLoadController(FSModel* fem = nullptr);
};
