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
#include "DlgPlaneCut.h"
#include "MainWindow.h"
#include "GLView.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QValidator>
#include "DragBox.h"
#include <QRadioButton>
#include <QButtonGroup>

class UIDlgPlaneCut
{
public:
	CGLView*	m_view;
	CDragBox*	w[4];
	QRadioButton*	m_rb[2];

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		QFormLayout* f = new QFormLayout;
		f->addRow("X-normal:", w[0] = new CDragBox); w[0]->setRange(-1.0, 1.0); w[0]->setSingleStep(0.01);
		f->addRow("Y-normal:", w[1] = new CDragBox); w[1]->setRange(-1.0, 1.0); w[1]->setSingleStep(0.01);
		f->addRow("Z-normal:", w[2] = new CDragBox); w[2]->setRange(-1.0, 1.0); w[2]->setSingleStep(0.01);
		f->addRow("offset:"  , w[3] = new CDragBox); w[3]->setRange(-1.0, 1.0); w[3]->setSingleStep(0.01);
		f->addRow("", m_rb[0] = new QRadioButton("plane cut"));
		f->addRow("", m_rb[1] = new QRadioButton("hide elements"));
		m_rb[0]->setChecked(true);

		QButtonGroup* bg = new QButtonGroup;
		bg->addButton(m_rb[0], 0);
		bg->addButton(m_rb[1], 1);

		w[0]->setValue(1.0);
		w[1]->setValue(0.0);
		w[2]->setValue(0.0);
		w[3]->setValue(0.0);

		l->addLayout(f);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(w[0], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[1], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[2], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[3], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(bg, SIGNAL(buttonClicked(QAbstractButton*)), dlg, SLOT(onDataChanged()));
	}
};

CDlgPlaneCut::CDlgPlaneCut(CMainWindow* wnd) : QDialog(wnd), ui(new UIDlgPlaneCut)
{
	setWindowTitle("Plane cut");
	ui->m_view = wnd->GetGLView();
	ui->setup(this);
}

CDlgPlaneCut::~CDlgPlaneCut()
{

}

void CDlgPlaneCut::Update()
{

}

void CDlgPlaneCut::showEvent(QShowEvent* ev)
{
	onDataChanged();
	ui->m_view->ShowPlaneCut(true);
}

void CDlgPlaneCut::closeEvent(QCloseEvent* ev)
{
	ui->m_view->ShowPlaneCut(false);
}

void CDlgPlaneCut::reject()
{
	ui->m_view->ShowPlaneCut(false);
	QDialog::reject();
}

void CDlgPlaneCut::onDataChanged()
{
	double a[4] = { 0 };
	a[0] = ui->w[0]->value();
	a[1] = ui->w[1]->value();
	a[2] = ui->w[2]->value();
	a[3] = ui->w[3]->value();

	double L = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
	if (L != 0)
	{
		a[0] /= L;
		a[1] /= L;
		a[2] /= L;
	}
	else a[0] = 1.0;

	ui->m_view->SetPlaneCut(a);

	int nop = 0;
	if (ui->m_rb[0]->isChecked()) nop = 0;
	if (ui->m_rb[1]->isChecked()) nop = 1;
	ui->m_view->SetPlaneCutMode(nop);
}
