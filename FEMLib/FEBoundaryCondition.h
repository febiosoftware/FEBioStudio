#pragma once
#include "FEStepComponent.h"
#include "MeshTools/FEGroup.h"
#include "MeshTools/FEItemListBuilder.h"
#include <list>
using namespace std;

class FEModel;

//-----------------------------------------------------------------------------
// Base class for all boundary conditions
// 

class FEBoundaryCondition : public FEStepComponent
{
public:
	enum {NAME, PARAMS, LIST, STEP};

public:
	FEBoundaryCondition(int ntype, FEModel* ps, int nstep = 0);
	FEBoundaryCondition(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep = 0);

	virtual ~FEBoundaryCondition(void);

	int Type() { return m_ntype; }
	
	FEItemListBuilder* GetItemList() { return m_pItem; }
	void SetItemList(FEItemListBuilder* pi) { m_pItem = pi; }

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	FEModel* GetFEModel() { return m_ps; }

protected:
	int			m_ntype;	// type of boundary condition
	FEModel*	m_ps;		// pointer to model

	FEItemListBuilder*	m_pItem;	// list of item indices to apply the BC too
};

//=============================================================================
// Base class for fixed and prescribed BCs
class FEEssentialBC : public FEBoundaryCondition
{
public:
	FEEssentialBC(int ntype, FEModel* fem, int nstep = 0) : FEBoundaryCondition(ntype, fem, nstep){}
	FEEssentialBC(int ntype, FEModel* fem, FEItemListBuilder* pi, int nstep) : FEBoundaryCondition(ntype, fem, pi, nstep){}
};

//=============================================================================
class FEFixedDOF : public FEEssentialBC
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
// This is the base class for all BC classes that define a LC
class FEPrescribedBC : public FEEssentialBC
{
public:
	FEPrescribedBC(int ntype, FEModel* ps, int nstep = 0) : FEEssentialBC(ntype, ps, nstep){}
	FEPrescribedBC(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep = 0) : FEEssentialBC(ntype, ps, pi, nstep) {}

public:
	virtual FELoadCurve* GetLoadCurve() = 0;
};

// TODO: Try to incorporate this class in FEPrescribedBC.
//       Problem is that some classes are derived from FEPrescribedBC although 
//       they should not (e.g. FENodalLoad)
class FEPrescribedDOF : public FEPrescribedBC
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
// NODAL LOADS
//=============================================================================
class FENodalLoad : public FEPrescribedBC
{
public:
	enum {BC, LOAD};

public:
	FENodalLoad(FEModel* ps);
	FENodalLoad(FEModel* ps, FEItemListBuilder* pi, int bc, double f, int nstep = 0);

	FELoadCurve* GetLoadCurve() { return GetParamLC(LOAD); }
	
	int GetBC() { return GetIntValue(BC); }
	void SetBC(int n) { SetIntValue(BC, n); }

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }
};

//=============================================================================
// Body loads
//=============================================================================
// Base class for all volumetric body loads
class FEBodyLoad : public FEBoundaryCondition
{
public:
	FEBodyLoad(int ntype, FEModel* ps, int nstep) : FEBoundaryCondition(ntype, ps, nstep) {}
};

//-----------------------------------------------------------------------------
// Body Force
class FEBodyForce : public FEBodyLoad
{
public:
	enum {LOAD1, LOAD2, LOAD3};

	FELoadCurve* GetLoadCurve(int n);

	double GetLoad(int n) { return GetFloatValue(LOAD1 + n); }

	void SetLoad(int n, double v) { SetFloatValue(LOAD1 + n, v); }

public:
	FEBodyForce(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// Heat Source
class FEHeatSource : public FEBodyLoad
{
public:
	enum {LOAD};

public:
	FEHeatSource(FEModel* ps, int nstep = 0);

	FELoadCurve* GetLoadCurve() { return GetParamLC(LOAD); }

	double GetLoad() { return GetFloatValue(LOAD); }
	void SetLoad(double v) { SetFloatValue(LOAD, v); }
};

//-----------------------------------------------------------------------------
// SBM source (experimental feature)
class FESBMPointSource : public FEBodyLoad
{
public:
	enum { SBM, VALUE, POS_X, POS_Y, POS_Z };

public:
	FESBMPointSource(FEModel* ps, int nstep = 0);
};
