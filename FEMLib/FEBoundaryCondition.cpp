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

#include "FEBoundaryCondition.h"
#include <MeshLib/FEMesh.h>
#include <MeshTools/FEModel.h>
#include <MeshTools/GGroup.h>
#include <FECore/units.h>
#include "enums.h"

//=============================================================================
FEFixedDOF::FEFixedDOF(int ntype, FSModel* fem) : FSBoundaryCondition(ntype, fem)
{
	m_nvar = -1;
	AddIntParam(0)->SetState(Param_HIDDEN); // the BC parameter
}

FEFixedDOF::FEFixedDOF(int ntype, FSModel* fem, FEItemListBuilder* pi, int nstep) : FSBoundaryCondition(ntype, fem, pi, nstep)
{
	m_nvar = -1;
	AddIntParam(0)->SetState(Param_HIDDEN); // the BC parameter
}

void FEFixedDOF::SetVarID(int nid)
{
	FSModel& fem = *GetFSModel();
	FEDOFVariable& var = fem.Variable(nid);
	m_nvar = nid;
	char sz[128]= {0};
	sprintf(sz, "$(%s)", var.name());
	GetParam(0).CopyEnumNames(sz);
}

//=============================================================================
FEFixedDisplacement::FEFixedDisplacement(FSModel* ps) : FEFixedDOF(FE_FIXED_DISPLACEMENT, ps)
{
	SetTypeString("Fixed Displacement");
	SetVarID(ps->GetVariableIndex("displacement"));
}

//-----------------------------------------------------------------------------
FEFixedDisplacement::FEFixedDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_DISPLACEMENT, ps, pi, nstep)
{
	SetTypeString("Fixed Displacement");
	SetVarID(ps->GetVariableIndex("displacement"));
	SetBC(bc);
}

//=============================================================================
FEFixedShellDisplacement::FEFixedShellDisplacement(FSModel* ps) : FEFixedDOF(FE_FIXED_SHELL_DISPLACEMENT, ps)
{
	SetTypeString("Fixed Shell Displacement");
	SetVarID(ps->GetVariableIndex("shell displacement"));
}

//-----------------------------------------------------------------------------
FEFixedShellDisplacement::FEFixedShellDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_SHELL_DISPLACEMENT, ps, pi, nstep)
{
	SetTypeString("Fixed Shell Displacement");
	SetVarID(ps->GetVariableIndex("shell displacement"));
	SetBC(bc);
}

//=============================================================================
FEFixedRotation::FEFixedRotation(FSModel* ps) : FEFixedDOF(FE_FIXED_ROTATION, ps)
{
	SetTypeString("Fixed Rotation");
	SetVarID(ps->GetVariableIndex("shell rotation"));
}

//-----------------------------------------------------------------------------
FEFixedRotation::FEFixedRotation(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_ROTATION, ps, pi, nstep)
{
	SetTypeString("Fixed Rotation");
	SetVarID(ps->GetVariableIndex("shell rotation"));
	SetBC(bc);
}

