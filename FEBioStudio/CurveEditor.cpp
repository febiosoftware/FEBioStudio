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
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <MeshTools/GModel.h>
#include <sstream>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include "DlgAddPhysicsItem.h"
#include <FEMLib/FEMKernel.h>
#include <FEBioLink/FEBioInterface.h>


//=============================================================================
QRect CCurveEditor::m_preferredSize;

CCurveEditor::CCurveEditor(CMainWindow* wnd) : m_wnd(wnd), QMainWindow(wnd), ui(new Ui::CCurveEdior)
{
	m_fem = nullptr;
	m_plc = nullptr;
	m_currentItem = 0;
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
	ui->plot->Clear();
	m_currentItem = 0;

	// get the active document and FSModel
	CModelDocument* doc = m_wnd->GetModelDocument();
	if (doc == nullptr) return;

	m_fem = doc->GetFSModel();
	if (m_fem == nullptr) return;

	// fill the load controller selection widget
	ui->selectLC->clear();
	ui->selectLC->addItem("(none)");
	for (int i = 0; i < m_fem->LoadControllers(); ++i)
	{
		FSLoadController* plc = m_fem->GetLoadController(i);
		ui->selectLC->addItem(QString::fromStdString(plc->GetName()), plc->GetID());
	}

	// build the model tree
	if (m_nflt == FLT_LOAD_CURVES)
	{
		BuildLoadCurves();
	}
	else
	{
		BuildModelTree();
	}
}

void CCurveEditor::BuildLoadCurves(QTreeWidgetItem* t1, FSModelComponent* po)
{
	FSModel* fem = po->GetFSModel();
	int np = po->Parameters();
	for (int n = 0; n < np; ++n)
	{
		Param& p = po->GetParam(n);
		if (p.GetLoadCurveID() > 0)
		{
			assert(p.IsVolatile());
			string name = po->GetName() + "." + p.GetLongName();
			ui->addTreeItem(t1, QString::fromStdString(name), &p);
		}
	}
}

