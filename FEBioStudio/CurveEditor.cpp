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
#include "ui_curveeditor.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include "DlgFormula.h"
#include <MathLib/MathParser.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FERigidConstraint.h>
#include <MeshTools/GModel.h>
#include <sstream>

CCmdAddPoint::CCmdAddPoint(FELoadCurve* plc, LOADPOINT& pt) : CCommand("Add point")
{
	m_lc = plc;
	m_pt = pt;
	m_index = -1;
}

void CCmdAddPoint::Execute()
{
	m_index = m_lc->Add(m_pt);
    m_lc->Update();
}

void CCmdAddPoint::UnExecute()
{
	m_lc->Delete(m_index);
    m_lc->Update();
}

CCmdRemovePoint::CCmdRemovePoint(FELoadCurve* plc, const vector<int>& index) : CCommand("Remove point")
{
	m_lc = plc;
	m_index = index;
}

void CCmdRemovePoint::Execute()
{
	m_copy = *m_lc;
	m_lc->Delete(m_index);
    m_lc->Update();
}

void CCmdRemovePoint::UnExecute()
{
	*m_lc = m_copy;
    m_lc->Update();
}

CCmdMovePoint::CCmdMovePoint(FELoadCurve* plc, int index, LOADPOINT to) : CCommand("Move point")
{
	m_lc = plc;
	m_index = index;
	m_p = to;
    m_lc->Update();
}

void CCmdMovePoint::Execute()
{
	LOADPOINT tmp = m_lc->Item(m_index);
	m_lc->Item(m_index) = m_p;
	m_p = tmp;
    m_lc->Update();
}

void CCmdMovePoint::UnExecute()
{
	Execute();
}

CCmdDeleteCurve::CCmdDeleteCurve(Param* pp) : CCommand("Delete Curve")
{
	m_plc = nullptr;
	m_pp = pp; assert(m_pp);
}

CCmdDeleteCurve::~CCmdDeleteCurve()
{
	if (m_plc) delete m_plc;
	m_plc = nullptr;
}

void CCmdDeleteCurve::Execute()
{
	m_plc = m_pp->RemoveLoadCurve();
}

void CCmdDeleteCurve::UnExecute()
{
	if (m_plc) m_pp->SetLoadCurve(*m_plc);
	m_plc = nullptr;
}

//=============================================================================

QRect CCurveEditor::m_preferredSize;

CCurveEditor::CCurveEditor(CMainWindow* wnd) : m_wnd(wnd), QMainWindow(wnd), ui(new Ui::CCurveEdior)
{
	m_currentItem = 0;
	m_plc_copy = 0;
	m_nflt = FLT_ALL;
	ui->setupUi(this);
	resize(600, 400);

	setWindowTitle("Curve Editor");

	if (m_preferredSize.isValid())
	{
		// copy the x,y coordinates
		QRect rt = geometry();
		m_preferredSize.setX(rt.x());
		m_preferredSize.setY(rt.y());
		setGeometry(m_preferredSize);
	}
}

QRect CCurveEditor::preferredSize()
{
	return m_preferredSize;
}

void CCurveEditor::setPreferredSize(const QRect& rt)
{
	m_preferredSize = rt;
}

void CCurveEditor::closeEvent(QCloseEvent* ev)
{
	m_preferredSize = geometry();
}

void CCurveEditor::Update()
{
	// clear the tree
	ui->tree->clear();
	ui->plot->clearData();
	m_currentItem = 0;

	if (m_nflt == FLT_LOAD_CURVES)
	{
		BuildLoadCurves();
	}
	else
	{
		BuildModelTree();
	}
}

void CCurveEditor::BuildLoadCurves(QTreeWidgetItem* t1, FSObject* po)
{
	int np = po->Parameters();
	for (int n = 0; n < np; ++n)
	{
		Param& p = po->GetParam(n);
		FELoadCurve* plc = p.GetLoadCurve();
		if (plc)
		{
            plc->Update();
			string name = po->GetName() + "." + p.GetLongName();
			ui->addTreeItem(t1, QString::fromStdString(name), plc, &p);
		}
	}
}

