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

#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "ModelDocument.h"
#include "DlgAddPhysicsItem.h"
#include "DlgAddNodalLoad.h"
#include "DlgAddStep.h"
#include "DlgSoluteTable.h"
#include "DlgAddChemicalReaction.h"
#include "DlgAddMembraneReaction.h"
#include "DlgAddRigidConstraint.h"
#include "DlgAddRigidConnector.h"
#include "MaterialEditor.h"
#include <MeshTools/FEModel.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include "Commands.h"
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>
#include <QMessageBox>
#include <sstream>

void CMainWindow::on_actionAddBC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Boundary Condition", FE_ESSENTIAL_BC, prj, true, this);
	if (dlg.exec())
	{
		FSBoundaryCondition* pbc = fecore_new<FSBoundaryCondition>(&fem, FE_ESSENTIAL_BC, FE_FEBIO_BC); assert(pbc);
		FEBio::CreateModelComponent(dlg.GetClassID(), pbc);
		if (pbc)
		{
			FEStep* step = fem.GetStep(dlg.GetStep());

			pbc->SetStep(step->GetID());

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultBCName(&fem, pbc);
			pbc->SetName(name);

			// figure out the selection
			FESelection* psel = doc->GetCurrentSelection();
			if (psel && psel->Size())
			{
				int ntype = psel->Type();
				switch (ntype)
				{
				case SELECT_SURFACES:
				case SELECT_CURVES:
				case SELECT_NODES:
				case SELECT_FE_FACES:
				case SELECT_FE_EDGES:
				case SELECT_FE_NODES:
					{
						FEItemListBuilder* items = psel->CreateItemList();
						if (items) pbc->SetItemList(items);
					}
					break;
				}
			}

			step->AddBC(pbc);
			UpdateModel(pbc);
		}
	}
}

void CMainWindow::on_actionAddNodalLoad_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Nodal Load", FE_NODAL_LOAD, prj, true, this);
	if (dlg.exec())
	{
		FENodalLoad* pnl = fecore_new<FENodalLoad>(&fem, FE_NODAL_LOAD, FE_FEBIO_NODAL_LOAD); assert(pnl);
		FEBio::CreateModelComponent(dlg.GetClassID(), pnl);
		if (pnl)
		{
			string name = dlg.GetName();
			if (name.empty()) name = defaultLoadName(&fem, pnl);
			pnl->SetName(name);

			// figure out the selection
			FESelection* psel = doc->GetCurrentSelection();
			if (psel && psel->Size())
			{
				int ntype = psel->Type();
				switch (ntype)
				{
				case SELECT_SURFACES:
				case SELECT_CURVES:
				case SELECT_NODES:
				case SELECT_FE_FACES:
				case SELECT_FE_EDGES:
				case SELECT_FE_NODES:
				{
					FEItemListBuilder* items = psel->CreateItemList();
					pnl->SetItemList(items);
				}
				break;
				}
			}

			FEStep* step = fem.GetStep(dlg.GetStep());
			step->AddLoad(pnl);
			UpdateModel(pnl);
		}
	}
}

void CMainWindow::on_actionAddSurfLoad_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Surface Load", FE_SURFACE_LOAD, prj, true, this);
	if (dlg.exec())
	{
		FSSurfaceLoad* psl = fecore_new<FSSurfaceLoad>(&fem, FE_SURFACE_LOAD, FE_FEBIO_SURFACE_LOAD); assert(psl);
		FEBio::CreateModelComponent(dlg.GetClassID(), psl);
		if (psl)
		{
			string name = dlg.GetName();
			if (name.empty()) name = defaultLoadName(&fem, psl);
			psl->SetName(name);

			// figure out the selection
			FESelection* psel = doc->GetCurrentSelection();
			if (psel && psel->Size())
			{
				int ntype = psel->Type();
				switch (ntype)
				{
				case SELECT_SURFACES:
				case SELECT_FE_FACES:
					{
						FEItemListBuilder* items = psel->CreateItemList();
						psl->SetItemList(items);
					}
					break;
				}
			}

			FEStep* step = fem.GetStep(dlg.GetStep());
			psl->SetStep(step->GetID());
			step->AddLoad(psl);
			UpdateModel(psl);
		}
	}
}

void CMainWindow::on_actionAddBodyLoad_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Body Load", FE_BODY_LOAD, prj, true, this);
	if (dlg.exec())
	{
		FSBodyLoad* pbl = fecore_new<FSBodyLoad>(&fem, FE_BODY_LOAD, FE_FEBIO_BODY_LOAD); assert(pbl);
		FEBio::CreateModelComponent(dlg.GetClassID(), pbl);
		if (pbl)
		{
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultLoadName(&fem, pbl);
			pbl->SetName(name);

			FEStep* step = fem.GetStep(dlg.GetStep());
			pbl->SetStep(step->GetID());
			step->AddLoad(pbl);
			UpdateModel(pbl);
		}
	}
}

