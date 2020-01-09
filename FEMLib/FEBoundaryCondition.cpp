#include "FEBoundaryCondition.h"
#include <MeshLib/FEMesh.h>
#include <MeshTools/FEModel.h>
#include <MeshTools/GGroup.h>

//-----------------------------------------------------------------------------
// FEModelComponent
//-----------------------------------------------------------------------------

FEModelComponent::FEModelComponent(int ntype, FEModel* ps, int nstep)
{
	m_ps = ps;
	m_pItem = 0;
	m_nstepID = nstep;
	m_ntype = ntype;
	m_sztype = "(not defined)";
}

FEModelComponent::FEModelComponent(int ntype, FEModel* ps, FEItemListBuilder* pi, int nstep)
{
	m_ps = ps;
	m_ntype = ntype;
	m_nstepID = nstep;
	m_pItem = pi;
	m_sztype = "(not defined)";
}

FEModelComponent::~FEModelComponent(void)
{
	if (m_pItem) delete m_pItem;
}

void FEModelComponent::Save(OArchive &ar)
{
	// write the name
	ar.WriteChunk(NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// write the step
	ar.WriteChunk(STEP, m_nstepID);

	// write the parameters
	ar.BeginChunk(PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

	// write the list
	if (m_pItem)
	{
		ar.BeginChunk(LIST);
		{
			ar.BeginChunk(m_pItem->Type());
			{
				m_pItem->Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------

void FEModelComponent::Load(IArchive &ar)
{
	TRACE("FEModelComponent::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case STEP: ar.read(m_nstepID); break;
		case PARAMS:
			ParamContainer::Load(ar);
			break;
		case LIST:
			{
				ar.OpenChunk();
				{
					int ntype = ar.GetChunkID();
					m_pItem = 0;
					switch (ntype)
					{
					case GO_NODE: m_pItem = new GNodeList(m_ps); break;
					case GO_EDGE: m_pItem = new GEdgeList(m_ps); break;
					case GO_FACE: m_pItem = new GFaceList(m_ps); break;
					case GO_PART: m_pItem = new GPartList(m_ps); break;
					case FE_NODESET: m_pItem = new FENodeSet((GObject*)0); break;
					case FE_SURFACE: m_pItem = new FESurface((GObject*)0); break;
					case FE_PART   : m_pItem = new FEPart   ((GObject*)0); break;
					default:
						assert(false);
						throw ReadError("Unknown FEItemListBuilder type in FEBoundaryCondition::Load");
					}
					m_pItem->Load(ar);

					// set the parent mesh for FEGroup's
					FEGroup* pg = dynamic_cast<FEGroup*>(m_pItem);
					if (pg)
					{
						if (m_ps->FindGroupParent(pg) == false) throw ReadError("Invalid mesh ID in FEBoundaryCondition::Load");
					}
				}
				ar.CloseChunk();
			}
			break;
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FEFixedDOF::FEFixedDOF(int ntype, FEModel* fem) : FEBoundaryCondition(ntype, fem)
{
	m_nvar = -1;
	AddIntParam(0)->SetState(Param_HIDDEN); // the BC parameter
}

FEFixedDOF::FEFixedDOF(int ntype, FEModel* fem, FEItemListBuilder* pi, int nstep) : FEBoundaryCondition(ntype, fem, pi, nstep)
{
	m_nvar = -1;
	AddIntParam(0)->SetState(Param_HIDDEN); // the BC parameter
}

void FEFixedDOF::SetVarID(int nid)
{
	FEModel& fem = *GetFEModel();
	FEDOFVariable& var = fem.Variable(nid);
	m_nvar = nid;
	char sz[128]= {0};
	sprintf(sz, "$(%s)", var.name());
	GetParam(0).CopyEnumNames(sz);
}

//=============================================================================
FEFixedDisplacement::FEFixedDisplacement(FEModel* ps) : FEFixedDOF(FE_FIXED_DISPLACEMENT, ps)
{
	SetTypeString("Fixed Displacement");
	SetVarID(ps->GetVariableIndex("Displacement"));
}

//-----------------------------------------------------------------------------
FEFixedDisplacement::FEFixedDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_DISPLACEMENT, ps, pi, nstep)
{
	SetTypeString("Fixed Displacement");
	SetVarID(ps->GetVariableIndex("Displacement"));
	SetBC(bc);
}

//=============================================================================
FEFixedShellDisplacement::FEFixedShellDisplacement(FEModel* ps) : FEFixedDOF(FE_FIXED_SHELL_DISPLACEMENT, ps)
{
	SetTypeString("Fixed Shell Displacement");
	SetVarID(ps->GetVariableIndex("Shell Displacement"));
}

//-----------------------------------------------------------------------------
FEFixedShellDisplacement::FEFixedShellDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_SHELL_DISPLACEMENT, ps, pi, nstep)
{
	SetTypeString("Fixed Shell Displacement");
	SetVarID(ps->GetVariableIndex("Shell Displacement"));
	SetBC(bc);
}

//=============================================================================
FEFixedRotation::FEFixedRotation(FEModel* ps) : FEFixedDOF(FE_FIXED_ROTATION, ps)
{
	SetTypeString("Fixed Rotation");
	SetVarID(ps->GetVariableIndex("Rotation"));
}

//-----------------------------------------------------------------------------
FEFixedRotation::FEFixedRotation(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_ROTATION, ps, pi, nstep)
{
	SetTypeString("Fixed Rotation");
	SetVarID(ps->GetVariableIndex("Rotation"));
	SetBC(bc);
}

//=============================================================================
FEFixedFluidPressure::FEFixedFluidPressure(FEModel* ps) : FEFixedDOF(FE_FIXED_FLUID_PRESSURE, ps)
{
	SetTypeString("Fixed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("Effective Fluid Pressure"));
}

//-----------------------------------------------------------------------------
FEFixedFluidPressure::FEFixedFluidPressure(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_FLUID_PRESSURE, ps, pi, nstep)
{
	SetTypeString("Fixed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("Effective Fluid Pressure"));
	SetBC(bc);
}

//=============================================================================
FEFixedTemperature::FEFixedTemperature(FEModel* ps) : FEFixedDOF(FE_FIXED_TEMPERATURE, ps)
{
	SetTypeString("Fixed Temperature");
	SetVarID(ps->GetVariableIndex("Temperature"));
}

//-----------------------------------------------------------------------------
FEFixedTemperature::FEFixedTemperature(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_TEMPERATURE, ps, pi, nstep)
{
	SetTypeString("Fixed Temperature");
	SetVarID(ps->GetVariableIndex("Temperature"));
	SetBC(bc);
}

//=============================================================================
// FIXED CONCENTRATION
//=============================================================================
FEFixedConcentration::FEFixedConcentration(FEModel* ps) : FEFixedDOF(FE_FIXED_CONCENTRATION, ps)
{
	SetTypeString("Fixed Effective Concentration");
	SetVarID(ps->GetVariableIndex("Effective Solute Concentration"));
}

//-----------------------------------------------------------------------------
FEFixedConcentration::FEFixedConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_CONCENTRATION, ps, pi, nstep)
{
	SetTypeString("Fixed Effective Concentration");
	SetVarID(ps->GetVariableIndex("Effective Solute Concentration"));
	SetBC(bc);
}

//=============================================================================
// FIXED FLUID VELOCITY
//=============================================================================
FEFixedFluidVelocity::FEFixedFluidVelocity(FEModel* ps) : FEFixedDOF(FE_FIXED_FLUID_VELOCITY, ps)
{
    SetTypeString("Fixed Fluid Velocity");
	SetVarID(ps->GetVariableIndex("Fluid Velocity"));
}

//-----------------------------------------------------------------------------
FEFixedFluidVelocity::FEFixedFluidVelocity(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_FLUID_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fixed Fluid Velocity");
	SetVarID(ps->GetVariableIndex("Fluid Velocity"));
	SetBC(bc);
}

//=============================================================================
// FIXED FLUID DILATATION
//=============================================================================
FEFixedFluidDilatation::FEFixedFluidDilatation(FEModel* ps) : FEFixedDOF(FE_FIXED_DILATATION, ps)
{
    SetTypeString("Fixed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("Fluid Dilatation"));
}

//-----------------------------------------------------------------------------
FEFixedFluidDilatation::FEFixedFluidDilatation(FEModel* ps, FEItemListBuilder* pi, int bc, int nstep) : FEFixedDOF(FE_FIXED_DILATATION, ps, pi, nstep)
{
    SetTypeString("Fixed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("Fluid Dilatation"));
	SetBC(bc);
}

//=============================================================================
// PRESCRIBED DOF
//=============================================================================

FEPrescribedDOF::FEPrescribedDOF(int ntype, FEModel* ps, int nstep) : FEBoundaryCondition(ntype, ps, nstep)
{
	m_nvar = -1;
	AddIntParam(0, "dof", "Degree of freedom")->SetState(Param_EDITABLE | Param_PERSISTENT);	// degree of freedom
	AddDoubleParam(1, "scale", "scale")->MakeVariable(true)->SetLoadCurve();
	AddBoolParam(0, "relative", "relative");
}

FEPrescribedDOF::FEPrescribedDOF(int ntype, FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEBoundaryCondition(ntype, ps, pi, nstep)
{
	m_nvar = -1;
	AddIntParam(0, "dof", "Degree of freedom")->SetState(Param_EDITABLE | Param_PERSISTENT);	// degree of freedom
	AddDoubleParam(s, "scale", "scale")->MakeVariable(true)->SetLoadCurve();
	AddBoolParam(0, "relative", "relative");
	SetDOF(bc);
}

void FEPrescribedDOF::SetVarID(int nid)
{
	FEModel& fem = *GetFEModel();
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
			FEModel& fem = *GetFEModel();
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
FEPrescribedDisplacement::FEPrescribedDisplacement(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_DISPLACEMENT, ps)
{
	SetTypeString("Prescribed Displacement");
	SetVarID(ps->GetVariableIndex("Displacement"));
}

//-----------------------------------------------------------------------------
FEPrescribedDisplacement::FEPrescribedDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_DISPLACEMENT, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Displacement");
	SetVarID(ps->GetVariableIndex("Displacement"));
}

//=============================================================================
// PRESCRIBED SHELL BACK FACE DISPLACEMENT
//=============================================================================
FEPrescribedShellDisplacement::FEPrescribedShellDisplacement(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_SHELL_DISPLACEMENT, ps)
{
	SetTypeString("Prescribed Shell Displacement");
	SetVarID(ps->GetVariableIndex("Shell Displacement"));
}

//-----------------------------------------------------------------------------
FEPrescribedShellDisplacement::FEPrescribedShellDisplacement(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_SHELL_DISPLACEMENT, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Shell Displacement");
	SetVarID(ps->GetVariableIndex("Shell Displacement"));
}

//=============================================================================
// PRESCRIBED ROTATION
//=============================================================================
FEPrescribedRotation::FEPrescribedRotation(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_ROTATION, ps)
{
	SetTypeString("Prescribed Rotation");
	SetVarID(ps->GetVariableIndex("Rotation"));
}

//-----------------------------------------------------------------------------
FEPrescribedRotation::FEPrescribedRotation(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_ROTATION, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Rotation");
	SetVarID(ps->GetVariableIndex("Rotation"));
}

//=============================================================================
// PRESCRIBED FLUID PRESSURE
//=============================================================================
FEPrescribedFluidPressure::FEPrescribedFluidPressure(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_FLUID_PRESSURE, ps)
{
	SetTypeString("Prescribed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("Effective Fluid Pressure"));
}

//-----------------------------------------------------------------------------
FEPrescribedFluidPressure::FEPrescribedFluidPressure(FEModel* ps, FEItemListBuilder* pi, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_FLUID_PRESSURE, ps, pi, 0, s, nstep)
{
	SetTypeString("Prescribed Effective Fluid Pressure");
	SetVarID(ps->GetVariableIndex("Effective Fluid Pressure"));
}

//=============================================================================
// PRESCRIBED TEMPERATURE
//=============================================================================
FEPrescribedTemperature::FEPrescribedTemperature(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_TEMPERATURE, ps)
{
	SetTypeString("Prescribed Temperature");
	SetVarID(ps->GetVariableIndex("Temperature"));
}

//-----------------------------------------------------------------------------
FEPrescribedTemperature::FEPrescribedTemperature(FEModel* ps, FEItemListBuilder* pi, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_TEMPERATURE, ps, pi, 0, s, nstep)
{
	SetTypeString("Prescribed Temperature");
	SetVarID(ps->GetVariableIndex("Temperature"));
}

//=============================================================================
// PRESCRIBED SOLUTE CONCENTRATION
//=============================================================================
FEPrescribedConcentration::FEPrescribedConcentration(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_CONCENTRATION, ps)
{
	SetTypeString("Prescribed Effective Concentration");
	SetVarID(ps->GetVariableIndex("Effective Solute Concentration"));
}

//-----------------------------------------------------------------------------
FEPrescribedConcentration::FEPrescribedConcentration(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_CONCENTRATION, ps, pi, bc, s, nstep)
{
	SetTypeString("Prescribed Effective Concentration");
	SetVarID(ps->GetVariableIndex("Effective Solute Concentration"));
}

//=============================================================================
// PRESCRIBED FLUID VELOCITY
//=============================================================================
FEPrescribedFluidVelocity::FEPrescribedFluidVelocity(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_FLUID_VELOCITY, ps)
{
    SetTypeString("Prescribed Fluid Velocity");
    SetVarID(ps->GetVariableIndex("Fluid Velocity"));
}

//-----------------------------------------------------------------------------
FEPrescribedFluidVelocity::FEPrescribedFluidVelocity(FEModel* ps, FEItemListBuilder* pi, int bc, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_SOLID_VELOCITY, ps, pi, bc, s, nstep)
{
    SetTypeString("Prescribed Fluid Velocity");
    SetVarID(ps->GetVariableIndex("Fluid Velocity"));
}

//=============================================================================
// PRESCRIBED FLUID DILATATION
//=============================================================================
FEPrescribedFluidDilatation::FEPrescribedFluidDilatation(FEModel* ps) : FEPrescribedDOF(FE_PRESCRIBED_DILATATION, ps)
{
    SetTypeString("Prescribed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("Fluid Dilatation"));
}

//-----------------------------------------------------------------------------
FEPrescribedFluidDilatation::FEPrescribedFluidDilatation(FEModel* ps, FEItemListBuilder* pi, double s, int nstep) : FEPrescribedDOF(FE_PRESCRIBED_DILATATION, ps, pi, 0, s, nstep)
{
    SetTypeString("Prescribed Fluid Dilatation");
	SetVarID(ps->GetVariableIndex("Fluid Dilatation"));
}
