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
#include "MeshMorphTool.h"
#include "ModelDocument.h"
#include <MeshTools/LaplaceSolver.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GGroup.h>
#include <MeshLib/FENodeData.h>
#include <MeshLib/FEElementData.h>
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
#include "MainWindow.h"
#include "Commands.h"

class CMeshMorphTool::Ui : public QWidget
{
public:
	QPushButton* m_add;
	QPushButton* m_del;
	QPushButton* m_clr;
	QPushButton* m_apply;
	QTableWidget* m_table;
	QLineEdit* m_val;

	QLineEdit* m_maxIters;
	QLineEdit* m_tol;
	QLineEdit* m_sor;

public:
	Ui(CMeshMorphTool* tool)
	{
		QVBoxLayout* l = new QVBoxLayout;

		m_add = new QPushButton("Add");
		m_del = new QPushButton("Remove");
		m_clr = new QPushButton("Clear");
		m_val = new QLineEdit;
		m_val->setText(Vec3dToString(vec3d(0,0,0)));

		QHBoxLayout* h2 = new QHBoxLayout;
		h2->addWidget(new QLabel("Value"));
		h2->addWidget(m_val);
		h2->addWidget(m_add);
		h2->addWidget(m_del);
		h2->addWidget(m_clr);

		l->addLayout(h2);
		l->addWidget(m_table = new QTableWidget);
		m_table->setColumnCount(2);
		m_table->horizontalHeader()->setStretchLastSection(true);
		m_table->setHorizontalHeaderLabels(QStringList() << "selection" << "value");
		m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

		QFormLayout* f = new QFormLayout;
		f->setContentsMargins(0, 0, 0, 0);
		f->addRow("Max iterations:", m_maxIters = new QLineEdit); m_maxIters->setText(QString::number(1000));
		f->addRow("Tolerance:", m_tol = new QLineEdit); m_tol->setText(QString::number(1e-4));
		f->addRow("SOR parameter:", m_sor = new QLineEdit); m_sor->setText(QString::number(1.8));

		m_maxIters->setValidator(new QIntValidator());
		m_tol->setValidator(new QDoubleValidator());
		m_sor->setValidator(new QDoubleValidator());

		l->addLayout(f);

		l->addWidget(m_apply = new QPushButton("Apply"));

		l->addStretch();

		setLayout(l);

		connect(m_add  , &QPushButton::clicked, tool, &CMeshMorphTool::OnAddClicked);
		connect(m_del  , &QPushButton::clicked, tool, &CMeshMorphTool::OnRemoveClicked);
		connect(m_clr  , &QPushButton::clicked, tool, &CMeshMorphTool::OnClearClicked);
		connect(m_apply, &QPushButton::clicked, tool, &CMeshMorphTool::OnApply);
	}
};

CMeshMorphTool::CMeshMorphTool(CMainWindow* wnd) : CAbstractTool(wnd, "Mesh Morph")
{
	m_po = nullptr;
	ui = nullptr;
}

QWidget* CMeshMorphTool::createUi()
{
	if (ui == nullptr) ui = new CMeshMorphTool::Ui(this);
	return ui;
}

void CMeshMorphTool::OnAddClicked()
{
	vec3d v = StringToVec3d(ui->m_val->text());

	GObject* po = GetMainWindow()->GetActiveObject();
	if (m_po == nullptr) m_po = po;
	if (m_po == po)
	{
		CModelDocument* doc = GetMainWindow()->GetModelDocument();
		FESelection* sel = doc->GetCurrentSelection();
		if (sel)
		{
			FEItemListBuilder* items = sel->CreateItemList();
			if (items)
			{
				m_data.push_back(items);

				int n = ui->m_table->rowCount();
				ui->m_table->insertRow(n);

				QString name = QString("selection%1").arg(n);
				if (items->size() == 1)
				{
					if (dynamic_cast<GFaceList*>(items))
					{
						GFaceList* fl = dynamic_cast<GFaceList*>(items);
						name = QString::fromStdString(fl->GetFaceList()[0]->GetName());
					}
					else if (dynamic_cast<GEdgeList*>(items))
					{
						GEdgeList* el = dynamic_cast<GEdgeList*>(items);
						name = QString::fromStdString(el->GetEdgeList()[0]->GetName());
					}
					else if (dynamic_cast<GNodeList*>(items))
					{
						GNodeList* nl = dynamic_cast<GNodeList*>(items);
						name = QString::fromStdString(nl->GetNodeList()[0]->GetName());
					}
					else if (dynamic_cast<GPartList*>(items))
					{
						GPartList* pl = dynamic_cast<GPartList*>(items);
						name = QString::fromStdString(pl->GetPartList()[0]->GetName());
					}
				}

				QTableWidgetItem* it0 = new QTableWidgetItem;
				it0->setText(name);
				it0->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				ui->m_table->setItem(n, 0, it0);

				QTableWidgetItem* it1 = new QTableWidgetItem;
				it1->setText(Vec3dToString(v));
				it1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
				ui->m_table->setItem(n, 1, it1);
			}
		}
	}
}

