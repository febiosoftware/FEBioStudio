#include "FEInitialCondition.h"
#include <FSCore/paramunit.h>

//-----------------------------------------------------------------------------
FENodalVelocities::FENodalVelocities(FEModel* ps) : FEInitialCondition(FE_NODAL_VELOCITIES, ps)
{
	SetTypeString("Nodal Velocities");
	AddVecParam(vec3d(0, 0, 0), "vel", "Velocity")->SetUnit(UNIT_VELOCITY);
}

FENodalVelocities::FENodalVelocities(FEModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep) : FEInitialCondition(FE_NODAL_VELOCITIES, ps, pi, nstep)
{
	SetTypeString("Nodal Velocities");
	AddVecParam(vel, "vel", "Velocity")->SetUnit(UNIT_VELOCITY);;
}

//-----------------------------------------------------------------------------
FENodalShellVelocities::FENodalShellVelocities(FEModel* ps) : FEInitialCondition(FE_NODAL_SHELL_VELOCITIES, ps)
{
	SetTypeString("Shell Nodal Velocities");
	AddVecParam(vec3d(0, 0, 0), "vel", "Velocity")->SetUnit(UNIT_VELOCITY);
}

FENodalShellVelocities::FENodalShellVelocities(FEModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep) : FEInitialCondition(FE_NODAL_SHELL_VELOCITIES, ps, pi, nstep)
{
	SetTypeString("Shell Nodal Velocities");
	AddVecParam(vel, "vel", "Velocity")->SetUnit(UNIT_VELOCITY);
}

//-----------------------------------------------------------------------------
FEInitConcentration::FEInitConcentration(FEModel* ps) : FEInitialCondition(FE_INIT_CONCENTRATION, ps)
{
	SetTypeString("Initial effective concentration");
	AddDoubleParam(0, "value", "Value");
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

FEInitConcentration::FEInitConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep) : FEInitialCondition(FE_INIT_CONCENTRATION, ps, pi, nstep)
{
	SetTypeString("Initial effective concentration");
	AddDoubleParam(val, "value", "Value");
	AddChoiceParam(bc, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//-----------------------------------------------------------------------------
FEInitShellConcentration::FEInitShellConcentration(FEModel* ps) : FEInitialCondition(FE_INIT_SHELL_CONCENTRATION, ps)
{
    SetTypeString("Initial effective shell concentration");
    AddDoubleParam(0, "value", "Value");
    AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

FEInitShellConcentration::FEInitShellConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep) : FEInitialCondition(FE_INIT_SHELL_CONCENTRATION, ps, pi, nstep)
{
    SetTypeString("Initial effective shell concentration");
    AddDoubleParam(val, "value", "Value");
    AddChoiceParam(bc, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//-----------------------------------------------------------------------------
FEInitFluidPressure::FEInitFluidPressure(FEModel* ps) : FEInitialCondition(FE_INIT_FLUID_PRESSURE, ps)
{
	SetTypeString("Initial Fluid Pressure");
	AddDoubleParam(0, "Fluid pressure")->SetUnit(UNIT_PRESSURE);
}

FEInitFluidPressure::FEInitFluidPressure(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialCondition(FE_INIT_FLUID_PRESSURE, ps, pi, nstep)
{
	SetTypeString("Initial Fluid Pressure");
	AddDoubleParam(val, "Fluid pressure")->SetUnit(UNIT_PRESSURE);
}

//-----------------------------------------------------------------------------
FEInitShellFluidPressure::FEInitShellFluidPressure(FEModel* ps) : FEInitialCondition(FE_INIT_SHELL_FLUID_PRESSURE, ps)
{
    SetTypeString("Initial Shell Fluid Pressure");
    AddDoubleParam(0, "Shell fluid pressure");
}

FEInitShellFluidPressure::FEInitShellFluidPressure(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialCondition(FE_INIT_SHELL_FLUID_PRESSURE, ps, pi, nstep)
{
    SetTypeString("Initial Shell Fluid Pressure");
    AddDoubleParam(val, "Shell fluid pressure");
}

//-----------------------------------------------------------------------------
FEInitTemperature::FEInitTemperature(FEModel* ps) : FEInitialCondition(FE_INIT_TEMPERATURE, ps)
{
	SetTypeString("Initial Temperature");
	AddDoubleParam(0, "Temperature")->SetUnit(UNIT_TEMPERATURE);
}

FEInitTemperature::FEInitTemperature(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialCondition(FE_INIT_TEMPERATURE, ps, pi, nstep)
{
	SetTypeString("Initial Temperature");
	AddDoubleParam(val, "Temperature")->SetUnit(UNIT_TEMPERATURE);
}

//-----------------------------------------------------------------------------
FEInitFluidDilatation::FEInitFluidDilatation(FEModel* ps) : FEInitialCondition(FE_INIT_FLUID_DILATATION, ps)
{
    SetTypeString("Initial Fluid Dilatation");
    AddDoubleParam(0, "Fluid dilatation")->SetUnit(UNIT_NONE);
}

FEInitFluidDilatation::FEInitFluidDilatation(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialCondition(FE_INIT_FLUID_DILATATION, ps, pi, nstep)
{
    SetTypeString("Initial Fluid Dilatation");
    AddDoubleParam(val, "Fluid dilatation")->SetUnit(UNIT_NONE);
}
