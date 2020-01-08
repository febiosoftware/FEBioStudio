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
// Body Force
class FEBodyForce : public FEBodyLoad
{
public:
	enum { LOAD1, LOAD2, LOAD3 };

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

