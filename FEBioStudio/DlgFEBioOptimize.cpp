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
#include "DlgFEBioOptimize.h"
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QStackedWidget>
#include "MainWindow.h"
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QMessageBox>
#include <FEMLib/FEMaterial.h>
#include "ModelDocument.h"
#include "DlgFormula.h"
#include <QClipboard>
#include <QApplication>
#include <QtCore/QMimeData>
#include "DlgImportData.h"

class CDlgGenerateData : public QDialog
{
public:
	CDlgGenerateData(QWidget* parent) : QDialog(parent)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("start index:", m_start = new QLineEdit);
		form->addRow("end index:", m_end = new QLineEdit);
		form->addRow("value:", m_val = new QLineEdit);

		m_start->setValidator(new QIntValidator);
		m_end->setValidator(new QIntValidator);
		m_val->setValidator(new QDoubleValidator);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addWidget(bb);

		setLayout(l);

		setWindowTitle("Generate data points");

		QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
	}

	int GetStart() { return m_start->text().toInt(); }
	int GetEnd() { return m_end->text().toInt(); }
	double GetValue() { return m_val->text().toDouble(); }

private:
	QLineEdit* m_start;
	QLineEdit* m_end;
	QLineEdit* m_val;
};

class Ui::CDlgSelectParam
{
public:
	FSModel*		m_fem;
	QTreeWidget*	tree;
	QString			m_text;
	int				m_paramOption;

public:
	void setup(QDialog* w)
	{
		tree = new QTreeWidget;
		tree->header()->setVisible(false);
		tree->setColumnCount(1);
		BuildTree();

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(new QLabel("Select the parameter to add:"));
		l->addWidget(tree);
		l->addWidget(bb);

		w->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), w, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), w, SLOT(reject()));
	}

	void AddMaterial(FSCoreBase* mat, QTreeWidgetItem* item)
	{
		for (int j = 0; j < mat->Parameters(); ++j)
		{
			Param& paramj = mat->GetParam(j);
			QTreeWidgetItem* paramItem = new QTreeWidgetItem(QStringList() << paramj.GetShortName(), 2);
			item->addChild(paramItem);
		}

		for (int j = 0; j < mat->Properties(); ++j)
		{
			FSProperty& prop = mat->GetProperty(j);
			int nsize = prop.Size();
			QString propName = QString::fromStdString(prop.GetName());
			for (int k = 0; k < nsize; ++k)
			{
				FSCoreBase* mj = prop.GetComponent(k);
				if (mj)
				{
					QString matName = propName;
					if (nsize > 1) matName += QString("[%1]").arg(k);
					QTreeWidgetItem* matItem = new QTreeWidgetItem(QStringList() << matName, 2);
					AddMaterial(mj, matItem);
					item->addChild(matItem);
				}
			}
		}
	}

	void BuildTree()
	{
		QTreeWidgetItem* root = new QTreeWidgetItem(QStringList() << "fem", 0);
		switch (m_paramOption)
		{
		case 0: // input parameters
		{
			int NMAT = m_fem->Materials();
			for (int i = 0; i < NMAT; ++i)
			{
				GMaterial* mat = m_fem->GetMaterial(i);
				QTreeWidgetItem* matItem = new QTreeWidgetItem(QStringList() << QString::fromStdString(mat->GetName()), 1);
				FSMaterial* matProps = mat->GetMaterialProperties();

				AddMaterial(matProps, matItem);
				root->addChild(matItem);
			}
		}
		break;
		case 1: // output parameters
		{
			int NMAT = m_fem->Materials();
			for (int i = 0; i < NMAT; ++i)
			{
				GMaterial* mat = m_fem->GetMaterial(i);
				FSMaterial* matProps = mat->GetMaterialProperties();
				if (matProps->IsRigid()) {
					QTreeWidgetItem* matItem = new QTreeWidgetItem(QStringList() << QString::fromStdString(mat->GetName()), 3);
					matItem->addChild(new QTreeWidgetItem(QStringList() << "Fx", 2));
					matItem->addChild(new QTreeWidgetItem(QStringList() << "Fy", 2));
					matItem->addChild(new QTreeWidgetItem(QStringList() << "Fz", 2));
					root->addChild(matItem);
				}
			}
		}
		break;
		}
		tree->insertTopLevelItem(0, root);
		root->setExpanded(true);
	}
};

