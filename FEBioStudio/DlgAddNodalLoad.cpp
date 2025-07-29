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
#include "DlgAddNodalLoad.h"
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

class Ui::CDlgAddNodalLoad
{
public:
	QLineEdit* name;
	QComboBox* step;
	QComboBox* var;
	CFloatInput* value;

public:
	void setupUi(QWidget* parent)
	{
		name = new QLineEdit; name->setPlaceholderText("(leave empty for default)");
		step = new QComboBox;

		var = new QComboBox;

		value = new CFloatInput;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QFormLayout* form = new QFormLayout;
		form->setLabelAlignment(Qt::AlignRight);
		form->addRow("Name:", name);
		form->addRow("Step:", step);
		form->addRow("Variable:", var);
		form->addRow("Load:", value);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(bb);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));

		parent->setLayout(mainLayout);
	}
};

CDlgAddNodalLoad::CDlgAddNodalLoad(FSModel& fem, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddNodalLoad)
{
	setWindowTitle("Add Nodal Load");

	ui->setupUi(this);

	// add the steps
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// set the variables
	ui->var->addItem("x-force");
	ui->var->addItem("y-force");
	ui->var->addItem("z-force");
    ui->var->addItem("shell x-force");
    ui->var->addItem("shell y-force");
    ui->var->addItem("shell z-force");
	ui->var->addItem("fluid volumetric flow rate");

	ui->value->setValue(1.0);
}

void CDlgAddNodalLoad::accept()
{
	// get the name
	QString name = ui->name->text();
	m_name = name.toStdString();

	// get the step
	m_nstep = ui->step->currentIndex();

	// get the variable
	m_nvar = ui->var->currentIndex();

	// get the value
	m_val = ui->value->value();

	QDialog::accept();
}