void CMainWindow::on_actionAddRigidLoad_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Rigid Load", FE_RIGID_LOAD, prj, true, this);
	if (dlg.exec())
	{
		FSRigidLoad* prl = fecore_new<FSRigidLoad>(&fem, FE_RIGID_LOAD, FE_FEBIO_RIGID_LOAD); assert(prl);
		FEBio::CreateModelComponent(dlg.GetClassID(), prl);
		if (prl)
		{
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultLoadName(&fem, prl);
			prl->SetName(name);

			FEStep* step = fem.GetStep(dlg.GetStep());
			prl->SetStep(step->GetID());
			step->AddRigidLoad(prl);
			UpdateModel(prl);
		}
	}
}

void CMainWindow::on_actionAddIC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Initial Condition", FE_INITIAL_CONDITION, prj, true, this);
	if (dlg.exec())
	{
//		FSInitialCondition* pic = fecore_new<FSInitialCondition>(&fem, FE_INITIAL_CONDITION, dlg.GetClassID()); assert(pic);
		FSInitialCondition* pic = fecore_new<FSInitialCondition>(&fem, FE_INITIAL_CONDITION, FE_FEBIO_INITIAL_CONDITION); assert(pic);
		FEBio::CreateModelComponent(dlg.GetClassID(), pic);
		if (pic)
		{
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultICName(&fem, pic);
			pic->SetName(name);

			if (dynamic_cast<FEInitialNodalDOF*>(pic))
			{
				// figure out the selection
				FESelection* psel = doc->GetCurrentSelection();
				if (psel && psel->Size())
				{
					int ntype = psel->Type();
					switch (ntype)
					{
					case SELECT_PARTS:
					case SELECT_SURFACES:
					case SELECT_CURVES:
					case SELECT_NODES:
					case SELECT_FE_ELEMENTS:
					case SELECT_FE_FACES:
					case SELECT_FE_EDGES:
					case SELECT_FE_NODES:
					{
						FEItemListBuilder* items = psel->CreateItemList();
						pic->SetItemList(items);
					}
					break;
					}
				}
			}

			FEStep* step = fem.GetStep(dlg.GetStep());
			pic->SetStep(step->GetID());
			step->AddIC(pic);
			UpdateModel(pic);
		}
	}
}

void CMainWindow::on_actionAddContact_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Contact Interface", FE_INTERFACE, prj, true, this);
	if (dlg.exec())
	{
//		FSInterface* pi = fecore_new<FSInterface>(&fem, FE_INTERFACE, dlg.GetClassID()); assert(pi);
		FSInterface* pi = fecore_new<FSInterface>(&fem, FE_INTERFACE, FE_FEBIO_INTERFACE); assert(pi);
		FEBio::CreateModelComponent(dlg.GetClassID(), pi);
		if (pi)
		{
			// create a name
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultInterfaceName(&fem, pi);
			pi->SetName(name);

			// assign selected surfaces (for solo interfaces)
			if (dynamic_cast<FESoloInterface*>(pi))
			{
				FESoloInterface& si = dynamic_cast<FESoloInterface&>(*pi);

				// figure out the selection
				FESelection* psel = doc->GetCurrentSelection();
				if (psel && psel->Size())
				{
					int ntype = psel->Type();
					switch (ntype)
					{
					case SELECT_SURFACES:
					case SELECT_FE_FACES:
						{
							FEItemListBuilder* items = psel->CreateItemList();
							si.SetItemList(items);
						}
						break;
					}
				}
			}

			// assign it to the correct step
			FEStep* step = fem.GetStep(dlg.GetStep());
			pi->SetStep(step->GetID());
			step->AddInterface(pi);
			UpdateModel(pi);
		}
	}
}

void CMainWindow::on_actionAddConstraint_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Constraint", FE_CONSTRAINT, prj, true, this);
	if (dlg.exec())
	{
//		FSModelConstraint* pi = fecore_new<FSModelConstraint>(&fem, FE_CONSTRAINT, dlg.GetClassID()); assert(pi);
		FSModelConstraint* pi = fecore_new<FSModelConstraint>(&fem, FE_CONSTRAINT, FE_FEBIO_NLCONSTRAINT); assert(pi);
		FEBio::CreateModelComponent(dlg.GetClassID(), pi);
		if (pi)
		{
			// create a name
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultConstraintName(&fem, pi);
			pi->SetName(name);

			FESurfaceConstraint* psc = dynamic_cast<FESurfaceConstraint*>(pi);
			if (psc)
			{
				// figure out the selection
				FESelection* psel = doc->GetCurrentSelection();
				if (psel && psel->Size())
				{
					int ntype = psel->Type();
					switch (ntype)
					{
					case SELECT_SURFACES:
					case SELECT_FE_FACES:
					{
						FEItemListBuilder* items = psel->CreateItemList();
						psc->SetItemList(items);
					}
					break;
					}
				}
			}

			// assign it to the correct step
			FEStep* step = fem.GetStep(dlg.GetStep());
			pi->SetStep(step->GetID());
			step->AddConstraint(pi);
			UpdateModel(pi);
		}
	}
}

