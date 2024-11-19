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

#include "FEAnalysisStep.h"
#include "FSModel.h"
#include "FERigidConstraint.h"
#include "FERigidLoad.h"
#include "FSProject.h"
#include "FEInitialCondition.h"
#include "FESurfaceLoad.h"
#include "FEMKernel.h"
#include "FEInterface.h"
#include <FEBioLink/FEBioInterface.h>
#include "FEModelConstraint.h"
#include <FSCore/FSObjectList.h>
using namespace std;

int FSStep::m_ncount = 0;

//-----------------------------------------------------------------------------
// FSStep
//-----------------------------------------------------------------------------

class FSStep::Imp
{
public:
	// boundary conditions
	FSObjectList<FSBoundaryCondition>	m_BC;

	// loads
	FSObjectList<FSLoad>	m_FC;

	// initial condition
	FSObjectList<FSInitialCondition>		m_IC;

	// contact interfaces
	FSObjectList<FSInterface>	m_Int;

	// constraints
	FSObjectList<FSModelConstraint>	m_NLC;

	// rigid constraints (obsolete)
	FSObjectList<FSRigidConstraint>	m_RC;

	// rigid loads
	FSObjectList<FSRigidLoad>	m_RL;

	// rigid BCs
	FSObjectList<FSRigidBC>		m_RBC;

	// rigid ICs
	FSObjectList<FSRigidIC>		m_RIC;

	// linear constraints
	FSObjectList<FSLinearConstraintSet>	m_LC;

	// rigid connectors (nonlinear constraints)
	FSObjectList<FSRigidConnector>	m_CN;

	// mesh adaptors
	FSObjectList<FSMeshAdaptor>	m_MA;
};


FSStep::FSStep(FSModel* ps, int ntype) : FSModelComponent(ps), m_ntype(ntype), imp(new FSStep::Imp)
{
	m_nID = ++m_ncount;
	m_superClassID = FEANALYSIS_ID;
}

//-----------------------------------------------------------------------------
FSStep::~FSStep()
{
	delete imp;
}

//-----------------------------------------------------------------------------
void FSStep::ResetCounter()
{
	m_ncount = 0;
}

//-----------------------------------------------------------------------------
void FSStep::DecreaseCounter()
{
	m_ncount--;
	assert(m_ncount >= 0);
}

//-----------------------------------------------------------------------------
void FSStep::SetCounter(int n)
{
	m_ncount = n;
}

//-----------------------------------------------------------------------------
int FSStep::GetCounter()
{
	return m_ncount;
}

//-----------------------------------------------------------------------------

void FSStep::SetID(int nid)
{
	if (nid > m_ncount) m_ncount = nid;
	m_nID = nid;
}

//-----------------------------------------------------------------------------
int FSStep::BCs() { return (int) imp->m_BC.Size(); }

//-----------------------------------------------------------------------------
int FSStep::ActiveBCs()
{
	int n = 0;
	for (int i = 0; i < imp->m_BC.Size(); ++i)
		if (imp->m_BC[i]->IsActive()) n++;
	return n;
}

//-----------------------------------------------------------------------------
FSBoundaryCondition* FSStep::BC(int i) { return imp->m_BC[i]; }

//-----------------------------------------------------------------------------
void FSStep::AddBC(FSBoundaryCondition* pbc)
{ 
	imp->m_BC.Add(pbc);
	pbc->SetStep(GetID());
}

//-----------------------------------------------------------------------------
void FSStep::InsertBC(int n, FSBoundaryCondition* pbc)
{ 
	imp->m_BC.Insert(n, pbc);
	pbc->SetStep(GetID());
}

