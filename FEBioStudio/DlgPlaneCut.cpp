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
#include "DlgPlaneCut.h"
#include "MainWindow.h"
#include "GLView.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QValidator>
#include <QLabel>
#include <QPushButton>
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

		QGridLayout* g = new QGridLayout;
		l->addLayout(g);

		QPushButton *pbx, *pby, *pbz;

		g->addWidget(new QLabel("X-normal:"), 0, 0); g->addWidget(w[0] = new CDragBox, 0, 1); g->addWidget(pbx = new QPushButton("X"), 0, 2);
		g->addWidget(new QLabel("Y-normal:"), 1, 0); g->addWidget(w[1] = new CDragBox, 1, 1); g->addWidget(pby = new QPushButton("Y"), 1, 2);
		g->addWidget(new QLabel("Z-normal:"), 2, 0); g->addWidget(w[2] = new CDragBox, 2, 1); g->addWidget(pbz = new QPushButton("Z"), 2, 2);
		g->addWidget(new QLabel("offset:"  ), 3, 0); g->addWidget(w[3] = new CDragBox, 3, 1);

		g->setColumnStretch(1, 2);

		w[0]->setRange(-1.0, 1.0); w[0]->setSingleStep(0.01);
		w[1]->setRange(-1.0, 1.0); w[1]->setSingleStep(0.01);
		w[2]->setRange(-1.0, 1.0); w[2]->setSingleStep(0.01);
		w[3]->setRange(-1.0, 1.0); w[3]->setSingleStep(0.01);
		
		g->addWidget(new QLabel("method:"), 4, 0);
		g->addWidget(m_rb[0] = new QRadioButton("plane cut"), 4, 1);
		g->addWidget(m_rb[1] = new QRadioButton("hide elements"), 5, 1);
		m_rb[0]->setChecked(true);

		QButtonGroup* bg = new QButtonGroup;
		bg->addButton(m_rb[0], 0);
		bg->addButton(m_rb[1], 1);

		w[0]->setValue(1.0);
		w[1]->setValue(0.0);
		w[2]->setValue(0.0);
		w[3]->setValue(0.0);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(w[0], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[1], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[2], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[3], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(bg, SIGNAL(buttonClicked(QAbstractButton*)), dlg, SLOT(onDataChanged()));

		QObject::connect(pbx, SIGNAL(clicked()), dlg, SLOT(onXClicked()));
		QObject::connect(pby, SIGNAL(clicked()), dlg, SLOT(onYClicked()));
		QObject::connect(pbz, SIGNAL(clicked()), dlg, SLOT(onZClicked()));
	}
};

CDlgPlaneCut::CDlgPlaneCut(CMainWindow* wnd) : QDialog(wnd), ui(new UIDlgPlaneCut)
{
	setWindowTitle("Plane cut");
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
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

void CDlgPlaneCut::setOrientation(double x, double y, double z)
{
	ui->w[0]->blockSignals(true);
	ui->w[1]->blockSignals(true);
	ui->w[2]->blockSignals(true);

	ui->w[0]->setValue(x);
	ui->w[1]->setValue(y);
	ui->w[2]->setValue(z);

	ui->w[0]->blockSignals(false);
	ui->w[1]->blockSignals(false);
	ui->w[2]->blockSignals(false);

	onDataChanged();
}

void CDlgPlaneCut::onXClicked()
{
	setOrientation(1, 0, 0);
}

void CDlgPlaneCut::onYClicked()
{
	setOrientation(0, 1, 0);
}

void CDlgPlaneCut::onZClicked()
{
	setOrientation(0, 0, 1);
}
