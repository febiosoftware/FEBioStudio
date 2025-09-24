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
#include "MathEditWidget.h"
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QBoxLayout>
#include <QPainter>
#include <CUILib/PlotWidget.h>

#ifdef WIN32
#include <float.h>
#define ISNAN(x) _isnan(x)
#endif

#ifdef LINUX
#ifdef CENTOS
#define ISNAN(x) isnan(x)
#else
#define ISNAN(x) std::isnan(x)
#endif
#endif

#ifdef __APPLE__
#include <math.h>
#define ISNAN(x) isnan(x)
#endif

class UIMathEditWidget
{
public:
	QLineEdit* rngMin;
	QLineEdit* rngMax;
	QComboBox* leftExt;
	QComboBox* rghtExt;
	QLineEdit* edit;
	CMathPlotWidget* plot;
	QLabel* fnc;

	QWidget* rngOps;

public:
	void setup(QWidget* w)
	{
		rngMin = new QLineEdit; rngMin->setValidator(new QDoubleValidator()); rngMin->setText(QString::number(0.0));
		rngMax = new QLineEdit; rngMax->setValidator(new QDoubleValidator()); rngMax->setText(QString::number(1.0));

		leftExt = new QComboBox; leftExt->addItems(QStringList() << "zero" << "constant" << "repeat");
		rghtExt = new QComboBox; rghtExt->addItems(QStringList() << "zero" << "constant" << "repeat");

		QHBoxLayout* hx = new QHBoxLayout;
		hx->addWidget(new QLabel("min:")); hx->addWidget(rngMin);
		hx->addWidget(new QLabel("max:")); hx->addWidget(rngMax);
		hx->addWidget(new QLabel("left extend:")); hx->addWidget(leftExt);
		hx->addWidget(new QLabel("right extend:")); hx->addWidget(rghtExt);
		hx->addStretch();

		rngOps = new QWidget;
		rngOps->setLayout(hx);
		rngOps->hide();
		rngOps->setSizePolicy(rngOps->sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);

		edit = new QLineEdit;
		QHBoxLayout* editLayout = new QHBoxLayout;
		editLayout->addWidget(fnc = new QLabel("f(x) = "));
		editLayout->addWidget(edit);

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(rngOps);
		l->addLayout(editLayout);
		l->addWidget(plot = new CMathPlotWidget);
		w->setLayout(l);

		QObject::connect(edit, SIGNAL(editingFinished()), w, SLOT(onEditingFinished()));
		QObject::connect(leftExt, SIGNAL(currentIndexChanged(int)), w, SLOT(onLeftExtendChanged()));
		QObject::connect(rghtExt, SIGNAL(currentIndexChanged(int)), w, SLOT(onRightExtendChanged()));
		QObject::connect(rngMin, SIGNAL(textChanged(const QString&)), w, SLOT(onRangeMinChanged()));
		QObject::connect(rngMax, SIGNAL(textChanged(const QString&)), w, SLOT(onRangeMaxChanged()));
	}

	void showRangeOptions(bool b)
	{
		rngOps->setVisible(b);
	}
};

CMathEditWidget::CMathEditWidget(QWidget* parent) : QWidget(parent), ui(new UIMathEditWidget)
{
	ui->setup(this);
}

void CMathEditWidget::SetOrdinate(const QString& x)
{
	QString t = QString("f(%1) = ").arg(x);
	ui->fnc->setText(t);
	ui->plot->SetOrdinate(x.toStdString());
}

void CMathEditWidget::onEditingFinished()
{
	QString s = ui->edit->text();
	ui->plot->SetMath(s);

	emit mathChanged(s);
}

void CMathEditWidget::onLeftExtendChanged()
{
	int n = ui->leftExt->currentIndex();
	ui->plot->setLeftExtendMode(n);
	emit leftExtendChanged(n);
}

void CMathEditWidget::onRightExtendChanged()
{
	int n = ui->rghtExt->currentIndex();
	ui->plot->setRightExtendMode(n);
	emit rightExtendChanged(n);
}

void CMathEditWidget::onRangeMinChanged()
{
	double v = ui->rngMin->text().toDouble();

	double xmin, xmax;
	ui->plot->GetHighlightInterval(xmin, xmax);

	ui->plot->SetHighlightInterval(v, xmax);

	emit minChanged(v);
}

void CMathEditWidget::onRangeMaxChanged()
{
	double v = ui->rngMax->text().toDouble();

	double xmin, xmax;
	ui->plot->GetHighlightInterval(xmin, xmax);

	ui->plot->SetHighlightInterval(xmin, v);

	emit maxChanged(v);
}

void CMathEditWidget::showRangeOptions(bool b)
{
	ui->showRangeOptions(b);
}

void CMathEditWidget::setMinMaxRange(double rmin, double rmax)
{
	ui->plot->SetHighlightInterval(rmin, rmax);
}

void CMathEditWidget::ClearVariables() { ui->plot->ClearVariables(); }
void CMathEditWidget::SetVariable(const QString& name, double val) { ui->plot->SetVariable(name, val); }

void CMathEditWidget::SetMath(const QString& txt)
{
	ui->edit->setText(txt);
	ui->plot->SetMath(txt);
}

void CMathEditWidget::setLeftExtend(int n)
{
	ui->leftExt->setCurrentIndex(n);
}

void CMathEditWidget::setRightExtend(int n)
{
	ui->rghtExt->setCurrentIndex(n);
}

