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
#include <QLineEdit>
#include <QComboBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFormLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QToolButton>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <unordered_map>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FSProject.h>
#include "DlgMaterialTest.h"
#include <FEBioLink/FEBioInterface.h>
#include <FSCore/FSCore.h>
#include "PlotWidget.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "FEClassPropsView.h"
#include "units.h"

using namespace std;

class UIDlgMaterialTest
{
public:
	CMainWindow* m_wnd;
	QComboBox* m_mat;
	QComboBox* m_test;
	QLineEdit* m_steps;
	QLineEdit* m_time;
	QLineEdit* m_strain;
	FEClassEdit* m_props;

	CPlotWidget* plot;

public:
	void setup(CDlgMaterialTest* dlg)
	{
		m_mat = new QComboBox;
		m_test = new QComboBox;

		QFormLayout* f1 = new QFormLayout;
		f1->addRow("select material:", m_mat);

		QFormLayout* f2 = new QFormLayout;
		f2->addRow("select test:", m_test);
		f2->addRow("strain:", m_strain = new QLineEdit); m_strain->setValidator(new QDoubleValidator(-1, 1, 5));
		f2->addRow("simulation time:", m_time  = new QLineEdit); m_time->setValidator(new QDoubleValidator(0.01, 1000, 5));
		f2->addRow("time step:"      , m_steps = new QLineEdit); m_steps->setValidator(new QDoubleValidator(0.001, 100, 5));

		m_strain->setText("0.1");
		m_time->setText("1.0");
		m_steps->setText("0.05");

		m_props = new FEClassEdit(m_wnd);

		QPushButton* pb = new QPushButton("Run test");
		QPushButton* pc = new QPushButton("Clear graph");

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(f1);
		l->addWidget(m_props);
		l->addLayout(f2);
		l->addWidget(pb);
		l->addWidget(pc);

		QWidget* wl = new QWidget;
		wl->setLayout(l);

		plot = new CPlotWidget;

		QSplitter* split = new QSplitter;
		split->setOrientation(Qt::Horizontal);

		split->addWidget(wl);
		split->addWidget(plot);
		split->setStretchFactor(0, 1);
		split->setStretchFactor(1, 2);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(split);

		dlg->setLayout(h);

		QObject::connect(pb, SIGNAL(clicked()), dlg, SLOT(onRun()));
		QObject::connect(pc, SIGNAL(clicked()), dlg, SLOT(onClear()));
		QObject::connect(m_mat, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onMatChanged(int)));
		QObject::connect(m_test, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onTestChanged(int)));
	}

	void clearGraph()
	{
		plot->clear();
		plot->update();
	}
};

CDlgMaterialTest::CDlgMaterialTest(CMainWindow* wnd)	: QDialog(wnd), ui(new UIDlgMaterialTest)
{
	setWindowTitle("Material Test");
	setMinimumSize(1000, 600);
	ui->m_wnd = wnd;
	ui->setup(this);

	ui->m_test->addItem("uni-axial");
	ui->m_test->addItem("biaxial");
	ui->m_test->addItem("triaxial");
	ui->m_test->addItem("simple shear");

	CModelDocument* doc = wnd->GetModelDocument();
	if (doc)
	{
		FSModel* fem = doc->GetFSModel();
		int M = fem->Materials();
		for (int i = 0; i < M; i++)
		{
			GMaterial* pm = fem->GetMaterial(i);
			ui->m_mat->addItem(QString::fromStdString(pm->GetName()));
		}
	}
}

void CDlgMaterialTest::onClear()
{
	ui->clearGraph();
}

void CDlgMaterialTest::onRun()
{
	CModelDocument* doc = ui->m_wnd->GetModelDocument();
	if (doc == nullptr) return;

	FSModel* fem = doc->GetFSModel();
	int n = ui->m_mat->currentIndex();
	if ((n < 0) || (fem == nullptr)) return;

	GMaterial* mat = fem->GetMaterial(n);
	FSMaterial* pm = mat->GetMaterialProperties();
	if (pm == nullptr) return;

	if (pm->IsRigid())
	{
		QMessageBox::critical(this, "Material Test", "Cannot run material test on rigid material.");
		return;
	}

	MaterialTest mt;
	mt.mat = pm;
	mt.testName = ui->m_test->currentText().toStdString();
	mt.strain = ui->m_strain->text().toDouble();
	mt.time   = ui->m_time->text().toDouble();

	mt.yvalName = "sx";
	if (mt.testName == "simple shear")
		mt.xvalName = "Exz";
	else
		mt.xvalName = "Ex";

	double dt = ui->m_steps->text().toDouble();
	if (dt <= 0.0)
	{
		QMessageBox::critical(this, "Material Test", "time step must be positive.");
		return;
	}
	mt.steps  = (int)(mt.time / dt);
	if (mt.steps < 1)
	{
		QMessageBox::critical(this, "Material Test", "Invalid number of time steps.");
		return;
	}

	std::vector<pair<double, double> > data;
	if (FEBio::RunMaterialTest(mt, data))
	{
		int N = ui->plot->plots();
		QString s = QString::number(N + 1);
		CPlotData* pltData = new CPlotData;
		pltData->setLabel(s);
		for (int i = 0; i < data.size(); ++i)
		{
			pltData->addPoint(data[i].first, data[i].second);
		}
		pltData->setMarkerType(MarkerType::NO_MARKER);
		ui->plot->addPlotData(pltData);
		ui->plot->OnZoomToFit();

		QString unit = Units::GetUnitString(doc->GetUnitSystem(), Units::PRESSURE);

		QString xstr = QString::fromStdString(mt.xvalName);
		QString ystr = QString::fromStdString(mt.yvalName);

		ui->plot->setXAxisLabel(xstr);
		ui->plot->setYAxisLabel(QString("%1 [%2]").arg(ystr).arg(unit));
		ui->plot->setDrawAxesLabels(true);

		QString title = QString("%1 - %2").arg(ui->m_mat->currentText()).arg(ui->m_test->currentText());
		ui->plot->setTitle(title);
		ui->plot->update();
	}
	else
	{
		QMessageBox::critical(this, "Material Test", "Failed to run material test.");
	}
}

void CDlgMaterialTest::onMatChanged(int n)
{
	if (n < 0)
	{
		ui->m_props->SetFEClass(nullptr, nullptr);
		return;
	}

	CModelDocument* doc = ui->m_wnd->GetModelDocument();
	if (doc == nullptr) return;

	FSModel* fem = doc->GetFSModel();
	GMaterial* mat = fem->GetMaterial(n);

	ui->m_props->SetFEClass(mat->GetMaterialProperties(), fem);
}

void CDlgMaterialTest::onTestChanged(int n)
{
	ui->clearGraph();
	ui->plot->setTitle("");
	ui->plot->setDrawAxesLabels(false);
}
