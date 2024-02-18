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
#include "DlgCloneRevolve.h"
#include <QLineEdit>
#include <QValidator>
#include <QGridLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCheckBox>

class Ui::CDlgCloneRevolve
{
public:
	QLineEdit	*count;
	QLineEdit	*range;
	QLineEdit	*spiral;
	QLineEdit*	axis[3];
	QLineEdit*	center[3];
	QCheckBox*	check;

public:
	void setup(QWidget* w)
	{
		QGridLayout* grid = new QGridLayout;
		grid->addWidget(new QLabel("Clones:"), 0, 0);
		grid->addWidget(count = new QLineEdit, 0, 1); count->setValidator(new QIntValidator(1, 100)); count->setText("1");

		grid->addWidget(new QLabel("Range:"), 1, 0);
		grid->addWidget(range = new QLineEdit, 1, 1); range->setValidator(new QDoubleValidator); range->setText("360");

		grid->addWidget(new QLabel("Spiral Distance:"), 2, 0);
		grid->addWidget(spiral = new QLineEdit, 2, 1); spiral->setValidator(new QDoubleValidator); spiral->setText("0.0");

		grid->addWidget(new QLabel("Rotation Center:"), 3, 0);
		grid->addWidget(center[0] = new QLineEdit, 3, 1); center[0]->setValidator(new QDoubleValidator); center[0]->setText("0.0");
		grid->addWidget(center[1] = new QLineEdit, 3, 2); center[1]->setValidator(new QDoubleValidator); center[1]->setText("0.0");
		grid->addWidget(center[2] = new QLineEdit, 3, 3); center[2]->setValidator(new QDoubleValidator); center[2]->setText("0.0");

		grid->addWidget(new QLabel("Rotation Axis:"), 4, 0);
		grid->addWidget(axis[0] = new QLineEdit, 4, 1); axis[0]->setValidator(new QDoubleValidator); axis[0]->setText("0.0");
		grid->addWidget(axis[1] = new QLineEdit, 4, 2); axis[1]->setValidator(new QDoubleValidator); axis[1]->setText("0.0");
		grid->addWidget(axis[2] = new QLineEdit, 4, 3); axis[2]->setValidator(new QDoubleValidator); axis[2]->setText("1.0");

		grid->addWidget(check = new QCheckBox("Rotate clones")); check->setChecked(true);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(grid);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		w->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), w, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), w, SLOT(reject()));
	}
};

CDlgCloneRevolve::CDlgCloneRevolve(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgCloneRevolve)
{
	ui->setup(this);
}

void CDlgCloneRevolve::accept()
{
	m_count = ui->count->text().toInt();
	m_range = ui->range->text().toDouble();
	m_spiral = ui->spiral->text().toDouble();

	m_center.x = ui->center[0]->text().toDouble();
	m_center.y = ui->center[1]->text().toDouble();
	m_center.z = ui->center[2]->text().toDouble();

	m_axis.x = ui->axis[0]->text().toDouble();
	m_axis.y = ui->axis[1]->text().toDouble();
	m_axis.z = ui->axis[2]->text().toDouble();

	m_rotateClones = ui->check->isChecked();

	QDialog::accept();
}
