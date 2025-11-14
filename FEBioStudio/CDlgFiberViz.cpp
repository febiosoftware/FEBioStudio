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
#include "DlgFiberViz.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QStackedWidget>
#include <QGridLayout>
#include <QSlider>
#include <QCheckBox>
#include "MainWindow.h"
#include "GLModelScene.h"
#include "GLView.h"

class CDlgFiberViz_UI
{
public:
	CMainWindow* m_wnd;
	QSlider* m_scale;
	QSlider* m_width;
	QSlider* m_density;
	QComboBox* m_style;
	QComboBox* m_color;
	QCheckBox* m_selOnly;
	QCheckBox* m_showHidden;

public:
	void setup(QDialog* dlg)
	{
		m_color = new QComboBox();
		m_color->addItems(QStringList() << "Orientation" << "Material" << "Component");

		m_scale = new QSlider;
		m_scale->setOrientation(Qt::Horizontal);
		m_scale->setRange(0, 100);

		m_style = new QComboBox();
		m_style->addItems(QStringList() << "Lines" << "Shaded lines");

		m_width = new QSlider;
		m_width->setOrientation(Qt::Horizontal);
		m_width->setRange(0, 100);

		m_density = new QSlider;
		m_density->setOrientation(Qt::Horizontal);
		m_density->setRange(0, 100);

		QFormLayout* f = new QFormLayout;
		f->addRow("Color by:", m_color);
		f->addRow("Scale factor:", m_scale);
		f->addRow("Line width:", m_width);
		f->addRow("Line style:", m_style);
		f->addRow("Density:", m_density);
		f->addRow("Selected objects only:", m_selOnly = new QCheckBox);
		f->addRow("Show fibers on hidden parts:", m_showHidden = new QCheckBox);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(f);
		l->addStretch();

		dlg->setLayout(l);

		QObject::connect(m_color, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onDataChanged()));
		QObject::connect(m_scale, SIGNAL(valueChanged(int)), dlg, SLOT(onDataChanged()));
		QObject::connect(m_width, SIGNAL(valueChanged(int)), dlg, SLOT(onDataChanged()));
		QObject::connect(m_density, SIGNAL(valueChanged(int)), dlg, SLOT(onDataChanged()));
		QObject::connect(m_style, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onDataChanged()));
		QObject::connect(m_selOnly, SIGNAL(clicked(bool)), dlg, SLOT(onDataChanged()));
		QObject::connect(m_showHidden, SIGNAL(clicked(bool)), dlg, SLOT(onDataChanged()));
	}
};

CDlgFiberViz::CDlgFiberViz(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgFiberViz_UI)
{
	setWindowTitle("Fiber Viz tool");
	ui->setup(this);
	ui->m_wnd = wnd;
}

#define VMIN 0.01
#define VMAX 10

int double_to_int(double v)
{
	double w = (v - VMIN) / (VMAX - VMIN);
	if (w < 0.0) w = 0.0;
	if (w > 1.0) w = 1.0;
	w = sqrt(w);
	int n = (int)(100.0 * w);
	return n;
}

double int_to_double(int n)
{
	double w = (double)n / 100.0;
	if (w < 0.0) w = 0.0;
	if (w > 1.0) w = 1.0;
	w = w*w;
	double v = VMIN * (1.0 - w) + VMAX * w;
	return v;
}

void CDlgFiberViz::showEvent(QShowEvent* ev)
{
	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();
	view.m_bfiber = true;

	double v = view.m_fiber_scale;
	double w = view.m_fiber_width;
	double d = view.m_fiber_density;
	bool bhf = view.m_showHiddenFibers;
	bool bsf = view.m_showSelectFibersOnly;
	int c = view.m_fibColor;
	int ls = view.m_fibLineStyle;

	int n = double_to_int(v);
	ui->m_scale->setValue(n);

	int m = double_to_int(w);
	ui->m_width->setValue(m);

	ui->m_density->setValue((int)(100.0 * d));

	ui->m_color->setCurrentIndex(c);
	ui->m_style->setCurrentIndex(ls);

	ui->m_showHidden->setChecked(bhf);
	ui->m_selOnly->setChecked(bsf);

	ui->m_wnd->RedrawGL();
}

void CDlgFiberViz::closeEvent(QCloseEvent* ev)
{
	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();
	view.m_bfiber = false;
	ui->m_wnd->RedrawGL();
}

void CDlgFiberViz::onDataChanged()
{
	int ncol = ui->m_color->currentIndex();
	if (ncol < 0) ncol = 0;

	int nstyle = ui->m_style->currentIndex();
	if (nstyle < 0) nstyle = 0;

	double v = int_to_double(ui->m_scale->value());
	double w = int_to_double(ui->m_width->value());

	double d = ui->m_density->value() / 100.0;
	if (d < 0) d = 0;
	else if (d > 1) d = 1;

	bool selonly = ui->m_selOnly->isChecked();
	bool hiddenFibers = ui->m_showHidden->isChecked();

	GLViewSettings& view = ui->m_wnd->GetGLView()->GetViewSettings();

	view.m_fibColor = ncol;
	view.m_fiber_scale = v;
	view.m_fiber_width = w;
	view.m_fiber_density = d;
	view.m_fibLineStyle = nstyle;
	view.m_showSelectFibersOnly = selonly;
	view.m_showHiddenFibers = hiddenFibers;

	ui->m_wnd->RedrawGL();
}
