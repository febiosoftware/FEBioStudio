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
#include <GLLib/GLScene.h>

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

		QPushButton *pbx, *pby, *pbz, *flip;

		g->addWidget(new QLabel("X-normal:"), 0, 0); g->addWidget(w[0] = new CDragBox, 0, 1); g->addWidget(pbx = new QPushButton("X"), 0, 2);
		g->addWidget(new QLabel("Y-normal:"), 1, 0); g->addWidget(w[1] = new CDragBox, 1, 1); g->addWidget(pby = new QPushButton("Y"), 1, 2);
		g->addWidget(new QLabel("Z-normal:"), 2, 0); g->addWidget(w[2] = new CDragBox, 2, 1); g->addWidget(pbz = new QPushButton("Z"), 2, 2);
		g->addWidget(new QLabel("offset:"  ), 3, 0); g->addWidget(w[3] = new CDragBox, 3, 1); g->addWidget(flip = new QPushButton("Flip"), 3, 2);

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
		QObject::connect(flip, SIGNAL(clicked()), dlg, SLOT(onFlipClicked()));
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
	ui->m_view->GetViewSettings().m_showPlaneCut = true;
	ui->m_view->UpdateScene();
}

void CDlgPlaneCut::closeEvent(QCloseEvent* ev)
{
	ui->m_view->GetViewSettings().m_showPlaneCut = false;
	ui->m_view->UpdateScene();
}

void CDlgPlaneCut::reject()
{
	ui->m_view->GetViewSettings().m_showPlaneCut = false;
	ui->m_view->UpdateScene();
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

	setPlaneCoordinates(a);

	int nop = 0;
	if (ui->m_rb[0]->isChecked()) nop = 0;
	if (ui->m_rb[1]->isChecked()) nop = 1;
	ui->m_view->GetViewSettings().m_planeCutMode = nop;
	ui->m_view->UpdateScene();
}

void CDlgPlaneCut::setPlaneCoordinates(double d[4])
{
	GLScene* scene = ui->m_view->GetActiveScene();
	if (scene == nullptr) return;

	BOX box = scene->GetBoundingBox();

	double R = box.GetMaxExtent();
	if (R < 1e-12) R = 1.0;

	GLViewSettings& vs = ui->m_view->GetViewSettings();

	vec3d n(d[0], d[1], d[2]);

	vec3d a = box.r0();
	vec3d b = box.r1();
	vec3d r[8];
	r[0] = vec3d(a.x, a.y, a.z);
	r[1] = vec3d(b.x, a.y, a.z);
	r[2] = vec3d(b.x, b.y, a.z);
	r[3] = vec3d(a.x, b.y, a.z);
	r[4] = vec3d(a.x, a.y, b.z);
	r[5] = vec3d(b.x, a.y, b.z);
	r[6] = vec3d(b.x, b.y, b.z);
	r[7] = vec3d(a.x, b.y, b.z);
	double d0 = n * r[0];
	double d1 = d0;
	for (int i = 1; i < 8; ++i)
	{
		double d = n * r[i];
		if (d < d0) d0 = d;
		if (d > d1) d1 = d;
	}

	double d3 = d0 + 0.5 * (d[3] + 1) * (d1 - d0);

	vs.m_planeCut[0] = n.x;
	vs.m_planeCut[1] = n.y;
	vs.m_planeCut[2] = n.z;
	vs.m_planeCut[3] = -d3;
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

void CDlgPlaneCut::onFlipClicked()
{
	ui->w[0]->blockSignals(true);
	ui->w[1]->blockSignals(true);
	ui->w[2]->blockSignals(true);

	double x = ui->w[0]->value();
	double y = ui->w[1]->value();
	double z = ui->w[2]->value();
	double a = ui->w[3]->value();

	ui->w[0]->setValue(-x);
	ui->w[1]->setValue(-y);
	ui->w[2]->setValue(-z);
	ui->w[3]->setValue(-a);

	ui->w[0]->blockSignals(false);
	ui->w[1]->blockSignals(false);
	ui->w[2]->blockSignals(false);
	ui->w[3]->blockSignals(false);

	onDataChanged();
}
