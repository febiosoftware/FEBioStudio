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
#include "DlgEditProject.h"
#include <MeshTools/FEModel.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/FEMeshAdaptor.h>
#include <FEMLib/FEMeshDataGenerator.h>
#include "Commands.h"
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioModule.h>
#include <QMessageBox>
#include <sstream>

void CMainWindow::on_actionAddNodalBC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Boundary Condition", FEBC_ID, FEBio::GetBaseClassIndex("FENodalBC"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSBoundaryCondition* pbc = FEBio::CreateFEBioClass<FSBoundaryCondition>(dlg.GetClassID(), &fem); assert(pbc);
		if (pbc)
		{
			FEBio::InitDefaultProps(pbc);

			FSStep* step = fem.GetStep(dlg.GetStep());

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

void CMainWindow::on_actionAddSurfaceBC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Boundary Condition", FEBC_ID, FEBio::GetBaseClassIndex("FESurfaceBC"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSBoundaryCondition* pbc = FEBio::CreateFEBioClass<FSBoundaryCondition>(dlg.GetClassID(), &fem); assert(pbc);
		if (pbc)
		{
			FEBio::InitDefaultProps(pbc);

			FSStep* step = fem.GetStep(dlg.GetStep());

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
				case SELECT_FE_FACES:
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

void CMainWindow::on_actionAddGeneralBC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Boundary Condition", FEBC_ID, FEBio::GetBaseClassIndex("FEBoundaryCondition"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSBoundaryCondition* pbc = FEBio::CreateFEBioClass<FSBoundaryCondition>(dlg.GetClassID(), &fem); assert(pbc);
		if (pbc)
		{
			FEBio::InitDefaultProps(pbc);

			FSStep* step = fem.GetStep(dlg.GetStep());

			pbc->SetStep(step->GetID());

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultBCName(&fem, pbc);
			pbc->SetName(name);
			step->AddBC(pbc);
			UpdateModel(pbc);
		}
	}
}


void CMainWindow::on_actionAddNodalLoad_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Nodal Load", FELOAD_ID, FEBio::GetBaseClassIndex("FENodalLoad"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSNodalLoad* pnl = FEBio::CreateFEBioClass<FSNodalLoad>(dlg.GetClassID(), &fem); assert(pnl);
		if (pnl)
		{
			FEBio::InitDefaultProps(pnl);

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

			FSStep* step = fem.GetStep(dlg.GetStep());
			step->AddLoad(pnl);
			UpdateModel(pnl);
		}
	}
}

