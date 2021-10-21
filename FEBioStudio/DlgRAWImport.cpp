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
#include "DlgRAWImport.h"
#include <QLineEdit>
#include <QBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QValidator>

class Ui::CDlgRAWImport
{
public:
	QLineEdit*	nx;
	QLineEdit*	ny;
	QLineEdit*	nz;

	QLineEdit*	x0;
	QLineEdit*	y0;
	QLineEdit*	z0;

	QLineEdit*	w;
	QLineEdit*	h;
	QLineEdit*	d;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;
		nx = new QLineEdit; nx->setValidator(new QIntValidator(1, 4096));
		ny = new QLineEdit; ny->setValidator(new QIntValidator(1, 4096));
		nz = new QLineEdit; nz->setValidator(new QIntValidator(1, 4096));

		x0 = new QLineEdit; x0->setValidator(new QDoubleValidator);
		y0 = new QLineEdit; y0->setValidator(new QDoubleValidator);
		z0 = new QLineEdit; z0->setValidator(new QDoubleValidator);

		w = new QLineEdit; w->setValidator(new QDoubleValidator);
		h = new QLineEdit; h->setValidator(new QDoubleValidator);
		d = new QLineEdit; d->setValidator(new QDoubleValidator);

		QFormLayout* form = new QFormLayout;
		form->addRow("nx", nx);
		form->addRow("ny", ny);
		form->addRow("nz", nz);
		form->addRow("x0", x0);
		form->addRow("y0", y0);
		form->addRow("z0", z0);
		form->addRow("width", w);
		form->addRow("heght", h);
		form->addRow("depth", d);

		lo->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgRAWImport::CDlgRAWImport(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgRAWImport)
{
	ui->setupUi(this);
}

void CDlgRAWImport::accept()
{
	m_nx = ui->nx->text().toInt();	
	m_ny = ui->ny->text().toInt();
	m_nz = ui->nz->text().toInt();

	m_x0 = ui->x0->text().toDouble();
	m_y0 = ui->y0->text().toDouble();
	m_z0 = ui->z0->text().toDouble();

	m_w = ui->w->text().toDouble();
	m_h = ui->h->text().toDouble();
	m_d = ui->d->text().toDouble();

	QDialog::accept();
}