void CDlgSelectParam::accept()
{
	QTreeWidgetItem* item = ui->tree->currentItem();
	if ((item == nullptr) || (item->type() != 2))
	{
		QMessageBox::critical(this, "FEBio Studio", "You must select a valid parameter.");
		return;
	}

	QString s;
	while (item)
	{
		switch (item->type())
		{
		case 0: s.prepend("fem"); break;
		case 1: s.prepend("material('" + item->text(0) + "')"); break;
		case 2: s.prepend(item->text(0)); break;
		case 3: s.prepend("rigidbody('" + item->text(0) + "')"); break;
		}
		item = item->parent();
		if (item) s.prepend(".");
	};
	ui->m_text = s;

	QDialog::accept();
}

QString CDlgSelectParam::text()
{
	return ui->m_text;
}


CDlgSelectParam::CDlgSelectParam(FSModel* fem, int paramOption, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgSelectParam)
{
	ui->m_fem = fem;
	ui->m_paramOption = paramOption;
	ui->setup(this);
}

CSelectParam::CSelectParam(FSModel* fem, int paramOption, QWidget* parent) : m_fem(fem), QWidget(parent)
{
	m_paramOption = paramOption;

	QHBoxLayout* h = new QHBoxLayout;
	h->setContentsMargins(0,0,0,0);
	h->addWidget(m_edit = new QLineEdit);
	h->addWidget(m_push = new QPushButton("..."));
	m_push->setMaximumWidth(20);
	setLayout(h);

	QObject::connect(m_push, SIGNAL(clicked()), this, SLOT(onSelectClicked()));
}

void CSelectParam::clear() { m_edit->clear();	}

QString CSelectParam::text() { return m_edit->text(); }

void CSelectParam::onSelectClicked()
{
	CDlgSelectParam dlg(m_fem, m_paramOption);
	if (dlg.exec())
	{
		m_edit->setText(dlg.text());
	}
}

class Ui::CDlgFEBioOptimize
{
public:
	FSModel*	m_fem;
	FEBioOpt	opt;

	// page2 (Options)
	QComboBox*	method;
	QLineEdit*	objTol;
	QLineEdit*	fDiffScale;
	QComboBox*	out;
	QComboBox*	print;

	// page 3 (Parameters)
	CSelectParam* paramName;
	QLineEdit* initVal;
	QLineEdit* minVal;
	QLineEdit* maxVal;
	QTableWidget*	paramTable;

	// page 4 (Objective)
	QComboBox*		objFun;
	QStackedWidget* stack;
	// 4.1. data-fit pane
	CSelectParam*	objParam;
	QTableWidget*	dataTable;
	// 4.2 target pane
	QLineEdit* trgName;
	QLineEdit* trgVal;
	QTableWidget* trgVar;
	// 4.3 element-data pane
	QLineEdit* edVar;
	QTableWidget* elemDataTable;
	// 4.3 node-data pane
	QLineEdit* ndVar;
	QTableWidget* nodeDataTable;

public:
	void setup(QWizard* w)
	{
		w->addPage(GenerateIntro());
		w->addPage(GenerateOptionsPage());
		w->addPage(GenerateParametersPage());
		w->addPage(GenerateObjectivePage());

		// set initial values
		method->setCurrentIndex(opt.m_method);
		objTol->setText(QString::number(opt.m_obj_tol));
		fDiffScale->setText(QString::number(opt.m_f_diff_scale));
	}

	void Update()
	{
		// get control options
		opt.m_method = method->currentIndex();
		opt.m_obj_tol = objTol->text().toDouble();
		opt.m_f_diff_scale = fDiffScale->text().toDouble();
		opt.m_outLevel = out->currentIndex();
		opt.m_printLevel = print->currentIndex();

		// get parameters
		opt.m_params.clear();
		for (int i = 0; i < paramTable->rowCount(); ++i)
		{
			std::string param_name = paramTable->item(i, 0)->text().toStdString();
			FEBioOpt::Param param(param_name);
			param.m_initVal = paramTable->item(i, 1)->text().toDouble();
			param.m_minVal = paramTable->item(i, 2)->text().toDouble();
			param.m_maxVal = paramTable->item(i, 3)->text().toDouble();

			opt.AddParameter(param);
		}

		// get objective data
		opt.m_objective = objFun->currentIndex();
		opt.m_objParam = objParam->text().toStdString();

		opt.m_trgVar.clear();
		for (int i = 0; i < trgVar->rowCount(); ++i)
		{
			FEBioOpt::TargetVar var;
			var.m_name = trgVar->item(i, 0)->text().toStdString();
			var.m_val  = trgVar->item(i, 1)->text().toDouble();
			opt.m_trgVar.push_back(var);
		}

		// get data points
		opt.m_data.clear();
		for (int i = 0; i < dataTable->rowCount(); ++i)
		{
			double t = dataTable->item(i, 0)->text().toDouble();
			double v = dataTable->item(i, 1)->text().toDouble();

			opt.AddData(t, v);
		}

		// get element data points
		opt.m_edVar = edVar->text().toStdString();
		opt.m_edData.clear();
		for (int i = 0; i < elemDataTable->rowCount(); ++i)
		{
			int id   = elemDataTable->item(i, 0)->text().toInt();
			double v = elemDataTable->item(i, 1)->text().toDouble();
			opt.m_edData.push_back({ id, v });
		}

		// get node data points
		opt.m_ndVar = ndVar->text().toStdString();
		opt.m_ndData.clear();
		for (int i = 0; i < nodeDataTable->rowCount(); ++i)
		{
			int id = nodeDataTable->item(i, 0)->text().toInt();
			double v = nodeDataTable->item(i, 1)->text().toDouble();
			opt.m_ndData.push_back({ id, v });
		}
	}

