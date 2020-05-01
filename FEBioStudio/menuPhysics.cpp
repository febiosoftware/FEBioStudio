#include "stdafx.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "DlgAddBC.h"
#include "DlgAddNodalLoad.h"
#include "DlgAddSurfaceLoad.h"
#include "DlgAddBodyLoad.h"
#include "DlgAddIC.h"
#include "DlgAddContact.h"
#include "DlgAddConstraint.h"
#include "DlgAddStep.h"
#include "DlgSoluteTable.h"
#include "DlgAddChemicalReaction.h"
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
#include "Commands.h"
#include <QMessageBox>
#include <sstream>

void CMainWindow::on_actionAddBC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FEModel& fem = prj.GetFEModel();
	CDlgAddBC dlg(prj, this);
	if (dlg.exec())
	{
		FEBoundaryCondition* pbc = fecore_new<FEBoundaryCondition>(&fem, FE_ESSENTIAL_BC, dlg.m_bctype); assert(pbc);
		if (pbc)
		{
			FEStep* step = fem.GetStep(dlg.m_nstep);

			pbc->SetStep(step->GetID());

			std::string name = dlg.m_name;
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

	FEModel& fem = *doc->GetFEModel();
	CDlgAddNodalLoad dlg(fem, this);
	if (dlg.exec())
	{
		FENodalLoad* pnl = new FENodalLoad(&fem, 0, dlg.m_nvar, dlg.m_val, dlg.m_nstep);

		std::string name = dlg.m_name;
		if (name.empty()) name = defaultLoadName(&fem, pnl);
		pnl->SetName(name);
		FEStep* step = fem.GetStep(dlg.m_nstep);

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

		step->AddLoad(pnl);
		UpdateModel(pnl);
	}
}

void CMainWindow::on_actionAddSurfLoad_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FEModel& fem = prj.GetFEModel();
	CDlgAddSurfaceLoad dlg(prj, this);
	if (dlg.exec())
	{
		FESurfaceLoad* psl = fecore_new<FESurfaceLoad>(&fem, FE_SURFACE_LOAD, dlg.m_ntype); assert(psl);
		if (psl)
		{
			string name = dlg.m_name;
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

			FEStep* step = fem.GetStep(dlg.m_nstep);
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
	FEModel& fem = *doc->GetFEModel();
	CDlgAddBodyLoad dlg(prj, this);
	if (dlg.exec())
	{
		FEBodyLoad* pbl = fecore_new<FEBodyLoad>(&fem, FE_BODY_LOAD, dlg.m_ntype);
		if (pbl)
		{
			std::string name = dlg.m_name;
			if (name.empty()) name = defaultLoadName(&fem, pbl);
			pbl->SetName(name);

			FEStep* step = fem.GetStep(dlg.m_nstep);
			pbl->SetStep(step->GetID());
			step->AddLoad(pbl);
			UpdateModel(pbl);
		}
	}
}

void CMainWindow::on_actionAddIC_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FEModel& fem = *doc->GetFEModel();
	CDlgAddIC dlg(prj, this);
	if (dlg.exec())
	{
		FEInitialCondition* pic = fecore_new<FEInitialCondition>(&fem, FE_INITIAL_CONDITION, dlg.m_ntype); assert(pic);
		if (pic)
		{
			std::string name = dlg.m_name;
			if (name.empty()) name = defaultICName(&fem, pic);
			pic->SetName(name);

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

			FEStep* step = fem.GetStep(dlg.m_nstep);
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
	FEModel& fem = *doc->GetFEModel();
	CDlgAddContact dlg(prj, this);
	if (dlg.exec())
	{
		FEInterface* pi = fecore_new<FEInterface>(&fem, FE_INTERFACE, dlg.m_ntype); assert(pi);
		if (pi)
		{
			// create a name
			std::string name = dlg.m_name;
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
			FEStep* step = fem.GetStep(dlg.m_nstep);
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
	FEModel& fem = *doc->GetFEModel();
	CDlgAddConstraint dlg(prj, this);
	if (dlg.exec())
	{
		FEModelConstraint* pi = fecore_new<FEModelConstraint>(&fem, FE_CONSTRAINT, dlg.m_ntype); assert(pi);
		if (pi)
		{
			// create a name
			std::string name = dlg.m_name;
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
			FEStep* step = fem.GetStep(dlg.m_nstep);
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
		FEModel* fem = &prj.GetFEModel();
		FERigidConstraint* prc = fecore_new<FERigidConstraint>(fem, FE_RIGID_CONSTRAINT, dlg.m_type);
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
		FEModel* fem = doc->GetFEModel();
		FERigidConnector* pc = fecore_new<FERigidConnector>(fem, FE_RIGID_CONNECTOR, dlg.GetType());
		assert(pc);
		if (pc)
		{
			pc->SetPosition(doc->Get3DCursor());
			pc->m_rbA = dlg.GetMaterialA();
			pc->m_rbB = dlg.GetMaterialB();

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
	FEProject& prj = doc->GetProject();

	CMaterialEditor dlg(this);
	dlg.SetModules(prj.GetModule());
	if (dlg.exec())
	{
		FEMaterial* pmat = dlg.GetMaterial();
		if (pmat)
		{
			FEModel& fem = *doc->GetFEModel();

			// create a material
			// this also assigns a default name
			GMaterial* gmat = new GMaterial(pmat);

			// override name if specified
			std::string sname = dlg.GetMaterialName().toStdString();
			const char* sz = sname.c_str();
			if (sz && (strlen(sz) > 0)) gmat->SetName(sz);

			// add the material
			doc->DoCommand(new CCmdAddMaterial(&fem, gmat));

			UpdateModel(gmat);
		}
	}
}

void CMainWindow::on_actionAddStep_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEProject& prj = doc->GetProject();
	FEModel* fem = doc->GetFEModel();
	CDlgAddStep dlg(prj, this);
	if (dlg.exec())
	{
		FEAnalysisStep* ps = fecore_new<FEAnalysisStep>(fem, FE_ANALYSIS, dlg.m_ntype); 
		assert(ps);
		if (ps)
		{
			std::string name = dlg.m_name;
			if (name.empty()) name = defaultStepName(fem, ps);

			ps->SetName(name);
			doc->DoCommand(new CCmdAddStep(fem, ps));
			prj.ActivatePlotVariables(ps);
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
