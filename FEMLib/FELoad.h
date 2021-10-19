#pragma once
#include "FEModelComponent.h"

//=============================================================================
// Base class for all nodal, edge, surface, and body loads
class FELoad : public FEModelComponent
{
public:
	FELoad(int ntype, FEModel* fem) : FEModelComponent(ntype, fem) {}
	FELoad(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep) : FEModelComponent(ntype, ps, pi, nstep) {}
};

//=============================================================================
// NODAL LOADS
//=============================================================================
class FENodalLoad : public FELoad
{
public:
	enum { DOF, LOAD };

public:
	FENodalLoad(FEModel* ps);
	FENodalLoad(FEModel* ps, FEItemListBuilder* pi, int bc, double f, int nstep = 0);

	FELoadCurve* GetLoadCurve() { return GetParamLC(LOAD); }

	int GetDOF() { return GetIntValue(DOF); }
	void SetDOF(int n) { SetIntValue(DOF, n); }

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }
};