	void addParameter()
	{
		int rows = paramTable->rowCount();
		paramTable->setRowCount(rows + 1);
		paramTable->setItem(rows, 0, new QTableWidgetItem(paramName->text()));
		paramTable->setItem(rows, 1, new QTableWidgetItem(initVal->text()));
		paramTable->setItem(rows, 2, new QTableWidgetItem(minVal->text()));
		paramTable->setItem(rows, 3, new QTableWidgetItem(maxVal->text()));

		paramName->clear();
		initVal->clear();
		minVal->clear();
		maxVal->clear();
	}

	void addTargetVariable()
	{
		int rows = trgVar->rowCount();
		trgVar->setRowCount(rows + 1);
		trgVar->setItem(rows, 0, new QTableWidgetItem(trgName->text()));
		trgVar->setItem(rows, 1, new QTableWidgetItem(trgVal->text()));

		trgName->clear();
		trgVal->clear();
	}

	void addDataPoint(double x, double y)
	{
		int rows = dataTable->rowCount();
		dataTable->setRowCount(rows + 1);
		dataTable->setItem(rows, 0, new QTableWidgetItem(QString::number(x)));
		dataTable->setItem(rows, 1, new QTableWidgetItem(QString::number(y)));
	}

	void addElemDataPoint(int x, double y)
	{
		int rows = elemDataTable->rowCount();
		elemDataTable->setRowCount(rows + 1);
		elemDataTable->setItem(rows, 0, new QTableWidgetItem(QString::number(x)));
		elemDataTable->setItem(rows, 1, new QTableWidgetItem(QString::number(y)));
	}

	void addNodeDataPoint(int x, double y)
	{
		int rows = nodeDataTable->rowCount();
		nodeDataTable->setRowCount(rows + 1);
		nodeDataTable->setItem(rows, 0, new QTableWidgetItem(QString::number(x)));
		nodeDataTable->setItem(rows, 1, new QTableWidgetItem(QString::number(y)));
	}

	QWizardPage* GenerateIntro()
	{
		QWizardPage* intro = new QWizardPage;
		intro->setTitle("Introduction");

		QVBoxLayout* l = new QVBoxLayout;

		QLabel* label = new QLabel("This wizard will walk you through the steps of setting up an FEBio optimization problem.");
		label->setWordWrap(true);
		l->addWidget(label);

		intro->setLayout(l);

		return intro;
	}

	QWizardPage* GenerateOptionsPage()
	{
		QWizardPage* opsPage = new QWizardPage;
		opsPage->setTitle("Control Options");
		opsPage->setSubTitle("Set the optimization control options.");

		QFormLayout* l = new QFormLayout;

		l->addRow("Optimization method:", method = new QComboBox);
		method->addItem("Levenberg-Marquardt");
		method->addItem("constrained Levenberg-Marquardt");

		l->addRow("Objective tolerance:", objTol = new QLineEdit);
		objTol->setValidator(new QDoubleValidator);

		l->addRow("Forward difference scale:", fDiffScale = new QLineEdit);
		fDiffScale->setValidator(new QDoubleValidator);

		l->addRow("Output", out = new QComboBox);
		l->addRow("Print level", print = new QComboBox);

		out->addItems(QStringList() << "LOG_DEFAULT" << "LOG_NEVER" << "LOG_FILE_ONLY" << "LOG_SCREEN_ONLY" << "LOG_FILE_AND_SCREEN");
		print->addItems(QStringList() << "PRINT_ITERATIONS" << "PRINT_VERBOSE");

		opsPage->setLayout(l);

		return opsPage;
	}

