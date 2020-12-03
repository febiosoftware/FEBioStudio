/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "FEInitialCondition.h"
#include <FSCore/paramunit.h>

//-----------------------------------------------------------------------------
FENodalVelocities::FENodalVelocities(FEModel* ps) : FEInitialNodalDOF(FE_NODAL_VELOCITIES, ps)
{
	SetTypeString("Nodal Velocities");
	AddVecParam(vec3d(0, 0, 0), "vel", "Velocity")->SetUnit(UNIT_VELOCITY);
}

FENodalVelocities::FENodalVelocities(FEModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep) : FEInitialNodalDOF(FE_NODAL_VELOCITIES, ps, pi, nstep)
{
	SetTypeString("Nodal Velocities");
	AddVecParam(vel, "vel", "Velocity")->SetUnit(UNIT_VELOCITY);;
}

//-----------------------------------------------------------------------------
FENodalShellVelocities::FENodalShellVelocities(FEModel* ps) : FEInitialNodalDOF(FE_NODAL_SHELL_VELOCITIES, ps)
{
	SetTypeString("Shell Nodal Velocities");
	AddVecParam(vec3d(0, 0, 0), "vel", "Velocity")->SetUnit(UNIT_VELOCITY);
}

FENodalShellVelocities::FENodalShellVelocities(FEModel* ps, FEItemListBuilder* pi, vec3d vel, int nstep) : FEInitialNodalDOF(FE_NODAL_SHELL_VELOCITIES, ps, pi, nstep)
{
	SetTypeString("Shell Nodal Velocities");
	AddVecParam(vel, "vel", "Velocity")->SetUnit(UNIT_VELOCITY);
}

//-----------------------------------------------------------------------------
FEInitConcentration::FEInitConcentration(FEModel* ps) : FEInitialNodalDOF(FE_INIT_CONCENTRATION, ps)
{
	SetTypeString("Initial effective concentration");
	AddDoubleParam(0, "value", "Value");
	AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

FEInitConcentration::FEInitConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep) : FEInitialNodalDOF(FE_INIT_CONCENTRATION, ps, pi, nstep)
{
	SetTypeString("Initial effective concentration");
	AddDoubleParam(val, "value", "Value");
	AddChoiceParam(bc, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//-----------------------------------------------------------------------------
FEInitShellConcentration::FEInitShellConcentration(FEModel* ps) : FEInitialNodalDOF(FE_INIT_SHELL_CONCENTRATION, ps)
{
    SetTypeString("Initial effective shell concentration");
    AddDoubleParam(0, "value", "Value");
    AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

FEInitShellConcentration::FEInitShellConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double val, int nstep) : FEInitialNodalDOF(FE_INIT_SHELL_CONCENTRATION, ps, pi, nstep)
{
    SetTypeString("Initial effective shell concentration");
    AddDoubleParam(val, "value", "Value");
    AddChoiceParam(bc, "sol", "Solute")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//-----------------------------------------------------------------------------
FEInitFluidPressure::FEInitFluidPressure(FEModel* ps) : FEInitialNodalDOF(FE_INIT_FLUID_PRESSURE, ps)
{
	SetTypeString("Initial Fluid Pressure");
	AddDoubleParam(0, "Fluid pressure")->SetUnit(UNIT_PRESSURE);
}

FEInitFluidPressure::FEInitFluidPressure(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialNodalDOF(FE_INIT_FLUID_PRESSURE, ps, pi, nstep)
{
	SetTypeString("Initial Fluid Pressure");
	AddDoubleParam(val, "value", "Fluid pressure")->MakeVariable(true)->SetUnit(UNIT_PRESSURE);
}

//-----------------------------------------------------------------------------
FEInitShellFluidPressure::FEInitShellFluidPressure(FEModel* ps) : FEInitialNodalDOF(FE_INIT_SHELL_FLUID_PRESSURE, ps)
{
    SetTypeString("Initial Shell Fluid Pressure");
    AddDoubleParam(0, "Shell fluid pressure");
}

FEInitShellFluidPressure::FEInitShellFluidPressure(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialNodalDOF(FE_INIT_SHELL_FLUID_PRESSURE, ps, pi, nstep)
{
    SetTypeString("Initial Shell Fluid Pressure");
    AddDoubleParam(val, "Shell fluid pressure");
}

//-----------------------------------------------------------------------------
FEInitTemperature::FEInitTemperature(FEModel* ps) : FEInitialNodalDOF(FE_INIT_TEMPERATURE, ps)
{
	SetTypeString("Initial Temperature");
	AddDoubleParam(0, "Temperature")->SetUnit(UNIT_TEMPERATURE);
}

FEInitTemperature::FEInitTemperature(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialNodalDOF(FE_INIT_TEMPERATURE, ps, pi, nstep)
{
	SetTypeString("Initial Temperature");
	AddDoubleParam(val, "Temperature")->SetUnit(UNIT_TEMPERATURE);
}

//-----------------------------------------------------------------------------
FEInitFluidDilatation::FEInitFluidDilatation(FEModel* ps) : FEInitialNodalDOF(FE_INIT_FLUID_DILATATION, ps)
{
    SetTypeString("Initial Fluid Dilatation");
    AddDoubleParam(0, "Fluid dilatation")->SetUnit(UNIT_NONE);
}

FEInitFluidDilatation::FEInitFluidDilatation(FEModel* ps, FEItemListBuilder* pi, double val, int nstep) : FEInitialNodalDOF(FE_INIT_FLUID_DILATATION, ps, pi, nstep)
{
    SetTypeString("Initial Fluid Dilatation");
    AddDoubleParam(val, "Fluid dilatation")->SetUnit(UNIT_NONE);
}

//-----------------------------------------------------------------------------
FEInitPrestrain::FEInitPrestrain(FEModel* ps) : FEInitialCondition(FE_INIT_PRESTRAIN, ps)
{
	SetTypeString("Prestrain");

	AddBoolParam(true, "init");
	AddBoolParam(true, "reset");
}
