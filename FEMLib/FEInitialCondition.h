#pragma once
#include "FEBoundaryCondition.h"

//-----------------------------------------------------------------------------
// Base class for initial conditions
class FSInitialCondition : public FSDomainComponent
{
public:
	FSInitialCondition(int ntype, FSModel* ps, int nstep = 0) : FSDomainComponent(ntype, ps, nstep) { m_superClassID = FEIC_ID; }
	FSInitialCondition(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0) : FSDomainComponent(ntype, ps, pi, nstep) { m_superClassID = FEIC_ID; }
};

//-----------------------------------------------------------------------------
class FSInitialNodalDOF : public FSInitialCondition
{
public:
	FSInitialNodalDOF(int ntype, FSModel* ps) : FSInitialCondition(ntype, ps) {}
	FSInitialNodalDOF(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0) : FSInitialCondition(ntype, ps, pi, nstep) {}
};

//-----------------------------------------------------------------------------

class FSNodalVelocities : public FSInitialNodalDOF
{
public:
	enum { VEL };

public:
	FSNodalVelocities(FSModel* ps);
	FSNodalVelocities(FSModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep = 0);

	vec3d GetVelocity() { return GetVecValue(VEL); }
	void SetVelocity(vec3d v) { SetVecValue(VEL, v); }
};

//-----------------------------------------------------------------------------
class FSNodalShellVelocities : public FSInitialNodalDOF
{
public:
	enum { VEL };

public:
	FSNodalShellVelocities(FSModel* ps);
	FSNodalShellVelocities(FSModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep = 0);

	vec3d GetVelocity() { return GetVecValue(VEL); }
	void SetVelocity(vec3d v) { SetVecValue(VEL, v); }
};

//-----------------------------------------------------------------------------
class FSInitConcentration : public FSInitialNodalDOF
{
public:
	enum { VALUE, BC };

public:
	FSInitConcentration(FSModel* ps);
	FSInitConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }

	int GetBC() { return GetIntValue(BC); }
	void SetBC(int n) { SetIntValue(BC, n); }
};

//-----------------------------------------------------------------------------
class FSInitShellConcentration : public FSInitialNodalDOF
{
public:
    enum { VALUE, BC };
    
public:
    FSInitShellConcentration(FSModel* ps);
    FSInitShellConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep);
    
    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
    
    int GetBC() { return GetIntValue(BC); }
    void SetBC(int n) { SetIntValue(BC, n); }
};
//-----------------------------------------------------------------------------
class FSInitFluidPressure : public FSInitialNodalDOF
{
public:
	enum { VALUE };

public:
	FSInitFluidPressure(FSModel* ps);
	FSInitFluidPressure(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FSInitShellFluidPressure : public FSInitialNodalDOF
{
public:
    enum { VALUE };
    
public:
    FSInitShellFluidPressure(FSModel* ps);
    FSInitShellFluidPressure(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);
    
    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FSInitTemperature : public FSInitialNodalDOF
{
public:
	enum { VALUE };

public:
	FSInitTemperature(FSModel* ps);
	FSInitTemperature(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

	double GetValue() { return GetFloatValue(VALUE); }
	void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FSInitFluidDilatation : public FSInitialNodalDOF
{
public:
    enum { VALUE };

public:
    FSInitFluidDilatation(FSModel* ps);
    FSInitFluidDilatation(FSModel* ps, FEItemListBuilder* pi, double val, int nstep = 0);

    double GetValue() { return GetFloatValue(VALUE); }
    void SetValue(double v) { SetFloatValue(VALUE, v); }
};

//-----------------------------------------------------------------------------
class FSInitPrestrain : public FSInitialCondition
{
public:
	FSInitPrestrain(FSModel* ps);
};

//-----------------------------------------------------------------------------
class FEBioInitialCondition : public FSInitialCondition
{
public:
	FEBioInitialCondition(FSModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