	QWizardPage* GenerateParametersPage()
	{
		QWizardPage* paramPage = new QWizardPage;
		paramPage->setTitle("Parameters");
		paramPage->setSubTitle("Set the model parameters to optimize.");

		QFormLayout* form = new QFormLayout;
		form->addRow("parameter:", paramName = new CSelectParam(m_fem));
		form->addRow("initial value:", initVal = new QLineEdit); initVal->setValidator(new QDoubleValidator);
		form->addRow("minimum value:", minVal = new QLineEdit); minVal->setValidator(new QDoubleValidator);
		form->addRow("maximum value:", maxVal = new QLineEdit); maxVal->setValidator(new QDoubleValidator);

		QPushButton* add = new QPushButton("Add");
		add->setObjectName("addParameter");

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(add);
		h->addStretch();

		paramTable = new QTableWidget(0, 4);
		paramTable->setObjectName("paramTable");
		paramTable->setHorizontalHeaderLabels(QStringList() << "name" << "value" << "min" << "max");
		paramTable->horizontalHeader()->setStretchLastSection(true);
		paramTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		paramTable->setSelectionMode(QAbstractItemView::SingleSelection);
		paramTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		paramTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addLayout(h);
		l->addWidget(paramTable);

		paramPage->setLayout(l);

		return paramPage;
	}

	QWizardPage* GenerateObjectivePage()
	{
		QWizardPage* objPage = new QWizardPage;
		objPage->setTitle("Objective");
		objPage->setSubTitle("Define the objective function.");

		objFun = new QComboBox;
		objFun->addItems(QStringList() << "data-fit" << "target" << "element-data" << "node-data");

		QHBoxLayout* ol = new QHBoxLayout;
		ol->addWidget(new QLabel("Objective model:"));
		ol->addWidget(objFun);
		ol->addStretch();

		stack = new QStackedWidget;

		// pane for data-fit
		QWidget* w1 = new QWidget;
		QVBoxLayout* l1 = new QVBoxLayout;
		{
			QFormLayout* form = new QFormLayout;
			form->addRow("Objective parameter:", objParam = new CSelectParam(m_fem, 1));

			QHBoxLayout* h = new QHBoxLayout;
			QPushButton* add = new QPushButton("Generate points ...");
			add->setObjectName("addData");
			h->addWidget(add);

			QPushButton* paste = new QPushButton("Paste clipboard");
			paste->setObjectName("pasteData");
			h->addWidget(paste);

			h->addStretch();

			dataTable = new QTableWidget(0, 2);
			dataTable->setObjectName("dataTable");
			dataTable->setHorizontalHeaderLabels(QStringList() << "time" << "value");
			dataTable->horizontalHeader()->setStretchLastSection(true);
			dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
			dataTable->setSelectionMode(QAbstractItemView::SingleSelection);
			dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

			l1->addLayout(form);
			l1->addLayout(h);
			l1->addWidget(dataTable);

			w1->setLayout(l1);
		}
		stack->addWidget(w1);

		// pane for target
		QWidget* w2 = new QWidget;
		QVBoxLayout* l2 = new QVBoxLayout;
		{
			QFormLayout* form = new QFormLayout;
			form->addRow("parameter name:", trgName = new QLineEdit);
			form->addRow("parameter value:", trgVal = new QLineEdit); trgVal->setValidator(new QDoubleValidator);
			l2->addLayout(form);

			QHBoxLayout* h = new QHBoxLayout;
			QPushButton* addVar = new QPushButton("Add"); addVar->setObjectName("addvar");
			h->addWidget(addVar); h->addStretch();
			l2->addLayout(h);

			trgVar = new QTableWidget();
			trgVar->setColumnCount(2);
			trgVar->setHorizontalHeaderLabels(QStringList() << "Name" << "value");
			trgVar->horizontalHeader()->setStretchLastSection(true);
			trgVar->setSelectionBehavior(QAbstractItemView::SelectRows);
			trgVar->setSelectionMode(QAbstractItemView::SingleSelection);
			trgVar->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
			trgVar->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
			l2->addWidget(trgVar);
		}
		w2->setLayout(l2);
		stack->addWidget(w2);

		// pane for element-data
		QWidget* w3 = new QWidget;
		QVBoxLayout* l3 = new QVBoxLayout;
		{
			QFormLayout* form = new QFormLayout;
			form->addRow("variable:", edVar = new QLineEdit());
			l3->addLayout(form);

			QHBoxLayout* h = new QHBoxLayout;
			QPushButton* add = new QPushButton("Generate data ...");
			add->setObjectName("addElemData");
			h->addWidget(add);
			QPushButton* paste = new QPushButton("Paste clipboard");
			paste->setObjectName("pasteElemData");
			h->addWidget(paste);
			h->addStretch();
			l3->addLayout(h);

			elemDataTable = new QTableWidget(0, 2);
			elemDataTable->setObjectName("elemDataTable");
			elemDataTable->setHorizontalHeaderLabels(QStringList() << "ID" << "value");
			elemDataTable->horizontalHeader()->setStretchLastSection(true);
			elemDataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
			elemDataTable->setSelectionMode(QAbstractItemView::SingleSelection);
			elemDataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
			l3->addWidget(elemDataTable);
		}
		w3->setLayout(l3);
		stack->addWidget(w3);

		// pane for node-data
		QWidget* w4 = new QWidget;
		QVBoxLayout* l4 = new QVBoxLayout;
		{
			QFormLayout* form = new QFormLayout;
			form->addRow("variable:", ndVar = new QLineEdit());
			l4->addLayout(form);

			QHBoxLayout* h = new QHBoxLayout;
			QPushButton* add = new QPushButton("Generate data ...");
			add->setObjectName("addNodeData");
			h->addWidget(add);
			QPushButton* paste = new QPushButton("Paste clipboard");
			paste->setObjectName("pasteNodeData");
			h->addWidget(paste);
			h->addStretch();
			l4->addLayout(h);

			nodeDataTable = new QTableWidget(0, 2);
			nodeDataTable->setObjectName("nodeDataTable");
			nodeDataTable->setHorizontalHeaderLabels(QStringList() << "ID" << "value");
			nodeDataTable->horizontalHeader()->setStretchLastSection(true);
			nodeDataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
			nodeDataTable->setSelectionMode(QAbstractItemView::SingleSelection);
			nodeDataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
			l4->addWidget(nodeDataTable);
		}
		w4->setLayout(l4);
		stack->addWidget(w4);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(ol);
		l->addWidget(stack);
		objPage->setLayout(l);

		QObject::connect(objFun, &QComboBox::currentIndexChanged, stack, &QStackedWidget::setCurrentIndex);

		return objPage;
	}
};

