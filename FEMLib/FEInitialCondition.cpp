/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include <FECore/units.h>

FSInitialCondition::FSInitialCondition(int ntype, FSModel* ps, int nstep) : FSDomainComponent(ntype, ps, nstep) 
{
	m_superClassID = FEIC_ID; 
	SetMeshItemType(FE_ALL_FLAGS);
}

FSInitialCondition::FSInitialCondition(int ntype, FSModel* ps, FSItemListBuilder* pi, int nstep) : FSDomainComponent(ntype, ps, pi, nstep) 
{ 
	m_superClassID = FEIC_ID; 
	SetMeshItemType(FE_ALL_FLAGS);
}

//-----------------------------------------------------------------------------
FSNodalVelocities::FSNodalVelocities(FSModel* ps) : FSInitialNodalDOF(FE_INIT_NODAL_VELOCITIES, ps)
{
	SetTypeString("Nodal Velocities");
	AddVecParam(vec3d(0, 0, 0), "value", "Velocity")->SetUnit(UNIT_VELOCITY);
}

FSNodalVelocities::FSNodalVelocities(FSModel* ps, FSItemListBuilder* pi, vec3d vel, int nstep) : FSInitialNodalDOF(FE_INIT_NODAL_VELOCITIES, ps, pi, nstep)
{
	SetTypeString("Nodal Velocities");
	AddVecParam(vel, "value", "Velocity")->SetUnit(UNIT_VELOCITY);;
}

//-----------------------------------------------------------------------------
FSNodalShellVelocities::FSNodalShellVelocities(FSModel* ps) : FSInitialNodalDOF(FE_INIT_NODAL_SHELL_VELOCITIES, ps)
{
	SetTypeString("Shell Nodal Velocities");
	AddVecParam(vec3d(0, 0, 0), "value", "Velocity")->SetUnit(UNIT_VELOCITY);
}

FSNodalShellVelocities::FSNodalShellVelocities(FSModel* ps, FSItemListBuilder* pi, vec3d vel, int nstep) : FSInitialNodalDOF(FE_INIT_NODAL_SHELL_VELOCITIES, ps, pi, nstep)
{
	SetTypeString("Shell Nodal Velocities");
	AddVecParam(vel, "value", "Velocity")->SetUnit(UNIT_VELOCITY);
}

