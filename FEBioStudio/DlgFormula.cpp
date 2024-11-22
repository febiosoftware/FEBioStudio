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
#include "DlgFormula.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QGridLayout>
#include <QValidator>
#include <QMessageBox>
#include <QCheckBox>
#include <FECore/MathObject.h>

CDlgFormula::CDlgFormula(QWidget* parent) : QDialog(parent)
{
	QVBoxLayout* l = new QVBoxLayout;

	m_math = new QLineEdit;

	QGridLayout* g = new QGridLayout;

	g->addWidget(new QLabel("f(t) = "), 0, 0, Qt::AlignRight);
	g->addWidget(m_math, 0, 1, 1, 3);

	g->addWidget(new QLabel("min:"), 1, 0, Qt::AlignRight);

	m_min = new QLineEdit; m_min->setValidator(new QDoubleValidator);
	m_min->setText("0");
	g->addWidget(m_min, 1, 1);

	g->addWidget(new QLabel("max:"), 1, 2, Qt::AlignRight);

	m_max = new QLineEdit; m_max->setValidator(new QDoubleValidator);
	m_max->setText("1");
	g->addWidget(m_max, 1, 3);

	g->addWidget(new QLabel("samples:"), 2, 0, Qt::AlignRight);
	
	m_samples = new QLineEdit; m_samples->setValidator(new QIntValidator(2, 10000));
	m_samples->setText("11");
	g->addWidget(m_samples, 2, 1);

	m_insert = new QCheckBox("Insert into curve");
	g->addWidget(m_insert, 3, 1);

	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	l->addLayout(g);
	l->addWidget(bb);

	setLayout(l);

	QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}

QString CDlgFormula::GetMath()
{
	return m_math->text();
}

void CDlgFormula::SetMath(const QString& math)
{
	m_math->setText(math);
}

double CDlgFormula::GetMin()
{
	return m_min->text().toDouble();
}

double CDlgFormula::GetMax()
{
	return m_max->text().toDouble();
}

int CDlgFormula::GetSamples()
{
	return m_samples->text().toInt();
}

bool CDlgFormula::Insert()
{
	return m_insert->isChecked();
}

void CDlgFormula::accept()
{
	if (GetMath().isEmpty())
	{
		QMessageBox::critical(this, "", "Please enter an expresion for f(t)");
		return;
	}

	double fmin = GetMin();
	double fmax = GetMax();
	if (fmin >= fmax)
	{
		QMessageBox::critical(this, "", "Please make sure that max is larger than min");
		return;
	}

	QDialog::accept();
}

std::vector<vec2d> CDlgFormula::GetPoints()
{
	QString math = GetMath();
	std::string smath = math.toStdString();

	double fmin = GetMin();
	double fmax = GetMax();
	int samples = GetSamples();

	std::vector<vec2d> pts;
	MSimpleExpression m;
	MVariable* tvar = m.AddVariable("t");
	bool b = m.Create(smath);
	if (b == false)
	{
		QMessageBox::critical(this, "Equation editor", "Error in expression.");
	}
	else
	{
		for (int i = 0; i < samples; ++i)
		{
			double x = fmin + i * (fmax - fmin) / (samples - 1);
			tvar->value(x);
			double y = m.value();
			pts.push_back(vec2d(x, y));
		}
	}

	return pts;
}