void CCurveEditor::BuildMaterialCurves(QTreeWidgetItem* t1, FSMaterial* mat, const std::string& name)
{
	FSModel* fem = mat->GetFSModel();
	int NP = mat->Parameters();
	for (int n = 0; n < NP; ++n)
	{
		Param& p = mat->GetParam(n);
		if (p.GetLoadCurveID() > 0)
		{
			assert(p.IsVolatile());
			string paramName = name + "." + p.GetShortName();
			ui->addTreeItem(t1, QString::fromStdString(paramName), &p);
		}
	}

	NP = mat->Properties();
	for (int n = 0; n < NP; ++n)
	{
		FSProperty& matProp = mat->GetProperty(n);

		int np = matProp.Size();
		if (np == 1)
		{
			FSMaterial* pm = mat->GetMaterialProperty(n);
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
				FSMaterial* pm = mat->GetMaterialProperty(n, j);
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

	FSModel& fem = *doc->GetFSModel();
	GModel& model = fem.GetModel();

	for (int i = 0; i<model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		GLinearSpring* pls = dynamic_cast<GLinearSpring*>(po);
		if (pls)
		{
			LoadCurve* plc = nullptr;// fem.GetParamCurve(pls->GetParam(GLinearSpring::MP_E));
			if (plc)
			{
                plc->Update();
				string name = pls->GetName() + ".E";
				ui->addTreeItem(t1, QString::fromStdString(name));
			}
		}

		GGeneralSpring* pgs = dynamic_cast<GGeneralSpring*>(po);
		if (pgs)
		{
			LoadCurve* plc = nullptr;// fem.GetParamCurve(pgs->GetParam(GGeneralSpring::MP_F));
			if (plc)
			{
                plc->Update();
				string name = pls->GetName() + ".F";
				ui->addTreeItem(t1, QString::fromStdString(name));
			}
		}
	}

	// add the materials
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pgm = fem.GetMaterial(i);
		FSMaterial* pm = pgm->GetMaterialProperties();
		if (pm)
		{
			BuildMaterialCurves(t1, pm, pgm->GetName());
		}
	}

	// add the boundary condition data
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		int nbc = pstep->BCs();
		for (int j = 0; j<nbc; ++j)
		{
			FSBoundaryCondition* pbc = pstep->BC(j);
			if (pbc) BuildLoadCurves(t1, pbc);
		}
	}

	// add the load data
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		int nbc = pstep->Loads();
		for (int j = 0; j < nbc; ++j)
		{
			FSLoad* plj = pstep->Load(j);
			BuildLoadCurves(t1, plj);
		}
	}

	// add contact interfaces
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Interfaces(); ++j)
		{
			FSInterface* pi = pstep->Interface(j);
			BuildLoadCurves(t1, pi);
		}
	}

	// add nonlinear constraints
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j < pstep->Constraints(); ++j)
		{
			FSModelConstraint* pmc = pstep->Constraint(j);
			BuildLoadCurves(t1, pmc);
		}
	}

	// add rigid constraints
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->RigidBCs(); ++j)
		{
			FSRigidBC* pc = pstep->RigidBC(j);
			if (pc) BuildLoadCurves(t1, pc);
		}
	}

	// add rigid initial conditions
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->RigidICs(); ++j)
		{
			FSRigidIC* pc = pstep->RigidIC(j);
			if (pc) BuildLoadCurves(t1, pc);
		}
	}

	// add rigid lodas
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->RigidLoads(); ++j)
		{
			FSRigidLoad* pc = pstep->RigidLoad(j);
			if (pc) BuildLoadCurves(t1, pc);
		}
	}

	// add rigid connectors
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->RigidConnectors(); ++j)
		{
			FSRigidConnector* pc = pstep->RigidConnector(j);
			if (pc) BuildLoadCurves(t1, pc);
		}
	}

	// discrete materials
	for (int i = 0; i < model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
//		if (po) BuildLoadCurves(t1, po);

/*		GDiscreteSpringSet* dss = dynamic_cast<GDiscreteSpringSet*>(po);
		if (dss)
		{
			FSDiscreteMaterial* dm = dss->GetMaterial();
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
		FSStep* pstep = fem.GetStep(i);
		FSAnalysisStep* pas = dynamic_cast<FSAnalysisStep*>(pstep);
		if (pas && pas->GetSettings().bmust)
		{
			string name = pas->GetName() + ".must point";
			ui->addTreeItem(t1, QString::fromStdString(name));
		}
	}
}

void CCurveEditor::AddParameterList(QTreeWidgetItem* t1, FSModelComponent* po)
{
	FSModel* fem = po->GetFSModel();
	for (int n = 0; n < po->Parameters(); ++n)
	{
		Param& param = po->GetParam(n);
		if (param.IsEditable() && param.IsVolatile())
		{
			ui->addTreeItem(t1, QString::fromStdString(param.GetShortName()), &param);
		}
	}
}

void CCurveEditor::BuildModelTree()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

	QTreeWidgetItem* t1 = new QTreeWidgetItem(ui->tree);
	t1->setExpanded(true);
	t1->setText(0, "Model");

	FSModel& fem = *doc->GetFSModel();
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
//				LoadCurve* plc = fem.GetParamCurve(pls->GetParam(GLinearSpring::MP_E));
//				if (plc) ui->addTreeItem(t3, "E", plc);
			}

			GGeneralSpring* pgs = dynamic_cast<GGeneralSpring*>(po);
			if (pgs)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(pgs->GetName()));
//				LoadCurve* plc = fem.GetParamCurve(pgs->GetParam(GGeneralSpring::MP_F));
//				if (plc) ui->addTreeItem(t3, "F", plc);
			}
		}
	}

	// add data maps
	if (Filter(FLT_MESH_DATA))
	{
		t2 = ui->addTreeItem(t1, "Mesh data");
		for (int i = 0; i < fem.MeshDataGenerators(); ++i)
		{
			FSMeshDataGenerator* map = fem.GetMeshDataGenerator(i);
			t3 = ui->addTreeItem(t2, QString::fromStdString(map->GetName()));
			AddParameterList(t3, map);
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

			FSMaterial* pm = pgm->GetMaterialProperties();
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
			FSStep* pstep = fem.GetStep(i);
			int nbc = pstep->BCs();
			for (int j = 0; j<nbc; ++j)
			{
				FSBoundaryCondition* pbc = pstep->BC(j);
				if (pbc)
				{
					QTreeWidgetItem* t3 = ui->addTreeItem(t2, QString::fromStdString(pbc->GetName()));
					AddParameterList(t3, pbc);
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
			FSStep* pstep = fem.GetStep(i);
			int nbc = pstep->Loads();
			for (int j = 0; j<nbc; ++j)
			{
				FSLoad* plj = pstep->Load(j);
				t3 = ui->addTreeItem(t2, QString::fromStdString(plj->GetName()));
				AddParameterList(t3, plj);
			}
		}
	}

	// add contact interfaces
	if (Filter(FLT_CONTACT))
	{
		t2 = ui->addTreeItem(t1, "Contact");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FSStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->Interfaces(); ++j)
			{
				FSInterface* pi = pstep->Interface(j);
				t3 = ui->addTreeItem(t2, QString::fromStdString(pi->GetName()));
				AddParameterList(t3, pi);
			}
		}
	}

	// add nonlinear constraints
	if (Filter(FLT_NLCONSTRAINT))
	{
		t2 = ui->addTreeItem(t1, "Constraints");
		for (int i = 0; i < fem.Steps(); ++i)
		{
			FSStep* pstep = fem.GetStep(i);
			for (int j = 0; j < pstep->Constraints(); ++j)
			{
				FSModelConstraint* pmc = pstep->Constraint(j);
				t3 = ui->addTreeItem(t2, QString::fromStdString(pmc->GetName()));
				AddParameterList(t3, pmc);
			}
		}
	}

	// add rigid constraints
	if (Filter(FLT_RIGID_CONSTRAINT))
	{
		t2 = ui->addTreeItem(t1, "Rigid BC");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FSStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->RigidBCs(); ++j)
			{
				FSRigidBC* pc = pstep->RigidBC(j);
				t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
				AddParameterList(t3, pc);
			}
		}

		for (int i = 0; i < fem.Steps(); ++i)
		{
			FSStep* pstep = fem.GetStep(i);
			for (int j = 0; j < pstep->RigidICs(); ++j)
			{
				FSRigidIC* pc = pstep->RigidIC(j);
				t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
				AddParameterList(t3, pc);
			}
		}

		for (int i = 0; i < fem.Steps(); ++i)
		{
			FSStep* pstep = fem.GetStep(i);
			for (int j = 0; j < pstep->RigidLoads(); ++j)
			{
				FSRigidLoad* pc = pstep->RigidLoad(j);
				t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
				AddParameterList(t3, pc);
			}
		}
	}

	// add rigid connectors
	if (Filter(FLT_RIGID_CONNECTOR))
	{
		t2 = ui->addTreeItem(t1, "Rigid Connectors");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FSStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->RigidConnectors(); ++j)
			{
				FSRigidConnector* pc = pstep->RigidConnector(j);
				t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
				AddParameterList(t3, pc);
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
			if (po && (po->Parameters() > 0))
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(po->GetName()));
//				AddParameterList(t3, po);
			}

			GDiscreteSpringSet* dss = dynamic_cast<GDiscreteSpringSet*>(po);
			if (dss)
			{
				FSDiscreteMaterial* dm = dss->GetMaterial();
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
			FSStep* pstep = fem.GetStep(i);
			t3 = ui->addTreeItem(t2, QString::fromStdString(pstep->GetName()));
			FSAnalysisStep* pas = dynamic_cast<FSAnalysisStep*>(pstep);
//			if (pas && pas->GetSettings().bmust) ui->addTreeItem(t3, "must point", pas->GetMustPointLoadCurve());
		}
	}
}

//-----------------------------------------------------------------------------
void CCurveEditor::AddMaterial(FSMaterial* pm, QTreeWidgetItem* tp)
{
	AddParameterList(tp, pm);
	AddMultiMaterial(pm, tp);
}

//-----------------------------------------------------------------------------
void CCurveEditor::AddMultiMaterial(FSMaterial* pm, QTreeWidgetItem* tp)
{
	for (int k = 0; k<pm->Properties(); ++k)
	{
		FSProperty& pmc = pm->GetProperty(k);
		for (int l=0; l<pmc.Size(); ++l)
		{
			FSMaterial* pmat = pm->GetMaterialProperty(k, l);
			if (pmat)
			{
				QString name = QString::fromStdString(pmc.GetName());
				const char* sztype = pmat->GetTypeString();
				if (sztype) name = QString("%1 [%2]").arg(name).arg(sztype);
				QTreeWidgetItem* tc = ui->addTreeItem(tp, name);
				AddParameterList(tc, pmat);
				AddMultiMaterial(pmat, tc);
			}
		}
	}
}

void CCurveEditor::on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
	m_currentItem = dynamic_cast<CCurveEditorItem*>(current);
	if (m_currentItem)
	{
		Param* p = m_currentItem->GetParam();
		if (p)
		{
			int lcId = p->GetLoadCurveID();
			ui->setCurrentLC(lcId);
		}
		else ui->deactivate();
	}
	else
	{
		ui->setCurrentLC(-1);
		ui->deactivate();
	}
}

void CCurveEditor::SetActiveLoadController(FSLoadController* plc)
{
	m_plc = plc;

	if (plc == nullptr)
	{
		ui->stack->setCurrentIndex(0);
		return;
	}

	int panel = 0;
	if (plc->IsType("loadcurve"))
	{
		panel = 1;
		ui->plot->SetLoadCurve(plc->CreateLoadCurve());
		ui->plot->repaint();
	}
	else if (plc->IsType("math"))
	{
		panel = 2;
		Param* p = plc->GetParam("math"); assert(p);
		ui->math->SetMath(QString::fromStdString(p->GetStringValue()));
	}
	else panel = 3;
	{
		ui->props->SetFEClass(plc, m_fem);
	}
	ui->stack->setCurrentIndex(panel);
}

void CCurveEditor::on_filter_currentIndexChanged(int n)
{
	m_nflt = n;
	Update();
}

vector<double> processLine(const char* szline, char delim)
{
	vector<double> v;
	const char* sz = szline;
	while (sz)
	{
		double w = atof(sz);
		v.push_back(w);
		sz = strchr(sz, delim);
		if (sz) sz++;
	}
	return v;
}

void CCurveEditor::on_newLC_clicked(bool b)
{
	CModelDocument* doc = m_wnd->GetModelDocument();
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();

	CDlgAddPhysicsItem dlg("Add Load Controller", FELOADCONTROLLER_ID, -1, prj, true, false, this);
	if (dlg.exec())
	{
		FSModel* fem = &prj.GetFSModel();
		FSLoadController* plc = FEBio::CreateFEBioClass<FSLoadController>(dlg.GetClassID(), fem);
		assert(plc);
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
			m_wnd->UpdateModel();

			// add it to the list
			ui->selectLC->addItem(QString::fromStdString(plc->GetName()), plc->GetID());
			ui->setCurrentLC(plc->GetID());
		}
	}
}

void CCurveEditor::on_selectLC_currentIndexChanged(int index)
{
	if (index < 0) return;

	if (m_currentItem == nullptr) return;
	Param* p = m_currentItem->GetParam();
	if (p == nullptr) return;

	if (index == 0)
	{
		p->SetLoadCurveID(-1);
		SetActiveLoadController(nullptr);
	}
	else
	{
		FSLoadController* plc = m_fem->GetLoadController(index - 1); assert(plc);
		p->SetLoadCurveID(plc->GetID());
		SetActiveLoadController(plc);
	}
}

void CCurveEditor::on_plot_dataChanged()
{
	if (m_currentItem == nullptr) return;
	Param* p = m_currentItem->GetParam();
	if (p == nullptr) return;
	
	int lcid = p->GetLoadCurveID();
	if (lcid < 0) return;

	FSLoadController* plc = m_fem->GetLoadControllerFromID(lcid); assert(plc);
	if (plc) plc->UpdateData(true);
}

void CCurveEditor::on_math_mathChanged(QString s)
{
	if (m_plc == nullptr) return;
	if (m_plc->IsType("math") == false) return;

	Param* p = m_plc->GetParam("math"); assert(p);

	std::string t = s.toStdString();
	p->SetStringValue(t);
}
