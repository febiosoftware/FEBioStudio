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
#include "DlgVTKExport.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

class Ui::CDlgVTKExport
{
public:
	QCheckBox* partIds;
	QCheckBox* shellThick;
	QCheckBox* scalarData;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;

		lo->addWidget(partIds    = new QCheckBox("Write part numbers as CELL_DATA"));
		lo->addWidget(shellThick = new QCheckBox("Write shell thickness"));
		lo->addWidget(scalarData = new QCheckBox("Write scalar data"));
		partIds->setChecked(true);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgVTKExport::CDlgVTKExport(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgVTKExport)
{
	ui->setupUi(this);
}

void CDlgVTKExport::accept()
{
	m_bpart_ids    = ui->partIds->isChecked();
	m_bshell_thick = ui->shellThick->isChecked();
	m_bscalar_data = ui->scalarData->isChecked();

	QDialog::accept();
}
