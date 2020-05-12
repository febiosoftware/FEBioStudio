#include "stdafx.h"
#include "FELoad.h"
#include <FSCore/paramunit.h>

//=============================================================================
// NODAL LOAD
//=============================================================================
FENodalLoad::FENodalLoad(FEModel* ps) : FELoad(FE_NODAL_LOAD, ps)
{
	SetTypeString("Nodal Load");
	AddIntParam(0, "bc", "bc")->SetEnumNames("x-force\0y-force\0z-force\0");
	AddScienceParam(1.0, UNIT_FORCE, "load", "load")->SetLoadCurve();
}

//-----------------------------------------------------------------------------
FENodalLoad::FENodalLoad(FEModel* ps, FEItemListBuilder* pi, int bc, double f, int nstep) : FELoad(FE_NODAL_LOAD, ps, pi, nstep)
{
	SetTypeString("Nodal Load");
	AddIntParam(bc, "bc", "bc")->SetEnumNames("x-force\0y-force\0z-force\0");
	AddScienceParam(f, UNIT_FORCE, "load", "load")->SetLoadCurve();
}