void CCurveEditor::BuildMaterialCurves(QTreeWidgetItem* t1, FEMaterial* mat, const std::string& name)
{
	int NP = mat->Parameters();
	for (int n = 0; n < NP; ++n)
	{
		Param& p = mat->GetParam(n);
		FELoadCurve* plc = p.GetLoadCurve();
		if (plc)
		{
            plc->Update();
			string paramName = name + "." + p.GetShortName();
			ui->addTreeItem(t1, QString::fromStdString(paramName), plc, &p);
		}
	}

	NP = mat->Properties();
	for (int n = 0; n < NP; ++n)
	{
		FEMaterialProperty& matProp = mat->GetProperty(n);

		int np = matProp.Size();
		if (np == 1)
		{
			FEMaterial* pm = matProp.GetMaterial(0);
			if (pm)
			{
				string paramName = name + "." + matProp.GetName();
				BuildMaterialCurves(t1, pm, paramName);
			}
		}
		else
		{
			for (int j = 0; j < np; ++j)
			{
				FEMaterial* pm = matProp.GetMaterial(j);
				if (pm)
				{
					std::stringstream ss; 
					ss << name << "." << matProp.GetName() << "[" << j << "]";
					string paramName = ss.str();
					BuildMaterialCurves(t1, pm, paramName);
				}
			}
		}
	}
}

void CCurveEditor::BuildLoadCurves()
{
	QTreeWidgetItem* t1 = new QTreeWidgetItem(ui->tree);
	t1->setExpanded(true);
	t1->setText(0, "Model");

	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	if (doc == nullptr) return;

	FEModel& fem = *doc->GetFEModel();
	GModel& model = fem.GetModel();

	for (int i = 0; i<model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		GLinearSpring* pls = dynamic_cast<GLinearSpring*>(po);
		if (pls)
		{
			FELoadCurve* plc = pls->GetParam(GLinearSpring::MP_E).GetLoadCurve();
			if (plc)
			{
                plc->Update();
				string name = pls->GetName() + ".E";
				ui->addTreeItem(t1, QString::fromStdString(name), plc);
			}
		}

		GGeneralSpring* pgs = dynamic_cast<GGeneralSpring*>(po);
		if (pgs)
		{
			FELoadCurve* plc = pgs->GetParam(GGeneralSpring::MP_F).GetLoadCurve();
			if (plc)
			{
                plc->Update();
				string name = pls->GetName() + ".F";
				ui->addTreeItem(t1, QString::fromStdString(name), plc);
			}
		}
	}

	// add the materials
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pgm = fem.GetMaterial(i);
		FEMaterial* pm = pgm->GetMaterialProperties();
		if (pm)
		{
			BuildMaterialCurves(t1, pm, pgm->GetName());
		}
	}

	// add the boundary condition data
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		int nbc = pstep->BCs();
		for (int j = 0; j<nbc; ++j)
		{
			FEPrescribedDOF* pbc = dynamic_cast<FEPrescribedDOF*>(pstep->BC(j));
			if (pbc) BuildLoadCurves(t1, pbc);
		}
	}

	// add the load data
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		int nbc = pstep->Loads();
		for (int j = 0; j < nbc; ++j)
		{
			FELoad* plj = pstep->Load(j);
			BuildLoadCurves(t1, plj);
		}
	}

	// add contact interfaces
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Interfaces(); ++j)
		{
			FEInterface* pi = pstep->Interface(j);
			BuildLoadCurves(t1, pi);
		}
	}

	// add constraints
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->RigidConstraints(); ++j)
		{
			FERigidPrescribed* pc = dynamic_cast<FERigidPrescribed*>(pstep->RigidConstraint(j));
			if (pc) BuildLoadCurves(t1, pc);
		}
	}

	// add rigid connectors
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->RigidConnectors(); ++j)
		{
			FERigidConnector* pc = pstep->RigidConnector(j);
			if (pc) BuildLoadCurves(t1, pc);
		}
	}

	// discrete materials
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		if (po) BuildLoadCurves(t1, po);

/*		GDiscreteSpringSet* dss = dynamic_cast<GDiscreteSpringSet*>(po);
		if (dss)
		{
			FEDiscreteMaterial* dm = dss->GetMaterial();
			if (dm)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(dss->GetName()));
				AddMaterial(dm, t3);
			}
		}
*/
	}

	// must point curves
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		FEAnalysisStep* pas = dynamic_cast<FEAnalysisStep*>(pstep);
		if (pas && pas->GetSettings().bmust)
		{
			string name = pas->GetName() + ".must point";
			ui->addTreeItem(t1, QString::fromStdString(name), pas->GetMustPointLoadCurve());
		}
	}
}

