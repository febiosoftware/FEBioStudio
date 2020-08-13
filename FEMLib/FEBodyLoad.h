#pragma once
#include "FELoad.h"

//=============================================================================
// Body loads
//=============================================================================
// Base class for all volumetric body loads
class FEBodyLoad : public FELoad
{
public:
	FEBodyLoad(int ntype, FEModel* ps, int nstep) : FELoad(ntype, ps, 0, nstep) {}
};

//-----------------------------------------------------------------------------
// Constant Body Force (Obsolete)
class FEConstBodyForce : public FEBodyLoad
{
public:
	enum { FORCE_X, FORCE_Y, FORCE_Z };

	FELoadCurve* GetLoadCurve(int n);

	double GetLoad(int n);

	void SetLoad(int n, double v);

public:
	FEConstBodyForce(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// Non-constant Body Force (Obsolete)
class FENonConstBodyForce : public FEBodyLoad
{
public:
	enum { FORCE_X, FORCE_Y, FORCE_Z };

	FELoadCurve* GetLoadCurve(int n);

public:
	FENonConstBodyForce(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
// Heat Source
class FEHeatSource : public FEBodyLoad
{
public:
	enum { LOAD };

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

//-----------------------------------------------------------------------------
// Centrifugal body force
class FECentrifugalBodyForce : public FEBodyLoad
{
public:
    enum { ANGSPD, ROT_AXIS, ROT_CNTR };
    
public:
    FECentrifugalBodyForce(FEModel* ps, int nstep = 0);
    
    FELoadCurve* GetLoadCurve() { return GetParamLC(ANGSPD); }
    
    double GetLoad() { return GetFloatValue(ANGSPD); }
    void SetLoad(double v) { SetFloatValue(ANGSPD, v); }
};

