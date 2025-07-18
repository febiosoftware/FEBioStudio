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
#include "DlgSelectRange.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <CUILib/InputWidgets.h>

class Ui::CDlgSelectRange
{
public:
	CFloatInput *pmin, *pmax;
	QCheckBox* prange;

public:
	void setupUi(QDialog* parent)
	{
		QVBoxLayout* pv = new QVBoxLayout;

		QFormLayout* pform = new QFormLayout;
		pform->addRow("min:", pmin = new CFloatInput);
		pform->addRow("max:", pmax = new CFloatInput);
		pv->addLayout(pform);

		prange = new QCheckBox("Apply to current selection");
		pv->addWidget(prange);

		QDialogButtonBox* pb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(pb);

		parent->setLayout(pv);

		QObject::connect(pb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(pb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgSelectRange::CDlgSelectRange(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgSelectRange)
{
	ui->setupUi(this);

	ui->pmin->setValue(0);
	ui->pmax->setValue(0);
}

int CDlgSelectRange::exec()
{
	ui->pmin->setValue(m_min);
	ui->pmax->setValue(m_max);

	return QDialog::exec();
}

void CDlgSelectRange::accept()
{
	m_min = ui->pmin->value();
	m_max = ui->pmax->value();
	m_brange = ui->prange->isChecked();

	QDialog::accept();
}
