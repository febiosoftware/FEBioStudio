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
#include "FiberGeneratorTool.h"
#include "ModelDocument.h"
#include "MainWindow.h"
#include <MeshTools/GradientMap.h>
#include <MeshTools/LaplaceSolver.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FSNodeData.h>
#include <MeshLib/FSElementData.h>
#include <QLineEdit>
#include <QBoxLayout>
#include <QFormLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QValidator>
#include <QMessageBox>
#include <QLabel>
#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>

class UIFiberGeneratorTool : public QWidget
{
public:
	QPushButton*	m_add;
	QPushButton*	m_apply;
	QTableWidget*	m_table;
	QLineEdit*		m_val;
	QCheckBox*		m_matAxes;
	QCheckBox*		m_cross;
	QLineEdit*		m_otPlane;
	QLineEdit*		m_inPlane;

	QComboBox*	m_matList;

	QLineEdit*	m_maxIters;
	QLineEdit*	m_tol;
	QLineEdit*	m_sor;

	QLineEdit* m_normal;

	QCheckBox* m_genMap;
	QLineEdit* m_mapName;

public:
	UIFiberGeneratorTool(CFiberGeneratorTool* w)
	{
		QVBoxLayout* l = new QVBoxLayout;

		m_add = new QPushButton("Add");
		m_val = new QLineEdit; m_val->setValidator(new QDoubleValidator);
		m_val->setText("0");

		QHBoxLayout* h2 = new QHBoxLayout;
		h2->addWidget(new QLabel("Value"));
		h2->addWidget(m_val);
		h2->addWidget(m_add);

		l->addLayout(h2);
		l->addWidget(m_table = new QTableWidget);
		m_table->setColumnCount(2);
		m_table->horizontalHeader()->setStretchLastSection(true);
		m_table->setHorizontalHeaderLabels(QStringList() << "selection" << "value");

		QFormLayout* f = new QFormLayout;
		f->setContentsMargins(0,0,0,0);
		f->addRow("Material:", m_matList = new QComboBox);
		f->addRow("Max iterations:", m_maxIters = new QLineEdit); m_maxIters->setText(QString::number(1000));
		f->addRow("Tolerance:", m_tol = new QLineEdit); m_tol->setText(QString::number(1e-4));
		f->addRow("SOR parameter:", m_sor = new QLineEdit); m_sor->setText(QString::number(1.8));
		f->addRow("Generate mat axes:", m_matAxes = new QCheckBox);
		f->addRow("Generate cross product:", m_cross = new QCheckBox);
		f->addRow("Normal vector:", m_normal = new QLineEdit);
		f->addRow("Out-plane rotation (deg):", m_otPlane = new QLineEdit()); m_otPlane->setValidator(new QDoubleValidator());
		f->addRow("In-plane rotation (deg):", m_inPlane = new QLineEdit()); m_inPlane->setValidator(new QDoubleValidator());
		f->addRow(m_genMap = new QCheckBox("Generate map"), m_mapName = new QLineEdit()); m_mapName->setDisabled(true);
		m_normal->setText(Vec3dToString(vec3d(0, 0, 1)));

		m_otPlane->setText(QString::number(0));
		m_inPlane->setText(QString::number(0));

		m_maxIters->setValidator(new QIntValidator());
		m_tol->setValidator(new QDoubleValidator());
		m_sor->setValidator(new QDoubleValidator());

		l->addLayout(f);

		l->addWidget(m_apply = new QPushButton("Apply"));

		l->addStretch();

		setLayout(l);

		QObject::connect(m_add, SIGNAL(clicked()), w, SLOT(OnAddClicked()));
		QObject::connect(m_apply, SIGNAL(clicked()), w, SLOT(OnApply()));
		QObject::connect(m_normal, SIGNAL(editingFinished()), w, SLOT(validateNormal()));
		QObject::connect(m_genMap, SIGNAL(toggled(bool)), m_mapName, SLOT(setEnabled(bool)));
	}
};


CFiberGeneratorTool::CFiberGeneratorTool(CMainWindow* wnd) : CAbstractTool(wnd, "Fiber generator")
{
	m_nsmoothIters = 0;
	m_po = nullptr;
	ui = nullptr;
}

QWidget* CFiberGeneratorTool::createUi()
{
	if (ui == nullptr) ui = new UIFiberGeneratorTool(this);
	return ui;
}

void CFiberGeneratorTool::Activate()
{
	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	ui->m_matList->clear();
	if (doc)
	{
		FSModel* fem = doc->GetFSModel();
		if (fem)
		{
			int nmat = fem->Materials();
			for (int i = 0; i < nmat; ++i)
			{
				GMaterial* mat = fem->GetMaterial(i); assert(mat);
				if (mat)
				{
					ui->m_matList->addItem(QString::fromStdString(mat->GetName()));
				}
				else ui->m_matList->addItem(QString("(material %1)").arg(i));
			}
		}
	}

	Clear();
	CAbstractTool::Activate();
}

