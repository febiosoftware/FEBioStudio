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
#include "DlgLameConvertor.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QValidator>

class Ui::CDlgLameConvertor
{
public:
	QComboBox* from;
	QComboBox* to;

	QLineEdit*	in[2];
	QLineEdit*	out[2];

	QLabel* inLabel[2];
	QLabel* outLabel[2];

public:
	void setupUi(QWidget* parent)
	{
		from = new QComboBox;
		QLabel* fromLabel = new QLabel("From:");
		fromLabel->setBuddy(from);

		in[0] = new QLineEdit; in[0]->setValidator(new QDoubleValidator); in[0]->setText(QString::number(0.0)); in[0]->setMaximumWidth(150);
		in[1] = new QLineEdit; in[1]->setValidator(new QDoubleValidator); in[1]->setText(QString::number(0.0)); in[1]->setMaximumWidth(150);

		to = new QComboBox;
		QLabel* toLabel = new QLabel("To:");
		toLabel->setBuddy(to);

		out[0] = new QLineEdit; out[0]->setReadOnly(true); out[0]->setMaximumWidth(150);
		out[1] = new QLineEdit; out[1]->setReadOnly(true); out[1]->setMaximumWidth(150);

		QFormLayout* formIn = new QFormLayout;
		formIn->setLabelAlignment(Qt::AlignRight);
		formIn->addRow(inLabel[0] = new QLabel("E"), in[0]);
		formIn->addRow(inLabel[1] = new QLabel("v"), in[1]);

		QFormLayout* formOut = new QFormLayout;
		formOut->setLabelAlignment(Qt::AlignRight);
		formOut->addRow(outLabel[0] = new QLabel("E"), out[0]);
		formOut->addRow(outLabel[1] = new QLabel("v"), out[1]);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);

		QHBoxLayout* fromLayout = new QHBoxLayout;
		fromLayout->addWidget(fromLabel);
		fromLayout->addWidget(from);
		fromLayout->addStretch();

		QHBoxLayout* toLayout = new QHBoxLayout;
		toLayout->addWidget(toLabel);
		toLayout->addWidget(to);
		toLayout->addStretch();

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(fromLayout);
		mainLayout->addLayout(formIn);
		mainLayout->addLayout(toLayout);
		mainLayout->addLayout(formOut);
		mainLayout->addWidget(bb);

		parent->setLayout(mainLayout);

		QObject::connect(bb  , SIGNAL(clicked(QAbstractButton*)), parent, SLOT(accept()));	
		QObject::connect(from, SIGNAL(currentIndexChanged(int)), parent, SLOT(fromChanged(int)));
		QObject::connect(to  , SIGNAL(currentIndexChanged(int)), parent, SLOT(toChanged(int)));
		QObject::connect(in[0], SIGNAL(textChanged(const QString&)), parent, SLOT(InputChanged()));
		QObject::connect(in[1], SIGNAL(textChanged(const QString&)), parent, SLOT(InputChanged()));
	}
};

CDlgLameConvertor::CDlgLameConvertor(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgLameConvertor)
{
	ui->setupUi(this);

	ui->from->addItem("Young's modulus & Poisson's ratio");
	ui->from->addItem("Bulk modulus & Shear modulus");
	ui->from->addItem("Lame parameters");

	ui->to->addItem("Young's modulus & Poisson's ratio");
	ui->to->addItem("Bulk modulus & Shear modulus");
	ui->to->addItem("Lame parameters");

	resize(300, 200);
	setWindowTitle("Elasticity Converter");
}


void Ev2KG(double E, double v, double& K, double& G)
{
	K = E / (3.0*(1.0 - 2.0*v));
	G = 0.5*E / (1.0 + v);
}

void KG2Ev(double K, double G, double& E, double& v)
{
	E = 9.0*K*G / (3.0*K + G);
	v = 0.5*(3.0*K - 2.0*G) / (3.0*K + G);
}

void Ev2Lame(double E, double v, double& l, double& m)
{
	l = E*v / ((1.0 + v)*(1.0 - 2.0*v));
	m = 0.5*E / (1.0 + v);
}

void Lame2Ev(double l, double m, double& E, double& v)
{
	E = m*(3.0*l + 2.0*m) / (l + m);
	v = 0.5*l / (l + m);
}

void KG2Lame(double K, double G, double& l, double& m)
{
	l = K - 2.0*G / 3.0;
	m = G;
}

void Lame2KG(double l, double m, double& K, double& G)
{
	K = l + 2.0*m / 3.0;
	G = m;
}

void CDlgLameConvertor::fromChanged(int fromIndex)
{
	switch (fromIndex)
	{
	case 0: ui->inLabel[0]->setText("E"); ui->inLabel[1]->setText("v"); break;
	case 1: ui->inLabel[0]->setText("K"); ui->inLabel[1]->setText("G"); break;
	case 2: ui->inLabel[0]->setText("lambda"); ui->inLabel[1]->setText("mu"); break;
	}

	int toIndex = ui->to->currentIndex();

	UpdateData(fromIndex, toIndex);
}

void CDlgLameConvertor::toChanged(int toIndex)
{
	switch (toIndex)
	{
	case 0: ui->outLabel[0]->setText("E"); ui->outLabel[1]->setText("v"); break;
	case 1: ui->outLabel[0]->setText("K"); ui->outLabel[1]->setText("G"); break;
	case 2: ui->outLabel[0]->setText("lambda"); ui->outLabel[1]->setText("mu"); break;
	}

	int fromIndex = ui->from->currentIndex();

	UpdateData(fromIndex, toIndex);
}

void CDlgLameConvertor::InputChanged()
{
	int fromIndex = ui->from->currentIndex();
	int toIndex = ui->to->currentIndex();
	UpdateData(fromIndex, toIndex);
}

void CDlgLameConvertor::UpdateData(int fromIndex, int toIndex)
{
	// get input data
	double a1 = ui->in[0]->text().toDouble();
	double a2 = ui->in[1]->text().toDouble();

	// calculate output data
	double b1, b2;
	b1 = a1;
	b2 = a2;

	if ((fromIndex == 0) && (toIndex == 1)) Ev2KG  (a1, a2, b1, b2);
	if ((fromIndex == 0) && (toIndex == 2)) Ev2Lame(a1, a2, b1, b2);
	if ((fromIndex == 1) && (toIndex == 0)) KG2Ev  (a1, a2, b1, b2);
	if ((fromIndex == 1) && (toIndex == 2)) KG2Lame(a1, a2, b1, b2);
	if ((fromIndex == 2) && (toIndex == 0)) Lame2Ev(a1, a2, b1, b2);
	if ((fromIndex == 2) && (toIndex == 1)) Lame2KG(a1, a2, b1, b2);

	// update UI
	ui->out[0]->setText(QString::number(b1));
	ui->out[1]->setText(QString::number(b2));
}