CMathPlotWidget::CMathPlotWidget(QWidget* parent) : CPlotWidget(parent)
{
	showLegend(false);
	m_ord = "x";
	m_math.AddVariable(m_ord);
	m_math.Create("0");

	setFullScreenMode(true);
	setXAxisLabelAlignment(ALIGN_LABEL_TOP);
	setYAxisLabelAlignment(ALIGN_LABEL_RIGHT);
	setBackgroundColor(QColor(48, 48, 48));
	setGridColor(QColor(128, 128, 128));
	setXAxisColor(QColor(200, 200, 200));
	setYAxisColor(QColor(200, 200, 200));
	setXAxisTickColor(QColor(255, 255, 255));
	setYAxisTickColor(QColor(255, 255, 255));
	setSelectionColor(QColor(255, 255, 192));

	CPlotData* data = new CPlotData;
	addPlotData(data);
	data->setLineColor(QColor(255, 92, 164));

	m_leftExtend = 0;
	m_rghtExtend = 0;

	QObject::connect(this, SIGNAL(regionSelected(QRect)), this, SLOT(onRegionSelected(QRect)));
	QObject::connect(this, SIGNAL(pointClicked(QPointF, bool)), this, SLOT(onPointClicked(QPointF, bool)));
}

double CMathPlotWidget::value(double x, MVariable* var, int& region)
{
	double xmin, xmax;
	GetHighlightInterval(xmin, xmax);
	double Dx = xmax - xmin;

	if (x <= xmin)
	{
		region = -(int)((xmin - x) / Dx) - 1;
		switch (m_leftExtend)
		{
		case ExtendMode::ZERO: return 0.0; break;
		case ExtendMode::CONSTANT: x = xmin; break;
		case ExtendMode::REPEAT: x = xmax - fmod(xmin - x, Dx); break;
		}
	}
	else if (x >= xmax)
	{
		region = (int)((x - xmin) / Dx);
		switch (m_rghtExtend)
		{
		case ExtendMode::ZERO: return 0.0; break;
		case ExtendMode::CONSTANT: x = xmax; break;
		case ExtendMode::REPEAT: x = xmin + fmod(x - xmin, Dx); break;
		}
	}
	else region = 0;

	var->value(x);
	double y = m_math.value();
	return y;
}

void CMathPlotWidget::DrawPlotData(QPainter& painter, CPlotData& data)
{
	MVariable* var = m_math.FindVariable(m_ord);
	if (var == nullptr) return;

	double xmin, xmax;
	GetHighlightInterval(xmin, xmax);
	bool useInterval = false;
	if (xmax > xmin) useInterval = true;

	// draw the line
	painter.setPen(QPen(data.lineColor(), data.lineWidth()));
	QRect rt = ScreenRect();
	QPointF p0, p1;
	int prevRegion = 0;
	int curRegion = 0;
	bool newSection = true;
	for (int i = rt.left(); i < rt.right(); i += 2)
	{
		p1.setX(i);
		QPointF p = ScreenToView(p1);

		double x = p.x();
		double y = 0.0;
		if (useInterval)
		{
			y = value(x, var, curRegion);
		}
		else
		{
			var->value(x);
			y = m_math.value();
		}

		if (ISNAN(y))
		{
			newSection = true;
		}
		else
		{
			p.setY(y);
			p1 = ViewToScreen(p);

			if (newSection == false)
			{
				if (curRegion != prevRegion) p0.setY(p1.y());
				painter.drawLine(p0, p1);
			}

			p0 = p1;
			newSection = false;
		}

		prevRegion = curRegion;
	}
}

void CMathPlotWidget::SetOrdinate(const std::string& x)
{
	if (x.empty()) m_ord = "x";
	else m_ord = x;
}

void CMathPlotWidget::ClearVariables()
{
	m_Var.clear();
}

void CMathPlotWidget::SetVariable(const QString& name, double val)
{
	m_Var.push_back(std::pair<QString, double>(name, val));
}

void CMathPlotWidget::SetMath(const QString& txt)
{
	std::string m = txt.toStdString();
	m_math.Clear();
	m_math.AddVariable(m_ord);

	for (int i = 0; i < m_Var.size(); ++i)
	{
		std::pair<QString, double>& vi = m_Var[i];
		std::string mi = vi.first.toStdString();
		m_math.AddVariable(mi, vi.second);
	}

	getPlotData(0).clear();

	if (m.empty()) m = "0";
	m_math.Create(m);

	repaint();
}

void CMathPlotWidget::setLeftExtendMode(int n)
{
	m_leftExtend = n;
	repaint();
}

void CMathPlotWidget::setRightExtendMode(int n)
{
	m_rghtExtend = n;
	repaint();
}

void CMathPlotWidget::onRegionSelected(QRect rt)
{
	clearSelection();
	getPlotData(0).clear();
	fitToRect(rt);
}

void CMathPlotWidget::onPointClicked(QPointF pt, bool shift)
{
	MVariable* x = m_math.FindVariable(m_ord);
	if (x == nullptr) return;

	x->value(pt.x());
	double y = m_math.value();

	QPointF px(pt.x(), y);
	QPointF p0 = ViewToScreen(pt);
	QPointF p1 = ViewToScreen(px);

	CPlotData& data = getPlotData(0);
	data.clear();

	double D = abs(p0.x() - p1.x()) + abs(p0.y() - p1.y());
	if (D < 5)
	{
		data.addPoint(pt.x(), y);
		selectPoint(0, 0);
	}
	repaint();
}