void CFiberGeneratorTool::Clear()
{
	m_po = nullptr;
	for (int i = 0; i < m_data.size(); ++i) delete m_data[i];
	m_data.clear();
	ui->m_table->clear();
	ui->m_table->setRowCount(0);
}

void CFiberGeneratorTool::OnAddClicked()
{
	double v = ui->m_val->text().toDouble();

	GObject* po = GetMainWindow()->GetActiveObject();
	if (m_po == nullptr) m_po = po;
	if (m_po == po)
	{
		CModelDocument* doc = GetMainWindow()->GetModelDocument();
		FESelection* sel = doc->GetCurrentSelection();
		if (sel)
		{
			FSItemListBuilder* items = sel->CreateItemList();
			if (items)
			{
				m_data.push_back(items);

				int n = ui->m_table->rowCount();
				ui->m_table->insertRow(n);

				QTableWidgetItem* it0 = new QTableWidgetItem;
				it0->setText(QString("selection%1").arg(n));
				it0->setFlags(Qt::ItemIsSelectable);
				ui->m_table->setItem(n, 0, it0);

				QTableWidgetItem* it1 = new QTableWidgetItem;
				it1->setText(QString::number(v));
				ui->m_table->setItem(n, 1, it1);
			}
		}
	}
}

void CFiberGeneratorTool::validateNormal()
{
	QString s = ui->m_normal->text();
	vec3d v = StringToVec3d(s);
	s = Vec3dToString(v);
	ui->m_normal->blockSignals(true);
	ui->m_normal->setText(s);
	ui->m_normal->blockSignals(false);
}