//-----------------------------------------------------------------------------
int FSStep::RemoveBC(FSBoundaryCondition* pbc)
{
	return (int)imp->m_BC.Remove(pbc);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllBCs()
{
	imp->m_BC.Clear();
}

//-----------------------------------------------------------------------------
FSBoundaryCondition* FSStep::ReplaceBC(int n, FSBoundaryCondition* newBC)
{
	return imp->m_BC.Replace(n, newBC);
}

//-----------------------------------------------------------------------------
int FSStep::Loads() { return (int)imp->m_FC.Size(); }

//-----------------------------------------------------------------------------
FSLoad* FSStep::Load(int i) { return imp->m_FC[i]; }

//-----------------------------------------------------------------------------
void FSStep::AddLoad(FSLoad* pfc)
{ 
	imp->m_FC.Add(pfc);
	pfc->SetStep(GetID()); 
}

//-----------------------------------------------------------------------------
void FSStep::InsertLoad(int n, FSLoad* pfc)
{ 
	imp->m_FC.Insert(n, pfc);
	pfc->SetStep(GetID());
}

//-----------------------------------------------------------------------------
int FSStep::RemoveLoad(FSLoad* pfc)
{
	return (int)imp->m_FC.Remove(pfc);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllLoads()
{
	imp->m_FC.Clear();
}

//-----------------------------------------------------------------------------
FSLoad* FSStep::ReplaceLoad(int n, FSLoad* pl)
{
	return imp->m_FC.Replace(n, pl);
}

//-----------------------------------------------------------------------------
int FSStep::ICs() { return (int)imp->m_IC.Size(); }

//-----------------------------------------------------------------------------
FSInitialCondition* FSStep::IC(int i) { return imp->m_IC[i]; }

//-----------------------------------------------------------------------------
void FSStep::AddIC(FSInitialCondition* pic)
{
	imp->m_IC.Add(pic);
	pic->SetStep(GetID());
}

void FSStep::InsertIC(int n, FSInitialCondition* pic)
{ 
	imp->m_IC.Insert(n, pic);
	pic->SetStep(GetID());
}

//-----------------------------------------------------------------------------
int FSStep::RemoveIC(FSInitialCondition* pic)
{
	return (int)imp->m_IC.Remove(pic);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllICs()
{
	imp->m_IC.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::Interfaces() { return (int)imp->m_Int.Size(); }

//-----------------------------------------------------------------------------
FSInterface* FSStep::Interface(int i) { return imp->m_Int[i]; }

//-----------------------------------------------------------------------------
void FSStep::AddInterface(FSInterface* pi)
{ 
	imp->m_Int.Add(pi);
	pi->SetStep(GetID());
}

//-----------------------------------------------------------------------------
void FSStep::InsertInterface(int n, FSInterface* pi)
{ 
	imp->m_Int.Insert(n, pi);
	pi->SetStep(GetID());
}

//-----------------------------------------------------------------------------
int FSStep::RemoveInterface(FSInterface* pi)
{
	return (int)imp->m_Int.Remove(pi);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllInterfaces()
{
	imp->m_Int.Clear();
}

//-----------------------------------------------------------------------------
void FSStep::ReplaceInterface(FSInterface* pold, FSInterface* pnew)
{
	imp->m_Int.Replace(pold, pnew);
}

//-----------------------------------------------------------------------------
int FSStep::Constraints()
{
	return (int) imp->m_NLC.Size();
}

//-----------------------------------------------------------------------------
int FSStep::Constraints(int ntype)
{
	int nc = 0;
	for (int i = 0; i<(int)imp->m_NLC.Size(); ++i)
	{
		if (imp->m_NLC[i]->Type() == ntype) nc++;
	}
	return nc;
}

//-----------------------------------------------------------------------------
FSModelConstraint* FSStep::Constraint(int i)
{
	return imp->m_NLC[i];
}

//-----------------------------------------------------------------------------
void FSStep::AddConstraint(FSModelConstraint* pc)
{
	imp->m_NLC.Add(pc);
	pc->SetStep(GetID());
}

//-----------------------------------------------------------------------------
void FSStep::InsertConstraint(int n, FSModelConstraint* pc)
{
	imp->m_NLC.Insert(n, pc);
	pc->SetStep(GetID());
}

//-----------------------------------------------------------------------------
void FSStep::RemoveConstraint(FSModelConstraint* pc)
{
	imp->m_NLC.Remove(pc);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllConstraints()
{
	imp->m_NLC.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::RigidConstraints() { return (int)imp->m_RC.Size(); }

//-----------------------------------------------------------------------------
FSRigidConstraint* FSStep::RigidConstraint(int i) { return imp->m_RC[i]; }

//-----------------------------------------------------------------------------
int FSStep::RigidConstraints(int ntype)
{
	int nc = 0;
	for (int i = 0; i<(int)imp->m_RC.Size(); ++i)
	{
		if (imp->m_RC[i]->Type() == ntype) nc++;
	}
	return nc;
}

//-----------------------------------------------------------------------------
void FSStep::AddRC(FSRigidConstraint* prc)
{ 
	imp->m_RC.Add(prc);
	prc->SetStep(GetID());
}

//-----------------------------------------------------------------------------
void FSStep::InsertRC(int n, FSRigidConstraint* prc)
{ 
	imp->m_RC.Insert(n, prc);
	prc->SetStep(GetID());
}

//-----------------------------------------------------------------------------
int FSStep::RemoveRC(FSRigidConstraint* prc)
{
	return (int)imp->m_RC.Remove(prc);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllRigidConstraints()
{
	imp->m_RC.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::RigidLoads()
{
	return imp->m_RL.Size();
}

int FSStep::RigidLoads(int ntype)
{
	int nrl = 0;
	for (int i = 0; i < (int)imp->m_RL.Size(); ++i)
	{
		if (imp->m_RL[i]->Type() == ntype) nrl++;
	}
	return nrl;
}

FSRigidLoad* FSStep::RigidLoad(int i)
{
	return imp->m_RL[i];
}

void FSStep::AddRigidLoad(FSRigidLoad* prc)
{
	imp->m_RL.Add(prc);
}

void FSStep::InsertRigidLoad(int n, FSRigidLoad* prc)
{
	imp->m_RL.Insert(n, prc);
}

int FSStep::RemoveRigidLoad(FSRigidLoad* prc)
{
	return imp->m_RL.Remove(prc);
}

void FSStep::RemoveAllRigidLoads()
{
	imp->m_RL.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::RigidBCs()
{
	return imp->m_RBC.Size();
}

int FSStep::RigidBCs(int ntype)
{
	int nrl = 0;
	for (int i = 0; i < (int)imp->m_RBC.Size(); ++i)
	{
		if (imp->m_RBC[i]->Type() == ntype) nrl++;
	}
	return nrl;
}

FSRigidBC* FSStep::RigidBC(int i)
{
	return imp->m_RBC[i];
}

void FSStep::AddRigidBC(FSRigidBC* prc)
{
	imp->m_RBC.Add(prc);
}

void FSStep::InsertRigidBC(int n, FSRigidBC* prc)
{
	imp->m_RBC.Insert(n, prc);
}

int FSStep::RemoveRigidBC(FSRigidBC* prc)
{
	return imp->m_RBC.Remove(prc);
}

void FSStep::RemoveAllRigidBCs()
{
	imp->m_RBC.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::RigidICs()
{
	return imp->m_RIC.Size();
}

int FSStep::RigidICs(int ntype)
{
	int nrl = 0;
	for (int i = 0; i < (int)imp->m_RIC.Size(); ++i)
	{
		if (imp->m_RIC[i]->Type() == ntype) nrl++;
	}
	return nrl;
}

FSRigidIC* FSStep::RigidIC(int i)
{
	return imp->m_RIC[i];
}

void FSStep::AddRigidIC(FSRigidIC* prc)
{
	imp->m_RIC.Add(prc);
}

void FSStep::InsertRigidIC(int n, FSRigidIC* prc)
{
	imp->m_RIC.Insert(n, prc);
}

int FSStep::RemoveRigidIC(FSRigidIC* prc)
{
	return imp->m_RIC.Remove(prc);
}

void FSStep::RemoveAllRigidICs()
{
	imp->m_RIC.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::LinearConstraints() { return (int)imp->m_LC.Size(); }

//-----------------------------------------------------------------------------
FSLinearConstraintSet* FSStep::LinearConstraint(int i) { return imp->m_LC[i]; }

//-----------------------------------------------------------------------------
void FSStep::AddLinearConstraint(FSLinearConstraintSet* plc)
{ 
	imp->m_LC.Add(plc);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllLinearConstraints()
{
	imp->m_LC.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::RigidConnectors() { return (int)imp->m_CN.Size(); }

//-----------------------------------------------------------------------------
FSRigidConnector* FSStep::RigidConnector(int i) { return imp->m_CN[i]; }

//-----------------------------------------------------------------------------
void FSStep::AddRigidConnector(FSRigidConnector* pi)
{
	imp->m_CN.Add(pi);
	pi->SetStep(GetID());
}

//-----------------------------------------------------------------------------
void FSStep::InsertRigidConnector(int n, FSRigidConnector* pi)
{ 
	imp->m_CN.Insert(n, pi);
	pi->SetStep(GetID());
}

//-----------------------------------------------------------------------------
int FSStep::RemoveRigidConnector(FSRigidConnector* pi)
{
	return (int)imp->m_CN.Remove(pi);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllRigidConnectors()
{
	imp->m_CN.Clear();
}

//-----------------------------------------------------------------------------
int FSStep::MeshAdaptors() { return (int)imp->m_MA.Size(); }

//-----------------------------------------------------------------------------
FSMeshAdaptor* FSStep::MeshAdaptor(int i) { return imp->m_MA[i]; }

//-----------------------------------------------------------------------------
void FSStep::AddMeshAdaptor(FSMeshAdaptor* pi)
{
	imp->m_MA.Add(pi);
	pi->SetStep(GetID());
}

//-----------------------------------------------------------------------------
void FSStep::InsertMeshAdaptor(int n, FSMeshAdaptor* pi)
{
	imp->m_MA.Insert(n, pi);
	pi->SetStep(GetID());
}

//-----------------------------------------------------------------------------
int FSStep::RemoveMeshAdaptor(FSMeshAdaptor* pi)
{
	return (int)imp->m_MA.Remove(pi);
}

//-----------------------------------------------------------------------------
void FSStep::RemoveAllMeshAdaptors()
{
	imp->m_MA.Clear();
}

//-----------------------------------------------------------------------------
#define MoveComponent(Type, Fnc) (dynamic_cast<Type*>(pc)) Fnc(dynamic_cast<Type*>(pc))

void FSStep::AddComponent(FSStepComponent* pc)
{
	// remove it from the old step
	pc->SetStep(GetID());
	if      MoveComponent(FSBoundaryCondition  , AddBC);
	else if MoveComponent(FSLoad               , AddLoad);
	else if MoveComponent(FSInitialCondition   , AddIC);
	else if MoveComponent(FSInterface          , AddInterface);
	else if MoveComponent(FSModelConstraint    , AddConstraint);
	else if MoveComponent(FSRigidConstraint    , AddRC);
	else if MoveComponent(FSRigidLoad          , AddRigidLoad);
	else if MoveComponent(FSRigidBC            , AddRigidBC);
	else if MoveComponent(FSRigidIC            , AddRigidIC);
	else if MoveComponent(FSLinearConstraintSet, AddLinearConstraint);
	else if MoveComponent(FSRigidConnector     , AddRigidConnector);
	else if MoveComponent(FSMeshAdaptor        , AddMeshAdaptor);
	else assert(false);
}

//-----------------------------------------------------------------------------
#define TryRemoveComponent(Type, Cont) (dynamic_cast<Type*>(pc)) imp->Cont.Remove(dynamic_cast<Type*>(pc))

void FSStep::RemoveComponent(FSStepComponent* pc)
{
	assert(pc->GetStep() == GetID());
	if      TryRemoveComponent(FSBoundaryCondition  , m_BC);
	else if TryRemoveComponent(FSLoad               , m_FC);
	else if TryRemoveComponent(FSInitialCondition   , m_IC);
	else if TryRemoveComponent(FSInterface          , m_Int);
	else if TryRemoveComponent(FSModelConstraint    , m_NLC);
	else if TryRemoveComponent(FSRigidConstraint    , m_RC);
	else if TryRemoveComponent(FSRigidLoad          , m_RL);
	else if TryRemoveComponent(FSRigidBC            , m_RBC);
	else if TryRemoveComponent(FSRigidIC            , m_RIC);
	else if TryRemoveComponent(FSLinearConstraintSet, m_LC);
	else if TryRemoveComponent(FSRigidConnector     , m_CN);
	else if TryRemoveComponent(FSMeshAdaptor        , m_MA);
	else assert(false);
}

int FSStep::StepComponents()
{
	int n = 0;
	n += BCs();
	n += Loads();
	n += ICs();
	n += Interfaces();
	n += LinearConstraints();
	n += RigidBCs();
	n += RigidICs();
	n += RigidLoads();
	n += RigidConnectors();
	n += MeshAdaptors();
	return n;
}

//-----------------------------------------------------------------------------
void FSStep::Save(OArchive &ar)
{
	// write the name
	ar.WriteChunk(CID_STEP_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// write the step
	ar.WriteChunk(CID_STEP_ID, m_nID);

	// save the step parameters
	ar.BeginChunk(CID_STEP_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

	// save the control properties
	if (Properties() > 0)
	{
		for (int i = 0; i < Properties(); ++i)
		{
			FSProperty& prop = GetProperty(i);
			ar.BeginChunk(CID_STEP_PROPERTY);
			{
				// store the property name
				ar.WriteChunk(CID_STEP_PROPERTY_NAME, prop.GetName());

				// store the property data
				FSCoreBase* pc = prop.GetComponent();
				if (pc)
				{
					string typeStr = pc->GetTypeString();
					ar.WriteChunk(CID_STEP_PROPERTY_TYPESTR, typeStr);
					ar.BeginChunk(CID_STEP_PROPERTY_DATA);
					{
						pc->Save(ar);
					}
					ar.EndChunk();
				}
			}
			ar.EndChunk();
		}
	}

	// save the boundary conditions
	int nbc = BCs();
	if (nbc > 0)
	{
		ar.BeginChunk(CID_BC_SECTION);
		{
			for (int i=0; i<nbc; ++i)
			{
				FSBoundaryCondition* pb = BC(i);
				int ntype = pb->Type();
				ar.BeginChunk(ntype);
				{
					pb->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the loads
	int nfc = Loads();
	if (nfc > 0)
	{
		ar.BeginChunk(CID_FC_SECTION);
		{
			for (int i=0; i<nfc; ++i)
			{
				FSLoad* pb = Load(i);
				int ntype = pb->Type();
				ar.BeginChunk(ntype);
				{
					pb->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save initial conditions
	int nic = ICs();
	if (nic > 0)
	{
		ar.BeginChunk(CID_IC_SECTION);
		{
			for (int i=0; i<nic; ++i)
			{
				FSInitialCondition* pi = IC(i);
				int ntype = pi->Type();
				ar.BeginChunk(ntype);
				{
					pi->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the interfaces
	int nintfs = Interfaces();
	if (nintfs > 0)
	{
		ar.BeginChunk(CID_INTERFACE_SECTION);
		{
			for (int i=0; i<nintfs; ++i)
			{
				FSInterface* pi = Interface(i);
				int ntype = pi->Type();
				ar.BeginChunk(ntype);
				{
					pi->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the constraints
	int nmlc = Constraints();
	if (nmlc > 0)
	{
		ar.BeginChunk(CID_CONSTRAINT_SECTION);
		{
			for (int i = 0; i < nmlc; ++i)
			{
				FSModelConstraint* pmc = Constraint(i);
				int ntype = pmc->Type();
				ar.BeginChunk(ntype);
				{
					pmc->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the rigid constriants
	int nrc = RigidConstraints();
	if (nrc > 0)
	{
		ar.BeginChunk(CID_RC_SECTION_OLD);
		{
			for (int i=0; i<nrc; ++i)
			{
				FSRigidConstraint* pr = RigidConstraint(i);
				int ntype = pr->Type();
				ar.BeginChunk(ntype);
				{
					pr->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
    
    // save the connectors
    int ncnct = RigidConnectors();
    if (ncnct > 0)
    {
        ar.BeginChunk(CID_CONNECTOR_SECTION);
        {
            for (int i=0; i<ncnct; ++i)
            {
				FSRigidConnector* pi = RigidConnector(i);
                int ntype = pi->Type();
                ar.BeginChunk(ntype);
                {
                    pi->Save(ar);
                }
                ar.EndChunk();
            }
        }
        ar.EndChunk();
    }

	// save the rigid loads
	int nrl = RigidLoads();
	if (nrl > 0)
	{
		ar.BeginChunk(CID_RBL_SECTION);
		{
			for (int i = 0; i < nrl; ++i)
			{
				FSRigidLoad* prl = RigidLoad(i);
				int ntype = prl->Type();
				ar.BeginChunk(ntype);
				{
					prl->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the rigid conditions
	int nrbc = RigidBCs();
	if (nrbc > 0)
	{
		ar.BeginChunk(CID_RBC_SECTION);
		{
			for (int i = 0; i < nrbc; ++i)
			{
				FSRigidBC* prc = RigidBC(i);
				int ntype = prc->Type();
				ar.BeginChunk(ntype);
				{
					prc->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the rigid initial conditions
	int nric = RigidICs();
	if (nric > 0)
	{
		ar.BeginChunk(CID_RBI_SECTION);
		{
			for (int i = 0; i < nric; ++i)
			{
				FSRigidIC* pic = RigidIC(i);
				int ntype = pic->Type();
				ar.BeginChunk(ntype);
				{
					pic->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the mesh adaptors
	int nma = MeshAdaptors();
	if (nma > 0)
	{
		ar.BeginChunk(CID_MESH_ADAPTOR_SECTION);
		{
			for (int i = 0; i < nma; ++i)
			{
				FSMeshAdaptor* pma = MeshAdaptor(i);
				int ntype = pma->Type();
				ar.BeginChunk(ntype);
				{
					pma->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void FSStep::Load(IArchive &ar)
{
	TRACE("FSStep::Load");

	FSModel* fem = GetFSModel();

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_STEP_NAME      : { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO     : { string info; ar.read(info); SetInfo(info); } break;
		case CID_STEP_ID        : { int nid; ar.read(nid); SetID(nid); } break;
		case CID_STEP_PARAMS:
		{
			ParamContainer::Load(ar);
		}
		break;
		case CID_STEP_PROPERTY:
		{
			FSProperty* pc = nullptr;
			string typeString;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int cid = ar.GetChunkID();
				switch (cid)
				{
				case CID_STEP_PROPERTY_NAME:
				{
					std::string propName;
					ar.read(propName);
					pc = FindProperty(propName); assert(pc);
				}
				break;
				case CID_STEP_PROPERTY_TYPESTR: ar.read(typeString); break;
				case CID_STEP_PROPERTY_DATA: 
				{
					FSModelComponent* pmc = FEBio::CreateFSClass(pc->GetSuperClassID(), -1, fem); assert(pmc);
					pmc->Load(ar);
					assert(pc->GetComponent() == nullptr);
					pc->AddComponent(pmc);
				}
				break;
				default:
					assert(false);
				}
				ar.CloseChunk();
			}
		}
		break;
		case CID_BC_SECTION: // boundary conditions
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();

					FSDomainComponent* pb = 0;
					switch (ntype)
					{
					case FE_FIXED_DISPLACEMENT		 : pb = new FSFixedDisplacement         (fem); break;
					case FE_FIXED_ROTATION           : pb = new FSFixedRotation             (fem); break;
					case FE_FIXED_FLUID_PRESSURE	 : pb = new FSFixedFluidPressure        (fem); break;
					case FE_FIXED_TEMPERATURE        : pb = new FSFixedTemperature          (fem); break;
					case FE_FIXED_CONCENTRATION      : pb = new FSFixedConcentration        (fem); break;
                    case FE_FIXED_FLUID_VELOCITY     : pb = new FSFixedFluidVelocity        (fem); break;
                    case FE_FIXED_FLUID_DILATATION   : pb = new FSFixedFluidDilatation      (fem); break;
					case FE_PRESCRIBED_DISPLACEMENT	 : pb = new FSPrescribedDisplacement    (fem); break;
					case FE_PRESCRIBED_ROTATION      : pb = new FSPrescribedRotation        (fem); break;
					case FE_PRESCRIBED_FLUID_PRESSURE: pb = new FSPrescribedFluidPressure   (fem); break;
					case FE_PRESCRIBED_TEMPERATURE   : pb = new FSPrescribedTemperature     (fem); break;
					case FE_PRESCRIBED_CONCENTRATION : pb = new FSPrescribedConcentration   (fem); break;
                    case FE_PRESCRIBED_FLUID_VELOCITY: pb = new FSPrescribedFluidVelocity   (fem); break;
                    case FE_PRESCRIBED_FLUID_DILATATION: pb = new FSPrescribedFluidDilatation (fem); break;
					case FE_FIXED_SHELL_DISPLACEMENT : pb = new FSFixedShellDisplacement    (fem); break;
					case FE_PRESCRIBED_SHELL_DISPLACEMENT: pb = new FSPrescribedShellDisplacement(fem); break;
					case FE_FEBIO_BC                 : pb = new FEBioBoundaryCondition(fem); break;
                    case FE_FIXED_FLUID_ANGULAR_VELOCITY     : pb = new FSFixedFluidAngularVelocity        (fem); break;
                    case FE_PRESCRIBED_FLUID_ANGULAR_VELOCITY: pb = new FSPrescribedFluidAngularVelocity   (fem); break;
					default:
						if (ar.Version() < 0x00020000)
						{
							// initial conditions used to be grouped with the boundary conditions, but
							// now have their own section
							switch(ntype)
							{
							case FE_INIT_NODAL_VELOCITIES      : pb = new FSNodalVelocities       (fem); break;
							case FE_INIT_NODAL_SHELL_VELOCITIES: pb = new FSNodalShellVelocities  (fem); break;
                            case FE_INIT_FLUID_PRESSURE        : pb = new FSInitFluidPressure     (fem); break;
                            case FE_INIT_SHELL_FLUID_PRESSURE  : pb = new FSInitShellFluidPressure(fem); break;
							case FE_INIT_CONCENTRATION         : pb = new FSInitConcentration     (fem); break;
                            case FE_INIT_SHELL_CONCENTRATION   : pb = new FSInitShellConcentration(fem); break;
							case FE_INIT_TEMPERATURE           : pb = new FSInitTemperature       (fem); break;
							default:
								throw ReadError("error parsing CID_BC_SECTION in FSStep::Load");
							}
						}
						else
						{
							throw ReadError("error parsing CID_BC_SECTION in FSStep::Load");
						}
					}

					pb->Load(ar);
					pb->SetStep(GetID());

					if (ar.Version() < 0x0001000F)
					{
						// In older version, the rotational dofs were actually stored in the FSFixedDisplacement
						// but now, they are stored in FSFixedRotation
						FSFixedDisplacement* pbc = dynamic_cast<FSFixedDisplacement*>(pb);
						if (pbc)
						{
							int bc = pbc->GetBC();
							if (bc >= 8)
							{
								bc = (bc>>3)&7;
								FSFixedRotation* prc = new FSFixedRotation(fem);
								prc->SetName(pbc->GetName());
								prc->SetBC(bc);
								prc->SetItemList(pb->GetItemList()->Copy());
								AddBC(prc);
							}
						}

						FSPrescribedDisplacement* pdc = dynamic_cast<FSPrescribedDisplacement*>(pb);
						if (pdc)
						{
							int bc = pdc->GetDOF();
							if (bc > 2)
							{
								bc -= 3;
								FSPrescribedRotation* prc = new FSPrescribedRotation(fem);
								prc->SetName(pdc->GetName());
								prc->SetDOF(bc);
								prc->SetItemList(pdc->GetItemList()->Copy());
								delete pdc;
								pb = prc;
							}
						}
					}

					// add BC
					if (ar.Version() < 0x00020000)
					{
						if (dynamic_cast<FSInitialCondition*>(pb)) AddIC(dynamic_cast<FSInitialCondition*>(pb));
						else AddBC(dynamic_cast<FSBoundaryCondition*>(pb));
					}
					else AddBC(dynamic_cast<FSBoundaryCondition*>(pb));

					ar.CloseChunk();
				}
			}
			break;
		case CID_FC_SECTION: // loads
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();

					FSLoad* pl = 0;

					if (ntype == FE_NODAL_DOF_LOAD) pl = new FSNodalDOFLoad(fem);
					else if (ntype == FE_FEBIO_NODAL_LOAD) pl = new FEBioNodalLoad(fem);
					else
					{
						// see if it's a model load
						pl = fscore_new<FSLoad>(fem, FELOAD_ID, ntype);
					}

					// make sure we have something
					if (pl == 0)
					{
						throw ReadError("error parsing CID_FC_SECTION FSStep::Load");
					}

					// read BC data
					pl->Load(ar);

					// add BC
					pl->SetStep(GetID());
					AddLoad(pl);

					ar.CloseChunk();
				}
			}
			break;
		case CID_IC_SECTION: // initial conditions
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();

					FSInitialCondition* pi = fscore_new<FSInitialCondition>(fem, FEIC_ID, ntype); assert(pi);
					if (pi == nullptr) ar.log("error parsing CID_IC_SECTION in FSStep::Load");

					// read data
					if (pi)
					{
						pi->Load(ar);
						pi->SetStep(GetID());
						AddIC(pi);
					}

					ar.CloseChunk();
				}
			}
			break;
		case CID_INTERFACE_SECTION: // interfaces
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();

					FSInterface* pi = 0;

					// check obsolete interfaces first
					if      (ntype == FE_SLIDING_INTERFACE   ) pi = new FSSlidingInterface(fem);
					else if (ntype == FE_SPRINGTIED_INTERFACE) pi = new FSSpringTiedInterface(fem);
					else pi = fscore_new<FSInterface>(fem, FESURFACEINTERFACE_ID, ntype);

					// make sure we were able to allocate an interface
					if (pi == 0)
					{
						// some "contact" interfaces were moved to constraints
						FSModelConstraint* pc = fscore_new<FSModelConstraint>(fem, FENLCONSTRAINT_ID, ntype);
						if (pc)
						{
							pc->Load(ar);
							pc->SetStep(GetID());
							AddConstraint(pc);
						}
						else throw ReadError("error parsing unknown CID_INTERFACE_SECTION FSStep::Load");
					}
					else
					{
						// load the interface data
						pi->Load(ar);

						// add interface to step
						pi->SetStep(GetID());
						AddInterface(pi);
					}

					ar.CloseChunk();
				}
			}
			break;
		case CID_CONSTRAINT_SECTION: // model constraints
		{
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int ntype = ar.GetChunkID();

				FSModelConstraint* pmc = fscore_new<FSModelConstraint>(fem, FENLCONSTRAINT_ID, ntype);

				// make sure we were able to allocate a constraint
				if (pmc == 0)
				{
					throw ReadError("error parsing unknown CID_INTERFACE_SECTION FSStep::Load");
				}
				else
				{
					// load the constraint data
					pmc->Load(ar);

					// add constraint to step
					pmc->SetStep(GetID());
					AddConstraint(pmc);
				}

				ar.CloseChunk();
			}
		}
		break;
		case CID_RC_SECTION_OLD: // rigid constraints
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int ntype = ar.GetChunkID();

					if (ar.Version() < 0x00020000)
					{
						FSRigidConstraintOld* rc_old = new FSRigidConstraintOld(ntype, GetID());

						// read RC data
						rc_old->Load(ar);

						vector<FSRigidConstraint*> rc = convertOldToNewRigidConstraint(fem, rc_old);

						// add rigid constraints
						for (int i = 0; i < (int)rc.size(); ++i)
						{
							rc[i]->SetStep(GetID());
							AddRC(rc[i]);
						}
					}
					else
					{
						FSRigidConstraint* rc = fscore_new<FSRigidConstraint>(fem, FERIGIDBC_ID, ntype); assert(rc);
						if (rc == nullptr) ar.log("error parsing CID_RC_SECTION_OLD in FSStep::Load");

						if (rc)
						{
							rc->Load(ar);
							rc->SetStep(GetID());
							AddRC(rc);
						}
					}

					ar.CloseChunk();
				}
			}
			break;
        case CID_CONNECTOR_SECTION: // connectors
            {
                while (IArchive::IO_OK == ar.OpenChunk())
                {
                    int ntype = ar.GetChunkID();
                    
					FSRigidConnector* pi = fscore_new<FSRigidConnector>(fem, FENLCONSTRAINT_ID, ntype); assert(pi);
					if (pi == nullptr) ar.log("error parsing CID_CONNECTOR_SECTION FSStep::Load");
                    
                    // load the interface data
					if (pi)
					{
						pi->Load(ar);
						pi->SetStep(GetID());
						AddRigidConnector(pi);
					}
                    
                    ar.CloseChunk();
                }
            }
            break;
        case CID_RBL_SECTION: // rigid loads
            {
                while (IArchive::IO_OK == ar.OpenChunk())
                {
                    int ntype = ar.GetChunkID();
                    
					FEBioRigidLoad* pl = fscore_new<FEBioRigidLoad>(fem, FELOAD_ID, ntype); assert(pl);
					if (pl == nullptr) ar.log("error parsing CID_RL_SECTION FSStep::Load");
                    
					if (pl)
					{
						pl->Load(ar);
						pl->SetStep(GetID());
						AddRigidLoad(pl);
					}
                    
                    ar.CloseChunk();
                }
            }
            break;
		case CID_RBC_SECTION: // rigid body constraints
		{
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int ntype = ar.GetChunkID();

				FEBioRigidBC* pc = fscore_new<FEBioRigidBC>(fem, FEBC_ID, ntype); assert(pc);
				if (pc == nullptr) ar.log("error parsing CID_RBC_SECTION FSStep::Load");

				if (pc)
				{
					pc->Load(ar);
					pc->SetStep(GetID());
					AddRigidBC(pc);
				}

				ar.CloseChunk();
			}
		}
		break;
		case CID_RBI_SECTION: // rigid body initial conditions
		{
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int ntype = ar.GetChunkID();

				FEBioRigidIC* ic = fscore_new<FEBioRigidIC>(fem, FEIC_ID, ntype); assert(ic);
				if (ic == nullptr) ar.log("error parsing CID_RBI_SECTION FSStep::Load");

				if (ic)
				{
					ic->Load(ar);
					ic->SetStep(GetID());
					AddRigidIC(ic);
				}

				ar.CloseChunk();
			}
		}
		break;		case CID_MESH_ADAPTOR_SECTION:
		{
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int ntype = ar.GetChunkID();

				FSMeshAdaptor* pma = fscore_new<FSMeshAdaptor>(fem, FEMESHADAPTOR_ID, ntype); assert(pma);
				if (pma == nullptr) ar.log("error parsing CID_MESH_ADAPTOR_SECTION FSStep::Load");

				if (pma)
				{
					pma->Load(ar);
					pma->SetStep(GetID());
					AddMeshAdaptor(pma);
				}

				ar.CloseChunk();
			}
		}
		break;
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// FSInitialStep
//-----------------------------------------------------------------------------

FSInitialStep::FSInitialStep(FSModel* ps) : FSStep(ps, FE_STEP_INITIAL)
{ 
	SetName("Initial"); 
	SetTypeString("Initial");

	SetID(0);
	DecreaseCounter();
}

void FSInitialStep::Save(OArchive &ar)
{
	// save the step data
	ar.BeginChunk(CID_STEP_DATA);
	{
		FSStep::Save(ar);
	}
	ar.EndChunk();
}

void FSInitialStep::Load(IArchive &ar)
{
	TRACE("FSInitialStep::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_STEP_DATA      : FSStep::Load(ar); break;
		}
		ar.CloseChunk();
	}	
}

//-----------------------------------------------------------------------------
// FSAnalysisStep
//-----------------------------------------------------------------------------

void STEP_SETTINGS::Defaults()
{
	sztitle[0] = 0;

	// time settings
	ntime = 10;
	dt = 0.1;
	bauto = true;
	bmust = false;
	mxback = 5;
	iteopt = 10;
	dtmin = 0.01;
	dtmax = 0.1;
	ncut = 0;	// constant cutback
	tfinal = 0;
	bdivref = true;
	brefstep = true;
	dtforce = false;

	// solver settings
	mthsol = 0;
	bminbw = false;
	nmatfmt = 0;	// = default matrix storage
	neqscheme = 0;	// = default equation scheme
	ilimit = 10;
	maxref = 15;
	logSolve = false;

	// analysis settings
	nanalysis = 0;

	// time integration parameters
	override_rhoi = false;		// use rhoi instead of these parameters
    alpha = 1;
    beta = 0.25;
    gamma = 0.50;

	// output optios
	plot_level = 1; // Major iterations
	plot_stride = 1;
	plot_zero = false;
	plot_range[0] = 0;
	plot_range[1] = -1;
	output_level = 1; // = FE_Output_Level::FE_OUTPUT_MAJOR_ITRS

	adapter_re_solve = true;

	// constants
//	Rc = 0; //8.314e-6;
//	Ta = 0; //298;
}

FSAnalysisStep::FSAnalysisStep(FSModel* ps, int ntype) : FSStep(ps, ntype)
{
	// set default options
	m_ops.Defaults();

	// reset must point curve
	m_MP.Clear();
	m_MP.SetInterpolator(PointCurve::STEP);
	m_MP.Add(0, 0);
	m_MP.Add(1, 1);
}

vector<string> FSAnalysisStep::GetAnalysisStrings() const
{
	vector<string> s;
	s.push_back("static");
	s.push_back("dynamic");
	return s;
}

void FSAnalysisStep::Save(OArchive &ar)
{
	// save the step settings
	ar.BeginChunk(CID_STEP_SETTINGS);
	{
		STEP_SETTINGS& o = m_ops;
		ar.WriteChunk(CID_STEP_TITLE     , o.sztitle);
		ar.WriteChunk(CID_STEP_NTIME     , o.ntime);
		ar.WriteChunk(CID_STEP_DT        , o.dt);
		ar.WriteChunk(CID_STEP_BAUTO     , o.bauto);
		ar.WriteChunk(CID_STEP_BMUST     , o.bmust);
		ar.WriteChunk(CID_STEP_MXBACK    , o.mxback);
		ar.WriteChunk(CID_STEP_ITEOPT    , o.iteopt);
		ar.WriteChunk(CID_STEP_DTMIN     , o.dtmin);
		ar.WriteChunk(CID_STEP_DTMAX     , o.dtmax);
		ar.WriteChunk(CID_STEP_MTHSOL    , o.mthsol);
		ar.WriteChunk(CID_STEP_BMINBW    , o.bminbw);
		ar.WriteChunk(CID_STEP_MATFMT	 , o.nmatfmt);
		ar.WriteChunk(CID_STEP_ILIMIT    , o.ilimit);
		ar.WriteChunk(CID_STEP_MAXREF    , o.maxref);
		ar.WriteChunk(CID_STEP_DIVREF	 , o.bdivref);
		ar.WriteChunk(CID_STEP_REFSTEP   , o.brefstep);
//		ar.WriteChunk(CID_STEP_DTOL      , o.dtol);
//		ar.WriteChunk(CID_STEP_ETOL      , o.etol);
//		ar.WriteChunk(CID_STEP_RTOL      , o.rtol);
//		ar.WriteChunk(CID_STEP_PTOL		 , o.ptol);
//		ar.WriteChunk(CID_STEP_CTOL		 , o.ctol);
//		ar.WriteChunk(CID_STEP_VTOL      , o.vtol);
//		ar.WriteChunk(CID_STEP_TOLLS     , o.tolls);
		ar.WriteChunk(CID_STEP_ANALYSIS  , o.nanalysis);
//		ar.WriteChunk(CID_STEP_PRES_STIFF, o.nprstf);
//		ar.WriteChunk(CID_STEP_SYMM_PORO , o.bsymmporo);
		ar.WriteChunk(CID_STEP_CUTBACK   , o.ncut);
//		ar.WriteChunk(CID_STEP_MINRES    , o.minres);
		ar.WriteChunk(CID_STEP_PLOTSTRIDE, o.plot_stride);
		ar.WriteChunk(CID_STEP_PLOTLEVEL , o.plot_level);
	}
	ar.EndChunk();

	// save the step parameters
	ar.BeginChunk(CID_STEP_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

	// save the must point
	ar.BeginChunk(CID_STEP_MUST_POINT);
	{
		m_MP.Save(ar);
	}
	ar.EndChunk();

	// save the step data
	ar.BeginChunk(CID_STEP_DATA);
	{
		FSStep::Save(ar);
	}
	ar.EndChunk();
}

void FSAnalysisStep::Load(IArchive &ar)
{
	TRACE("FSAnalysisStep::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_STEP_PARAMS:
		{
			ParamContainer::Load(ar);
		}
		break;
		case CID_STEP_SETTINGS:
			{
				STEP_SETTINGS& o = m_ops;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					switch (nid)
					{
					case CID_STEP_TITLE     : ar.read(o.sztitle  ); break;
					case CID_STEP_NTIME     : ar.read(o.ntime    ); break;
					case CID_STEP_DT        : ar.read(o.dt       ); break;
					case CID_STEP_BAUTO     : ar.read(o.bauto    ); break;
					case CID_STEP_BMUST     : ar.read(o.bmust    ); break;
					case CID_STEP_MXBACK    : ar.read(o.mxback   ); break;
					case CID_STEP_ITEOPT    : ar.read(o.iteopt   ); break;
					case CID_STEP_DTMIN     : ar.read(o.dtmin    ); break;
					case CID_STEP_DTMAX     : ar.read(o.dtmax    ); break;
					case CID_STEP_MTHSOL    : ar.read(o.mthsol   ); break;
					case CID_STEP_BMINBW    : ar.read(o.bminbw   ); break;
					case CID_STEP_MATFMT    : ar.read(o.nmatfmt  ); break;
					case CID_STEP_ILIMIT    : ar.read(o.ilimit   ); break;
					case CID_STEP_MAXREF    : ar.read(o.maxref   ); break;
					case CID_STEP_DIVREF	: ar.read(o.bdivref  ); break;
					case CID_STEP_REFSTEP	: ar.read(o.brefstep ); break;
//					case CID_STEP_DTOL      : ar.read(o.dtol     ); break;
//					case CID_STEP_ETOL      : ar.read(o.etol     ); break;
//					case CID_STEP_RTOL      : ar.read(o.rtol     ); break;
//					case CID_STEP_PTOL      : ar.read(o.ptol     ); break;
//					case CID_STEP_CTOL      : ar.read(o.ctol     ); break;
//					case CID_STEP_VTOL      : ar.read(o.vtol     ); break;
//					case CID_STEP_TOLLS     : ar.read(o.tolls    ); break;
					case CID_STEP_ANALYSIS  : ar.read(o.nanalysis); break;
//					case CID_STEP_PSTIFFNESS: ar.read(o.bpress_stiff); break;
//					case CID_STEP_PRES_STIFF: ar.read(o.nprstf   ); break;
//					case CID_STEP_SYMM_PORO : ar.read(o.bsymmporo); break;
					case CID_STEP_CUTBACK   : ar.read(o.ncut     ); break;
//					case CID_STEP_MINRES    : ar.read(o.minres   ); break;
					case CID_STEP_PLOTSTRIDE: ar.read(o.plot_stride); break;
					case CID_STEP_PLOTLEVEL : ar.read(o.plot_level); break;
					}
					ar.CloseChunk();

					// for old versions, biphasic and biphasic-solute problems
					// only knew transient analyses, but the analysis parameter was not 
					// used, so it may contain an incorrect value now. Therefore
					// we need to fix it.
					if (ar.Version() < 0x00010008)
					{
						if ((GetType() == FE_STEP_BIPHASIC) || 
							(GetType() == FE_STEP_BIPHASIC_SOLUTE) ||
							(GetType() == FE_STEP_MULTIPHASIC)) o.nanalysis = FE_DYNAMIC;
					}
				}
			}
			break;
		case CID_STEP_MUST_POINT: m_MP.Load(ar); break;
		case CID_STEP_DATA      : FSStep::Load(ar); break;
		}
		ar.CloseChunk();
	}	
}

//-----------------------------------------------------------------------------
FSNonLinearMechanics::FSNonLinearMechanics(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_MECHANICS)
{
	SetTypeString("Structural Mechanics");

	AddDoubleParam(0.001, "dtol", "Displacement tolerance");
	AddDoubleParam(0.01 , "etol", "Energy tolerance");
	AddDoubleParam(0    , "rtol", "Residual tolerance");
	AddDoubleParam(0.9  , "lstol", "Line search tolerance");
	AddDoubleParam(1e-20, "min_residual", "Minumum residual");
	AddChoiceParam(0, "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");
	AddDoubleParam(0, "rhoi", "Spectral radius");
}

double FSNonLinearMechanics::GetDisplacementTolerance() { return GetParam(MP_DTOL).GetFloatValue(); }
double FSNonLinearMechanics::GetEnergyTolerance() { return GetParam(MP_ETOL).GetFloatValue(); }
double FSNonLinearMechanics::GetResidualTolerance() { return GetParam(MP_RTOL).GetFloatValue(); }
double FSNonLinearMechanics::GetLineSearchTolerance() { return GetParam(MP_LSTOL).GetFloatValue(); }

void FSNonLinearMechanics::SetDisplacementTolerance(double dtol) { GetParam(MP_DTOL).SetFloatValue(dtol); }
void FSNonLinearMechanics::SetEnergyTolerance(double etol) { GetParam(MP_ETOL).SetFloatValue(etol); }
void FSNonLinearMechanics::SetResidualTolerance(double rtol) { GetParam(MP_RTOL).SetFloatValue(rtol); }
void FSNonLinearMechanics::SetLineSearchTolerance(double lstol) { GetParam(MP_LSTOL).SetFloatValue(lstol); }

//-----------------------------------------------------------------------------
FSExplicitSolidAnalysis::FSExplicitSolidAnalysis(FSModel* fem) : FSAnalysisStep(fem, FE_STEP_EXPLICIT_SOLID)
{
	SetTypeString("explicit-solid");
}

//-----------------------------------------------------------------------------
FSHeatTransfer::FSHeatTransfer(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_HEAT_TRANSFER)
{
	SetTypeString("Heat Transfer");
}

vector<string> FSHeatTransfer::GetAnalysisStrings() const
{
	vector<string> s;
	s.push_back("steady-state");
	s.push_back("transient");
	return s;
}

//-----------------------------------------------------------------------------
FSNonLinearBiphasic::FSNonLinearBiphasic(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_BIPHASIC)
{
	SetTypeString("Biphasic");

	AddDoubleParam(0.001, "dtol", "Displacement tolerance");
	AddDoubleParam(0.01 , "etol", "Energy tolerance");
	AddDoubleParam(0    , "rtol", "Residual tolerance");
	AddDoubleParam(0.01 , "ptol", "Pressure tolerance");
	AddDoubleParam(0.9  , "lstol", "Line search tolerance");
	AddDoubleParam(1e-20, "min_residual", "Minumum residual");
	AddChoiceParam(0, "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");
	AddChoiceParam(0, "mixed_formulation", "formulation")->SetEnumNames("default\0mixed formulation\0");

	m_ops.nanalysis = 1; // set transient analysis
}

vector<string> FSNonLinearBiphasic::GetAnalysisStrings() const
{
	vector<string> s;
	s.push_back("steady-state");
	s.push_back("transient");
	return s;
}

//-----------------------------------------------------------------------------
FSBiphasicSolutes::FSBiphasicSolutes(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_BIPHASIC_SOLUTE)
{
	SetTypeString("Biphasic-solute");

	AddDoubleParam(0.001, "dtol", "Displacement tolerance");
	AddDoubleParam(0.01 , "etol", "Energy tolerance");
	AddDoubleParam(0    , "rtol", "Residual tolerance");
	AddDoubleParam(0.01 , "ptol", "Pressure tolerance");
	AddDoubleParam(0.01 , "ctol", "Concentration tolerance");
	AddDoubleParam(0.9  , "lstol", "Line search tolerance");
	AddDoubleParam(1e-20, "min_residual", "Minumum residual");
	AddChoiceParam(0, "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");

	m_ops.nanalysis = 1; // set transient analysis
}

vector<string> FSBiphasicSolutes::GetAnalysisStrings() const
{
	vector<string> s;
	s.push_back("steady-state");
	s.push_back("transient");
	return s;
}

//-----------------------------------------------------------------------------
FSMultiphasicAnalysis::FSMultiphasicAnalysis(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_MULTIPHASIC)
{
	SetTypeString("Multiphasic");

	AddDoubleParam(0.001, "dtol", "Displacement tolerance");
	AddDoubleParam(0.01 , "etol", "Energy tolerance");
	AddDoubleParam(0    , "rtol", "Residual tolerance");
	AddDoubleParam(0.01 , "ptol", "Pressure tolerance");
	AddDoubleParam(0.01 , "ctol", "Concentration tolerance");
	AddDoubleParam(0.9  , "lstol", "Line search tolerance");
	AddDoubleParam(1e-20, "min_residual", "Minumum residual");
	AddChoiceParam(0, "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");

	// NOTE: This parameter cannot be written to febio3 files so we are hiding this for now. 
	//       In FEBio3, domain parameters are written to the "domain" tags of the "instance", 
	//       but this is not yet supported in FEBioStudio. 
	AddBoolParam(true, "shell_normal_nodal", "Shell normal nodal")->SetState(0);

	m_ops.nanalysis = 1; // set transient analysis
    m_ops.nmatfmt = 0;   // set non-symmetric flag
}

vector<string> FSMultiphasicAnalysis::GetAnalysisStrings() const
{
	vector<string> s;
	s.push_back("steady-state");
	s.push_back("transient");
	return s;
}

//-----------------------------------------------------------------------------
FSFluidAnalysis::FSFluidAnalysis(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_FLUID)
{
    SetTypeString("Fluid");

	AddDoubleParam(0.001, "vtol", "Fluid velocity tolerance");
    AddDoubleParam(0.001, "ftol", "Fluid dilatation tolerance");
	AddDoubleParam(0.01 , "etol", "Energy tolerance");
	AddDoubleParam(0    , "rtol", "Residual tolerance");
	AddDoubleParam(0.9  , "lstol", "Line search tolerance");
	AddDoubleParam(1e-20, "min_residual", "Minumum residual");
    AddDoubleParam(1e+20, "max_residual", "Maximum residual");
    AddDoubleParam(0    , "rhoi", "Spectral radius");
	AddChoiceParam(1    , "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");
	AddChoiceParam(1    , "equation_scheme", "Equation Scheme");

    m_ops.nanalysis = 1; // set dynamic analysis
    m_ops.nmatfmt = 0;   // set non-symmetric flag
}

//-----------------------------------------------------------------------------
FSFluidFSIAnalysis::FSFluidFSIAnalysis(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_FLUID_FSI)
{
    SetTypeString("Fluid-FSI");
    
    AddDoubleParam(0.001, "dtol", "Solid displacement tolerance");
    AddDoubleParam(0.001, "vtol", "Fluid velocity tolerance");
    AddDoubleParam(0.001, "ftol", "Fluid dilatation tolerance");
    AddDoubleParam(0.01 , "etol", "Energy tolerance");
    AddDoubleParam(0    , "rtol", "Residual tolerance");
    AddDoubleParam(0.9  , "lstol", "Line search tolerance");
	AddDoubleParam(1e-20, "min_residual", "Minumum residual");
    AddDoubleParam(1e+20, "max_residual", "Maximum residual");
    AddDoubleParam(0    , "rhoi", "Spectral radius");
    AddChoiceParam(1    , "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");
    
    m_ops.nanalysis = 1; // set dynamic analysis
    m_ops.nmatfmt = 0;   // set non-symmetric flag
}

//-----------------------------------------------------------------------------
FSReactionDiffusionAnalysis::FSReactionDiffusionAnalysis(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_REACTION_DIFFUSION)
{
	SetTypeString("Reaction diffusion");

	AddDoubleParam(0.001, "Ctol", "Concentration tolerance");
	AddDoubleParam(0.01 , "Rtol", "Residual tolerance");
	AddDoubleParam(0.0  , "Stol", "SBS concentration tolerance");
	AddDoubleParam(0.5  , "alpha", "Time integration parameter");
	AddDoubleParam(1e-20, "min_residual", "Minumum residual");

	m_ops.nanalysis = 1; // set dynamic analysis
	m_ops.nmatfmt = 0;   // set non-symmetric flag
}


vector<string> FSReactionDiffusionAnalysis::GetAnalysisStrings() const
{
	vector<string> s;
	s.push_back("steady-state");
	s.push_back("transient");
	return s;
}

//-----------------------------------------------------------------------------
FSPolarFluidAnalysis::FSPolarFluidAnalysis(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_POLAR_FLUID)
{
    SetTypeString("Polar Fluid");
    
    AddDoubleParam(0.001, "vtol", "Fluid velocity tolerance");
    AddDoubleParam(0.001, "ftol", "Fluid dilatation tolerance");
    AddDoubleParam(0.001, "gtol", "Angular fluid velocity tolerance");
    AddDoubleParam(0.01 , "etol", "Energy tolerance");
    AddDoubleParam(0.001, "rtol", "Residual tolerance");
    AddDoubleParam(0.9  , "lstol", "Line search tolerance");
    AddDoubleParam(1e-20, "min_residual", "Minumum residual");
    AddDoubleParam(1e+20, "max_residual", "Maximum residual");
    AddDoubleParam(0    , "rhoi", "Spectral radius");
    AddChoiceParam(1    , "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");
    AddChoiceParam(1    , "equation_scheme", "Equation Scheme");
    
    m_ops.nanalysis = 1; // set dynamic analysis
    m_ops.nmatfmt = 0;   // set non-symmetric flag
}

//-----------------------------------------------------------------------------
FSFluidSolutesAnalysis::FSFluidSolutesAnalysis(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_FLUID_SOLUTES)
{
    SetTypeString("Fluid-Solutes");
    
    AddDoubleParam(0.001, "vtol", "Fluid velocity tolerance");
    AddDoubleParam(0.001, "ftol", "Fluid dilatation tolerance");
    AddDoubleParam(0.01 , "ctol", "Solute concentration tolerance");
    AddDoubleParam(0.01 , "etol", "Energy tolerance");
    AddDoubleParam(0.001, "rtol", "Residual tolerance");
    AddDoubleParam(0.9  , "lstol", "Line search tolerance");
    AddDoubleParam(1e-20, "min_residual", "Minumum residual");
    AddDoubleParam(1e+20, "max_residual", "Maximum residual");
    AddDoubleParam(0    , "rhoi", "Spectral radius");
    AddChoiceParam(1    , "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");
    AddChoiceParam(1    , "equation_scheme", "Equation Scheme");
    
    m_ops.nanalysis = 1; // set dynamic analysis
    m_ops.nmatfmt = 0;   // set non-symmetric flag
}

//-----------------------------------------------------------------------------
FSThermoFluidAnalysis::FSThermoFluidAnalysis(FSModel* ps) : FSAnalysisStep(ps, FE_STEP_THERMO_FLUID)
{
    SetTypeString("Thermofluid");
    
    AddDoubleParam(0.001, "vtol", "Fluid velocity tolerance");
    AddDoubleParam(0.001, "ftol", "Fluid dilatation tolerance");
    AddDoubleParam(0.01 , "ttol", "Temperature tolerance");
    AddDoubleParam(0.01 , "etol", "Energy tolerance");
    AddDoubleParam(0.001, "rtol", "Residual tolerance");
    AddDoubleParam(0.9  , "lstol", "Line search tolerance");
    AddDoubleParam(1e-20, "min_residual", "Minumum residual");
    AddDoubleParam(1e+20, "max_residual", "Maximum residual");
    AddDoubleParam(0    , "rhoi", "Spectral radius");
    AddChoiceParam(1    , "qnmethod", "Quasi-Newton method")->SetEnumNames("BFGS\0BROYDEN\0");
    AddChoiceParam(1    , "equation_scheme", "Equation Scheme");
    
    m_ops.nanalysis = 1; // set dynamic analysis
    m_ops.nmatfmt = 0;   // set non-symmetric flag
}

//==================================================================================
FEBioAnalysisStep::FEBioAnalysisStep(FSModel* ps) : FSStep(ps, FE_STEP_FEBIO_ANALYSIS)
{
}

void FEBioAnalysisStep::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSStep::Save(ar);
	}
	ar.EndChunk();
}

void FEBioAnalysisStep::Load(IArchive& ar)
{
	TRACE("FEBioAnalysisStep::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSStep::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}
