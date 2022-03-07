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

FEDefaultShellFormulation::FEDefaultShellFormulation(FSModel* fem)
{
	AddBoolParam(true, "shell_normal_nodal", "shell nodal normals");
	AddBoolParam(false, "laugon", "incompressibility constraint");
	AddDoubleParam(0.01, "atol", "incompressibility tolerance");
}

void FEDefaultShellFormulation::setShellNormalNodal(bool b)
{
	SetBoolValue(0, b);
}

bool FEDefaultShellFormulation::shellNormalNodal() const
{
	return GetBoolValue(0);
}

void FEDefaultShellFormulation::setLaugon(bool b)
{
	SetBoolValue(1, b);
}

bool FEDefaultShellFormulation::laugon() const
{
	return GetBoolValue(1);
}

void FEDefaultShellFormulation::setAugTol(double d)
{
	SetFloatValue(2, d);
}

double FEDefaultShellFormulation::augTol() const
{
	return GetFloatValue(2);
}