void CFiberGeneratorTool::OnApply()
{
	CGLDocument* pdoc = GetDocument();
	GObject* po = pdoc->GetActiveObject();
	if (po == 0) 
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "You must select an object first.");
		return;
	}

	FSMesh* pm = po->GetFEMesh();
	if (pm == 0) 
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "The selected object does not have a mesh.");
		return;
	}

	if (m_data.size() == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "No selections were added.");
		return;
	}

	bool genMap = ui->m_genMap->isChecked();
	QString mapName;
	if (genMap)
	{
		mapName = ui->m_mapName->text();
		if (mapName.isEmpty())
		{
			QMessageBox::critical(GetMainWindow(), "Tool", "Please provide a name for the map.");
			return;
		}
	}

	int NN = pm->Nodes();
	std::vector<int> bn(NN, 0);
	std::vector<double> val(NN, 0.0);

	for (int i = 0; i < m_data.size(); ++i)
	{
		FSItemListBuilder* item = m_data[i];
		double v = ui->m_table->item(i, 1)->text().toDouble();

		FSNodeList* node = item->BuildNodeList(); assert(node);
		if (node)
		{
			for (int i = 0; i < NN; ++i) pm->Node(i).m_ntag = i;

			FSNodeList::Iterator it = node->First();
			int nn = node->Size();
			for (int j = 0; j < nn; ++j, it++)
			{
				assert(it->m_pm == pm);
				int nid = it->m_pi->m_ntag;
				assert((nid >= 0) && (nid < NN));
				val[nid] = v;
				bn[nid] = 1;
			}
		}
	}

	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry("Starting Laplace solve ...\n");

	// tag all elements that should be included in the solve
	pm->TagAllElements(-1);
	int n = ui->m_matList->currentIndex();
	int matId = -1;
	if (n >= 0)
	{
		CModelDocument* doc = GetMainWindow()->GetModelDocument();
		if (doc)
		{
			FSModel* fem = doc->GetFSModel();
			if (fem)
			{
				GMaterial* mat = fem->GetMaterial(n);
				if (mat)
				{
					matId = mat->GetID();

					int NE = pm->Elements();
					for (int i = 0; i < NE; ++i)
					{
						FSElement& el = pm->Element(i);

						int pid = el.m_gid;
						GPart* pg = po->Part(pid); assert(pg);
						if (pg && (pg->GetMaterialID() == matId))
						{
							el.m_ntag = 1;
						}
					}
				}
			}
		}
	}

	// get parameters
	int maxIter = ui->m_maxIters->text().toInt();
	double tol = ui->m_tol->text().toDouble();
	double w = ui->m_sor->text().toDouble();

	wnd->AddLogEntry(QString("max iters     = %1\n").arg(maxIter));
	wnd->AddLogEntry(QString("tolerance     = %1\n").arg(tol));
	wnd->AddLogEntry(QString("SOR parameter = %1\n").arg(w));

	// solve Laplace equation
	LaplaceSolver L;
	L.SetMaxIterations(maxIter);
	L.SetTolerance(tol);
	L.SetRelaxation(w);
	bool b = L.Solve(pm, val, bn, 1);
	int niters = L.GetIterationCount();
	wnd->AddLogEntry(QString("%1").arg(b ? "Converged!\n" : "NOT converged!\n"));
	wnd->AddLogEntry(QString("iteration count: %1\n").arg(niters));
	wnd->AddLogEntry(QString("Final relative norm: %1\n").arg(L.GetRelativeNorm()));

	// create a temporary node set from the mesh
	FSNodeSet nodeSet(pm);
	nodeSet.CreateFromMesh();

	// create node data
	FSNodeData data(m_po);
	data.Create(&nodeSet, 0.0);
	for (int i = 0; i < NN; i++) data.setScalar(i, val[i]);

	// calculate gradient and assign to element fiber
	std::vector<vec3d> grad;
	GradientMap G;
	G.Apply(data, grad, m_nsmoothIters);

	vec3d N = StringToVec3d(ui->m_normal->text());
	N.unit();

	bool cross = ui->m_cross->isChecked();
	if (cross)
	{
		// calculate cross product
		for (int i = 0; i < grad.size(); ++i)
		{
			vec3d a = grad[i];
			vec3d b = a ^ N;
			b.unit();
			grad[i] = b;
		}
	}

	// do plane rotations
	double outAngle = ui->m_otPlane->text().toDouble();
	if (outAngle != 0)
	{
		for (int i = 0; i < grad.size(); ++i)
		{
			vec3d a = grad[i] ^ N;
			quatd q(outAngle * PI / 180.0, a);
			q.RotateVector(grad[i]);
		}
	}

	double inAngle  = ui->m_inPlane->text().toDouble();
	if (inAngle != 0)
	{
		quatd q(inAngle* PI / 180.0, N);
		for (int i = 0; i < grad.size(); ++i)
		{
			q.RotateVector(grad[i]);
		}
	}


	bool matAxes = ui->m_matAxes->isChecked();
	if (matAxes == false)
	{
		if (genMap)
		{
			int parts = po->Parts();
			FSPartSet* partSet = new FSPartSet(pm);
			partSet->SetName(mapName.toStdString());
			for (int i = 0; i < parts; ++i)
			{
				GPart* pg = po->Part(i);
				if (pg->GetMaterialID() == matId)
				{
					partSet->add(i);
				}
			}
			po->AddFEPartSet(partSet);

			FSPartData* pdata = new FSPartData(po->GetFEMesh());
			pdata->SetName(mapName.toStdString());
			pdata->Create(partSet, DATA_VEC3, DATA_ITEM);
			pm->AddMeshDataField(pdata);

			for (int i = 0; i < pm->Elements(); ++i)
			{
				pm->Element(i).m_ntag = i;
			}

			FSElemList* elemList = pdata->BuildElemList();
			int NE = elemList->Size();
			auto it = elemList->First();
			int n = 0;
			for (int i = 0; i < NE; ++i, ++it)
			{
				FSElement_& el = *it->m_pi;
				pdata->FSMeshData::set(n++, grad[el.m_ntag]);
			}
			delete elemList;

			GetMainWindow()->UpdateModel(pdata);
		}
		else
		{
			// assign to element fibers
			int NE = pm->Elements();
			for (int i = 0; i < NE; ++i)
			{
				FSElement& el = pm->Element(i);
				if (el.m_ntag == 1)
				{
					el.m_fiber = grad[i];
				}
			}
		}
	}
	else
	{
		// assign to mat axes
		int NE = pm->Elements();
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.m_ntag == 1)
			{
				vec3d N(0, 0, 1);
				if (el.IsShell())
				{
					// get the normal to the shell
					vec3d r0 = pm->Node(el.m_node[0]).r;
					vec3d r1 = pm->Node(el.m_node[1]).r;
					vec3d r2 = pm->Node(el.m_node[2]).r;
					vec3d e1 = r1 - r0;
					vec3d e2 = r2 - r0;
					N = e1 ^ e2; N.Normalize();
				}

				// setup orthogonal axes
				vec3d a1 = grad[i]; a1.Normalize();
				vec3d a2 = N ^ a1;
				vec3d a3 = a1 ^ a2;

				// setup rotation matrix
				mat3d& Q = el.m_Q;
				Q[0][0] = a1.x; Q[0][1] = a2.x; Q[0][2] = a3.x;
				Q[1][0] = a1.y; Q[1][1] = a2.y; Q[1][2] = a3.y;
				Q[2][0] = a1.z; Q[2][1] = a2.z; Q[2][2] = a3.z;

				el.m_Qactive = true;
			}
		}
	}

	GetMainWindow()->RedrawGL();
}