//=============================================================================
FEFixedFluidPressure::FEFixedFluidPressure(FSModel* ps) : FEFixedDOF(FE_FIXED_FLUID_PRESSURE, ps)
{
	SetTypeString("Fixed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("fluid pressure"));
}

//-----------------------------------------------------------------------------
FEFixedFluidPressure::FEFixedFluidPressure(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_FLUID_PRESSURE, ps, pi, nstep)
{
	SetTypeString("Fixed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("fluid pressure"));
	SetBC(bc);
}

//=============================================================================
FEFixedTemperature::FEFixedTemperature(FSModel* ps) : FEFixedDOF(FE_FIXED_TEMPERATURE, ps)
{
	SetTypeString("Fixed Temperature");
	SetVarID(ps->GetVariableIndex("temperature"));
}

//-----------------------------------------------------------------------------
FEFixedTemperature::FEFixedTemperature(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_TEMPERATURE, ps, pi, nstep)
{
	SetTypeString("Fixed Temperature");
	SetVarID(ps->GetVariableIndex("temperature"));
	SetBC(bc);
}

//=============================================================================
// FIXED CONCENTRATION
//=============================================================================
FEFixedConcentration::FEFixedConcentration(FSModel* ps) : FEFixedDOF(FE_FIXED_CONCENTRATION, ps)
{
	SetTypeString("Fixed Effective Concentration");
	SetVarID(ps->GetVariableIndex("concentration"));
}

//-----------------------------------------------------------------------------
FEFixedConcentration::FEFixedConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_CONCENTRATION, ps, pi, nstep)
{
	SetTypeString("Fixed Effective Concentration");
	SetVarID(ps->GetVariableIndex("concentration"));
	SetBC(bc);
}

//=============================================================================
// FIXED FLUID VELOCITY
//=============================================================================
FEFixedFluidVelocity::FEFixedFluidVelocity(FSModel* ps) : FEFixedDOF(FE_FIXED_FLUID_VELOCITY, ps)
{
    SetTypeString("Fixed Fluid Velocity");
	SetVarID(ps->GetVariableIndex("relative fluid velocity"));
}

//-----------------------------------------------------------------------------
FEFixedFluidVelocity::FEFixedFluidVelocity(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_FLUID_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fixed Fluid Velocity");
	SetVarID(ps->GetVariableIndex("relative fluid velocity"));
	SetBC(bc);
}

//=============================================================================
// FIXED FLUID DILATATION
//=============================================================================
FEFixedFluidDilatation::FEFixedFluidDilatation(FSModel* ps) : FEFixedDOF(FE_FIXED_DILATATION, ps)
{
    SetTypeString("Fixed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("fluid dilatation"));
}

//-----------------------------------------------------------------------------
FEFixedFluidDilatation::FEFixedFluidDilatation(FSModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_DILATATION, ps, pi, nstep)
{
    SetTypeString("Fixed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("fluid dilatation"));
	SetBC(bc);
}

//=============================================================================
// PRESCRIBED DOF
//=============================================================================

FEPrescribedDOF::FEPrescribedDOF(int ntype, FSModel* ps, int nstep) : FSBoundaryCondition(ntype, ps, nstep)
{
	m_nvar = -1;
	AddIntParam(0, "dof", "Degree of freedom")->SetState(Param_EDITABLE | Param_PERSISTENT);	// degree of freedom
	AddDoubleParam(1, "scale", "scale")->MakeVariable(true)->SetLoadCurve();
	AddBoolParam(0, "relative", "relative");
}

FEPrescribedDOF::FEPrescribedDOF(int ntype, FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FSBoundaryCondition(ntype, ps, pi, nstep)
{
	m_nvar = -1;
	AddIntParam(0, "dof", "Degree of freedom")->SetState(Param_EDITABLE | Param_PERSISTENT);	// degree of freedom
	AddDoubleParam(s, "scale", "scale")->MakeVariable(true)->SetLoadCurve();
	AddBoolParam(0, "relative", "relative");
	SetDOF(bc);
}

void FEPrescribedDOF::SetVarID(int nid)
{
	FSModel& fem = *GetFSModel();
	FEDOFVariable& var = fem.Variable(nid);
	m_nvar = nid;
	char sz[128] = { 0 };
	sprintf(sz, "$(%s)", var.name());
	GetParam(0).CopyEnumNames(sz);
}

// used only for reading parameters for old file formats ( < 2.0)
void FEPrescribedDOF::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: 
		{
			// The old format did not always use the BC value consistently.
			// Therefor it is possible that the BC value read may not fit in the variable's range.
			// Fortunately, in most such situations, the dof can be set to zero. 
			int ndof = p.GetIntValue();
			FSModel& fem = *GetFSModel();
			FEDOFVariable& var = fem.Variable(GetVarID());
			int dofs = var.DOFs();
			if ((ndof >= 0) && (ndof < dofs))
			{
				SetDOF(ndof);
			}
		}
		break;
	case 1: SetScaleFactor(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetRelativeFlag(p.GetIntValue() == 1); break;
	}
}

//=============================================================================
// PRESCRIBED DISPLACEMENT
//=============================================================================
FEPrescribedDisplacement::FEPrescribedDisplacement(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_DISPLACEMENT, ps)
{
	SetTypeString("Prescribed Displacement");
	SetVarID(ps->GetVariableIndex("Displacement"));
	SetScaleUnit(UNIT_LENGTH);
}

//-----------------------------------------------------------------------------
FEPrescribedDisplacement::FEPrescribedDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_DISPLACEMENT, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Displacement");
	SetVarID(ps->GetVariableIndex("displacement"));
	SetScaleUnit(UNIT_LENGTH);
}

//=============================================================================
// PRESCRIBED SHELL BACK FACE DISPLACEMENT
//=============================================================================
FEPrescribedShellDisplacement::FEPrescribedShellDisplacement(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_SHELL_DISPLACEMENT, ps)
{
	SetTypeString("Prescribed Shell Displacement");
	SetVarID(ps->GetVariableIndex("shell displacement"));
	SetScaleUnit(UNIT_LENGTH);
}

//-----------------------------------------------------------------------------
FEPrescribedShellDisplacement::FEPrescribedShellDisplacement(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_SHELL_DISPLACEMENT, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Shell Displacement");
	SetVarID(ps->GetVariableIndex("shell displacement"));
	SetScaleUnit(UNIT_LENGTH);
}

//=============================================================================
// PRESCRIBED ROTATION
//=============================================================================
FEPrescribedRotation::FEPrescribedRotation(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_ROTATION, ps)
{
	SetTypeString("Prescribed Rotation");
	SetVarID(ps->GetVariableIndex("Rotation"));
	SetScaleUnit(UNIT_RADIAN);
}

//-----------------------------------------------------------------------------
FEPrescribedRotation::FEPrescribedRotation(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_ROTATION, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Rotation");
	SetVarID(ps->GetVariableIndex("Rotation"));
	SetScaleUnit(UNIT_RADIAN);
}

//=============================================================================
// PRESCRIBED FLUID PRESSURE
//=============================================================================
FEPrescribedFluidPressure::FEPrescribedFluidPressure(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_FLUID_PRESSURE, ps)
{
	SetTypeString("Prescribed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("Effective Fluid Pressure"));
	SetScaleUnit(UNIT_PRESSURE);
}

//-----------------------------------------------------------------------------
FEPrescribedFluidPressure::FEPrescribedFluidPressure(FSModel* ps, FEItemListBuilder* pi, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_FLUID_PRESSURE, ps, pi, 0, s, nstep)
{
	SetTypeString("Prescribed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("fluid pressure"));
	SetScaleUnit(UNIT_PRESSURE);
}

//=============================================================================
// PRESCRIBED TEMPERATURE
//=============================================================================
FEPrescribedTemperature::FEPrescribedTemperature(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_TEMPERATURE, ps)
{
	SetTypeString("Prescribed Temperature");
	SetVarID(ps->GetVariableIndex("temperature"));
	SetScaleUnit(UNIT_TEMPERATURE);
}

//-----------------------------------------------------------------------------
FEPrescribedTemperature::FEPrescribedTemperature(FSModel* ps, FEItemListBuilder* pi, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_TEMPERATURE, ps, pi, 0, s, nstep)
{
	SetTypeString("Prescribed Temperature");
	SetVarID(ps->GetVariableIndex("temperature"));
	SetScaleUnit(UNIT_TEMPERATURE);
}

//=============================================================================
// PRESCRIBED SOLUTE CONCENTRATION
//=============================================================================
FEPrescribedConcentration::FEPrescribedConcentration(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_CONCENTRATION, ps)
{
	SetTypeString("Prescribed Effective Concentration");
	SetVarID(ps->GetVariableIndex("concentration"));
    SetScaleUnit(UNIT_CONCENTRATION);
}

//-----------------------------------------------------------------------------
FEPrescribedConcentration::FEPrescribedConcentration(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_CONCENTRATION, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Effective Concentration");
	SetVarID(ps->GetVariableIndex("concentration"));
    SetScaleUnit(UNIT_CONCENTRATION);
}

//=============================================================================
// PRESCRIBED FLUID VELOCITY
//=============================================================================
FEPrescribedFluidVelocity::FEPrescribedFluidVelocity(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_FLUID_VELOCITY, ps)
{
    SetTypeString("Prescribed Fluid Velocity");
    SetVarID(ps->GetVariableIndex("relative fluid velocity"));
	SetScaleUnit(UNIT_VELOCITY);
}

//-----------------------------------------------------------------------------
FEPrescribedFluidVelocity::FEPrescribedFluidVelocity(FSModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_FLUID_VELOCITY, ps, pi, bc, s, nstep)
{
    SetTypeString("Prescribed Fluid Velocity");
    SetVarID(ps->GetVariableIndex("relative fluid velocity"));
	SetScaleUnit(UNIT_VELOCITY);
}

//=============================================================================
// PRESCRIBED FLUID DILATATION
//=============================================================================
FEPrescribedFluidDilatation::FEPrescribedFluidDilatation(FSModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_DILATATION, ps)
{
    SetTypeString("Prescribed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("fluid dilatation"));
}

//-----------------------------------------------------------------------------
FEPrescribedFluidDilatation::FEPrescribedFluidDilatation(FSModel* ps, FEItemListBuilder* pi, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_DILATATION, ps, pi, 0, s, nstep)
{
    SetTypeString("Prescribed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("fluid dilatation"));
}

//=============================================================================
FEBioBoundaryCondition::FEBioBoundaryCondition(FSModel* ps) : FSBoundaryCondition(FE_FEBIO_BC, ps)
{
}

void FEBioBoundaryCondition::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSBoundaryCondition::Save(ar);
	}
	ar.EndChunk();
}

void FEBioBoundaryCondition::Load(IArchive& ar)
{
	TRACE("FEBioBoundaryCondition::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSBoundaryCondition::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
