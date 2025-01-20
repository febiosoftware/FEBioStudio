#pragma once
#include "FEDomainComponent.h"
#include <GeomLib/FSGroup.h>
#include <MeshLib/FEItemListBuilder.h>
#include <list>

//=============================================================================
// Base class for fixed and prescribed BCs
class FSBoundaryCondition : public FSDomainComponent
{
public:
	FSBoundaryCondition(int ntype, FSModel* fem, int nstep = 0) : FSDomainComponent(ntype, fem, nstep){
		m_superClassID = FEBC_ID;
	}
	FSBoundaryCondition(int ntype, FSModel* fem, FEItemListBuilder* pi, int nstep) : FSDomainComponent(ntype, fem, pi, nstep){
		m_superClassID = FEBC_ID;
	}
};

//=============================================================================
class FSFixedDOF : public FSBoundaryCondition
{
	enum {BC};

public:
	FSFixedDOF(int ntype, FSModel* fem);
	FSFixedDOF(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep = 0);

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
class FSFixedDisplacement : public FSFixedDOF
{
public:
	FSFixedDisplacement(FSModel* ps);
	FSFixedDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};

//=============================================================================
// FIXED SHELL BACK FACE DISPLACEMENT
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-displacement
// 1: y-displacement
// 2: z-displacement
class FSFixedShellDisplacement : public FSFixedDOF
{
public:
	FSFixedShellDisplacement(FSModel* ps);
	FSFixedShellDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};


//=============================================================================
// FIXED ROTATION
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-rotation
// 1: y-rotation
// 2: z-rotation
//
class FSFixedRotation : public FSFixedDOF
{
public:
	FSFixedRotation(FSModel* ps);
	FSFixedRotation(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};

//=============================================================================
// FIXED FLUID PRESSURE
//=============================================================================
class FSFixedFluidPressure : public FSFixedDOF
{
public:
	FSFixedFluidPressure(FSModel* ps);
	FSFixedFluidPressure(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// FIXED TEMPERATURE
//=============================================================================
class FSFixedTemperature : public FSFixedDOF
{
public:
	FSFixedTemperature(FSModel* ps);
	FSFixedTemperature(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// FIXED SOLUTE CONCENTRATION
//=============================================================================
class FSFixedConcentration : public FSFixedDOF
{
public:
	FSFixedConcentration(FSModel* ps);
	FSFixedConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// FIXED FLUID VELOCITY
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-velocity
// 1: y-velocity
// 2: z-velocity
//
class FSFixedFluidVelocity : public FSFixedDOF
{
public:
    FSFixedFluidVelocity(FSModel* ps);
    FSFixedFluidVelocity(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};

//=============================================================================
// FIXED FLUID DILATATION
//=============================================================================
class FSFixedFluidDilatation : public FSFixedDOF
{
public:
    enum {BC};
    
public:
    FSFixedFluidDilatation(FSModel* ps);
    FSFixedFluidDilatation(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep);
};

//=============================================================================
// FIXED ANGULAR FLUID VELOCITY
//=============================================================================
// The BC parameter stores the constraints that are fixed. The order is:
// 0: x-velocity
// 1: y-velocity
// 2: z-velocity
//
class FSFixedFluidAngularVelocity : public FSFixedDOF
{
public:
    FSFixedFluidAngularVelocity(FSModel* ps);
    FSFixedFluidAngularVelocity(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep = 0);
};

//=============================================================================
// Prescribed boundary condition base class
//=============================================================================
class FSPrescribedDOF : public FSBoundaryCondition
{
public:
	enum { BC, SCALE, NTYPE };

public:
	FSPrescribedDOF(int ntype, FSModel* ps, int nstep = 0);
	FSPrescribedDOF(int ntype, FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);

	int GetVarID() const { return m_nvar; }
	void SetVarID(int n);

	int GetDOF() { return GetIntValue(BC); }
	void SetDOF(int n) { SetIntValue(BC, n); }

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
class FSPrescribedDisplacement : public FSPrescribedDOF
{
public:
	FSPrescribedDisplacement(FSModel* ps);
	FSPrescribedDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED SHELL BACK FACE DISPLACEMENT
//=============================================================================
class FSPrescribedShellDisplacement : public FSPrescribedDOF
{
public:
	FSPrescribedShellDisplacement(FSModel* ps);
	FSPrescribedShellDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED ROTATION
//=============================================================================
class FSPrescribedRotation : public FSPrescribedDOF
{
public:
	FSPrescribedRotation(FSModel* ps);
	FSPrescribedRotation(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED FLUID PRESSURE
//=============================================================================
class FSPrescribedFluidPressure : public FSPrescribedDOF
{
public:
	FSPrescribedFluidPressure(FSModel* ps);
	FSPrescribedFluidPressure(FSModel* ps, FEItemListBuilder* pi, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED TEMPERATURE
//=============================================================================
class FSPrescribedTemperature : public FSPrescribedDOF
{
public:
	FSPrescribedTemperature(FSModel* ps);
	FSPrescribedTemperature(FSModel* ps, FEItemListBuilder* pi, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED SOLUTE CONCENTRATION
//=============================================================================
class FSPrescribedConcentration : public FSPrescribedDOF
{
public:
	FSPrescribedConcentration(FSModel* ps);
	FSPrescribedConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep);
};

//=============================================================================
// PRESCRIBED FLUID VELOCITY
//=============================================================================
class FSPrescribedFluidVelocity : public FSPrescribedDOF
{
public:
    FSPrescribedFluidVelocity(FSModel* ps);
    FSPrescribedFluidVelocity(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED FLUID DILATATION
//=============================================================================
class FSPrescribedFluidDilatation : public FSPrescribedDOF
{
public:
    FSPrescribedFluidDilatation(FSModel* ps);
    FSPrescribedFluidDilatation(FSModel* ps, FEItemListBuilder* pi, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED ANGULAR FLUID VELOCITY
//=============================================================================
class FSPrescribedFluidAngularVelocity : public FSPrescribedDOF
{
public:
    FSPrescribedFluidAngularVelocity(FSModel* ps);
    FSPrescribedFluidAngularVelocity(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep = 0);
};

//=============================================================================
// PRESCRIBED NORMAL DISPLACEMENT
//=============================================================================
class FSNormalDisplacementBC : public FSBoundaryCondition
{
public:
	FSNormalDisplacementBC(FSModel* ps);
};

//=============================================================================
class FEBioBoundaryCondition : public FSBoundaryCondition
{
public:
	FEBioBoundaryCondition(FSModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
