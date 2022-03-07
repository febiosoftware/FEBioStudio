#include "FEElementFormulation.h"

FEUT4Formulation::FEUT4Formulation(FSModel* fem)
{
	AddDoubleParam(0.0, "alpha");
	AddBoolParam(false, "iso_stab");
}

FEUDGHexFormulation::FEUDGHexFormulation(FSModel* fem)
{
	AddDoubleParam(0.0, "hg", "hourglass");
}

//========================================================================
FENewShellFormulation::FENewShellFormulation(FSModel* fem)
{
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
}

void FENewShellFormulation::setShellNormalNodal(bool b) { SetBoolValue(0, b); }
bool FENewShellFormulation::shellNormalNodal() const { return GetBoolValue(0); }

//========================================================================
FEElasticEASShellFormulation::FEElasticEASShellFormulation(FSModel* fem)
{
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
}

//========================================================================
FEElasticANSShellFormulation::FEElasticANSShellFormulation(FSModel* fem)
{
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
}

//========================================================================
FE3FieldShellFormulation::FE3FieldShellFormulation(FSModel* fem)
{
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
	AddBoolParam(false, "laugon", "incompressibility constraint");
	AddDoubleParam(0.01, "atol", "incompressibility tolerance");
}

void FE3FieldShellFormulation::setShellNormalNodal(bool b) { SetBoolValue(0, b); }
bool FE3FieldShellFormulation::shellNormalNodal() const { return GetBoolValue(0); }

void FE3FieldShellFormulation::setLaugon(bool b) { SetBoolValue(1, b); }
bool FE3FieldShellFormulation::laugon() const { return GetBoolValue(1); }

void FE3FieldShellFormulation::setAugTol(double d) { SetFloatValue(2, d); }
double FE3FieldShellFormulation::augTol() const { return GetFloatValue(2); }

//========================================================================
FEOldShellFormulation::FEOldShellFormulation(FSModel* fem)
{
}