void CMainWindow::on_actionAddRigidConstraint_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	CDlgAddRigidConstraint dlg(prj, this);
	if (dlg.exec())
	{
		FSModel* fem = &prj.GetFSModel();
		FSRigidConstraint* prc = fecore_new<FSRigidConstraint>(fem, FE_RIGID_CONSTRAINT, FE_FEBIO_RIGID_CONSTRAINT);
		FEBio::CreateModelComponent(dlg.m_type, prc);
		assert(prc);
		if (prc)
		{
			FEStep* step = fem->GetStep(dlg.m_nstep);
			prc->SetStep(step->GetID());

			std::string name = dlg.m_name;
			if (name.empty()) name = defaultRigidConstraintName(fem, prc);

			GMaterial* pmat = dlg.GetMaterial();
			if (pmat) prc->SetMaterialID(pmat->GetID());

			prc->SetName(name);
			step->AddRC(prc);
			UpdateModel(prc);
		}
	}
}

void CMainWindow::on_actionAddRigidConnector_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	CDlgAddRigidConnector dlg(prj, this);
	if (dlg.exec())
	{
		FSModel* fem = doc->GetFSModel();
		FSRigidConnector* pc = fecore_new<FSRigidConnector>(fem, FE_RIGID_CONNECTOR, FE_FEBIO_RIGID_CONNECTOR);
		FEBio::CreateModelComponent(dlg.GetType(), pc);
		assert(pc);
		if (pc)
		{
			pc->SetPosition(GetGLView()->Get3DCursor());
			pc->SetRigidBody1(dlg.GetMaterialA());
			pc->SetRigidBody2(dlg.GetMaterialB());

			FEStep* step = fem->GetStep(dlg.GetStep());
			int stepID = step->GetID();
			pc->SetStep(stepID);

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultRigidConnectorName(fem, pc);

			pc->SetName(name);
			step->AddRigidConnector(pc);
			UpdateModel(pc);
		}
	}
}

void CMainWindow::on_actionAddMaterial_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();

	CDlgAddPhysicsItem dlg("Add Material", FE_MATERIAL, prj, false, this);
	if (dlg.exec())
	{
		FSMaterial* pmat = FEMaterialFactory::Create(FE_FEBIO_MATERIAL); assert(pmat);
		FEBio::CreateMaterial(dlg.GetClassID(), dynamic_cast<FEBioMaterial*>(pmat));
		if (pmat)
		{
			FSModel& fem = *doc->GetFSModel();

			// create a material
			// this also assigns a default name
			GMaterial* gmat = new GMaterial(pmat);

			// override name if specified
			std::string sname = dlg.GetName();
			const char* sz = sname.c_str();
			if (sz && (strlen(sz) > 0)) gmat->SetName(sz);

			// add the material
			doc->DoCommand(new CCmdAddMaterial(&fem, gmat));

			UpdateModel(gmat);
		}
	}
}

void CMainWindow::on_actionAddMeshAdaptor_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();

	CDlgAddPhysicsItem dlg("Add Mesh Adaptor", FE_MESH_ADAPTOR, prj, true, this);
	if (dlg.exec())
	{

	}
}

void CMainWindow::on_actionAddStep_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FSModel* fem = doc->GetFSModel();
//	CDlgAddStep dlg(prj, this);
//	if (dlg.exec())
	{
//		FSAnalysisStep* ps = fecore_new<FSAnalysisStep>(fem, FE_ANALYSIS, dlg.m_ntype); 

		vector<FEBio::FEBioClassInfo> l = FEBio::FindAllClasses(0, FE_ANALYSIS, -1);
		assert(l.size() == 1);

		FEStep* ps = fecore_new<FEStep>(fem, FE_ANALYSIS, FE_STEP_FEBIO_ANALYSIS);
		FEBio::CreateStep(l[0].classId, ps);
		assert(ps);
		if (ps)
		{
//			std::string name = dlg.m_name;
//			if (name.empty()) name = defaultStepName(fem, ps);
			std::string name = defaultStepName(fem, ps);

			ps->SetName(name);
			doc->DoCommand(new CCmdAddStep(fem, ps, -1));
			UpdateModel(ps);
		}
	}
}

void CMainWindow::on_actionAddReaction_triggered()
{
	CDlgAddChemicalReaction dlg(this);
	dlg.exec();

	// update the model editor
	UpdateModel(0, true);
	Update();
}

void CMainWindow::on_actionAddMembraneReaction_triggered()
{
    CDlgAddMembraneReaction dlg(this);
    dlg.exec();
    
    // update the model editor
    UpdateModel(0, true);
    Update();
}

void CMainWindow::on_actionSoluteTable_triggered()
{
	CDlgSoluteTable dlg(CDlgSoluteTable::ShowSolutes, this);
	dlg.exec();

	// since the number of solutes could have changed, let's rebuild the entire model tree
	UpdateModel(0, true);
}

void CMainWindow::on_actionSBMTable_triggered()
{
	CDlgSoluteTable dlg(CDlgSoluteTable::ShowSBMs, this);
	dlg.exec();

	// since the number of solutes could have changed, let's rebuild the entire model tree
	UpdateModel(0, true);
}