void CMainWindow::on_actionAddSurfLoad_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Surface Load", FELOAD_ID, FEBio::GetBaseClassIndex("FESurfaceLoad"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSSurfaceLoad* psl = FEBio::CreateFEBioClass<FSSurfaceLoad>(dlg.GetClassID(), &fem); assert(psl);
		if (psl)
		{
			FEBio::InitDefaultProps(psl);

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

			FSStep* step = fem.GetStep(dlg.GetStep());
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

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Body Load", FELOAD_ID, FEBio::GetBaseClassIndex("FEBodyLoad"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSBodyLoad* pbl = FEBio::CreateFEBioClass<FSBodyLoad>(dlg.GetClassID(), &fem); assert(pbl);
		if (pbl)
		{
			FEBio::InitDefaultProps(pbl);

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultLoadName(&fem, pbl);
			pbl->SetName(name);

			FSStep* step = fem.GetStep(dlg.GetStep());
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

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Rigid Load", FELOAD_ID, FEBio::GetBaseClassIndex("FERigidLoad"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSRigidLoad* prl = FEBio::CreateFEBioClass<FSRigidLoad>(dlg.GetClassID(), &fem); assert(prl);
		if (prl)
		{
			FEBio::InitDefaultProps(prl);

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultRigidLoadName(&fem, prl);
			prl->SetName(name);

			FSStep* step = fem.GetStep(dlg.GetStep());
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

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Initial Condition", FEIC_ID, -1, &fem, true, true, this);
	if (dlg.exec())
	{
		FSInitialCondition* pic = FEBio::CreateFEBioClass<FSInitialCondition>(dlg.GetClassID(), &fem); assert(pic);
		if (pic)
		{
			FEBio::InitDefaultProps(pic);

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultICName(&fem, pic);
			pic->SetName(name);

			if (dynamic_cast<FSInitialNodalDOF*>(pic))
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

			FSStep* step = fem.GetStep(dlg.GetStep());
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

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Contact Interface", FESURFACEINTERFACE_ID, -1, &fem, true, true, this);
	if (dlg.exec())
	{
		FSInterface* pi = FEBio::CreateFEBioClass<FSInterface>(dlg.GetClassID(), &fem); assert(pi);
		if (pi)
		{
			FEBio::InitDefaultProps(pi);

			// create a name
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultInterfaceName(&fem, pi);
			pi->SetName(name);

			// assign selected surfaces (for solo interfaces)
			if (dynamic_cast<FSSoloInterface*>(pi))
			{
				FSSoloInterface& si = dynamic_cast<FSSoloInterface&>(*pi);

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
			FSStep* step = fem.GetStep(dlg.GetStep());
			pi->SetStep(step->GetID());
			step->AddInterface(pi);
			UpdateModel(pi);
		}
	}
}

void CMainWindow::on_actionAddGenericNLC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Constraint", FENLCONSTRAINT_ID, FEBio::GetBaseClassIndex("FENLConstraint"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSModelConstraint* pi = FEBio::CreateFEBioClass<FSModelConstraint>(dlg.GetClassID(), &fem); assert(pi);
		if (pi)
		{
			FEBio::InitDefaultProps(pi);

			// create a name
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultConstraintName(&fem, pi);
			pi->SetName(name);

			// assign it to the correct step
			FSStep* step = fem.GetStep(dlg.GetStep());
			pi->SetStep(step->GetID());
			step->AddConstraint(pi);
			UpdateModel(pi);
		}
	}
}

void CMainWindow::on_actionAddSurfaceNLC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Surface Constraint", FENLCONSTRAINT_ID, FEBio::GetBaseClassIndex("FESurfaceConstraint"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSSurfaceConstraint* pi = FEBio::CreateFEBioClass<FSSurfaceConstraint>(dlg.GetClassID(), &fem); assert(pi);
		if (pi)
		{
			FEBio::InitDefaultProps(pi);

			// create a name
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultConstraintName(&fem, pi);
			pi->SetName(name);

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
					pi->SetItemList(items);
				}
				break;
				}
			}

			// assign it to the correct step
			FSStep* step = fem.GetStep(dlg.GetStep());
			pi->SetStep(step->GetID());
			step->AddConstraint(pi);
			UpdateModel(pi);
		}
	}
}

void CMainWindow::on_actionAddBodyNLC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();
	CDlgAddPhysicsItem dlg("Add Body Constraint", FENLCONSTRAINT_ID, FEBio::GetBaseClassIndex("FEBodyConstraint"), &fem, true, true, this);
	if (dlg.exec())
	{
		FSBodyConstraint* pi = FEBio::CreateFEBioClass<FSBodyConstraint>(dlg.GetClassID(), &fem); assert(pi);
		if (pi)
		{
			FEBio::InitDefaultProps(pi);

			// create a name
			std::string name = dlg.GetName();
			if (name.empty()) name = defaultConstraintName(&fem, pi);
			pi->SetName(name);

			// assign it to the correct step
			FSStep* step = fem.GetStep(dlg.GetStep());
			pi->SetStep(step->GetID());
			step->AddConstraint(pi);
			UpdateModel(pi);
		}
	}
}

void CMainWindow::on_actionAddRigidBC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel* fem = &prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Rigid Constraint", FEBC_ID, FEBio::GetBaseClassIndex("FERigidBC"), fem, true, true, this);
	if (dlg.exec())
	{
		FSRigidBC* prc = FEBio::CreateFEBioClass<FSRigidBC>(dlg.GetClassID(), fem);
		assert(prc);
		if (prc)
		{
			FEBio::InitDefaultProps(prc);

			FSStep* step = fem->GetStep(dlg.GetStep());
			prc->SetStep(step->GetID());

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultRigidBCName(fem, prc);

			prc->SetName(name);
			step->AddRigidBC(prc);
			UpdateModel(prc);
		}
	}
}

void CMainWindow::on_actionAddRigidIC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel* fem = &prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Rigid Initial Condition", FEIC_ID, FEBio::GetBaseClassIndex("FERigidIC"), fem, true, true, this);
	if (dlg.exec())
	{
		FSRigidIC* prc = FEBio::CreateFEBioClass<FSRigidIC>(dlg.GetClassID(), fem);
		assert(prc);
		if (prc)
		{
			FEBio::InitDefaultProps(prc);

			FSStep* step = fem->GetStep(dlg.GetStep());
			prc->SetStep(step->GetID());

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultRigidICName(fem, prc);

			prc->SetName(name);
			step->AddRigidIC(prc);
			UpdateModel(prc);
		}
	}
}