void CCurveEditor::BuildModelTree()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

	QTreeWidgetItem* t1 = new QTreeWidgetItem(ui->tree);
	t1->setExpanded(true);
	t1->setText(0, "Model");

	FEModel& fem = *doc->GetFEModel();
	GModel& model = fem.GetModel();

	QTreeWidgetItem *t2, *t3;

	// add the discrete objects
	if (Filter(FLT_GEO))
	{
		t2 = ui->addTreeItem(t1, "Geometry");
		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GDiscreteObject* po = model.DiscreteObject(i);
			GLinearSpring* pls = dynamic_cast<GLinearSpring*>(po);
			if (pls)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(pls->GetName()));
				FELoadCurve* plc = pls->GetParam(GLinearSpring::MP_E).GetLoadCurve();
				if (plc) ui->addTreeItem(t3, "E", plc);
			}

			GGeneralSpring* pgs = dynamic_cast<GGeneralSpring*>(po);
			if (pgs)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(pgs->GetName()));
				FELoadCurve* plc = pgs->GetParam(GGeneralSpring::MP_F).GetLoadCurve();
				if (plc) ui->addTreeItem(t3, "F", plc);
			}
		}
	}

	// add the materials
	if (Filter(FLT_MAT))
	{
		t2 = ui->addTreeItem(t1, "Materials");
		for (int i = 0; i<fem.Materials(); ++i)
		{
			GMaterial* pgm = fem.GetMaterial(i);
			t3 = ui->addTreeItem(t2, QString::fromStdString(pgm->GetName()));

			FEMaterial* pm = pgm->GetMaterialProperties();
			if (pm)
			{
				AddMaterial(pm, t3);
			}
		}
	}

	// add the boundary condition data
	if (Filter(FLT_BC))
	{
		t2 = ui->addTreeItem(t1, "BCs");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			int nbc = pstep->BCs();
			for (int j = 0; j<nbc; ++j)
			{
				FEPrescribedDOF* pbc = dynamic_cast<FEPrescribedDOF*>(pstep->BC(j));
				if (pbc)
				{
					t3 = ui->addTreeItem(t2, QString::fromStdString(pbc->GetName()));
					Param& p = pbc->GetParam(FEPrescribedDOF::SCALE);
					ui->addTreeItem(t3, QString::fromStdString(pbc->GetName()), p.GetLoadCurve(), &p);
				}
			}
		}
	}

	// add the load data
	if (Filter(FLT_LOAD))
	{
		t2 = ui->addTreeItem(t1, "Loads");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			int nbc = pstep->Loads();
			for (int j = 0; j<nbc; ++j)
			{
				FELoad* plj = pstep->Load(j);

                FEFluidFlowResistance* pfr = dynamic_cast<FEFluidFlowResistance*>(pstep->Load(j));
                FEFluidFlowRCR* prcr = dynamic_cast<FEFluidFlowRCR*>(pstep->Load(j));
                if (pfr) {
					t3 = ui->addTreeItem(t2, QString::fromStdString(plj->GetName()));
                    ui->addTreeItem(t3, QString::fromStdString("R"), pfr->GetLoadCurve());
                    ui->addTreeItem(t3, QString::fromStdString("pressure_offset"), pfr->GetPOLoadCurve());
                }
                else if (prcr) {
					t3 = ui->addTreeItem(t2, QString::fromStdString(plj->GetName()));
					ui->addTreeItem(t3, QString::fromStdString("R"), prcr->GetLoadCurve());
                    ui->addTreeItem(t3, QString::fromStdString("Rd"), prcr->GetRDLoadCurve());
                    ui->addTreeItem(t3, QString::fromStdString("capacitance"), prcr->GetCOLoadCurve());
                    ui->addTreeItem(t3, QString::fromStdString("pressure_offset"), prcr->GetPOLoadCurve());
                    ui->addTreeItem(t3, QString::fromStdString("initial_pressure"), prcr->GetIPLoadCurve());
                }
				else {
					FEConstBodyForce* pbl = dynamic_cast<FEConstBodyForce*>(pstep->Load(j));
					if (pbl)
					{
						t3 = ui->addTreeItem(t2, QString::fromStdString(pbl->GetName()));
						if (pbl->GetLoadCurve(0)) ui->addTreeItem(t3, "x-force", pbl->GetLoadCurve(0));
						if (pbl->GetLoadCurve(1)) ui->addTreeItem(t3, "y-force", pbl->GetLoadCurve(1));
						if (pbl->GetLoadCurve(2)) ui->addTreeItem(t3, "z-force", pbl->GetLoadCurve(2));
					}
					else {
						FEHeatSource* phs = dynamic_cast<FEHeatSource*>(pstep->Load(j));
                        FECentrifugalBodyForce* pcs = dynamic_cast<FECentrifugalBodyForce*>(pstep->Load(j));
						if (phs)
						{
							if (phs->GetLoadCurve())
							{
								t3 = ui->addTreeItem(t2, QString::fromStdString(plj->GetName()));
								ui->addTreeItem(t3, QString::fromStdString(phs->GetName()), phs->GetLoadCurve());
							}
						}
                        else if (pcs)
                        {
							if (pcs->GetLoadCurve())
							{
								t3 = ui->addTreeItem(t2, QString::fromStdString(plj->GetName()));
								ui->addTreeItem(t3, QString::fromStdString(pcs->GetName()), pcs->GetLoadCurve());
							}
                        }
						else
						{
							t3 = ui->addTreeItem(t2, QString::fromStdString(plj->GetName()));
							int N = plj->Parameters();
							for (int k = 0; k < N; ++k)
							{
								Param& pk = plj->GetParam(k);
								if (pk.IsEditable() && (pk.GetParamType() != Param_INT))
								{
									FELoadCurve* plc = pk.GetLoadCurve();
									ui->addTreeItem(t3, pk.GetLongName(), plc, &pk);
								}
							}
						}
					}
				}
			}
		}
	}

	// add contact interfaces
	if (Filter(FLT_CONTACT))
	{
		t2 = ui->addTreeItem(t1, "Contact");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->Interfaces(); ++j)
			{
				FEInterface* pi = pstep->Interface(j);
				FERigidWallInterface* pw = dynamic_cast<FERigidWallInterface*>(pi);
				if (pw) 
				{
					t3 = ui->addTreeItem(t2, QString::fromStdString(pw->GetName()));
					ui->addTreeItem(t3, "tolerance", pw->GetParamLC(FERigidWallInterface::ALTOL  ), pw->GetParamPtr(FERigidWallInterface::ALTOL  ));
					ui->addTreeItem(t3, "penalty"  , pw->GetParamLC(FERigidWallInterface::PENALTY), pw->GetParamPtr(FERigidWallInterface::PENALTY));
					ui->addTreeItem(t3, "plane displacement", pw->GetParamLC(FERigidWallInterface::OFFSET ), pw->GetParamPtr(FERigidWallInterface::OFFSET ));
				}
				else
				{
					FERigidSphereInterface* prs = dynamic_cast<FERigidSphereInterface*>(pi);
					if (prs)
					{
						t3 = ui->addTreeItem(t2, QString::fromStdString(prs->GetName()));
						ui->addTreeItem(t3, "ux", prs->GetLoadCurve(0));
						ui->addTreeItem(t3, "uy", prs->GetLoadCurve(1));
						ui->addTreeItem(t3, "uz", prs->GetLoadCurve(2));
					}
					else
					{
						FEInterface* pc = pstep->Interface(j);
						int NP = pc->Parameters();
						if (NP > 0)
						{
							t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
							for (int n = 0; n<NP; ++n)
							{
								Param& p = pc->GetParam(n);
								if (p.IsEditable())
								{
									FELoadCurve* plc = p.GetLoadCurve();
									ui->addTreeItem(t3, p.GetLongName(), plc, &p);
								}
							}
						}
					}
				}
			}
		}
	}

	// add constraints
	if (Filter(FLT_CONSTRAINT))
	{
		t2 = ui->addTreeItem(t1, "Rigid Constraints");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->RigidConstraints(); ++j)
			{
				FERigidPrescribed* pc = dynamic_cast<FERigidPrescribed*>(pstep->RigidConstraint(j));
				if (pc)
				{
					for (int n = 0; n < pc->Parameters(); ++n)
					{
						Param& p = pc->GetParam(n);
						if (p.IsEditable())
						{
							FELoadCurve* plc = p.GetLoadCurve();
							t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
							ui->addTreeItem(t3, p.GetLongName(), plc, &p);
						}
					}
				}
			}
		}
	}

	// add rigid connectors
	if (Filter(FLT_CONNECTOR))
	{
		t2 = ui->addTreeItem(t1, "Rigid Connectors");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->RigidConnectors(); ++j)
			{
				FERigidConnector* pc = pstep->RigidConnector(j);
				int NP = pc->Parameters();
				if (NP > 0)
				{
					t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
					for (int n = 0; n<NP; ++n)
					{
						Param& p = pc->GetParam(n);
						if (p.IsEditable())
						{
							FELoadCurve* plc = p.GetLoadCurve();
							ui->addTreeItem(t3, p.GetLongName(), plc, &p);
						}
					}
				}
			}
		}
	}

	// discrete materials
	if (Filter(FLT_DISCRETE))
	{
		t2 = ui->addTreeItem(t1, "Discrete");
		for (int i = 0; i < model.DiscreteObjects(); ++i)
		{
			GDiscreteObject* po = model.DiscreteObject(i);
			int NP = po->Parameters();
			if (NP > 0)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(po->GetName()));
				for (int n = 0; n<NP; ++n)
				{
					Param& p = po->GetParam(n);
					if (p.IsEditable())
					{
						FELoadCurve* plc = p.GetLoadCurve();
						ui->addTreeItem(t3, p.GetLongName(), plc, &p);
					}
				}
			}

			GDiscreteSpringSet* dss = dynamic_cast<GDiscreteSpringSet*>(po);
			if (dss)
			{
				FEDiscreteMaterial* dm = dss->GetMaterial();
				if (dm)
				{
					t3 = ui->addTreeItem(t2, QString::fromStdString(dss->GetName()));
					AddMaterial(dm, t3);
				}
			}
		}
	}

	// must point curves
	if (Filter(FLT_STEP))
	{
		t2 = ui->addTreeItem(t1, "Steps");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			t3 = ui->addTreeItem(t2, QString::fromStdString(pstep->GetName()));
			FEAnalysisStep* pas = dynamic_cast<FEAnalysisStep*>(pstep);
			if (pas && pas->GetSettings().bmust) ui->addTreeItem(t3, "must point", pas->GetMustPointLoadCurve());
		}
	}
}

