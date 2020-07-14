#pragma once
#include "FEBoundaryCondition.h"

//-----------------------------------------------------------------------------
// Base class for initial conditions
class FEInitialCondition : public FEModelComponent
{
public:
	FEInitialCondition(int ntype, FEModel* ps, int nstep = 0) : FEModelComponent(ntype, ps, nstep) {}
	FEInitialCondition(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep = 0) : FEModelComponent(ntype, ps, pi, nstep) {}
};

//-----------------------------------------------------------------------------
class FEInitialNodalDOF : public FEInitialCondition
{
public:
	FEInitialNodalDOF(int ntype, FEModel* ps) : FEInitialCondition(ntype, ps) {}
	FEInitialNodalDOF(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep = 0) : FEInitialCondition(ntype, ps, pi, nstep) {}
};

//-----------------------------------------------------------------------------

class FENodalVelocities : public FEInitialNodalDOF
{
public:
	enum { VEL };

public:
	FENodalVelocities(FEModel* ps);
	FENodalVelocities(FEModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep = 0);

	vec3d GetVelocity() { return GetVecValue(VEL); }
	void SetVelocity(vec3d v) { SetVecValue(VEL, v); }
};

//-----------------------------------------------------------------------------
class FENodalShellVelocities : public FEInitialNodalDOF
{
public:
	enum { VEL };

public:
	FENodalShellVelocities(FEModel* ps);
	FENodalShellVelocities(FEModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep = 0);

	vec3d GetVelocity() { return GetVecValue(VEL); }
	void SetVelocity(vec3d v) { SetVecValue(VEL, v); }
};

//-----------------------------------------------------------------------------
class FEInitConcentration : public FEInitialNodalDOF
{
public:
	enum { VALUE, BC };

public:
	FEInitConcentration(FEModel* ps);
	FEInitConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep);

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
    FEInitShellConcentration(FEModel* ps);
    FEInitShellConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep);
    
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
	FEInitFluidPressure(FEModel* ps);
	FEInitFluidPressure(FEModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitShellFluidPressure : public FEInitialNodalDOF
{
public:
    enum { VALUE };
    
public:
    FEInitShellFluidPressure(FEModel* ps);
    FEInitShellFluidPressure(FEModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);
    
    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitTemperature : public FEInitialNodalDOF
{
public:
	enum { VALUE };

public:
	FEInitTemperature(FEModel* ps);
	FEInitTemperature(FEModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitFluidDilatation : public FEInitialNodalDOF
{
public:
    enum { VALUE };

public:
    FEInitFluidDilatation(FEModel* ps);
    FEInitFluidDilatation(FEModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FEInitPrestrain : public FEInitialCondition
{
public:
	FEInitPrestrain(FEModel* ps);
};