void CMeshMorphTool::OnRemoveClicked()
{
	int n = ui->m_table->currentRow();
	if (n >= 0)
	{
		ui->m_table->removeRow(n);
		m_data.erase(m_data.begin() + n);
	}
}

void CMeshMorphTool::OnClearClicked()
{
	Clear();
}

void CMeshMorphTool::Activate()
{
	Clear();
	CAbstractTool::Activate();
}

void CMeshMorphTool::Clear()
{
	m_po = nullptr;
	for (int i = 0; i < m_data.size(); ++i) delete m_data[i];
	m_data.clear();
	ui->m_table->clear();
	ui->m_table->setRowCount(0);
}

void CMeshMorphTool::OnApply()
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());

	// get the currently selected object
	GObject* po = pdoc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "You must first select an object.");
		return;
	}
	GMeshObject* pmo = dynamic_cast<GMeshObject*>(po);
	if (pmo == nullptr)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "This tool can only be applied to an editable mesh.");
		return;
	}

	// make sure there is a mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "The object needs to be meshed before you can apply this tool.");
		return;
	}

	// make sure there is work to do
	if (m_data.empty())
	{
		QMessageBox::critical(GetMainWindow(), "Tool", "You need to define some boundary conditions.");
		return;
	}

	//get the model and nodeset
	FSModel* ps = pdoc->GetFSModel();
	GModel& model = ps->GetModel();

	int NN = pm->Nodes();
	std::vector<int> bn(NN, 0);
	std::vector<double> val[3];
	val[0].assign(NN, 0.0);
	val[1].assign(NN, 0.0);
	val[2].assign(NN, 0.0);

	for (int i = 0; i < m_data.size(); ++i)
	{
		FEItemListBuilder* item = m_data[i];
		vec3d v = StringToVec3d(ui->m_table->item(i, 1)->text());

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
				val[0][nid] = v.x;
				val[1][nid] = v.y;
				val[2][nid] = v.z;
				bn[nid] = 1;
			}
		}
	}

	CMainWindow* wnd = GetMainWindow();
	wnd->AddLogEntry("Starting Laplace solve ...\n");

	// tag all elements that should be included in the solve
	pm->TagAllElements(1);

	// get parameters
	int maxIter = ui->m_maxIters->text().toInt();
	double tol = ui->m_tol->text().toDouble();
	double w = ui->m_sor->text().toDouble();

	wnd->AddLogEntry(QString("max iters     = %1\n").arg(maxIter));
	wnd->AddLogEntry(QString("tolerance     = %1\n").arg(tol));
	wnd->AddLogEntry(QString("SOR parameter = %1\n").arg(w));

	// solve Laplace equation
#pragma omp parallel for
	for (int i = 0; i < 3; ++i)
	{
		LaplaceSolver L;
		L.SetMaxIterations(maxIter);
		L.SetTolerance(tol);
		L.SetRelaxation(w);
		bool b = L.Solve(pm, val[i], bn, 1);
		int niters = L.GetIterationCount();
	}

	FSMesh* newMesh = new FSMesh(*pm);

	// apply morph
#pragma omp parallel for
	for (int i = 0; i < newMesh->Nodes(); ++i)
	{
		vec3d& ri = newMesh->Node(i).r;
		double dx = val[0][i];
		double dy = val[1][i];
		double dz = val[2][i];
		ri.x += dx;
		ri.y += dy;
		ri.z += dz;
	}
	newMesh->UpdateNormals();
	newMesh->UpdateBoundingBox();

	pdoc->DoCommand(new CCmdChangeFEMesh(po, newMesh));
}
