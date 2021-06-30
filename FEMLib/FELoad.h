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
	FENodalLoad(int ntype, FEModel* fem) : FELoad(ntype, fem) { m_superClassID = FE_NODAL_LOAD; }
	FENodalLoad(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep) : FELoad(ntype, ps, pi, nstep) { m_superClassID = FE_NODAL_LOAD; }
};

class FENodalDOFLoad : public FENodalLoad
{
public:
	enum { DOF, LOAD };

public:
	FENodalDOFLoad(FEModel* ps);
	FENodalDOFLoad(FEModel* ps, FEItemListBuilder* pi, int bc, double f, int nstep = 0);

	FELoadCurve* GetLoadCurve() { return GetParamLC(LOAD); }

	int GetDOF() { return GetIntValue(DOF); }
	void SetDOF(int n) { SetIntValue(DOF, n); }

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }
};

class FEBioNodalLoad : public FENodalLoad
{
public:
	FEBioNodalLoad(FEModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
