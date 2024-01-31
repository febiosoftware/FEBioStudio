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
#include "DlgStepViewer.h"
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QHeaderView>
#include "MainWindow.h"
#include "ModelDocument.h"
#include <FEMLib/FELoad.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEInterface.h>

class CDlgStepViewer::Ui
{
	QTableWidget* table;

public:
	void setup(CDlgStepViewer* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		table = new QTableWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		
		l->addWidget(table);
		l->addWidget(bb);
		dlg->setLayout(l);

		connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
		connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
	}

	void build(FSModel* fem)
	{
		std::vector<FSStepComponent*> allComponents;
		for (int i = 0; i < fem->Steps(); ++i)
		{
			FSStep* step = fem->GetStep(i);
			for (int j = 0; j < step->BCs             (); ++j) allComponents.push_back(step->BC(j));
			for (int j = 0; j < step->Loads           (); ++j) allComponents.push_back(step->Load(j));
			for (int j = 0; j < step->ICs             (); ++j) allComponents.push_back(step->IC(j));
			for (int j = 0; j < step->Interfaces      (); ++j) allComponents.push_back(step->Interface(j));
			for (int j = 0; j < step->RigidBCs        (); ++j) allComponents.push_back(step->RigidBC(j));
			for (int j = 0; j < step->RigidLoads      (); ++j) allComponents.push_back(step->RigidLoad(j));
			for (int j = 0; j < step->RigidICs        (); ++j) allComponents.push_back(step->RigidIC(j));
			for (int j = 0; j < step->Constraints     (); ++j) allComponents.push_back(step->Constraint(j));
			for (int j = 0; j < step->RigidConstraints(); ++j) allComponents.push_back(step->RigidConstraint(j));
			for (int j = 0; j < step->RigidConnectors (); ++j) allComponents.push_back(step->RigidConnector(j));
			for (int j = 0; j < step->MeshAdaptors    (); ++j) allComponents.push_back(step->MeshAdaptor(j));
		}

		int steps = fem->Steps();

		table->setColumnCount(steps + 2);
		QStringList headerLabels; headerLabels << "Name" << "Type";
		for (int i = 1; i < steps; ++i) headerLabels << QString::fromStdString(fem->GetStep(i)->GetName());
		headerLabels << "";
		table->setHorizontalHeaderLabels(headerLabels);
		table->verticalHeader()->hide();
		table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
		table->horizontalHeader()->setStretchLastSection(true);
		table->setColumnWidth(0, 200);

		std::map<int, int> stepIDs;
		for (int i = 0; i < steps; ++i)
		{
			FSStep* step = fem->GetStep(i);
			stepIDs[step->GetID()] = i;
		}

		table->setRowCount(allComponents.size());
		QTableWidgetItem* it;
		for (int i=0; i<allComponents.size(); ++i)
		{
			FSStepComponent* comp = allComponents[i];
			it = new QTableWidgetItem(QString::fromStdString(comp->GetName()));
			it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (comp->IsActive() == false)
			{
				QFont font = it->font();
				font.setItalic(true);
				it->setFont(font);
			}
			table->setItem(i, 0, it);

			it = new QTableWidgetItem(QString(comp->GetTypeString()));
			it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			table->setItem(i, 1, it);
			if (comp->IsActive() == false)
			{
				QFont font = it->font();
				font.setItalic(true);
				it->setFont(font);
			}

			if (stepIDs.find(comp->GetStep()) != stepIDs.end())
			{
				int n = stepIDs[comp->GetStep()];
				for (int j = 1; j < steps; ++j)
				{
					it = new QTableWidgetItem("");
					it->setFlags(Qt::ItemIsEnabled);
					if ((n == 0) || (n == j))
					{
						if (comp->IsActive())
							it->setBackground(QBrush(QColor::fromRgb(0, 164, 0)));
						else
							it->setBackground(QBrush(QColor::fromRgb(0, 128, 0)));
					}
					else
					{
						it->setBackground(QBrush(QColor::fromRgb(220, 220, 220)));
					}
					table->setItem(i, j + 1, it);
				}
			}
		}
	}
};

CDlgStepViewer::CDlgStepViewer(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgStepViewer::Ui)
{
	setWindowTitle("Step Viewer");
	setMinimumSize(800, 600);
	ui->setup(this);

	CModelDocument* doc = wnd->GetModelDocument();
	if (doc && doc->GetFSModel())
	{
		FSModel* fem = doc->GetFSModel();
		ui->build(fem);
	}
}
