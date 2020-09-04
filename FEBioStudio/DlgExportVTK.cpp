/*This file is part of the PostView source code and is licensed under the MIT license
listed below.

See Copyright-PostView.txt for details.

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
#include "DlgExportVTK.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QRadioButton>

class CDlgExportVTK_UI
{
public:
	QRadioButton*	allStates;
	QRadioButton*	currState;

public:
	void setup(QDialog* dlg)
	{
		allStates = new QRadioButton("Export all states");
		currState = new QRadioButton("Export current state only");

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(allStates);
		l->addWidget(currState);

		allStates->setChecked(true);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgExportVTK::CDlgExportVTK(QWidget* parent) : QDialog(parent), ui(new CDlgExportVTK_UI)
{
	m_ops[0] = true;
	m_ops[1] = false;

	ui->setup(this);
}

void CDlgExportVTK::accept()
{
	m_ops[0] = ui->allStates->isChecked();
	m_ops[1] = ui->currState->isChecked();

	QDialog::accept();
}
