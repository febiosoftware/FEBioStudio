#include "stdafx.h"
#include "FELoad.h"

//=============================================================================
// NODAL LOAD
//=============================================================================
FENodalLoad::FENodalLoad(FEModel* ps) : FELoad(FE_NODAL_LOAD, ps)
{
	SetTypeString("Nodal Load");
	AddIntParam(0, "bc", "bc");
	AddDoubleParam(1, "load", "load")->SetLoadCurve();
}

//-----------------------------------------------------------------------------
FENodalLoad::FENodalLoad(FEModel* ps, FEItemListBuilder* pi, int bc, double f, int nstep) : FELoad(FE_NODAL_LOAD, ps, pi, nstep)
{
	SetTypeString("Nodal Load");
	AddIntParam(bc, "bc", "bc");
	AddDoubleParam(f, "load", "load")->SetLoadCurve();
}
