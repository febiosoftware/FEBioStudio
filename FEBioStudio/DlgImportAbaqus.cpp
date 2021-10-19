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
#include "DlgImportAbaqus.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QBoxLayout>

class Ui::CDlgImportAbaqus
{
public:
	QCheckBox* pc[6];

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		pc[0] = new QCheckBox("Import node sets");
		pc[1] = new QCheckBox("Import element sets");
		pc[2] = new QCheckBox("Import surface");
		pc[3] = new QCheckBox("Auto-partition from element sets");
		pc[4] = new QCheckBox("Auto-partition surface");
		pc[5] = new QCheckBox("Process solid sections");

		layout->addWidget(pc[0]);
		layout->addWidget(pc[1]);
		layout->addWidget(pc[2]);
		layout->addWidget(pc[3]);
		layout->addWidget(pc[4]);
		layout->addWidget(pc[5]);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		parent->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgImportAbaqus::CDlgImportAbaqus(AbaqusImport* fileReader, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgImportAbaqus)
{
	m_fileReader = fileReader;
	ui->setupUi(this);

	ui->pc[0]->setChecked(m_fileReader->m_bnodesets   );
	ui->pc[1]->setChecked(m_fileReader->m_belemsets   );
	ui->pc[2]->setChecked(m_fileReader->m_bfacesets   );
	ui->pc[3]->setChecked(m_fileReader->m_bautopart   );
	ui->pc[4]->setChecked(m_fileReader->m_bautosurf   );
	ui->pc[5]->setChecked(m_fileReader->m_bssection   );
}

void CDlgImportAbaqus::accept()
{
	m_fileReader->m_bnodesets    = ui->pc[0]->isChecked();
	m_fileReader->m_belemsets    = ui->pc[1]->isChecked();
	m_fileReader->m_bfacesets    = ui->pc[2]->isChecked();
	m_fileReader->m_bautopart    = ui->pc[3]->isChecked();
	m_fileReader->m_bautosurf    = ui->pc[4]->isChecked();
	m_fileReader->m_bssection    = ui->pc[5]->isChecked();
	QDialog::accept();
}
