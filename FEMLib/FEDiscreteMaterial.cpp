#include "stdafx.h"
#include "FEDiscreteMaterial.h"
#include <MeshTools/FEProject.h>

//===================================================================
FEDiscreteMaterial::FEDiscreteMaterial(int ntype) : FEMaterial(ntype)
{

}

//===================================================================
REGISTER_MATERIAL(FELinearSpringMaterial, MODULE_MECH, FE_DISCRETE_LINEAR_SPRING, FE_MAT_DISCRETE, "linear spring", 0);

FELinearSpringMaterial::FELinearSpringMaterial() : FEDiscreteMaterial(FE_DISCRETE_LINEAR_SPRING)
{
	AddDoubleParam(1, "E", "spring constant");
}

void FELinearSpringMaterial::SetSpringConstant(double E)
{
	SetFloatValue(0, E);
}

//===================================================================

REGISTER_MATERIAL(FENonLinearSpringMaterial, MODULE_MECH, FE_DISCRETE_NONLINEAR_SPRING, FE_MAT_DISCRETE, "nonlinear spring", 0);

FENonLinearSpringMaterial::FENonLinearSpringMaterial() : FEDiscreteMaterial(FE_DISCRETE_NONLINEAR_SPRING)
{
	AddDoubleParam(1, "force", "spring force")->SetLoadCurve();

	// create an initial linear ramp
	LOADPOINT p0(0, 0), p1(1, 1);
	GetParamLC(0)->Clear();
	GetParamLC(0)->Add(p0);
	GetParamLC(0)->Add(p1);
}

//===================================================================

REGISTER_MATERIAL(FEHillContractileMaterial, MODULE_MECH, FE_DISCRETE_HILL, FE_MAT_DISCRETE, "Hill", 0);

FEHillContractileMaterial::FEHillContractileMaterial() : FEDiscreteMaterial(FE_DISCRETE_HILL)
{
	AddDoubleParam(0, "Fmax", "Max force");
	AddDoubleParam(1, "Lmax", "Max length");
	AddDoubleParam(1, "L0"  , "Initial length");
	AddDoubleParam(1, "Ksh" , "Shape parameter");
	AddDoubleParam(0, "ac"  , "Activation");

	AddProperty("Sv" , FE_MAT_1DFUNC);
	AddProperty("Ftl", FE_MAT_1DFUNC);
	AddProperty("Fvl", FE_MAT_1DFUNC);

	AddProperty(0, new FE1DPointFunction);
	AddProperty(1, new FE1DPointFunction);
	AddProperty(2, new FE1DPointFunction);
}

//===================================================================

REGISTER_MATERIAL(FE1DPointFunction, MODULE_MECH, FE_FNC1D_POINT, FE_MAT_1DFUNC, "point", 0);

FE1DPointFunction::FE1DPointFunction() : FE1DFunction(FE_FNC1D_POINT) 
{
	// dummy parameter so we can use FEMaterial's serialization for the load curve
	AddDoubleParam(0, "points", "points")->SetLoadCurve();

	// constant value
	GetParamLC(0)->Clear();
	GetParamLC(0)->Add(0, 1);
	GetParamLC(0)->Add(1, 1);
}

FELoadCurve* FE1DPointFunction::GetPointCurve()
{
	return GetParamLC(0);
}
