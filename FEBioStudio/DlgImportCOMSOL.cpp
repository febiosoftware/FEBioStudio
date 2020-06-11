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
#include "DlgImportCOMSOL.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

class Ui::CDlgImportCOMSOL
{
public:
	QCheckBox* pc[10];

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;

		pc[0] = new QCheckBox("Convert domains to named selected");
		pc[1] = new QCheckBox("Auto-partition domain into parts");
		pc[2] = new QCheckBox("Segregate element types");
		pc[3] = new QCheckBox("Import TRI elements");
		pc[4] = new QCheckBox("Import QUAD elements");
		pc[5] = new QCheckBox("Import TET elements");
		pc[6] = new QCheckBox("Import PRISM elements");
		pc[7] = new QCheckBox("Import HEX elements");
		pc[8] = new QCheckBox("Import PYR elements (as 2 TETS each)");
        pc[9] = new QCheckBox("Import PYR elements");

		for (int i=0; i<10; ++i) lo->addWidget(pc[i]);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};


CDlgImportCOMSOL::CDlgImportCOMSOL(COMSOLimport* fileReader, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgImportCOMSOL)
{
	m_fileReader = fileReader;
	ui->setupUi(this);

	ui->pc[0]->setChecked(m_fileReader->m_domainstosets);
	ui->pc[1]->setChecked(m_fileReader->m_bautopart    );
	ui->pc[2]->setChecked(m_fileReader->m_eltypeseg    );
	ui->pc[3]->setChecked(m_fileReader->m_addtris      );
	ui->pc[4]->setChecked(m_fileReader->m_addquads     );
	ui->pc[5]->setChecked(m_fileReader->m_addtets      );
	ui->pc[6]->setChecked(m_fileReader->m_addprisms    );
	ui->pc[7]->setChecked(m_fileReader->m_addhexes     );
	ui->pc[8]->setChecked(m_fileReader->m_pyrstotets   );
    ui->pc[9]->setChecked(m_fileReader->m_addpyrs      );
}

void CDlgImportCOMSOL::accept()
{
	m_fileReader->m_domainstosets = (ui->pc[0]->isChecked());
	m_fileReader->m_bautopart     = (ui->pc[1]->isChecked());
	m_fileReader->m_eltypeseg     = (ui->pc[2]->isChecked());
	m_fileReader->m_addtris       = (ui->pc[3]->isChecked());
	m_fileReader->m_addquads      = (ui->pc[4]->isChecked());
	m_fileReader->m_addtets       = (ui->pc[5]->isChecked());
	m_fileReader->m_addprisms     = (ui->pc[6]->isChecked());
	m_fileReader->m_addhexes      = (ui->pc[7]->isChecked());
	m_fileReader->m_pyrstotets    = (ui->pc[8]->isChecked());
    m_fileReader->m_addpyrs       = (ui->pc[9]->isChecked());

	QDialog::accept();
}
