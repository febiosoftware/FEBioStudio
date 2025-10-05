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
#include "DlgAddStep.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <FEMLib/FSModel.h>
#include <CUILib/InputWidgets.h>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FSProject.h>

class Ui::CDlgAddStep
{
public:
	QLineEdit* name;
	QListWidget* type;
	QComboBox*	steps;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit;
		name->setPlaceholderText("(Leave blank for default)");
		type = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);

		steps = new QComboBox;
		form->addRow("Insert after:", steps);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(type);
		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(type, SIGNAL(itemDoubleClicked(QListWidgetItem*)), parent, SLOT(accept()));

		parent->setLayout(mainLayout);
	}
};

CDlgAddStep::CDlgAddStep(FSProject& prj, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddStep)
{
	setWindowTitle("Add Analysis Step");

	ui->setupUi(this);

	FSModel& fem = prj.GetFSModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		ui->steps->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}
	ui->steps->setCurrentIndex(fem.Steps() - 1);

	// set the types
	std::vector<FEClassFactory*> l = FEMKernel::FindAllClasses(prj.GetModule(), FEANALYSIS_ID);
	for (int i=0; i<l.size(); ++i)
	{
		QListWidgetItem* w = new QListWidgetItem;
		w->setText(l[i]->GetTypeStr());
		w->setData(Qt::UserRole, l[i]->GetClassID());
		ui->type->addItem(w);
	}
}

void CDlgAddStep::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the variable
	QListWidgetItem* item = ui->type->currentItem();
	if (item)
	{
		m_ntype = item->data(Qt::UserRole).toInt();
	}
	else
	{
		QMessageBox::critical(this, windowTitle(), "Please select a step type.");
		return;
	}

	QDialog::accept();
}

int CDlgAddStep::insertPosition() const
{
	return ui->steps->currentIndex();
}