//-----------------------------------------------------------------------------
FSInitConcentration::FSInitConcentration(FSModel* ps) : FSInitialNodalDOF(FE_INIT_CONCENTRATION, ps)
{
	SetTypeString("Initial effective concentration");
	AddDoubleParam(0, "value", "Value");
	AddChoiceParam(0, "dof", "Solute")->SetEnumNames("$(solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

FSInitConcentration::FSInitConcentration(FSModel* ps, FSItemListBuilder* pi, int bc, double val, int nstep) : FSInitialNodalDOF(FE_INIT_CONCENTRATION, ps, pi, nstep)
{
	SetTypeString("Initial effective concentration");
	AddDoubleParam(val, "value", "Value");
	AddChoiceParam(bc, "dof", "Solute")->SetEnumNames("$(solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//-----------------------------------------------------------------------------
FSInitShellConcentration::FSInitShellConcentration(FSModel* ps) : FSInitialNodalDOF(FE_INIT_SHELL_CONCENTRATION, ps)
{
    SetTypeString("Initial effective shell concentration");
    AddDoubleParam(0, "value", "Value");
    AddChoiceParam(0, "sol", "Solute")->SetEnumNames("$(solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

FSInitShellConcentration::FSInitShellConcentration(FSModel* ps, FSItemListBuilder* pi, int bc, double val, int nstep) : FSInitialNodalDOF(FE_INIT_SHELL_CONCENTRATION, ps, pi, nstep)
{
    SetTypeString("Initial effective shell concentration");
    AddDoubleParam(val, "value", "Value");
    AddChoiceParam(bc, "sol", "Solute")->SetEnumNames("$(solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//-----------------------------------------------------------------------------
FSInitFluidPressure::FSInitFluidPressure(FSModel* ps) : FSInitialNodalDOF(FE_INIT_FLUID_PRESSURE, ps)
{
	SetTypeString("Initial Fluid Pressure");
	AddDoubleParam(0, "value", "Fluid pressure")->SetUnit(UNIT_PRESSURE);
}

FSInitFluidPressure::FSInitFluidPressure(FSModel* ps, FSItemListBuilder* pi, double val, int nstep) : FSInitialNodalDOF(FE_INIT_FLUID_PRESSURE, ps, pi, nstep)
{
	SetTypeString("Initial Fluid Pressure");
	AddDoubleParam(val, "value", "Fluid pressure")->MakeVariable(true)->SetUnit(UNIT_PRESSURE);
}

//-----------------------------------------------------------------------------
FSInitShellFluidPressure::FSInitShellFluidPressure(FSModel* ps) : FSInitialNodalDOF(FE_INIT_SHELL_FLUID_PRESSURE, ps)
{
    SetTypeString("Initial Shell Fluid Pressure");
    AddDoubleParam(0, "Shell fluid pressure");
}

FSInitShellFluidPressure::FSInitShellFluidPressure(FSModel* ps, FSItemListBuilder* pi, double val, int nstep) : FSInitialNodalDOF(FE_INIT_SHELL_FLUID_PRESSURE, ps, pi, nstep)
{
    SetTypeString("Initial Shell Fluid Pressure");
    AddDoubleParam(val, "Shell fluid pressure");
}

//-----------------------------------------------------------------------------
FSInitTemperature::FSInitTemperature(FSModel* ps) : FSInitialNodalDOF(FE_INIT_TEMPERATURE, ps)
{
	SetTypeString("Initial Temperature");
	AddDoubleParam(0, "value", "Temperature")->SetUnit(UNIT_RELATIVE_TEMPERATURE);
}

FSInitTemperature::FSInitTemperature(FSModel* ps, FSItemListBuilder* pi, double val, int nstep) : FSInitialNodalDOF(FE_INIT_TEMPERATURE, ps, pi, nstep)
{
	SetTypeString("Initial Temperature");
	AddDoubleParam(val, "value", "Temperature")->SetUnit(UNIT_RELATIVE_TEMPERATURE);
}

//-----------------------------------------------------------------------------
FSInitFluidDilatation::FSInitFluidDilatation(FSModel* ps) : FSInitialNodalDOF(FE_INIT_FLUID_DILATATION, ps)
{
    SetTypeString("Initial Fluid Dilatation");
    AddDoubleParam(0, "Fluid dilatation")->SetUnit(UNIT_NONE);
}

FSInitFluidDilatation::FSInitFluidDilatation(FSModel* ps, FSItemListBuilder* pi, double val, int nstep) : FSInitialNodalDOF(FE_INIT_FLUID_DILATATION, ps, pi, nstep)
{
    SetTypeString("Initial Fluid Dilatation");
    AddDoubleParam(val, "Fluid dilatation")->SetUnit(UNIT_NONE);
}

//-----------------------------------------------------------------------------
FSInitPrestrain::FSInitPrestrain(FSModel* ps) : FSInitialCondition(FE_INIT_PRESTRAIN, ps)
{
	SetTypeString("Prestrain");

	AddBoolParam(true, "init");
	AddBoolParam(true, "reset");
}

//-----------------------------------------------------------------------------
FEBioInitialCondition::FEBioInitialCondition(FSModel* ps) : FSInitialCondition(FE_FEBIO_INITIAL_CONDITION, ps)
{

}

void FEBioInitialCondition::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSInitialCondition::Save(ar);
	}
	ar.EndChunk();
}

void FEBioInitialCondition::Load(IArchive& ar)
{
	TRACE("FEBioInitialCondition::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSInitialCondition::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