//-----------------------------------------------------------------------------
void CCurveEditor::AddMaterial(FEMaterial* pm, QTreeWidgetItem* tp)
{
	int n = pm->Parameters();
	for (int j = 0; j<n; ++j)
	{
		Param& p = pm->GetParam(j);
		if (p.IsEditable())
		{
			FELoadCurve* plc = p.GetLoadCurve();
			ui->addTreeItem(tp, p.GetLongName(), plc, &p);
		}
	}
	AddMultiMaterial(pm, tp);
}

//-----------------------------------------------------------------------------
void CCurveEditor::AddMultiMaterial(FEMaterial* pm, QTreeWidgetItem* tp)
{
	for (int k = 0; k<pm->Properties(); ++k)
	{
		FEMaterialProperty& pmc = pm->GetProperty(k);
		for (int l=0; l<pmc.Size(); ++l)
		{
			FEMaterial* pmat = pmc.GetMaterial(l);
			if (pmat)
			{
				QString name = QString::fromStdString(pmc.GetName());
				const char* sztype = FEMaterialFactory::TypeStr(pmat);
				if (sztype) name = QString("%1 [%2]").arg(name).arg(sztype);
				QTreeWidgetItem* tc = ui->addTreeItem(tp, name);
				int n = pmat->Parameters();
				for (int j = 0; j<n; ++j)
				{
					Param& p = pmat->GetParam(j);
					if (p.IsEditable())
					{
						FELoadCurve* plc = p.GetLoadCurve();
						ui->addTreeItem(tc, p.GetLongName(), plc, &p);
					}
				}
				AddMultiMaterial(pmat, tc);
			}
		}
	}
}

