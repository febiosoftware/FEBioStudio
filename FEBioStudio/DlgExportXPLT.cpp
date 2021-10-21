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
#include "DlgExportXPLT.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QCheckBox>

//-----------------------------------------------------------------------------
class Ui::CDlgExportXPLT
{
public:
	QCheckBox*	pc;

public:
	void setupUi(::CDlgExportXPLT* pwnd)
	{
		QVBoxLayout* pg = new QVBoxLayout(pwnd);

		pc = new QCheckBox("Data compression");

		pg->addWidget(pc);
		pg->addStretch();
	
		QHBoxLayout* ph = new QHBoxLayout;
		QPushButton* pb1 = new QPushButton("OK");
		QPushButton* pb2 = new QPushButton("Cancel");

		ph->addStretch();
		ph->addWidget(pb1);
		ph->addWidget(pb2);
		ph->addStretch();
		pg->addLayout(ph);

		QObject::connect(pb1, SIGNAL(clicked()), pwnd, SLOT(accept()));
		QObject::connect(pb2, SIGNAL(clicked()), pwnd, SLOT(reject()));
	}
};


CDlgExportXPLT::CDlgExportXPLT(CMainWindow* pwnd) : ui(new Ui::CDlgExportXPLT)
{
	m_bcompress = false;
	ui->setupUi(this);
}

void CDlgExportXPLT::accept()
{
	m_bcompress = ui->pc->isChecked();
	QDialog::accept();
}