void CMainWindow::on_actionAddRigidConnector_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel* fem = doc->GetFSModel();
	//	CDlgAddRigidConnector dlg(prj, this);
	CDlgAddPhysicsItem dlg("Add Rigid Connector", FENLCONSTRAINT_ID, FEBio::GetBaseClassIndex("FERigidConnector"), fem, true, true, this);
	if (dlg.exec())
	{
		FSRigidConnector* pc = FEBio::CreateFEBioClass<FSRigidConnector>(dlg.GetClassID(), fem);
		assert(pc);
		if (pc)
		{
			FEBio::InitDefaultProps(pc);

			pc->SetPosition(GetGLView()->Get3DCursor());
//			pc->SetRigidBody1(dlg.GetMaterialA());
//			pc->SetRigidBody2(dlg.GetMaterialB());

			FSStep* step = fem->GetStep(dlg.GetStep());
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

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();

	CDlgAddPhysicsItem dlg("Add Material", FEMATERIAL_ID, -1, &fem, true, false, this);
	if (dlg.exec())
	{
		FSMaterial* pmat = FEBio::CreateFEBioClass<FSMaterial>(dlg.GetClassID(), &fem);
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

	FSProject& prj = doc->GetProject();
	FSModel* fem = &prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Mesh Adaptor", FEMESHADAPTOR_ID, -1, fem, true, true, this);
	if (dlg.exec())
	{
		FSMeshAdaptor* pma = FEBio::CreateFEBioClass<FSMeshAdaptor>(dlg.GetClassID(), fem); assert(pma);
		if (pma)
		{
			FSStep* step = fem->GetStep(dlg.GetStep());
			pma->SetStep(step->GetID());

			std::string name = dlg.GetName();
			if (name.empty()) name = defaultMeshAdaptorName(fem, pma);

			pma->SetName(name);
			step->AddMeshAdaptor(pma);
			UpdateModel(pma);
		}
	}
}

void CMainWindow::on_actionAddLoadController_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel* fem = &prj.GetFSModel();
	CDlgAddPhysicsItem dlg("Add Load Controller", FELOADCONTROLLER_ID, -1, fem, true, false, this);
	if (dlg.exec())
	{
		FSLoadController* plc = FEBio::CreateFEBioClass<FSLoadController>(dlg.GetClassID(), fem); assert(plc);
		if (plc)
		{
			std::string name = dlg.GetName();
			if (name.empty())
			{
				int n = fem->LoadControllers();
				std::stringstream ss; ss << "LC" << n + 1;
				name = ss.str();
			}

			plc->SetName(name);
			fem->AddLoadController(plc);
			UpdateModel(plc);
		}
	}
}

void CMainWindow::on_actionAddMeshData_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel* fem = &prj.GetFSModel();

	CDlgAddPhysicsItem dlg("Add Mesh Data", FEMESHDATAGENERATOR_ID, -1, fem, true, false, this);
	if (dlg.exec())
	{
		FSMeshDataGenerator* pmd = FEBio::CreateFEBioClass<FSMeshDataGenerator>(dlg.GetClassID(), fem); assert(pmd);
		if (pmd)
		{
			std::string name = dlg.GetName();
			if (name.empty())
			{
				int n = fem->MeshDataGenerators();
				std::stringstream ss; ss << "MeshData" << n + 1;
				name = ss.str();
			}

			pmd->SetName(name);
			fem->AddMeshDataGenerator(pmd);
			UpdateModel(pmd);
		}
	}
}

void CMainWindow::on_actionAddStep_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel* fem = doc->GetFSModel();
//	CDlgAddStep dlg(prj, this);
//	if (dlg.exec())
	{
//		FSAnalysisStep* ps = fscore_new<FSAnalysisStep>(fem, FE_ANALYSIS, dlg.m_ntype); 

		FSStep* ps = FEBio::CreateStep(FEBio::GetActiveModuleName(), fem);
		assert(ps);
		if (ps)
		{
//			std::string name = dlg.m_name;
//			if (name.empty()) name = defaultStepName(fem, ps);
			std::string name = defaultStepName(fem, ps);

			FEBio::InitDefaultProps(ps);

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

void CMainWindow::on_actionEditProject_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgEditProject dlg(doc->GetProject(), this);
	dlg.exec();
	UpdatePhysicsUi();
}