void CCurveEditor::on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
	m_currentItem = dynamic_cast<CCurveEditorItem*>(current);
	m_cmd.Clear();
	if (m_currentItem)
	{
		FELoadCurve* plc = m_currentItem->GetLoadCurve();
		SetLoadCurve(plc);
	}
	else SetLoadCurve(0);
}

void CCurveEditor::SetLoadCurve(FELoadCurve* plc)
{
	ui->plot->clear();
	ui->plot->SetLoadCurve(plc);

	if (plc)
	{
		CPlotData* data = new CPlotData;
		for (int i=0; i<plc->Size(); ++i)
		{
			LOADPOINT pt = plc->Item(i);
			data->addPoint(pt.time, pt.load);
		}
		ui->plot->addPlotData(data);

		data->setLineColor(QColor(92, 255, 164));
		data->setFillColor(QColor(92, 255, 164));
		data->setLineWidth(2);
		data->setMarkerSize(5);

		ui->setCurveType(plc->GetType(), plc->GetExtend());
	}
	ui->plot->repaint();
}

void CCurveEditor::UpdateLoadCurve()
{
	if (m_currentItem == 0) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	if (plc == 0) return;

	CPlotData& data = ui->plot->getPlotData(0);
	assert(data.size() == plc->Size());
	for (int i = 0; i<data.size(); ++i)
	{
		LOADPOINT& pi = plc->Item(i);
		QPointF& po = data.Point(i);

		po.setX(pi.time);
		po.setY(pi.load);
	}
	ui->plot->repaint();
}


