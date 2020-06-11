/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "DlgCloneObject.h"
#include <QFormLayout>
#include <QBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>

class Ui::CDlgCloneObject
{
public:
	QString	_name;
	QLineEdit* name;

	static int count;

public:
	void setup(QWidget* w)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", name = new QLineEdit);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		w->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), w, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), w, SLOT(reject()));
	}
};

int Ui::CDlgCloneObject::count = 1;

CDlgCloneObject::CDlgCloneObject(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgCloneObject)
{
	ui->setup(this);

	ui->name->setText(QString("Clone%1").arg(ui->count));
}

void CDlgCloneObject::accept()
{
	ui->_name = ui->name->text();
	if (ui->_name.isEmpty())
	{
		QMessageBox::critical(this, "Clone Object", "Invalid name for cloned object");
	}
	else 
	{
		ui->count++;
		QDialog::accept();
	}
}

QString CDlgCloneObject::GetNewObjectName()
{
	return ui->_name;
}
