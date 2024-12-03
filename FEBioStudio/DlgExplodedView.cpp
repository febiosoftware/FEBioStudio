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
#include "DlgExplodedView.h"
#include "MainWindow.h"
#include <QLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QSlider>
#include "GLView.h"

class UIDlgExplodedView
{
public:
	QSlider* s = nullptr;
	QComboBox* d = nullptr;

public:
	void setup(CDlgExplodedView* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		QFormLayout* f = new QFormLayout;
		s = new QSlider;
		s->setOrientation(Qt::Horizontal);
		s->setRange(0, 100); s->setValue(50);
		d = new QComboBox;
		d->addItems({ "X", "Y", "Z" });
		f->addRow("Direction", d);
		f->addRow("Strength", s);
		l->addLayout(f);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		l->addStretch();
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(s, &QSlider::valueChanged, dlg, &CDlgExplodedView::on_slider_changed);
		QObject::connect(d, &QComboBox::currentIndexChanged, dlg, &CDlgExplodedView::on_direction_changed);
		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &CDlgExplodedView::hide);
	}

	void setStrength(double f)
	{
		int n = f * s->maximum();
		s->setValue(n);
	}

	double strength() const
	{
		int n = s->value();
		return (double)n / (double)s->maximum();
	}

	int direction() const
	{
		return d->currentIndex();
	}
};

CDlgExplodedView::CDlgExplodedView(CMainWindow* mainWnd) : QDialog(mainWnd), ui(new UIDlgExplodedView), wnd(mainWnd)
{
	setWindowTitle("Exploded View");
	setMinimumSize(QSize(300, 150));
	ui->setup(this);
}

void CDlgExplodedView::showEvent(QShowEvent* ev)
{
	if (wnd && wnd->GetModelDocument())
	{
		GLViewSettings& vs = wnd->GetGLView()->GetViewSettings();
		ui->setStrength(vs.m_explode_strength);
		vs.m_explode = true;
		wnd->RedrawGL();
	}
}

void CDlgExplodedView::hideEvent(QHideEvent* ev)
{
	if (wnd && wnd->GetModelDocument())
	{
		GLViewSettings& vs = wnd->GetGLView()->GetViewSettings();
		vs.m_explode = false;
		wnd->RedrawGL();
	}
}

CDlgExplodedView::~CDlgExplodedView()
{
	wnd = nullptr;
	delete ui;
}

void CDlgExplodedView::on_slider_changed(int n)
{
	if (wnd && wnd->GetModelDocument())
	{
		GLViewSettings& vs = wnd->GetGLView()->GetViewSettings();
		vs.m_explode_strength = ui->strength();
		wnd->RedrawGL();
	}
}

void CDlgExplodedView::on_direction_changed(int n)
{
	if (wnd && wnd->GetModelDocument())
	{
		GLViewSettings& vs = wnd->GetGLView()->GetViewSettings();
		vs.m_explode_direction = ui->direction();
		wnd->RedrawGL();
	}
}
