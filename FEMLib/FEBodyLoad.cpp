#include "stdafx.h"
#include "FEBodyLoad.h"

//-----------------------------------------------------------------------------
FEBodyForce::FEBodyForce(FEModel* ps, int nstep) : FEBodyLoad(FE_BODY_FORCE, ps, nstep)
{
	SetTypeString("Body Force");
	AddDoubleParam(0, "x", "x")->SetLoadCurve();
	AddDoubleParam(0, "y", "y")->SetLoadCurve();
	AddDoubleParam(0, "z", "z")->SetLoadCurve();
}

FELoadCurve* FEBodyForce::GetLoadCurve(int n)
{
	return GetParamLC(LOAD1 + n);
}

//-----------------------------------------------------------------------------
FEHeatSource::FEHeatSource(FEModel* ps, int nstep) : FEBodyLoad(FE_HEAT_SOURCE, ps, nstep)
{
	SetTypeString("Heat Source");
	AddDoubleParam(0, "Q", "Q")->SetLoadCurve();
}

//-----------------------------------------------------------------------------
FESBMPointSource::FESBMPointSource(FEModel* ps, int nstep) : FEBodyLoad(FE_SBM_POINT_SOURCE, ps, nstep)
{
	SetTypeString("SBM Point Source");
	AddIntParam(1, "sbm", "sbm");
	AddDoubleParam(0, "value", "value");
	AddDoubleParam(0, "x", "x");
	AddDoubleParam(0, "y", "y");
	AddDoubleParam(0, "z", "z");
	AddBoolParam(true, "weigh_volume", "weigh volume");
}
