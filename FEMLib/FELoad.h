#pragma once
#include "FEDomainComponent.h"

//=============================================================================
// Base class for all nodal, edge, surface, and body loads
class FELoad : public FEDomainComponent
{
public:
	FELoad(int ntype, FSModel* fem) : FEDomainComponent(ntype, fem) {}
	FELoad(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep) : FEDomainComponent(ntype, ps, pi, nstep) {}
};

//=============================================================================
// NODAL LOADS
//=============================================================================

class FENodalLoad : public FELoad
{
public:
	FENodalLoad(int ntype, FSModel* fem) : FELoad(ntype, fem) { m_superClassID = FE_NODAL_LOAD; }
	FENodalLoad(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep) : FELoad(ntype, ps, pi, nstep) { m_superClassID = FE_NODAL_LOAD; }
};

class FENodalDOFLoad : public FENodalLoad
{
public:
	enum { DOF, LOAD };

public:
	FENodalDOFLoad(FSModel* ps);
	FENodalDOFLoad(FSModel* ps, FEItemListBuilder* pi, int bc, double f, int nstep = 0);

	FELoadCurve* GetLoadCurve() { return GetParamLC(LOAD); }

	int GetDOF() { return GetIntValue(DOF); }
	void SetDOF(int n) { SetIntValue(DOF, n); }

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }
};

class FEBioNodalLoad : public FENodalLoad
{
public:
	FEBioNodalLoad(FSModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
