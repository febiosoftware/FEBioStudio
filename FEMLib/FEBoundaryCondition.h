#pragma once
#include "FEDomainComponent.h"
#include "MeshTools/FEGroup.h"
#include "MeshTools/FEItemListBuilder.h"
#include <list>
//using namespace std;

//=============================================================================
// Base class for fixed and prescribed BCs
class FEBoundaryCondition : public FEDomainComponent
{
public:
	FEBoundaryCondition(int ntype, FEModel* fem, int nstep = 0) : FEDomainComponent(ntype, fem, nstep){
		m_superClassID = FE_ESSENTIAL_BC;
	}
	FEBoundaryCondition(int ntype, FEModel* fem, FEItemListBuilder* pi, int nstep) : FEDomainComponent(ntype, fem, pi, nstep){
		m_superClassID = FE_ESSENTIAL_BC;
	}
};

//=============================================================================
class FEFixedDOF : public FEBoundaryCondition
{
	enum {BC};

public:
	FEFixedDOF(int ntype, FEModel* fem);
	FEFixedDOF(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep = 0);

	int GetVarID() const { return m_nvar; }

	int GetBC() { return GetIntValue(BC); }
	bool GetBC(int i) { int n = GetBC(); return (n & (1 << i) ? true : false); }

	void SetBC(int bc) { SetIntValue(BC, bc); }
	void SetBC(int i, bool b) 
	{
		int bc = GetBC();
		int n = (1 << i);
		if (b) bc |= n; else bc &= ~n;
		SetBC(bc);
	}

protected:
	void SetVarID(int nid);

private:
	int		m_nvar;	// variable ID
};

//=============================================================================
// FIXED DISPLACEMENT
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-displacement
// 1: y-displacement
// 2: z-displacement
class FEFixedDisplacement : public FEFixedDOF
{
public:
	FEFixedDisplacement(FEModel* ps);
	FEFixedDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};

//=============================================================================
// FIXED SHELL BACK FACE DISPLACEMENT
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-displacement
// 1: y-displacement
// 2: z-displacement
class FEFixedShellDisplacement : public FEFixedDOF
{
public:
	FEFixedShellDisplacement(FEModel* ps);
	FEFixedShellDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};


//=============================================================================
// FIXED ROTATION
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-rotation
// 1: y-rotation
// 2: z-rotation
//
class FEFixedRotation : public FEFixedDOF
{
public:
	FEFixedRotation(FEModel* ps);
	FEFixedRotation(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};

//=============================================================================
// FIXED FLUID PRESSURE
//=============================================================================
class FEFixedFluidPressure : public FEFixedDOF
{
public:
	FEFixedFluidPressure(FEModel* ps);
	FEFixedFluidPressure(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// FIXED TEMPERATURE
//=============================================================================
class FEFixedTemperature : public FEFixedDOF
{
public:
	FEFixedTemperature(FEModel* ps);
	FEFixedTemperature(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// FIXED SOLUTE CONCENTRATION
//=============================================================================
class FEFixedConcentration : public FEFixedDOF
{
public:
	FEFixedConcentration(FEModel* ps);
	FEFixedConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// FIXED FLUID VELOCITY
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-velocity
// 1: y-velocity
// 2: z-velocity
//
class FEFixedFluidVelocity : public FEFixedDOF
{
public:
    FEFixedFluidVelocity(FEModel* ps);
    FEFixedFluidVelocity(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};

//=============================================================================
// FIXED FLUID DILATATION
//=============================================================================
class FEFixedFluidDilatation : public FEFixedDOF
{
public:
    enum {BC};
    
public:
    FEFixedFluidDilatation(FEModel* ps);
    FEFixedFluidDilatation(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// Prescribed boundary condition base class
//=============================================================================
class FEPrescribedDOF : public FEBoundaryCondition
{
public:
	enum { BC, SCALE, NTYPE };

public:
	FEPrescribedDOF(int ntype, FEModel* ps, int nstep = 0);
	FEPrescribedDOF(int ntype, FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);

	int GetVarID() const { return m_nvar; }
	void SetVarID(int n);

	int GetDOF() { return GetIntValue(BC); }
	void SetDOF(int n) { SetIntValue(BC, n); }

	FELoadCurve* GetLoadCurve() { return GetParamLC(SCALE); }

	bool GetRelativeFlag() { return GetBoolValue(NTYPE); }
	void SetRelativeFlag(bool b) { SetBoolValue(NTYPE, b); }

	void SetScaleFactor(double s) { SetFloatValue(SCALE, s); }
	double GetScaleFactor() { return GetFloatValue(SCALE); }

	void SetScaleUnit(const char* szunit) { GetParam(SCALE).SetUnit(szunit); }

	// used only for reading parameters for old file formats
	void LoadParam(const Param& p);

private:
	int	m_nvar;
};

//=============================================================================
// PRESCRIBED DISPLACEMENT
//=============================================================================
class FEPrescribedDisplacement : public FEPrescribedDOF
{
public:
	FEPrescribedDisplacement(FEModel* ps);
	FEPrescribedDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED SHELL BACK FACE DISPLACEMENT
//=============================================================================
class FEPrescribedShellDisplacement : public FEPrescribedDOF
{
public:
	FEPrescribedShellDisplacement(FEModel* ps);
	FEPrescribedShellDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED ROTATION
//=============================================================================
class FEPrescribedRotation : public FEPrescribedDOF
{
public:
	FEPrescribedRotation(FEModel* ps);
	FEPrescribedRotation(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED FLUID PRESSURE
//=============================================================================
class FEPrescribedFluidPressure : public FEPrescribedDOF
{
public:
	FEPrescribedFluidPressure(FEModel* ps);
	FEPrescribedFluidPressure(FEModel* ps, FEItemListBuilder* pi, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED TEMPERATURE
//=============================================================================
class FEPrescribedTemperature : public FEPrescribedDOF
{
public:
	FEPrescribedTemperature(FEModel* ps);
	FEPrescribedTemperature(FEModel* ps, FEItemListBuilder* pi, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED SOLUTE CONCENTRATION
//=============================================================================
class FEPrescribedConcentration : public FEPrescribedDOF
{
public:
	FEPrescribedConcentration(FEModel* ps);
	FEPrescribedConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep);
};

//=============================================================================
// PRESCRIBED FLUID VELOCITY
//=============================================================================
class FEPrescribedFluidVelocity : public FEPrescribedDOF
{
public:
    FEPrescribedFluidVelocity(FEModel* ps);
    FEPrescribedFluidVelocity(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED FLUID DILATATION
//=============================================================================
class FEPrescribedFluidDilatation : public FEPrescribedDOF
{
public:
    FEPrescribedFluidDilatation(FEModel* ps);
    FEPrescribedFluidDilatation(FEModel* ps, FEItemListBuilder* pi, double s, int nstep = 0);
};

//=============================================================================
class FEBioBoundaryCondition : public FEBoundaryCondition
{
public:
	FEBioBoundaryCondition(FEModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
