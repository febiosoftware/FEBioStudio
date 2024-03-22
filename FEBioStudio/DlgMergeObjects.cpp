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
#include "DlgMergeObjects.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QValidator>

class Ui::CDlgMergeObjects
{
public:
	QLineEdit*	name;
	QCheckBox*	check;
	QLineEdit*	tol;

public:
	void setupUi(QWidget* parent)
	{
		QFormLayout* form = new	QFormLayout;
		
		form->addRow("Name:", name = new QLineEdit);
		form->addRow("", check = new QCheckBox("Weld"));
		form->addRow("Tolerance:", tol = new QLineEdit); tol->setValidator(new QDoubleValidator);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(bb);

		parent->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgMergeObjects::CDlgMergeObjects(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgMergeObjects)
{
	ui->setupUi(this);

	static int ncount = 1;
	char sz[256] = {0};
	snprintf(sz, 256, "MergedObject%d", ncount++);
	ui->name->setText(sz);
	ui->check->setChecked(true);
	ui->tol->setText(QString::number(1e-6));
}

void CDlgMergeObjects::accept()
{
	m_name = ui->name->text().toStdString();
	m_weld = ui->check->isChecked();
	m_tol  = ui->tol->text().toDouble();

	QDialog::accept();
}
