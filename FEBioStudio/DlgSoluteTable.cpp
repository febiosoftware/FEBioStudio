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
#include "DlgSoluteTable.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>

class Ui::CDlgSoluteTable
{
public:
	QTableWidget*	table;

public:
	void setupUi(QWidget* parent)
	{
		QPushButton* addButton    = new QPushButton("Add" ); addButton->setObjectName("addButton");
		QPushButton* removeButton = new QPushButton("Remove"  ); removeButton->setObjectName("removeButton");

		QStringList labels;
		labels << "Name" << "Charge Number" << "Molar Mass" << "Density"; 

		table = new QTableWidget;
		table->setObjectName("table");
		table->setColumnCount(4);
		table->setHorizontalHeaderLabels(labels);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
//		table->setSelectionBehavior(QAbstractItemView::SelectItems);
		table->setSelectionMode(QAbstractItemView::SingleSelection);
		table->horizontalHeader()->setStretchLastSection(true);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);

		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addWidget(addButton);
		buttonLayout->addWidget(removeButton);
		buttonLayout->addStretch();

		QVBoxLayout* mainLayout = new QVBoxLayout;

		mainLayout->addLayout(buttonLayout);
		mainLayout->addWidget(table);
		mainLayout->addWidget(bb);
		parent->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(accept()));

		QMetaObject::connectSlotsByName(parent);
	}
};

CDlgSoluteTable::CDlgSoluteTable(int mode, CMainWindow* wnd) : m_wnd(wnd), QDialog(wnd), ui(new Ui::CDlgSoluteTable)
{
	m_mode = mode;
	setWindowTitle(m_mode == ShowSolutes ? "Solute Table" : "SBM Table");
	setMinimumSize(400, 300);
	ui->setupUi(this);	

	Update();
}

void CDlgSoluteTable::Update()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FSModel* fem = doc->GetFSModel();

	switch (m_mode)
	{
	case ShowSolutes:
		{
			int NS = fem->Solutes();
			ui->table->clearContents();
			ui->table->setRowCount(NS);

			for (int i=0; i<NS; ++i)
			{
				SoluteData& s = fem->GetSoluteData(i);

				ui->table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s.GetName())));
				ui->table->setItem(i, 1, new QTableWidgetItem(QString::number(s.GetChargeNumber())));
				ui->table->setItem(i, 2, new QTableWidgetItem(QString::number(s.GetMolarMass())));
				ui->table->setItem(i, 3, new QTableWidgetItem(QString::number(s.GetDensity())));
			}
		}
		break;
	case ShowSBMs:
		{
			int NS = fem->SBMs();
			ui->table->clearContents();
			ui->table->setRowCount(NS);

			for (int i=0; i<NS; ++i)
			{
				SoluteData& s = fem->GetSBMData(i);

				ui->table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(s.GetName())));
				ui->table->setItem(i, 1, new QTableWidgetItem(QString::number(s.GetChargeNumber())));
				ui->table->setItem(i, 2, new QTableWidgetItem(QString::number(s.GetMolarMass())));
				ui->table->setItem(i, 3, new QTableWidgetItem(QString::number(s.GetDensity())));
			}
		}
		break;
	}
}

void CDlgSoluteTable::on_addButton_clicked()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FSModel* fem = doc->GetFSModel();

	switch (m_mode)
	{
	case ShowSolutes:
		{
			int NS = fem->Solutes();
			char szname[64] = {0};
            int max_len = sizeof szname;
			snprintf(szname, max_len, "solute%d", NS+1);
			fem->AddSolute(szname, 0, 1.0, 1.0);
		}
		break;
	case ShowSBMs:
		{
			int NS = fem->SBMs();
			char szname[64] = {0};
            int max_len = sizeof szname;
			snprintf(szname, max_len, "sbm%d", NS+1);
			fem->AddSBM(szname, 0, 1.0, 1.0);
		}
		break;
	default:
		assert(false);
	}
	Update();
}

void CDlgSoluteTable::on_table_itemChanged(QTableWidgetItem* item)
{
	int nrow = item->row();
	int ncol = item->column();

	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FSModel* fem = doc->GetFSModel();

	QString txt = item->text();

	SoluteData& solute = (m_mode == ShowSolutes ? fem->GetSoluteData(nrow) : fem->GetSBMData(nrow));
	switch (ncol)
	{
	case 0:  // name
		{
			std::string s = txt.toStdString();
			const char* szname = s.c_str();
			solute.SetName(szname);

			// also change the corresponding DOF name for solutes
			if (m_mode == ShowSolutes)
			{
				FEDOFVariable& var = fem->GetVariable("concentration");
				var.GetDOF(nrow).SetName(szname);
			}
		}
		break;
	case 1: // charge number
		{
			double z = txt.toDouble();
			solute.SetChargeNumber(z);
			txt = QString::number(z);
			item->setText(txt);
		}
		break;
	case 2:	// molar mass
		{
			double M = txt.toDouble();
			solute.SetMolarMass(M);
			txt = QString::number(M);
			item->setText(txt);
		}
		break;
	case 3: // density
		{
			double D = txt.toDouble();
			solute.SetDensity(D);
			txt = QString::number(D);
			item->setText(txt);
		}
		break;
	}
}

void CDlgSoluteTable::on_removeButton_clicked()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	FSModel* fem = doc->GetFSModel();
	QItemSelectionModel* mdl = ui->table->selectionModel();
	QModelIndexList selRows = mdl->selectedRows();
	if (selRows.isEmpty() == false)
	{
		QModelIndexList::iterator it = selRows.begin();
		int nrow = it->row();
		if (m_mode == ShowSolutes)
			fem->RemoveSolute(nrow);
		else if (m_mode == ShowSBMs)
			fem->RemoveSBM(nrow);
		Update();
	}
	else 
	{
		QString arg = (m_mode == ShowSolutes ? "solute" : "sbm");
		QMessageBox::information(this, QString("Remove %1").arg(arg), QString("No %1 selected").arg(arg));
	}
}
