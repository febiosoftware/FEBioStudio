#pragma once
#include "FEBoundaryCondition.h"

//-----------------------------------------------------------------------------
// Base class for initial conditions
class FSInitialCondition : public FSDomainComponent
{
public:
	FSInitialCondition(int ntype, FSModel* ps, int nstep = 0) : FSDomainComponent(ntype, ps, nstep) { m_superClassID = FE_INITIAL_CONDITION; }
	FSInitialCondition(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0) : FSDomainComponent(ntype, ps, pi, nstep) { m_superClassID = FE_INITIAL_CONDITION; }
};

//-----------------------------------------------------------------------------
class FEInitialNodalDOF : public FSInitialCondition
{
public:
	FEInitialNodalDOF(int ntype, FSModel* ps) : FSInitialCondition(ntype, ps) {}
	FEInitialNodalDOF(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0) : FSInitialCondition(ntype, ps, pi, nstep) {}
};

//-----------------------------------------------------------------------------

class FENodalVelocities : public FEInitialNodalDOF
{
public:
	enum { VEL };

public:
	FENodalVelocities(FSModel* ps);
	FENodalVelocities(FSModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep = 0);

	vec3d GetVelocity() { return GetVecValue(VEL); }
	void SetVelocity(vec3d v) { SetVecValue(VEL, v); }
};

//-----------------------------------------------------------------------------
class FENodalShellVelocities : public FEInitialNodalDOF
{
public:
	enum { VEL };

public:
	FENodalShellVelocities(FSModel* ps);
	FENodalShellVelocities(FSModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep = 0);

	vec3d GetVelocity() { return GetVecValue(VEL); }
	void SetVelocity(vec3d v) { SetVecValue(VEL, v); }
};

//-----------------------------------------------------------------------------
class FEInitConcentration : public FEInitialNodalDOF
{
public:
	enum { VALUE, BC };

public:
	FEInitConcentration(FSModel* ps);
	FEInitConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }

	int GetBC() { return GetIntValue(BC); }
	void SetBC(int n) { SetIntValue(BC, n); }
};

//-----------------------------------------------------------------------------
class FEInitShellConcentration : public FEInitialNodalDOF
{
public:
    enum { VALUE, BC };
    
public:
    FEInitShellConcentration(FSModel* ps);
    FEInitShellConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep);
    
    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
    
    int GetBC() { return GetIntValue(BC); }
    void SetBC(int n) { SetIntValue(BC, n); }
};
//-----------------------------------------------------------------------------
class FEInitFluidPressure : public FEInitialNodalDOF
{
public:
	enum { VALUE };

public:
	FEInitFluidPressure(FSModel* ps);
	FEInitFluidPressure(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitShellFluidPressure : public FEInitialNodalDOF
{
public:
    enum { VALUE };
    
public:
    FEInitShellFluidPressure(FSModel* ps);
    FEInitShellFluidPressure(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);
    
    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitTemperature : public FEInitialNodalDOF
{
public:
	enum { VALUE };

public:
	FEInitTemperature(FSModel* ps);
	FEInitTemperature(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitFluidDilatation : public FEInitialNodalDOF
{
public:
    enum { VALUE };

public:
    FEInitFluidDilatation(FSModel* ps);
    FEInitFluidDilatation(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitPrestrain : public FSInitialCondition
{
public:
	FEInitPrestrain(FSModel* ps);
};

//-----------------------------------------------------------------------------
class FEBioInitialCondition : public FSInitialCondition
{
public:
	FEBioInitialCondition(FSModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
