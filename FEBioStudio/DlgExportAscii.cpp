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

// DlgExportAscii.cpp: implementation of the CDlgExportAscii class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgExportAscii.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <string>
using namespace std;

class Ui::CDlgExportAscii
{
public:
	QCheckBox* coords;
	QCheckBox* face;
	QCheckBox* normals;
	QCheckBox* elem;
	QCheckBox* nodeData;
	QCheckBox* elemData;
	QCheckBox* selOnly;
	QComboBox* step;
	QLineEdit* fmt;

public:
	void setupUi(QWidget* wnd)
	{
		QFormLayout* form = new QFormLayout;
		form->addRow("Nodal coordinates"   , coords   = new QCheckBox);
		form->addRow("Facet connectivity"  , face     = new QCheckBox);
		form->addRow("Facet normals"       , normals  = new QCheckBox);
		form->addRow("Element connectivity", elem     = new QCheckBox);
		form->addRow("Nodal values"        , nodeData = new QCheckBox);
		form->addRow("Element values"      , elemData = new QCheckBox);
		form->addRow("Selection only"      , selOnly  = new QCheckBox);

		form->addRow("Time steps", step = new QComboBox);
		step->addItem("Current time step");
		step->addItem("All time steps");

		form->addRow("Format", fmt = new QLineEdit);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addWidget(bb);

		wnd->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), wnd, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), wnd, SLOT(reject()));
	}
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgExportAscii::CDlgExportAscii(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgExportAscii)
{
	ui->setupUi(this);
}

void CDlgExportAscii::accept()
{
	m_bcoords = ui->coords->isChecked();
	m_belem   = ui->elem->isChecked();
	m_bndata  = ui->nodeData->isChecked();
	m_bedata  = ui->elemData->isChecked();
	m_bface   = ui->face->isChecked();
	m_bsel    = ui->selOnly->isChecked();
	m_bfnormals= ui ->normals->isChecked();

	m_nstep = ui->step->currentIndex();

	string s = ui->fmt->text().toStdString();
	strcpy(m_szfmt, s.c_str());

	QDialog::accept();
}