CDlgFEBioOptimize::CDlgFEBioOptimize(CMainWindow* parent) : QWizard(parent), ui(new Ui::CDlgFEBioOptimize)
{
#ifdef WIN32
	// We need this style, since the default Aero style doesn't look right in dark mode.
	setWizardStyle(QWizard::ModernStyle);
#endif

	CModelDocument* doc = dynamic_cast<CModelDocument*>(parent->GetDocument());

	setWindowTitle("Generate FEBio optimization");
	ui->m_fem = doc->GetFSModel();
	ui->setup(this);

	QMetaObject::connectSlotsByName(this);
}

FEBioOpt CDlgFEBioOptimize::GetFEBioOpt()
{
	ui->Update();
	return ui->opt;
}

void CDlgFEBioOptimize::on_addParameter_clicked()
{
	ui->addParameter();
}

void CDlgFEBioOptimize::on_addData_clicked()
{
	CDlgFormula dlg(this);
	if (dlg.exec())
	{
		ui->dataTable->setRowCount(0);
		std::vector<vec2d> pts = dlg.GetPoints();
		for (int i = 0; i < pts.size(); ++i)
		{
			vec2d& p = pts[i];
			ui->addDataPoint(p.x(), p.y());
		}
	}
}

void CDlgFEBioOptimize::on_addElemData_clicked()
{
	CDlgGenerateData dlg(this);
	if (dlg.exec())
	{
		int start = dlg.GetStart();
		int end = dlg.GetEnd();
		double val = dlg.GetValue();
		ui->elemDataTable->setRowCount(0);
		for (int i = start; i <= end; ++i)
		{
			ui->addElemDataPoint(i, val);
		}
	}
}

void CDlgFEBioOptimize::on_addNodeData_clicked()
{
	CDlgGenerateData dlg(this);
	if (dlg.exec())
	{
		int start = dlg.GetStart();
		int end = dlg.GetEnd();
		double val = dlg.GetValue();
		ui->nodeDataTable->setRowCount(0);
		for (int i = start; i <= end; ++i)
		{
			ui->addNodeDataPoint(i, val);
		}
	}
}