void CCurveEditor::on_filter_currentIndexChanged(int n)
{
	m_nflt = n;
	Update();
}

void CCurveEditor::on_plot_pointClicked(QPointF p, bool shift)
{
	if (m_currentItem == 0) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (plc == 0)
	{
		Param* pp = m_currentItem->GetParam();
		if (pp)
		{
			assert(pp->GetLoadCurve() == 0);
			pp->SetLoadCurve();
			plc = pp->GetLoadCurve();
			plc->Clear();
			m_currentItem->SetLoadCurve(plc);
		}
	}

	if (plc && (ui->isAddPointChecked() || shift))
	{
		if (ui->isSnapToGrid()) p = ui->plot->SnapToGrid(p);

		LOADPOINT lp(p.x(), p.y());

		CCmdAddPoint* cmd = new CCmdAddPoint(plc, lp);
		m_cmd.DoCommand(cmd);

		SetLoadCurve(plc);
		ui->plot->selectPoint(0, cmd->Index());
	}
}

void CCurveEditor::on_plot_pointSelected(int n)
{
	UpdateSelection();
}

void CCurveEditor::UpdateSelection()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	if (plc == nullptr) return;

	vector<CPlotWidget::Selection> sel = ui->plot->selection();

	if (sel.size() == 1)
	{
		ui->enablePointEdit(true);
		LOADPOINT pt = plc->Item(sel[0].npointIndex);
		ui->setPointValues(pt.time, pt.load);
	}
	else
	{
		ui->enablePointEdit(false);
	}
}

void CCurveEditor::on_plot_pointDragged(QPoint p)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	vector<CPlotWidget::Selection> sel = ui->plot->selection();
	if (sel.size() == 0) return;

	QPointF pf = ui->plot->ScreenToView(p);
	double dx = pf.x() - ui->m_dragPt.x();
	double dy = pf.y() - ui->m_dragPt.y();

	if ((ui->m_dragIndex >= 0) && (ui->isSnapToGrid()))
	{
		LOADPOINT& lp = plc->Item(ui->m_dragIndex);
		QPointF p0 = ui->m_p0[ui->m_dragIndex];
		QPointF pi(p0.x() + dx, p0.y() + dy);
		pi = ui->plot->SnapToGrid(pi);
		dx = pi.x() - p0.x();
		dy = pi.y() - p0.y();
	}

	for (int i=0; i<sel.size(); ++i)
	{
		int n = sel[i].npointIndex;
		LOADPOINT& lp = plc->Item(n);

		QPointF p0 = ui->m_p0[i];
		QPointF pi(p0.x() +dx, p0.y() + dy);

		lp.time = pi.x();
		lp.load = pi.y();

		ui->plot->getPlotData(0).Point(n) = pi;

		if (sel.size() == 1) ui->setPointValues(pi.x(), pi.y());
	}
    plc->Update();
	ui->plot->repaint();
}

void CCurveEditor::on_plot_draggingStart(QPoint p)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	ui->m_dragPt = ui->plot->ScreenToView(p);

	vector<CPlotWidget::Selection> sel = ui->plot->selection();
	ui->m_dragIndex = -1;
	ui->m_p0.clear();
	if (sel.size() > 0)
	{
		ui->m_p0.resize(sel.size());
		for (int i = 0; i < sel.size(); ++i)
		{
			LOADPOINT& lp = plc->Item(sel[i].npointIndex);
			ui->m_p0[i].setX(lp.time);
			ui->m_p0[i].setY(lp.load);

			QPointF pf(lp.time, lp.load);
			QPoint pi = ui->plot->ViewToScreen(pf);

			double dx = fabs(pi.x() - p.x());
			double dy = fabs(pi.y() - p.y());
			if ((dx <= 5) && (dy <= 5)) ui->m_dragIndex = i;
		}
	}
}

