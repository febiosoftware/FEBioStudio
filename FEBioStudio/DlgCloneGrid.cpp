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
#include "DlgCloneGrid.h"
#include <QLineEdit>
#include <QValidator>
#include <QGridLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

class Ui::CDlgCloneGrid
{
public:
	QLineEdit	*x0, *x1;
	QLineEdit	*y0, *y1;
	QLineEdit	*z0, *z1;

	QLineEdit	*dx, *dy, *dz;

public:
	void setup(QWidget* w)
	{
		QGridLayout* grid = new QGridLayout;
		grid->addWidget(new QLabel("X range:"), 0, 0);
		grid->addWidget(x0 = new QLineEdit, 0, 1); x0->setValidator(new QIntValidator(-100, 100)); x0->setText("0");
		grid->addWidget(x1 = new QLineEdit, 0, 2); x1->setValidator(new QIntValidator(-100, 100)); x1->setText("0");

		grid->addWidget(new QLabel("Y range:"), 1, 0);
		grid->addWidget(y0 = new QLineEdit, 1, 1); y0->setValidator(new QIntValidator(-100, 100)); y0->setText("0");
		grid->addWidget(y1 = new QLineEdit, 1, 2); y1->setValidator(new QIntValidator(-100, 100)); y1->setText("0");

		grid->addWidget(new QLabel("Z range:"), 2, 0);
		grid->addWidget(z0 = new QLineEdit, 2, 1); z0->setValidator(new QIntValidator(-100, 100)); z0->setText("0");
		grid->addWidget(z1 = new QLineEdit, 2, 2); z1->setValidator(new QIntValidator(-100, 100)); z1->setText("0");

		grid->addWidget(new QLabel("separation:"), 3, 0);
		grid->addWidget(dx = new QLineEdit, 3, 1); dx->setValidator(new QDoubleValidator); dx->setText("0.0");
		grid->addWidget(dy = new QLineEdit, 3, 2); dy->setValidator(new QDoubleValidator); dy->setText("0.0");
		grid->addWidget(dz = new QLineEdit, 3, 3); dz->setValidator(new QDoubleValidator); dz->setText("0.0");

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(grid);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		w->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), w, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), w, SLOT(reject()));
	}
};

CDlgCloneGrid::CDlgCloneGrid(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgCloneGrid)
{
	ui->setup(this);
}

void CDlgCloneGrid::accept()
{
	m_rangeX[0] = ui->x0->text().toInt();
	m_rangeX[1] = ui->x1->text().toInt();

	m_rangeY[0] = ui->y0->text().toInt();
	m_rangeY[1] = ui->y1->text().toInt();

	m_rangeZ[0] = ui->z0->text().toInt();
	m_rangeZ[1] = ui->z1->text().toInt();

	m_inc[0] = ui->dx->text().toDouble();
	m_inc[1] = ui->dy->text().toDouble();
	m_inc[2] = ui->dz->text().toDouble();

	QDialog::accept();
}