void CDlgFEBioOptimize::on_pasteData_clicked()
{
	QClipboard* clipboard = QApplication::clipboard();
	if (clipboard == nullptr) return;
	const QMimeData *mimeData = clipboard->mimeData();
	if (mimeData == nullptr) return;

	if (mimeData->hasText())
	{
        QString text = clipboard->text();
        CDlgImportData dlg(text, DataType::DOUBLE, 2);
        if(dlg.exec())
        {
            QList<QStringList> values = dlg.GetValues();

            ui->dataTable->setRowCount(0);

            for(auto row : values)
            {
                double x = row.at(0).toDouble();
                double y = row.at(1).toDouble();
                ui->addDataPoint(x, y);
            }
        }
	}
	else
	{
		QMessageBox::information(this, "FEBio Studio", "No valid clipboard data found.");
	}
}

void CDlgFEBioOptimize::on_pasteElemData_clicked()
{
	QClipboard* clipboard = QApplication::clipboard();
	if (clipboard == nullptr) return;
	const QMimeData* mimeData = clipboard->mimeData();
	if (mimeData == nullptr) return;

	if (mimeData->hasText())
	{
		QString text = clipboard->text();
		CDlgImportData dlg(text, DataType::DOUBLE, 2);
		if (dlg.exec())
		{
			QList<QStringList> values = dlg.GetValues();

			ui->elemDataTable->setRowCount(0);

			for (auto row : values)
			{
				int x = row.at(0).toInt();
				double y = row.at(1).toDouble();
				ui->addElemDataPoint(x, y);
			}
		}
	}
	else
	{
		QMessageBox::information(this, "FEBio Studio", "No valid clipboard data found.");
	}
}

void CDlgFEBioOptimize::on_pasteNodeData_clicked()
{
	QClipboard* clipboard = QApplication::clipboard();
	if (clipboard == nullptr) return;
	const QMimeData* mimeData = clipboard->mimeData();
	if (mimeData == nullptr) return;

	if (mimeData->hasText())
	{
		QString text = clipboard->text();
		CDlgImportData dlg(text, DataType::DOUBLE, 2);
		if (dlg.exec())
		{
			QList<QStringList> values = dlg.GetValues();

			ui->nodeDataTable->setRowCount(0);

			for (auto row : values)
			{
				int x = row.at(0).toInt();
				double y = row.at(1).toDouble();
				ui->addNodeDataPoint(x, y);
			}
		}
	}
	else
	{
		QMessageBox::information(this, "FEBio Studio", "No valid clipboard data found.");
	}
}

void CDlgFEBioOptimize::on_addvar_clicked()
{
	ui->addTargetVariable();
}

//==========================================================================================================

class CDlgFEBioTangentUI
{
public:
	FEBioTangentDiagnostic data;

public:
	QComboBox* matList;
	QComboBox* scenarios;
	QLineEdit* strain;

	void setup(CDlgFEBioTangent* dlg)
	{
		matList = new QComboBox;
		scenarios = new QComboBox;
		strain = new QLineEdit;
		strain->setValidator(new QDoubleValidator);

		QFormLayout* form = new QFormLayout;
		form->addRow("Material:", matList);
		form->addRow("Scenario:", scenarios);
		form->addRow("strain:", strain);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgFEBioTangent::CDlgFEBioTangent(CMainWindow* parent) : QDialog(parent), ui(new CDlgFEBioTangentUI)
{
	ui->setup(this);

	CModelDocument* doc = parent->GetModelDocument();
	if (doc)
	{
		FSModel* fem = doc->GetFSModel(); assert(fem);
		int nmat = fem->Materials();
		for (int i = 0; i < nmat; ++i)
		{
			GMaterial* mat = fem->GetMaterial(i);
			ui->matList->addItem(QString::fromStdString(mat->GetName()));
		}
	}

	ui->scenarios->addItem("uni-axial");
	ui->scenarios->addItem("simple shear");

	ui->strain->setText(QString::number(0.1));
}

FEBioTangentDiagnostic CDlgFEBioTangent::GetData()
{
	return ui->data;
}

void CDlgFEBioTangent::accept()
{
	ui->data.m_matIndex = ui->matList->currentIndex();
	ui->data.m_scenario = ui->scenarios->currentIndex();
	ui->data.m_strain = ui->strain->text().toDouble();

	QDialog::accept();
}