void CCurveEditor::on_plot_draggingEnd(QPoint p)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	vector<CPlotWidget::Selection> sel = ui->plot->selection();

	if (sel.size() == 1)
	{
		int n = sel[0].npointIndex;

		LOADPOINT lp0;
		lp0.time = ui->m_p0[0].x();
		lp0.load = ui->m_p0[0].y();

		LOADPOINT lp = plc->Item(n);
		plc->Item(n) = lp0;

		m_cmd.DoCommand(new CCmdMovePoint(plc, n, lp));

		UpdateLoadCurve();
	}
}

void CCurveEditor::on_open_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All files (*)");
	if (fileName.isEmpty() == false)
	{
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();
		if (plc->LoadData(szfile))
		{
			SetLoadCurve(plc);
		}
		else QMessageBox::critical(this, "Open File", QString("Failed loading curve data from file %1").arg(szfile));
	}
}

void CCurveEditor::on_plot_backgroundImageChanged()
{
	if (ui->plot->HasBackgroundImage())
	{
		ui->plot->setXAxisColor(QColor(0, 0, 0));
		ui->plot->setYAxisColor(QColor(0, 0, 0));
	}
	else
	{
		ui->plot->setXAxisColor(QColor(255, 255, 255));
		ui->plot->setYAxisColor(QColor(255, 255, 255));
	}
}

void CCurveEditor::on_plot_doneZoomToRect()
{

}

void CCurveEditor::on_plot_regionSelected(QRect rt)
{
	UpdateSelection();
}

void CCurveEditor::on_plot_doneSelectingRect(QRect rt)
{
	CDlgPlotWidgetProps dlg;
	if (dlg.exec())
	{
		ui->plot->mapToUserRect(rt, QRectF(dlg.m_xmin, dlg.m_ymin, dlg.m_xmax - dlg.m_xmin, dlg.m_ymax - dlg.m_ymin));
	}
}

void CCurveEditor::on_save_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	QString fileName = QFileDialog::getSaveFileName(this, "Open File", "", "All files (*)");
	if (fileName.isEmpty() == false)
	{
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();
		if (plc->WriteData(szfile) == false)
		{
			QMessageBox::critical(this, "Save File", QString("Failed saving curve data to file %1").arg(szfile));
		}
	}
}

void CCurveEditor::on_clip_triggered()
{
	ui->plot->OnCopyToClipboard();
}

void CCurveEditor::on_copy_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_plc_copy == 0) m_plc_copy = new FELoadCurve;
	*m_plc_copy = *plc;
}

void CCurveEditor::on_paste_triggered()
{
	if (m_currentItem == 0) return;
	if (m_plc_copy == 0) return;

	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	if (m_currentItem->GetLoadCurve() == 0)
	{
		Param* p = m_currentItem->GetParam();
		if (p)
		{
			p->SetLoadCurve();
			plc = p->GetLoadCurve();
			m_currentItem->SetLoadCurve(plc);
		}
	}

	if (plc)
	{
		*plc = *m_plc_copy;
		SetLoadCurve(plc);
	}
}

void CCurveEditor::on_delete_triggered()
{
	if (m_currentItem == nullptr) return;
	Param* p = m_currentItem->GetParam();
	if (p && p->GetLoadCurve())
	{
		m_cmd.DoCommand(new CCmdDeleteCurve(p));
		m_currentItem->SetLoadCurve(0);
		SetLoadCurve(0);
	}
	else
	{
		FELoadCurve* plc = m_currentItem->GetLoadCurve();
		if (plc)
		{
			plc->Clear();
			SetLoadCurve(plc);
		}
	}
}

void CCurveEditor::on_xval_textEdited()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	vector<CPlotWidget::Selection> sel = ui->plot->selection();

	if (sel.size() == 1)
	{
		QPointF p = ui->getPointValue();
		LOADPOINT& it = plc->Item(sel[0].npointIndex);
		it.time = p.x();
		it.load = p.y();
		UpdateLoadCurve();
	}
}

void CCurveEditor::on_yval_textEdited()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	vector<CPlotWidget::Selection> sel = ui->plot->selection();

	if (sel.size() == 1)
	{
		QPointF p = ui->getPointValue();
		LOADPOINT& it = plc->Item(sel[0].npointIndex);
		it.time = p.x();
		it.load = p.y();
		UpdateLoadCurve();
	}
}

void CCurveEditor::on_deletePoint_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	vector<CPlotWidget::Selection> sel = ui->plot->selection();

	if (sel.empty() == false)
	{
		vector<int> points;
		for (int i = 0; i < sel.size(); ++i) points.push_back(sel[i].npointIndex);

		m_cmd.DoCommand(new CCmdRemovePoint(plc, points));
		SetLoadCurve(plc);
		ui->enablePointEdit(false);
	}
}

void CCurveEditor::on_zoomToFit_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	ui->plot->OnZoomToFit();
}

void CCurveEditor::on_zoomX_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	ui->plot->OnZoomToWidth();
}

void CCurveEditor::on_zoomY_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	ui->plot->OnZoomToHeight();
}

void CCurveEditor::on_map_clicked()
{
	ui->plot->mapToUserRect();
}

void CCurveEditor::on_undo_triggered()
{
	if (m_currentItem == 0) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_cmd.CanUndo()) m_cmd.UndoCommand();

	QString undo = m_cmd.CanUndo() ? m_cmd.GetUndoCmdName() : "(Nothing to undo)";
	QString redo = m_cmd.CanRedo() ? m_cmd.GetRedoCmdName() : "(Nothing to redo)";
	ui->setCmdNames(undo, redo);

	Param* pp = m_currentItem->GetParam();
	if (plc) SetLoadCurve(plc);
	else if (pp) {
		plc = pp->GetLoadCurve(); 
		m_currentItem->SetLoadCurve(plc);
		SetLoadCurve(plc);
	}

	ui->enablePointEdit(false);
}

void CCurveEditor::on_redo_triggered()
{	
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_cmd.CanRedo()) m_cmd.RedoCommand();

	QString undo = m_cmd.CanUndo() ? m_cmd.GetUndoCmdName() : "(Nothing to undo)";
	QString redo = m_cmd.CanRedo() ? m_cmd.GetRedoCmdName() : "(Nothing to redo)";
	ui->setCmdNames(undo, redo);

	SetLoadCurve(plc);
	ui->enablePointEdit(false);
}

void CCurveEditor::on_math_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	CDlgFormula dlg(this);

	dlg.SetMath(plc->GetName());
	
	if (dlg.exec())
	{
		std::vector<LOADPOINT> pts = dlg.GetPoints();
		QString math = dlg.GetMath();
		std::string smath = math.toStdString();

		CMathParser m;
		bool insertMode = dlg.Insert();
		if (insertMode == false) plc->Clear();
		plc->SetName(smath.c_str());
		if (pts.empty() && (insertMode == false))
		{
			LOADPOINT p0(0, 0), p1(0, 0);
			plc->Add(p0);
			plc->Add(p1);
		}
		else
		{
			for (int i = 0; i < (int)pts.size(); ++i)
			{
				LOADPOINT& pt = pts[i];
				plc->Add(pt);
			}
		}

		SetLoadCurve(plc);
		ui->enablePointEdit(false);
	}	
}

void CCurveEditor::on_lineType_currentIndexChanged(int n)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	plc->SetType(n);
    plc->Update();
	ui->plot->repaint();
}

void CCurveEditor::on_extendMode_currentIndexChanged(int n)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	plc->SetExtend(n);
	ui->plot->repaint();
}

//=======================================================================================
void CCurvePlotWidget::DrawPlotData(QPainter& painter, CPlotData& data)
{
	if (m_lc == 0) return;

	int N = data.size();

	// draw the line
	painter.setPen(QPen(data.lineColor(), data.lineWidth()));
	QRect rt = ScreenRect();
	QPoint p0, p1;
	for (int i=rt.left(); i<rt.right(); i += 2)
	{
		p1.setX(i);
		QPointF p = ScreenToView(p1);
		p.setY(m_lc->Value(p.x()));
		p1 = ViewToScreen(p);

		if (i != rt.left())
		{
			painter.drawLine(p0, p1);
		}

		p0 = p1;
	}

	// draw the marks
	if (data.markerType() > 0)
	{
		painter.setBrush(data.fillColor());
		for (int i = 0; i<N; ++i)
		{
			p1 = ViewToScreen(data.Point(i));
			QRect r(p1.x() - 2, p1.y() - 2, 5, 5);
			painter.drawRect(r);
		}
	}
}
