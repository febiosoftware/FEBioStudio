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

#include "GraphWindow.h"
#include "PlotWidget.h"
#include "DataFieldSelector.h"
#include <QToolBar>
#include <QStackedWidget>
#include <QLabel>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QRadioButton>
#include <QGuiApplication>
#include <QScreen>
#include "MainWindow.h"
#include "Document.h"
#include <PostLib/FEPostModel.h>
#include <QToolBox>
#include <QLineEdit>
#include <QSignalMapper>
#include <PostGL/GLDataMap.h>
#include <PostGL/GLModel.h>
#include "version.h"
#include <QValidator>
#include <QComboBox>
#include <QSpinBox>
#include <FECore/tools.h>
#include "CColorButton.h"
#include <GLWLib/convert.h>
#include "PostDocument.h"
#include <PostLib/constants.h>
#include <PostLib/evaluate.h>
#include <PostGL/GLPointProbe.h>
#include <PostGL/GLRuler.h>
#include <PostGL/GLMusclePath.h>
#include <PostGL/GLPlotGroup.h>
#include <PostLib/FEMeshData_T.h>
#include <FECore/MathObject.h>
#include <FECore/MObjBuilder.h>
#include <QApplication>
#include <QClipboard>
#include <QtCore/QMimeData>
#include "DlgImportData.h"
#include "units.h"

class TimeRangeOptionsUI
{
public:
	QRadioButton*	timeOption[3];
	QLineEdit*		timeRange;
	QCheckBox*		autoRange;

public:
	void setup(QWidget* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(timeOption[0] = new QRadioButton("Time step range"));
		l->addWidget(timeOption[1] = new QRadioButton("Current time step"));
		l->addWidget(timeOption[2] = new QRadioButton("User range:"));
		l->addWidget(timeRange = new QLineEdit);
		l->addWidget(autoRange = new QCheckBox("auto update plot range"));
		l->addStretch();
		w->setLayout(l);

		autoRange->setChecked(true);
		timeOption[0]->setChecked(true);

		QObject::connect(timeOption[0], SIGNAL(clicked()), w, SLOT(onOptionsChanged()));
		QObject::connect(timeOption[1], SIGNAL(clicked()), w, SLOT(onOptionsChanged()));
		QObject::connect(timeOption[2], SIGNAL(clicked()), w, SLOT(onOptionsChanged()));
		QObject::connect(timeRange, SIGNAL(editingFinished()), w, SLOT(onOptionsChanged()));
		QObject::connect(autoRange, SIGNAL(stateChanged(int)), w, SLOT(onOptionsChanged()));
	}
};


TimeRangeOptions::TimeRangeOptions(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent), ui(new TimeRangeOptionsUI)
{
	ui->setup(this);
}

void TimeRangeOptions::onOptionsChanged()
{
	emit optionsChanged();
}

int TimeRangeOptions::currentOption()
{
	if (ui->timeOption[0]->isChecked()) return 0;
	if (ui->timeOption[1]->isChecked()) return 1;
	if (ui->timeOption[2]->isChecked()) return 2;
	return -1;
}

bool TimeRangeOptions::autoRangeUpdate()
{
	return ui->autoRange->isChecked();
}

void TimeRangeOptions::setUserTimeRange(int imin, int imax, int istep)
{
	if (istep == 1)
		ui->timeRange->setText(QString("%1:%2").arg(imin).arg(imax));
	else
		ui->timeRange->setText(QString("%1:%2:%3").arg(imin).arg(imax).arg(istep));
}

void TimeRangeOptions::getUserTimeRange(int& imin, int& imax, int& istep)
{
	QStringList l = ui->timeRange->text().split(':');
	imin = imax = 0;
	if (l.size() == 1)
	{
		imin = imax = l.at(0).toInt();
		istep = 1;
	}
	else if (l.size() == 2)
	{
		imin = l.at(0).toInt();
		imax = l.at(1).toInt();
		istep = 1;
	}
	else if (l.size() > 2)
	{
		imin = l.at(0).toInt();
		imax = l.at(1).toInt();
		istep = l.at(2).toInt();
	}
}

//=============================================================================
class GraphOptionsUI
{
public:
	QCheckBox*		smoothLines;
	QCheckBox*		drawGrid;
	QCheckBox*		drawLegend;
	QCheckBox*		drawTitle;
	QCheckBox*		drawAxes;
	CColorButton*	bgcol;
	CColorButton*	selcol;
	QLineEdit*		title;
	QLineEdit*		xlabel;
	QLineEdit*		ylabel;

	CGraphWidget*	m_graph;

	void setup(QWidget* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(smoothLines = new QCheckBox("Smooth lines"));
		l->addWidget(drawGrid = new QCheckBox("Draw grid lines"));
		l->addWidget(drawLegend = new QCheckBox("Draw legend"));
		l->addWidget(drawTitle = new QCheckBox("Draw title"));
		l->addWidget(drawAxes = new QCheckBox("Draw axes labels"));

		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0,0,0,0);
		h->addWidget(new QLabel("Background color"));
		h->addWidget(bgcol = new CColorButton());
		l->addLayout(h);

		h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Selection color"));
		h->addWidget(selcol = new CColorButton());
		l->addLayout(h);

		h = new QHBoxLayout;
		h->setContentsMargins(0,0,0,0);
		h->addWidget(new QLabel("Title"));
		h->addWidget(title = new QLineEdit());
		l->addLayout(h);

		h = new QHBoxLayout;
		h->setContentsMargins(0,0,0,0);
		h->addWidget(new QLabel("X-label"));
		h->addWidget(xlabel = new QLineEdit());
		l->addLayout(h);

		h = new QHBoxLayout;
		h->setContentsMargins(0,0,0,0);
		h->addWidget(new QLabel("Y-label"));
		h->addWidget(ylabel = new QLineEdit());
		l->addLayout(h);

		l->addStretch();
		w->setLayout(l);

		smoothLines->setChecked(true);
		drawGrid->setChecked(true);
		drawLegend->setChecked(true);
		drawTitle->setChecked(true);

		QObject::connect(drawGrid, SIGNAL(stateChanged(int)), w, SLOT(onOptionsChanged()));
		QObject::connect(drawLegend, SIGNAL(stateChanged(int)), w, SLOT(onOptionsChanged()));
		QObject::connect(drawTitle, SIGNAL(stateChanged(int)), w, SLOT(onOptionsChanged()));
		QObject::connect(smoothLines, SIGNAL(stateChanged(int)), w, SLOT(onOptionsChanged()));
		QObject::connect(drawAxes, SIGNAL(stateChanged(int)), w, SLOT(onOptionsChanged()));
		QObject::connect(bgcol, SIGNAL(colorChanged(QColor)), w, SLOT(onOptionsChanged()));
		QObject::connect(selcol, SIGNAL(colorChanged(QColor)), w, SLOT(onOptionsChanged()));
		QObject::connect(title, SIGNAL(editingFinished()), w, SLOT(onOptionsChanged()));
		QObject::connect(xlabel, SIGNAL(editingFinished()), w, SLOT(onOptionsChanged()));
		QObject::connect(ylabel, SIGNAL(editingFinished()), w, SLOT(onOptionsChanged()));
	}

	void setDrawGrid(bool b)
	{
		drawGrid->blockSignals(true);
		drawGrid->setChecked(b);
		drawGrid->blockSignals(false);
	}

	void setDrawLegend(bool b)
	{
		drawLegend->blockSignals(true);
		drawLegend->setChecked(b);
		drawLegend->blockSignals(false);
	}

	void setDrawTitle(bool b)
	{
		drawTitle->blockSignals(true);
		drawTitle->setChecked(b);
		drawTitle->blockSignals(false);
	}

	void setLineSmoothing(bool b)
	{
		smoothLines->blockSignals(true);
		smoothLines->setChecked(b);
		smoothLines->blockSignals(false);
	}

	void setDrawAxes(bool b)
	{
		drawAxes->blockSignals(true);
		drawAxes->setChecked(b);
		drawAxes->blockSignals(false);
	}

	void setBGColor(QColor c)
	{
		bgcol->blockSignals(true);
		bgcol->setColor(c);
		bgcol->blockSignals(false);
	}

	void setSelectionColor(QColor c)
	{
		selcol->blockSignals(true);
		selcol->setColor(c);
		selcol->blockSignals(false);
	}

	void setTitle(const QString& t)
	{
		title->blockSignals(true);
		title->setText(t);
		title->blockSignals(false);
	}

	void setXLabel(const QString& t)
	{
		xlabel->blockSignals(true);
		xlabel->setText(t);
		xlabel->blockSignals(false);
	}

	void setYLabel(const QString& t)
	{
		ylabel->blockSignals(true);
		ylabel->setText(t);
		ylabel->blockSignals(false);
	}
};

GraphOptions::GraphOptions(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent), ui(new GraphOptionsUI)
{
	ui->m_graph = graph;
	ui->setup(this);
}

void GraphOptions::onOptionsChanged()
{
	if (ui->m_graph == nullptr) return;
	ui->m_graph->setLineSmoothing(lineSmoothing());
	ui->m_graph->setDrawGrid(drawGrid());
	ui->m_graph->showLegend(drawLegend());
	ui->m_graph->setDrawTitle(drawTitle());
	ui->m_graph->setDrawAxesLabels(drawAxesLabels());
	ui->m_graph->setBackgroundColor(backgroundColor());
	ui->m_graph->setSelectionColor(selectionColor());
	ui->m_graph->setTitle(ui->title->text());
	ui->m_graph->setXAxisLabel(ui->xlabel->text());
	ui->m_graph->setYAxisLabel(ui->ylabel->text());
	ui->m_graph->repaint();
}

void GraphOptions::Update()
{
	ui->setLineSmoothing(ui->m_graph->lineSmoothing());
	ui->setDrawGrid(ui->m_graph->drawGrid());
	ui->setDrawLegend(ui->m_graph->showLegend());
	ui->setDrawTitle(ui->m_graph->drawTitle());
	ui->setDrawAxes(ui->m_graph->drawAxesLabels());
	ui->setBGColor(ui->m_graph->backgroundColor());
	ui->setSelectionColor(ui->m_graph->selectionColor());
	ui->setTitle(ui->m_graph->title());
	ui->setXLabel(ui->m_graph->XAxisLabel());
	ui->setYLabel(ui->m_graph->YAxisLabel());
}

bool GraphOptions::lineSmoothing()
{
	return ui->smoothLines->isChecked();
}

bool GraphOptions::drawGrid()
{
	return ui->drawGrid->isChecked();
}

bool GraphOptions::drawTitle()
{
	return ui->drawTitle->isChecked();
}

bool GraphOptions::drawAxesLabels()
{
	return ui->drawAxes->isChecked();
}

bool GraphOptions::drawLegend()
{
	return ui->drawLegend->isChecked();
}

QColor GraphOptions::backgroundColor()
{
	return ui->bgcol->color();
}

QColor GraphOptions::selectionColor()
{
	return ui->selcol->color();
}

//=================================================================================================

RegressionUi::RegressionUi(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent), m_graph(graph)
{
	m_src = new QComboBox;

	m_fnc = new QComboBox;
	m_fnc->addItem("Linear");
	m_fnc->addItem("Quadratic");
	m_fnc->addItem("Exponential");

	m_col.setRgb(0,0,0);

	CColorButton* cb = new CColorButton;
	cb->setColor(m_col);
	cb->setMaximumWidth(20);

	QHBoxLayout* h = new QHBoxLayout;
	h->setContentsMargins(0,0,0,0);
	h->addWidget(m_fnc);
	h->addWidget(cb);

	QFormLayout* f = new QFormLayout;
	f->setLabelAlignment(Qt::AlignRight);
	f->setContentsMargins(0,0,0,0);
	f->addRow("Source", m_src);
	f->addRow("Function", h);
	f->addRow("", m_math = new QLabel(""));
	f->addRow(m_lbl[0] = new QLabel("a"), m_par[0] = new QLineEdit); m_lbl[0]->setBuddy(m_par[0]);
	m_par[0]->setValidator(new QDoubleValidator);
	m_par[0]->setReadOnly(true);
	f->addRow(m_lbl[1] = new QLabel("b"), m_par[1] = new QLineEdit); m_lbl[1]->setBuddy(m_par[1]);
	m_par[1]->setValidator(new QDoubleValidator);
	m_par[1]->setReadOnly(true);
	f->addRow(m_lbl[2] = new QLabel("c"), m_par[2] = new QLineEdit); m_lbl[2]->setBuddy(m_par[2]);
	m_par[2]->setValidator(new QDoubleValidator);
	m_par[2]->setReadOnly(true);

	QVBoxLayout* v = new QVBoxLayout;
	v->addLayout(f);

	QPushButton* pb = new QPushButton("Calculate");
	v->addWidget(pb);

	v->addStretch();
	setLayout(v);

	QObject::connect(pb, SIGNAL(clicked()), this, SLOT(onCalculate()));
	QObject::connect(m_fnc, SIGNAL(currentIndexChanged(int)), this, SLOT(onFunctionChanged(int)));
	QObject::connect(cb, SIGNAL(colorChanged(QColor)), this, SLOT(onColorChanged(QColor)));

	onFunctionChanged(0);

	Update();
}

void RegressionUi::showParameters(int numParam)
{
	if (numParam == 2) { m_lbl[2]->hide(); m_par[2]->hide(); }
	else { m_lbl[2]->show(); m_par[2]->show(); }
}

void RegressionUi::clearParameters()
{
	m_bvalid = false;
	m_par[0]->clear();
	m_par[1]->clear();
	m_par[2]->clear();
}

void RegressionUi::onFunctionChanged(int n)
{
	switch (n)
	{
	case 0: showParameters(2); m_math->setText("<p>y = <b>a</b>*x + <b>b</b></p>"); break;
	case 1: showParameters(3); m_math->setText("<p>y = <b>a</b>*x^2 + <b>b</b>*x + <b>c</b></p>"); break;
	case 2: showParameters(2); m_math->setText("<p>y = <b>a</b>*exp(<b>b</b>*x)</p>"); break;
	}

	clearParameters();
	m_graph->repaint();
}

void RegressionUi::onColorChanged(QColor c)
{
	m_col = c;
	m_graph->repaint();
}

void RegressionUi::onCalculate()
{
	clearParameters();

	if (m_graph == 0) return;

	// get the first data field
	int plots = m_graph->plots();
	if (plots == 0) return;

	int n = m_src->currentIndex();
	if ((n < 0) || (n > plots)) return;

	vector<pair<double, double> > pt;
	if (n < plots)
	{
		CPlotData& data = m_graph->getPlotData(n);
		int N = data.size();
		pt.resize(N);
		for (int i = 0; i < N; ++i)
		{
			QPointF p = data.Point(i);
			pt[i].first = p.x();
			pt[i].second = p.y();
		}
	}
	else
	{
		// if (n == plots), then use the selection
		std::vector<QPointF> sel = m_graph->SelectedPoints();
		for (QPointF& p : sel)
		{
			pt.push_back(pair<double, double>(p.x(), p.y()));
		}
	}
	if (pt.empty()) return;

	int nfc = m_fnc->currentIndex();
	if (nfc == 0)
	{
		pair<double, double> ans;
		if (LinearRegression(pt, ans))
		{
			m_a = ans.first;
			m_b = ans.second;
			m_par[0]->setText(QString::number(m_a));
			m_par[1]->setText(QString::number(m_b));
			m_bvalid = true;
			m_graph->repaint();
		}
	}
	else if (nfc == 1)
	{
		vector<double> ans(3, 0.0);
		if (NonlinearRegression(pt, ans, 1))
		{
			m_a = ans[0];
			m_b = ans[1];
			m_c = ans[2];
			m_par[0]->setText(QString::number(m_a));
			m_par[1]->setText(QString::number(m_b));
			m_par[2]->setText(QString::number(m_c));
			m_bvalid = true;
			m_graph->repaint();
		}
	}
	else if (nfc == 2)
	{
		// do a linear regression on the log
		for (int i=0; i<pt.size(); ++i)
		{
			pair<double, double>& pi = pt[i];
			if (pi.second > 0) pi.second = log(pi.second);
			else return;
		}
		pair<double, double> ans;
		if (LinearRegression(pt, ans))
		{
			m_a = exp(ans.second);
			m_b = ans.first;
			m_par[0]->setText(QString::number(m_a));
			m_par[1]->setText(QString::number(m_b));
			m_bvalid = true;
			m_graph->repaint();
		}
	}
}

void RegressionUi::draw(QPainter& p)
{
	if (m_bvalid == false) return;

	p.setPen(QPen(m_col, 2));

	QRectF vr = m_graph->m_viewRect;
	QRect sr = m_graph->ScreenRect();

	int func = m_fnc->currentIndex();

	QPointF p0, p1;
	int ierr = 0;
	for (int i = sr.left(); i < sr.right(); i += 2)
	{
		double x = vr.left() + (i - sr.left())*(vr.right() - vr.left()) / (sr.right() - sr.left());

		double y = 0;
		switch (func)
		{
		case 0: y = m_a*x + m_b; break;
		case 1: y = m_a*x*x + m_b*x + m_c; break;
		case 2: y = m_a*exp(m_b*x); break;
		}

		p1 = m_graph->ViewToScreen(QPointF(x, y));

		if (i != sr.left())
		{
			p.drawLine(p0, p1);
		}

		p0 = p1;
	}
}

void RegressionUi::Update()
{
	int nplots = m_graph->plots();
	m_src->clear();
	for (int i = 0; i < nplots; ++i)
	{
		QString l = m_graph->getPlotData(i).label();
		if (l.isEmpty())
		{
			if (nplots == 1)
				l = QString("<all>");
			else
				l = QString("<data%1>").arg(i+1);
		}
		m_src->addItem(l);
	}
	m_src->addItem("<selection>");
	clearParameters();
}

void RegressionUi::hideEvent(QHideEvent* ev)
{
	if (m_bvalid)
	{
		m_bvalid = false;
		m_graph->repaint();
	}
}

//=================================================================================================
MathPlot::MathPlot(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent), m_graph(graph)
{
	m_bvalid = false;

	m_col.setRgb(0, 0, 0);

	CColorButton* cb = new CColorButton;
	cb->setColor(m_col);
	cb->setMaximumWidth(20);

	m_edit = new QLineEdit;

	QHBoxLayout* h = new QHBoxLayout;
	h->setContentsMargins(0,0,0,0);
	h->addWidget(new QLabel("y(x) = "));
	h->addWidget(m_edit);
	h->addWidget(cb);

	QVBoxLayout* l = new QVBoxLayout;
	l->addLayout(h);

	QPushButton* b = new QPushButton("Plot");
	l->addWidget(b);
	l->addStretch();
	setLayout(l);

	QObject::connect(b, SIGNAL(clicked()), this, SLOT(onCalculate()));
	QObject::connect(cb, SIGNAL(colorChanged(QColor)), this, SLOT(onColorChanged(QColor)));
}

void MathPlot::onColorChanged(QColor c)
{
	m_col = c;
	m_graph->repaint();
}

void MathPlot::onCalculate()
{
	Update();

	QString t = m_edit->text();
	if (t.isEmpty() == false)
	{
		m_math = t.toStdString();
		m_bvalid = true;
		if (m_bvalid) m_graph->repaint();
	}
}

MathPlot* MathPlot::m_pThis = nullptr;

double MathPlot::graphdata(double x)
{ 
	if (m_pThis == nullptr) return 0.0;
	LoadCurve& lc = m_pThis->m_data;

	if (lc.Points() == 0) return 0.0;

	return lc.value(x); 
}

void MathPlot::draw(QPainter& p)
{
	if (m_bvalid == false) return;
	m_pThis = this;

	p.setPen(QPen(m_col, 2));

	MObjBuilder::Add1DFunction("_data", graphdata);
	m_data.Clear();
	if (m_graph->plots() == 1)
	{
		m_data.SetInterpolator(PointCurve::SMOOTH);
		m_data.SetExtendMode(PointCurve::CONSTANT);
		CPlotData& plt = m_pThis->m_graph->getPlotData(0);
		for (int i = 0; i < plt.size(); ++i)
		{
			QPointF& pi = plt.Point(i);
			m_data.Add(pi.x(), pi.y());
		}
	}

	MSimpleExpression m;
	MVariable* xvar = m.AddVariable("x");
	m.Create(m_math);

	QRectF vr = m_graph->m_viewRect;
	QRect sr = m_graph->ScreenRect();

	QPointF p0, p1;
	int ierr = 0;
	for (int i=sr.left(); i < sr.right(); i += 2)
	{
		double x = vr.left() + (i - sr.left())*(vr.right() - vr.left())/ (sr.right() - sr.left());

		xvar->value(x);

		double y = m.value();
		
		p1 = m_graph->ViewToScreen(QPointF(x,y));

		if (i != sr.left())
		{
			p.drawLine(p0, p1);
		}

		p0 = p1;
	}
}

void MathPlot::Update()
{
	m_bvalid = false;
}

void MathPlot::hideEvent(QHideEvent* ev)
{
	if (m_bvalid)
	{
		m_bvalid = false;
		m_graph->repaint();
	}
}

//=============================================================================

class DataOptionsUI
{
public:
	QComboBox*		m_data;
	QStackedWidget*	m_stack;
	CGraphWidget*	m_graph;

	QLineEdit*		m_label;
	CColorButton*	m_lineCol;
	QSpinBox*		m_lineWidth;
	QSpinBox*		m_markerSize;
	QComboBox*		m_markerType;

public:
	void setup(DataOptions* w)
	{
		m_data = new QComboBox;
		m_stack = new QStackedWidget;
		QLabel* dummy = new QLabel;
		m_stack->addWidget(dummy);

		QStringList markersTypes;
		markersTypes << "(None)" << "Square" << "Circle" << "Diamond" << "Triangle" << "Cross" << "Plus";

		QFormLayout* l = new QFormLayout;
		l->addRow("label", m_label = new QLineEdit);
		l->addRow("color", m_lineCol = new CColorButton);
		l->addRow("line width", m_lineWidth = new QSpinBox); m_lineWidth->setRange(1, 15);
		l->addRow("marker type", m_markerType = new QComboBox); m_markerType->addItems(markersTypes);
		l->addRow("marker size", m_markerSize = new QSpinBox); m_markerSize->setRange(1, 15);

		QWidget* ops = new QWidget;
		ops->setLayout(l);
		m_stack->addWidget(ops);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m_data);
		mainLayout->addWidget(m_stack);
		w->setLayout(mainLayout);

		QObject::connect(m_data, SIGNAL(currentIndexChanged(int)), w, SLOT(onIndexChange(int)));
		QObject::connect(m_label, SIGNAL(editingFinished()), w, SLOT(onLabelChanged()));

		QSignalMapper* mapper = new QSignalMapper;

		QObject::connect(m_lineCol, SIGNAL(colorChanged(QColor)), mapper, SLOT(map())); mapper->setMapping(m_lineCol, 0);
		QObject::connect(m_lineWidth, SIGNAL(valueChanged(int)), mapper, SLOT(map())); mapper->setMapping(m_lineWidth, 1);
		QObject::connect(m_markerType, SIGNAL(currentIndexChanged(int)), mapper, SLOT(map())); mapper->setMapping(m_markerType, 2);
		QObject::connect(m_markerSize, SIGNAL(valueChanged(int)), mapper, SLOT(map())); mapper->setMapping(m_markerSize, 3);

		QObject::connect(mapper, SIGNAL(mappedInt(int)), w, SLOT(onDataChange(int)));
	}
};

DataOptions::DataOptions(CGraphWidget* graph, QWidget* parent) : CPlotTool(parent), ui(new DataOptionsUI)
{
	ui->m_graph = graph;
	ui->setup(this);
}

void DataOptions::onLabelChanged()
{
	QString t = ui->m_label->text();
	int n = ui->m_data->currentIndex();
	ui->m_data->setItemText(n, t);
	ui->m_graph->getPlotData(n).setLabel(t);
	ui->m_graph->repaint();
}

void DataOptions::onIndexChange(int n)
{
	if (n < 0) ui->m_stack->setCurrentIndex(0);
	else
	{
		blockSignals(true);
		CPlotData& p = ui->m_graph->getPlotData(n);
		ui->m_label->setText(p.label());
		ui->m_lineCol->setColor(p.lineColor());
		ui->m_lineWidth->setValue(p.lineWidth());
		ui->m_markerType->setCurrentIndex(p.markerType());
		ui->m_markerSize->setValue(p.markerSize());
		ui->m_stack->setCurrentIndex(1);
		blockSignals(false);
	}
}

void DataOptions::onDataChange(int nprop)
{
	int n = ui->m_data->currentIndex();
	if (n < 0) return;

	// NOTE: make sure the order here corresponds to the values defined in the signal mapper above.
	CPlotData& d = ui->m_graph->getPlotData(n);
	switch (nprop)
	{
	case 0: d.setLineColor(ui->m_lineCol->color()); d.setFillColor(ui->m_lineCol->color()); break;
	case 1: d.setLineWidth(ui->m_lineWidth->value()); break;
	case 2: d.setMarkerType(ui->m_markerType->currentIndex()); break;
	case 3: d.setMarkerSize(ui->m_markerSize->value()); break;
	}

	ui->m_graph->repaint();
}

void DataOptions::Update()
{
	ui->m_data->clear();

	int n = ui->m_graph->plots();
	for (int i = 0; i < n; ++i)
	{
		CPlotData& di = ui->m_graph->getPlotData(i);
		QString l = di.label();
		if (l.isEmpty())
		{
			if (n == 1) l = QString("<data>");
			else l = QString("<data%1>").arg(i + 1);
		}
		ui->m_data->addItem(l);
	}
}


//=============================================================================
void CGraphWidget::paintEvent(QPaintEvent* pe)
{
	CPlotWidget::paintEvent(pe);

	QPainter p(this);
	p.setClipRect(m_plotRect);
	p.setRenderHint(QPainter::Antialiasing, true);
	for (size_t i = 0; i<m_tools.size(); ++i)
	{
		CPlotTool* tool = m_tools[i];
		tool->draw(p);
	}
}

void CGraphWidget::Update()
{
	for (size_t i = 0; i<m_tools.size(); ++i)
		m_tools[i]->Update();
	repaint();
}

//=============================================================================
class Ui::CGraphWindow
{
public:
	CGraphWidget*			plot;			// central graph widget
	QToolBar*				toolBar;		// top tool bar
	QToolBar*				zoomBar;		// bottom tool bar
	QComboBox*				selectPlot;		// choose the plot type
	QComboBox*				dataSource;		// source of data
	CDataSelectorButton*	selectX;		// select the X data field
	CDataSelectorButton*	selectY;		// select the Y data field
	QToolBox*				tools;			// the tools panel

	QVBoxLayout* layout;

	QAction* actionSave;
	QAction* actionAddToModel;
	QAction* actionCopy;
	QAction* actionPaste;
	QAction* actionSnapshot;
	QAction* actionProps;
	QAction* actionZoomSelect;
	QAction* actionZoomUser;

	QAction* actionSelectX;
	QAction* actionSelectY;

	TimeRangeOptions*	range;
	GraphOptions*		ops;
	DataOptions*		data;

	CPostDocument*	doc;

	QAction* actionType;
	QAction* actionPlot;
	QAction* actionSource;
	QAction* actionShowTools;

public:
	void setupUi(::CGraphWindow* parent, int flags)
	{
		QSplitter* centralWidget = new QSplitter;

		plot = new CGraphWidget(parent);
		plot->setObjectName("plot");

		centralWidget->addWidget(plot);
		centralWidget->addWidget(tools = new QToolBox);
		centralWidget->setStretchFactor(0, 4);
		tools->hide();

		range = nullptr;
		if (flags & ::CGraphWindow::SHOW_TIME_RANGE)
		{
			range = new TimeRangeOptions(plot); range->setObjectName("range");
			tools->addItem(range, "Time Range");
			plot->addTool(range);
		}

		ops = new GraphOptions(plot); ops->setObjectName("options");
		tools->addItem(ops, "Graph options");
		plot->addTool(ops);

		data = new DataOptions(plot); data->setObjectName("data");
		tools->addItem(data, "Data options");
		plot->addTool(data);

		CPlotTool* tool = new RegressionUi(plot);
		tools->addItem(tool, "Curve fitting");
		plot->addTool(tool);

		tool = new MathPlot(plot);
		tools->addItem(tool, "Math plot");
		plot->addTool(tool);

		// create plot selection combobox
		selectPlot = new QComboBox;
		selectPlot->setObjectName("selectPlot");
		selectPlot->addItem("Line");
		selectPlot->addItem("Scatter");
		selectPlot->addItem("Time-Scatter");

		// data source
		QWidget* sourceWidget = new QWidget;
		QHBoxLayout* sourceWidgetLayout = new QHBoxLayout;
		dataSource = new QComboBox;
		dataSource->setObjectName("dataSource");
		dataSource->addItem("selection");
		sourceWidgetLayout->addWidget(new QLabel("Source:"));
		sourceWidgetLayout->addWidget(dataSource);
		sourceWidget->setLayout(sourceWidgetLayout);

		// create X data selection box
		selectX = new CDataFieldSelector;
		selectX->setObjectName("selectX");
		selectX->setMinimumWidth(150);

		QWidget* x = new QWidget;
		QHBoxLayout* hx = new QHBoxLayout;
		hx->setContentsMargins(0,0,0,0);
		hx->addWidget(new QLabel(" X: "));
		hx->addWidget(selectX);
		x->setLayout(hx);

		// create Y data selection box
		selectY = new CDataFieldSelector;
		selectY->setObjectName("selectY");
		selectY->setMinimumWidth(150);

		QWidget* y = new QWidget;
		QHBoxLayout* hy = new QHBoxLayout;
		hy->setContentsMargins(0,0,0,0);
		hy->addWidget(new QLabel(" Y: "));
		hy->addWidget(selectY);
		y->setLayout(hy);

		toolBar = new QToolBar(parent);
		actionSave = toolBar->addAction(QIcon(QString(":/icons/save.png")), "Save to file"); actionSave->setObjectName("actionSave");
		actionCopy = toolBar->addAction(QIcon(QString(":/icons/clipboard.png")), "Copy to clipboard"); actionCopy->setObjectName("actionCopy");
		actionPaste= toolBar->addAction(QIcon(QString(":/icons/paste.png")), "Paste from clipboard"); actionPaste->setObjectName("actionPaste");
		actionSnapshot = toolBar->addAction(QIcon(QString(":/icons/bgimage.png")), "Save picture"); actionSnapshot->setObjectName("actionSnapshot");
		actionAddToModel = toolBar->addAction(QIcon(":/icons/addtomodel.png"), "Add to model tree"); actionAddToModel->setObjectName("actionAddToModel");

		actionSource = toolBar->addWidget(sourceWidget);
		actionType = toolBar->addWidget(new QLabel("Type: "));
		actionPlot = toolBar->addWidget(selectPlot);
		actionSelectX = toolBar->addWidget(x);
		actionSelectY = toolBar->addWidget(y);
		QPushButton* showTools = new QPushButton("Tools");
		showTools->setCheckable(true);
		showTools->setChecked(false);
		actionShowTools = toolBar->addWidget(showTools);

		zoomBar = new QToolBar(parent);
		QAction* actionZoomWidth  = zoomBar->addAction(QIcon(QString(":/icons/zoom_x.png" )), "Zoom Width" ); actionZoomWidth->setObjectName("actionZoomWidth" );
		QAction* actionZoomHeight = zoomBar->addAction(QIcon(QString(":/icons/zoom_y.png")), "Zoom Height"); actionZoomHeight->setObjectName("actionZoomHeight");
		QAction* actionZoomFit    = zoomBar->addAction(QIcon(QString(":/icons/zoom_fit.png"   )), "Zoom Fit"   ); actionZoomFit->setObjectName("actionZoomFit"   );
		actionZoomSelect = zoomBar->addAction(QIcon(QString(":/icons/zoom_select.png")), "Zoom Select"); actionZoomSelect->setObjectName("actionZoomSelect"); actionZoomSelect->setCheckable(true);
		actionZoomUser   = zoomBar->addAction(QIcon(QString(":/icons/zoom-fit-best-2.png")), "Map to Rectangle"); actionZoomUser->setObjectName("actionZoomUser");  actionZoomUser->setCheckable(true);
		zoomBar->addSeparator();
		actionProps = zoomBar->addAction(QIcon(QString(":/icons/properties.png")), "Properties"); actionProps->setObjectName("actionProps");

		QWidget* mainWidget = new QWidget;
		layout = new QVBoxLayout;

		layout->addWidget(toolBar);
		layout->addWidget(centralWidget);
		layout->addWidget(zoomBar);

		mainWidget->setLayout(layout);

		parent->setCentralWidget(mainWidget);

		QObject::connect(showTools, SIGNAL(clicked(bool)), tools, SLOT(setVisible(bool)));

		QMetaObject::connectSlotsByName(parent);
	}
};

QRect CGraphWindow::m_preferredSize;

CGraphWindow::CGraphWindow(CMainWindow* pwnd, CPostDocument* postDoc, int flags) : m_wnd(pwnd), QMainWindow(pwnd), ui(new Ui::CGraphWindow), CDocObserver(pwnd->GetDocument())
{
	static int n = 0;
	setWindowTitle(QString("Graph%1").arg(++n));

	m_nTrackTime = TRACK_TIME;
	m_nUserMin = 1;
	m_nUserMax = -1;
	m_nUserStep = 1;

	// delete the window when it's closed
	setAttribute(Qt::WA_DeleteOnClose);

	ui->setupUi(this, flags);
	ui->doc = postDoc;

	EnablePasteButton(false);

	if ((flags & SHOW_TYPE_OPTIONS) == 0)
	{
		ui->actionType->setVisible(false);
		ui->actionPlot->setVisible(false);
	}

	if ((flags & SHOW_DATA_SOURCE) == 0)
	{
		ui->actionSource->setVisible(false);
	}

	// hide the selectors by default
	ui->actionSelectX->setVisible(false);
	ui->actionSelectY->setVisible(false);

	if (ui->range)
	{
		ui->range->setUserTimeRange(m_nUserMin, m_nUserMax, m_nUserStep);
	}
	setMinimumWidth(500);

	if (m_preferredSize.isValid())
	{
		setGeometry(m_preferredSize);
	}
	else resize(800, 600);
}

//-----------------------------------------------------------------------------
void CGraphWindow::closeEvent(QCloseEvent* closeEvent)
{
	m_wnd->RemoveGraph(this);
    m_preferredSize = geometry();
}

//-----------------------------------------------------------------------------
void CGraphWindow::resizeEvent(QResizeEvent* resizeEvent)
{
    QMainWindow::resizeEvent(resizeEvent);
    m_preferredSize = geometry();
}

//-----------------------------------------------------------------------------
void CGraphWindow::moveEvent(QMoveEvent* moveEvent)
{
    QMainWindow::moveEvent(moveEvent);
    m_preferredSize = geometry();
}

void CGraphWindow::EnablePasteButton(bool b)
{
	ui->actionPaste->setEnabled(b);
}

//-----------------------------------------------------------------------------
QRect CGraphWindow::preferredSize()
{
	return m_preferredSize;
}

//-----------------------------------------------------------------------------
void CGraphWindow::setPreferredSize(const QRect& rt)
{
	m_preferredSize = rt;
}

//-----------------------------------------------------------------------------
CPlotWidget* CGraphWindow::GetPlotWidget()
{
	return ui->plot;
}

//-----------------------------------------------------------------------------
// get the post doc
CPostDocument* CGraphWindow::GetPostDoc()
{
	return ui->doc;
}

//-----------------------------------------------------------------------------
int CGraphWindow::GetTimeTrackOption()
{
	return m_nTrackTime;
}

//-----------------------------------------------------------------------------
void CGraphWindow::GetUserTimeRange(int& minTime, int& maxTime, int& step)
{
	minTime = m_nUserMin - 1;
	maxTime = (m_nUserMax != -1 ? m_nUserMax - 1 : -1);
	step = m_nUserStep;
}

//-----------------------------------------------------------------------------
void CGraphWindow::GetTimeRange(int& minTime, int& maxTime, int& timeStep)
{
	CPostDocument* doc = GetPostDoc();
	if (doc == nullptr) return;

	// get the document and current time point and time steps
	int ntime = doc->GetActiveState();
	int nsteps = doc->GetStates();

	// Figure out the time range
	int nmin = 0, nmax = 0, nstep = 1;
	int trackTime = GetTimeTrackOption();
	if (trackTime == TRACK_USER_RANGE)
	{
		// get the user defined range
		GetUserTimeRange(nmin, nmax, nstep);
	}
	else if (trackTime == TRACK_TIME)
	{
		TIMESETTINGS& timeSettings = doc->GetTimeSettings();
		nmin = timeSettings.m_start;
		nmax = timeSettings.m_end;
	}
	else if (trackTime == TRACK_CURRENT_TIME)
	{
		// simply set the min and max to the same value
		nmin = nmax = ntime;
	}

	// validate range
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;

	minTime = nmin;
	maxTime = nmax;
	timeStep = nstep;
}

//-----------------------------------------------------------------------------
// number of data plots
int CGraphWindow::Plots() { return ui->plot->plots(); }

//-----------------------------------------------------------------------------
// get the plot data
CPlotData* CGraphWindow::GetPlotData(int i) { return &ui->plot->getPlotData(i); }

//-----------------------------------------------------------------------------
void CGraphWindow::ClearPlots()
{
	ui->plot->clear();
}

//-----------------------------------------------------------------------------
void CGraphWindow::ResizePlots(int n)
{
	ui->plot->Resize(n);
}

//-----------------------------------------------------------------------------
// clear data on plots
void CGraphWindow::ClearPlotsData()
{
	ui->plot->clearData();
}

//-----------------------------------------------------------------------------
void CGraphWindow::AddPlotData(CPlotData* plot)
{
	ui->plot->addPlotData(plot);
}

//-----------------------------------------------------------------------------
void CGraphWindow::UpdatePlots()
{
	ui->plot->Update();
}

//-----------------------------------------------------------------------------
// redraw the plot widget
void CGraphWindow::RedrawPlot()
{
	ui->plot->repaint();
}

//-----------------------------------------------------------------------------
void CGraphWindow::FitPlotsToData()
{
	ui->plot->fitToData();
}

//-----------------------------------------------------------------------------
void CGraphWindow::SetPlotTitle(const QString& title)
{
	ui->plot->setTitle(title);
}

//-----------------------------------------------------------------------------
// set the axis labels
void CGraphWindow::SetXAxisLabel(const QString& label) { ui->plot->setXAxisLabel(label); }
void CGraphWindow::SetYAxisLabel(const QString& label) { ui->plot->setYAxisLabel(label); }

QString CGraphWindow::XAxisLabel() { return ui->plot->XAxisLabel(); }
QString CGraphWindow::YAxisLabel() { return ui->plot->YAxisLabel(); }

//-----------------------------------------------------------------------------
void CGraphWindow::SetXDataSelector(CDataSelector* sel, int nval)
{
	ui->selectX->SetDataSelector(sel);
	if (sel)
	{
		ui->actionSelectX->setVisible(true);
		if (nval != -1) ui->selectX->setCurrentValue(nval);
	}
	else ui->actionSelectX->setVisible(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::SetYDataSelector(CDataSelector* sel, int nval)
{
	ui->selectY->SetDataSelector(sel);
	if (sel)
	{
		ui->actionSelectY->setVisible(true);
		if (nval != -1) ui->selectY->setCurrentValue(nval);
	}
	else ui->actionSelectY->setVisible(false);
}

//-----------------------------------------------------------------------------
// get the text of the current selection in the X data selector
QString CGraphWindow::GetCurrentXText()
{
	return ui->selectX->text();
}

//-----------------------------------------------------------------------------
// get the data value associated with the current X selection
int CGraphWindow::GetCurrentXValue()
{
	return ui->selectX->currentValue();
}

//-----------------------------------------------------------------------------
// get the text of the current selection in the Y data selector
QString CGraphWindow::GetCurrentYText()
{
	return ui->selectY->text();
}

//-----------------------------------------------------------------------------
// get the data value associated with the current Y selection
int CGraphWindow::GetCurrentYValue()
{
	return ui->selectY->currentValue();
}

//-----------------------------------------------------------------------------
// get the current plot type
int CGraphWindow::GetCurrentPlotType()
{
	return ui->selectPlot->currentIndex();
}

//-----------------------------------------------------------------------------
// set source options
void CGraphWindow::SetDataSource(const QStringList& names)
{
	ui->dataSource->blockSignals(true);
	ui->dataSource->clear();
	ui->dataSource->addItems(names);
	ui->dataSource->blockSignals(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::AddToolBarWidget(QWidget* w)
{
	ui->toolBar->insertWidget(ui->actionShowTools, w);
}

//-----------------------------------------------------------------------------
void CGraphWindow::AddPanel(QWidget* w)
{
	if (w) ui->layout->addWidget(w);
}

//-----------------------------------------------------------------------------
void CGraphWindow::ShowAddToModelButton(bool b)
{
	ui->actionAddToModel->setVisible(b);
}

//-----------------------------------------------------------------------------
void CGraphWindow::DocumentDelete()
{
	CDocObserver::DocumentDelete();
	close();
}

//-----------------------------------------------------------------------------
void CGraphWindow::DocumentUpdate(bool newDoc)
{
	Update(newDoc, true);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_selectX_currentValueChanged(int)
{
	Update(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_selectY_currentValueChanged(int)
{
	Update(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_selectPlot_currentIndexChanged(int index)
{
	int n = ui->dataSource->currentIndex();
	setDataSource(n);
	Update(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionSave_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Save Graph Data", QString(), QString("All files (*)"));
	if (fileName.isEmpty() == false)
	{
		if (ui->plot->Save(fileName) == false)
			QMessageBox::critical(this, "Save Graph Data", "A problem occurred saving the data.");
	}
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionAddToModel_triggered()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetDocument());
	if (doc == nullptr) return;

	const CGraphData& data = GetPlotWidget()->GetGraphData();
	doc->AddGraph(data);
	m_wnd->Update(0, true);
}

void CGraphWindow::on_actionCopy_triggered()
{
	ui->plot->OnCopyToClipboard();
}

void CGraphWindow::on_actionPaste_triggered()
{
	PasteClipboardData();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionSnapshot_triggered()
{
	CPlotWidget* plot = this->GetPlotWidget();
	QImage pixmap(plot->size(), QImage::Format_ARGB32);
	plot->render(&pixmap);
	m_wnd->SaveImage(pixmap);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionProps_triggered()
{
	ui->plot->OnShowProps();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionZoomWidth_triggered()
{
	ui->plot->OnZoomToWidth();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionZoomHeight_triggered()
{
	ui->plot->OnZoomToHeight();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_actionZoomFit_triggered()
{
	ui->plot->OnZoomToFit();
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_plot_regionSelected(QRect rt)
{
	if (ui->actionZoomSelect->isChecked())
	{
		ui->plot->fitToRect(rt);
	}
	else if (ui->actionZoomUser->isChecked())
	{
		CDlgPlotWidgetProps dlg;
		if (dlg.exec())
		{
			ui->plot->mapToUserRect(rt, QRectF(dlg.m_xmin, dlg.m_ymin, dlg.m_xmax - dlg.m_xmin, dlg.m_ymax - dlg.m_ymin));
		}
	}
	else
	{
		ui->plot->regionSelect(rt);
	}
	ui->actionZoomSelect->setChecked(false);
	ui->actionZoomUser->setChecked(false);
}

//-----------------------------------------------------------------------------
void CGraphWindow::on_range_optionsChanged()
{
	if (ui->range == nullptr) return;

	int a = ui->range->currentOption();
	switch (a)
	{
	case 0: m_nTrackTime = TRACK_TIME; break;
	case 1: m_nTrackTime = TRACK_CURRENT_TIME; break;
	case 2: m_nTrackTime = TRACK_USER_RANGE;
	{
		ui->range->getUserTimeRange(m_nUserMin, m_nUserMax, m_nUserStep);

		CPostDocument* doc = GetPostDoc();
		if (doc)
		{
			// check the range. Note that user min and max are one-based!
			Post::FEPostModel* fem = doc->GetFSModel();
			int N = fem->GetStates();
			if (m_nUserMin < 1) m_nUserMin = 1;
			if (m_nUserMin > N) m_nUserMin = N;

			if (m_nUserMax != -1)
			{
				if (m_nUserMax < 1) m_nUserMax = 1;
				if (m_nUserMax > N) m_nUserMax = N;
				if (m_nUserMax < m_nUserMin)
				{
					int tmp = m_nUserMin;
					m_nUserMin = m_nUserMax;
					m_nUserMax = tmp;
				}
			}
		}

		ui->range->setUserTimeRange(m_nUserMin, m_nUserMax, m_nUserStep);
	}
	break;
	default:
		assert(false);
		m_nTrackTime = TRACK_TIME;
	}

	bool autoRng = ui->range->autoRangeUpdate();
	ui->plot->setAutoRangeUpdate(autoRng);

	Update(false);
}

void CGraphWindow::on_dataSource_currentIndexChanged(int n)
{
	setDataSource(n);
}

//-----------------------------------------------------------------------------
int CGraphWindow::currentDataSource()
{
	return ui->dataSource->currentIndex();
}

//=============================================================================
CDataGraphWindow::CDataGraphWindow(CMainWindow* wnd, CPostDocument* doc) : CGraphWindow(wnd, doc, 0)
{
	ShowAddToModelButton(false);
	EnablePasteButton(true);
	m_data = nullptr;
}

void CDataGraphWindow::SetData(CGraphData* data)
{
	m_data = data;
	GetPlotWidget()->SetGraphData(*data);
	UpdatePlots();
	FitPlotsToData();
}

void CDataGraphWindow::PasteClipboardData()
{
	QClipboard* clipboard = QApplication::clipboard();

	if (!clipboard->mimeData()->hasText()) return;

	QString data = clipboard->text();
	CDlgImportData dlg(data, DataType::DOUBLE, 2);
	if (dlg.exec())
	{
		QList<QStringList> values = dlg.GetValues();
		CPlotData* plt = new CPlotData;
		for (auto& row : values)
		{
			if (row.size() == 2)
				plt->addPoint(row[0].toDouble(), row[1].toDouble());
		}
		if (plt->size() > 0)
		{
			m_data->AddPlotData(plt);
			GetPlotWidget()->SetGraphData(*m_data);
			UpdatePlots();
			FitPlotsToData();
		}
		else
			delete plt;
	}
}

void CDataGraphWindow::Update(bool breset, bool bfit)
{
	UpdatePlots();
}

void CDataGraphWindow::closeEvent(QCloseEvent* ev)
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetDocument());
	if ((doc == nullptr) || (m_data == nullptr)) return;

	int n = doc->FindGraphData(m_data);
	if (n >= 0)
	{
		doc->ReplaceGraphData(n, GetPlotWidget()->GetGraphData());
	}

	CGraphWindow::closeEvent(ev);
}

//=============================================================================
CModelGraphWindow::CModelGraphWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc)
{
	m_firstState = -1;
	m_lastState = -1;
	m_incState = 1;

	m_dataX = -1;
	m_dataY = -1;
	m_dataXPrev = -1;
	m_dataYPrev = -1;

	m_xtype = m_xtypeprev = -1;

	if (postDoc)
	{
		std::string docTitle = postDoc->GetDocTitle();
		QString wndTitle = windowTitle();
		setWindowTitle(wndTitle + QString(" [%1]").arg(QString::fromStdString(docTitle)));
	}
}

//-----------------------------------------------------------------------------
// If breset==true, a new model was loaded. 
// If breset==false, the selection has changed
void CModelGraphWindow::Update(bool breset, bool bfit)
{
	CPostDocument* doc = GetPostDoc();
	if (doc==nullptr) return;

	SetXAxisLabel("");
	SetYAxisLabel("");

	if (breset)
	{
		Post::CGLModel* glm = doc->GetGLModel();
		Post::FEPostModel* fem = doc->GetFSModel();

		// update the data sources
		QStringList sourceNames;
		sourceNames << "selection";

		// global data 
		Post::FEDataManager* DM = fem->GetDataManager();
		for (int i = 0; i < DM->DataFields(); ++i)
		{
			Post::FEDataFieldPtr pdf = DM->DataField(i);
			if ((*pdf)->DataClass() == OBJECT_DATA)
			{
				sourceNames << QString::fromStdString((*pdf)->GetName());
			}
		}

		for (int i = 0; i < fem->PlotObjects(); ++i)
		{
			sourceNames << QString::fromStdString(fem->GetPlotObject(i)->GetName());
		}

		// plots
		for (int i = 0; i < glm->Plots(); ++i)
		{
			Post::GLPointProbe* p = dynamic_cast<Post::GLPointProbe*>(glm->Plot(i));
			if (p)
			{
				sourceNames << QString::fromStdString(p->GetName());
			}

			Post::GLRuler* r = dynamic_cast<Post::GLRuler*>(glm->Plot(i));
			if (r)
			{
				sourceNames << QString::fromStdString(r->GetName());
			}

			Post::GLMusclePath* mp = dynamic_cast<Post::GLMusclePath*>(glm->Plot(i));
			if (mp)
			{
				sourceNames << QString::fromStdString(mp->GetName());
			}

			Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(glm->Plot(i));
			if (pg)
			{
				QString s = QString::fromStdString(pg->GetName());
				for (int j = 0; j < pg->Plots(); ++j)
				{
					Post::GLMusclePath* pj = dynamic_cast<Post::GLMusclePath*>(pg->GetPlot(j));
					if (pj)
					{
						QString si = QString("%1.%2").arg(s).arg(QString::fromStdString(pj->GetName()));
						sourceNames << si;
					}
				}
			}
		}
		SetDataSource(sourceNames);

		int plot = GetCurrentPlotType();
		if (plot == LINE_PLOT)
			SetXDataSelector(new CTimeStepSelector(), 0);
		else
			SetXDataSelector(new CModelDataSelector(fem, Post::TENSOR_SCALAR));

		int dataField = -1;
		CPostDocument* doc = GetPostDoc();
		if (doc)
		{
			Post::CGLColorMap* map = doc->GetGLModel()->GetColorMap();
			if (map)
			{
				dataField = map->GetEvalField();
			}
		}
		SetYDataSelector(new CModelDataSelector(fem, Post::TENSOR_SCALAR), dataField);

		m_dataXPrev = -1;
		m_dataYPrev = -1;
	}

	// Currently, when the time step changes, Update is called with breset set to false.
	// Depending on the time range settings, we may or may not need to update the track view.

	// when the user sets the range, we don't have to do anything so let's return
	//	if (m_bUserRange && (breset == false)) return;

	// Get the time step range
	int nmin = 0, nmax = 0, nstep = 1;
	GetTimeRange(nmin, nmax, nstep);

	// plot type
	int nplotType = GetCurrentPlotType();

	// data selector values
	m_dataX = GetCurrentXValue();
	m_dataY = GetCurrentYValue();

	int ncx = m_dataX;
	switch (nplotType)
	{
	case LINE_PLOT: m_xtype = ncx; break;
	case SCATTER_PLOT: m_xtype = 2; break;
	case TIME_SCATTER_PLOT: m_xtype = 3; break;
	}

	// get the field data
	m_dataX = GetCurrentXValue();
	m_dataY = GetCurrentYValue();
	if ((nplotType != LINE_PLOT) && (m_dataX < 0))
	{
		ClearPlots();
		return;
	}
	if (m_dataY < 0)
	{
		ClearPlots();
		return;
	}

	// TODO: When the selection changed, breset is false, but we do need to update
	//       This if statement would prevent that, so I commented it out, but now
	//       we might be doing unneccessary updates. 
	// When a reset is not required, see if we actually need to update anything
//	if ((breset == false) && (bfit == false))
//	{
//		if ((nmin == m_firstState) && (nmax == m_lastState) && (m_dataX == m_dataXPrev) && (m_dataY == m_dataYPrev) && (m_xtype == m_xtypeprev)) return;
//	}

	int currentSource = currentDataSource();

	// set current time point index (TODO: Not sure if this is still used)
	//	pview->SetCurrentTimeIndex(ntime);

	Post::CGLModel* po = doc->GetGLModel();
	Post::FEPostModel& fem = *doc->GetFSModel();

	FSMesh& mesh = *fem.GetFEMesh(0);

	// get the title
	QString xtext = GetCurrentXText();
	QString ytext = GetCurrentYText();

	// get the units (if defined)
	if (currentSource == 0)
	{
		const char* szunits = fem.GetDataManager()->getDataUnits(m_dataX);
		if (szunits)
		{
			QString s = Units::GetUnitString(szunits);
			xtext += QString(" [%1]").arg(s);
		}
		szunits = fem.GetDataManager()->getDataUnits(m_dataY);
		if (szunits)
		{
			QString s = Units::GetUnitString(szunits);
			ytext += QString(" [%1]").arg(s);
		}
	}

	SetXAxisLabel(xtext);
	SetYAxisLabel(ytext);
	if (nplotType == LINE_PLOT)
	{
		SetPlotTitle(ytext);
	}
	else
	{
		SetPlotTitle(QString("%1 --- %2").arg(xtext).arg(ytext));
	}

	m_firstState = nmin;
	m_lastState = nmax;
	m_incState = nstep;

	// we need to update the displacement map for all time steps
	// since the strain calculations depend on it
	int nsteps = doc->GetStates();
	Post::CGLDisplacementMap* pdm = po->GetDisplacementMap();
	if (pdm)
	{
		for (int i = 0; i<nsteps; ++i) po->GetDisplacementMap()->UpdateState(i);
	}

	// clear data on plots (we don't delete the plots so that we can retain user changes)
	ClearPlotsData();
	m_pltCounter = 0;

	if (currentSource == 0)
	{
		// add selections
		addSelectedNodes();
		addSelectedEdges();
		addSelectedFaces();
		addSelectedElems();
	}
	else
	{
		// check global data
		int n = currentSource - 1;

		Post::FEDataManager* DM = fem.GetDataManager();
		for (int i = 0; i < DM->DataFields(); ++i)
		{
			Post::FEDataFieldPtr pdf = DM->DataField(i);
			if ((*pdf)->DataClass() == OBJECT_DATA)
			{
				if (n == 0)
				{
					addGlobalData(*pdf, i);
					n = -1;
					break;
				}
				else n--;
			}
		}

		if ((n >= 0) && (n < fem.PlotObjects()))
		{
			addObjectData(n);
		}
		else
		{
			n -= fem.PlotObjects();
			Post::CGLModel* glm = doc->GetGLModel();
			int m = 0;
			for (int i = 0; i < glm->Plots(); ++i)
			{
				Post::GLPointProbe* probe = dynamic_cast<Post::GLPointProbe*>(glm->Plot(i));
				if (probe)
				{
					if (m == n)
					{
						addProbeData(probe);
						break;
					}
					m++;
				}

				Post::GLRuler* ruler = dynamic_cast<Post::GLRuler*>(glm->Plot(i));
				if (ruler)
				{
					if (m == n)
					{
						addRulerData(ruler);
						break;
					}
					m++;
				}

				Post::GLMusclePath* musclePath = dynamic_cast<Post::GLMusclePath*>(glm->Plot(i));
				if (musclePath)
				{
					if (m == n)
					{
						addMusclePathData(musclePath);
						break;
					}
					m++;
				}

				Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(glm->Plot(i));
				if (pg)
				{
					bool found = false;
					for (int j = 0; j < pg->Plots(); ++j)
					{
						Post::GLMusclePath* mpj = dynamic_cast<Post::GLMusclePath*>(pg->GetPlot(j));
						if (mpj)
						{
							if (m == n)
							{
								addMusclePathData(mpj);
								found = true;
								break;
							}
							m++;
						}
					}
					if (found) break;
				}
			}
		}
	}

	ResizePlots(m_pltCounter);

	// redraw
	if ((m_dataX != m_dataXPrev) || (m_dataY != m_dataYPrev) || (m_xtype != m_xtypeprev) || bfit)
	{
		FitPlotsToData();
	}

	m_dataXPrev = m_dataX;
	m_dataYPrev = m_dataY;
	m_xtypeprev = m_xtype;

	UpdatePlots();
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::setDataSource(int n)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	if (n == 0)
	{
		Update(true, true);
	}
	else
	{
		n--;

		Post::FEDataManager* DM = fem.GetDataManager();
		for (int i = 0; i < DM->DataFields(); ++i)
		{
			Post::FEDataFieldPtr pdf = DM->DataField(i);
			if ((*pdf)->DataClass() == OBJECT_DATA)
			{
				if (n == 0)
				{
					SetYDataSelector(new CPlotGlobalDataSelector(pdf));
					n = -1;
					break;
				}
				else n--;
			}
		}

		if ((n >= 0) && (n < fem.PlotObjects()))
		{
			SetYDataSelector(new CPlotObjectDataSelector(fem.GetPlotObject(n)));

			int plotType = GetCurrentPlotType();
			if ((plotType == SCATTER_PLOT) || (plotType == TIME_SCATTER_PLOT))
			{
				SetXDataSelector(new CPlotObjectDataSelector(fem.GetPlotObject(n)));
			}
			else if (plotType == LINE_PLOT)
				SetXDataSelector(new CTimeStepSelector(), 0);

			Update(false, true);
		}
		n -= fem.PlotObjects();
		if (n >= 0)
		{
			Post::CGLModel* glm = doc->GetGLModel();
			int m = 0;
			for (int i = 0; i < glm->Plots(); ++i)
			{
				Post::GLPointProbe* probe = dynamic_cast<Post::GLPointProbe*>(glm->Plot(i));
				if (probe)
				{
					if (m == n)
					{
						if (probe->TrackModelData())
						{
							SetYDataSelector(new CModelDataSelector(&fem, Post::TENSOR_SCALAR));
						}
						else
						{
							SetYDataSelector(new CProbeDataSelector());
						}

						Update(false, true);
						break;
					}
					m++;
				}

				Post::GLRuler* ruler = dynamic_cast<Post::GLRuler*>(glm->Plot(i));
				if (ruler)
				{
					if (m == n)
					{
						SetYDataSelector(new CRulerDataSelector());
						Update(false, true);
						break;
					}
					m++;
				}


				Post::GLMusclePath* musclePath = dynamic_cast<Post::GLMusclePath*>(glm->Plot(i));
				if (musclePath)
				{
					if (m == n)
					{
						SetYDataSelector(new CMusclePathDataSelector());
						Update(false, true);
						break;
					}
					m++;
				}

				Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(glm->Plot(i));
				if (pg)
				{
					bool found = false;
					for (int j = 0; j < pg->Plots(); ++j)
					{
						Post::GLMusclePath* mpj = dynamic_cast<Post::GLMusclePath*>(pg->GetPlot(j));
						if (mpj)
						{
							if (m == n)
							{
								SetYDataSelector(new CMusclePathDataSelector());
								Update(false, true);
								found = true;
								break;
							}
							m++;
						}
					}
					if (found) break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
CPlotData* CModelGraphWindow::nextData()
{
	if (m_pltCounter >= Plots())
	{
		m_pltCounter++;
		CPlotData* data = new CPlotData();
		AddPlotData(data);
		return data;
	}
	else
	{
		return dynamic_cast<CPlotData*>(GetPlotData(m_pltCounter++));
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::TrackObjectHistory(int nobj, float* pval, int nfield)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();
	Post::FEPostModel::PlotObject* po = fem.GetPlotObject(nobj);
	int nsteps = fem.GetStates();

	for (int j = 0; j < nsteps; ++j)
	{
		Post::FEState* state = fem.GetState(j + m_firstState);
		Post::OBJECTDATA& pointData = state->GetObjectData(nobj);

		Post::ObjectData* data = pointData.data;

		// get the data ID
		int ndata = FIELD_CODE(nfield);

		// get the component
		int ncomp = FIELD_COMP(nfield);

		Post::ModelDataField* dataField = po->m_data[ndata];

		float val = 0.f;

		switch (dataField->Type())
		{
		case DATA_SCALAR:
		{
			val = data->get<float>(ndata);
		}
		break;
		case DATA_VEC3:
		{
			vec3f v = data->get<vec3f>(ndata);
			val = component(v, ncomp);
		}
		break;
		case DATA_MAT3:
		{
			mat3f v = data->get<mat3f>(ndata);
			val = component(v, ncomp);
		}
		break;
		default:
			assert(false);
		}

		pval[j] = val;
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addGlobalData(Post::ModelDataField* pdf, int n)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps, 0.f);

	for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

	for (int j = 0; j < nsteps; ++j)
	{
		Post::FEState* ps = fem.GetState(j);
		Post::FEMeshData& data = ps->m_Data[n];
		if (dynamic_cast<Post::FEGlobalData_T<float>*>(&data))
		{
			Post::FEGlobalData_T<float>& df = dynamic_cast<Post::FEGlobalData_T<float>&>(data);
			float val = df.value();
			ydata[j] = val;
		}
		else if (dynamic_cast<Post::FEGlobalArrayData*>(&data))
		{
			Post::FEGlobalArrayData& dv = dynamic_cast<Post::FEGlobalArrayData&>(data);
			float val = dv.eval(m_dataY - 1);
			ydata[j] = val;
		}
	}

	CPlotData* plot = nextData();
	plot->setLabel(QString::fromStdString(pdf->GetName()));
	for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addObjectData(int n)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();
	if ((n < 0) || (n >= fem.PlotObjects())) return;

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	Post::FEPostModel::PlotObject* po = fem.GetPlotObject(n);

	switch (m_xtype)
	{
	case 0: // time values
	{
		for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;
	}
	break;
	case 1: // step values
	{
		for (int j = 0; j < nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;
	}
	break;
	case 2: // scatter
	{
		TrackObjectHistory(n, xdata.data(), m_dataX);
	}
	break;
	case 3: // time-scatter
	{
		// TODO: implement this!
		TrackObjectHistory(n, xdata.data(), m_dataX);
	}
	break;
	}

	TrackObjectHistory(n, ydata.data(), m_dataY);

	CPlotData* plot = nextData();
	plot->setLabel(QString::fromStdString((po->GetName())));
	for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addProbeData(Post::GLPointProbe* probe)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

	for (int j = 0; j < nsteps; ++j)
	{
		double val = probe->DataValue(m_dataY, j);
		ydata[j] = (float) val;
	}

	CPlotData* plot = nextData();
	plot->setLabel(QString::fromStdString((probe->GetName())));
	for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addRulerData(Post::GLRuler* ruler)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

	for (int j = 0; j < nsteps; ++j)
	{
		double val = ruler->DataValue(m_dataY, j);
		ydata[j] = (float)val;
	}

	CPlotData* plot = nextData();
	plot->setLabel(QString::fromStdString((ruler->GetName())));
	for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addMusclePathData(Post::GLMusclePath* musclePath)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

	for (int j = 0; j < nsteps; ++j)
	{
		double val = musclePath->DataValue(m_dataY, j);
		ydata[j] = (float)val;
	}

	CPlotData* plot = nextData();
	plot->setLabel(QString::fromStdString((musclePath->GetName())));
	for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
}


//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedNodes()
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();
	FSMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected nodes
	int NN = mesh.Nodes();
	switch (m_xtype)
	{
	case 0: // time values
	{
		for (int i = 0; i<NN; i++)
		{
			FSNode& node = mesh.Node(i);
			if (node.IsSelected())
			{
				for (int j = 0; j<nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

				// evaluate y-field
				TrackNodeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				plot->setLabel(QString("N%1").arg(i + 1));
				for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			}
		}
	}
	break;
	case 1: // step values
	{
		for (int i = 0; i<NN; i++)
		{
			FSNode& node = mesh.Node(i);
			if (node.IsSelected())
			{
				for (int j = 0; j<nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;

				// evaluate y-field
				TrackNodeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				plot->setLabel(QString("N%1").arg(i + 1));
				for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			}
		}
	}
	break;
	case 2: // scatter
	{
		for (int i = 0; i<NN; i++)
		{
			FSNode& node = mesh.Node(i);
			if (node.IsSelected())
			{
				TrackNodeHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);

				// evaluate y-field
				TrackNodeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				plot->setLabel(QString("N%1").arg(i + 1));
				for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			}
		}
	}
	break;
	case 3: // time-scatter
	{
		vector<int> sel;
		for (int i = 0; i < NN; i++)
		{
			FSNode& node = mesh.Node(i);
			if (node.IsSelected()) sel.push_back(i);
		}

		if (sel.empty() == false)
		{
			int states = fem.GetStates();

			int state0 = m_firstState;
			int state1 = m_lastState;

			if (state0 < 0) state0 = 0;
			if (state0 >= states) state0 = states - 1;

			if (state1 < 0) state1 = 0;
			if (state1 >= states) state1 = states - 1;

			if (state1 < state0)
			{
				int tmp = state0;
				state0 = state1;
				state1 = tmp;
			}

			int ninc = m_incState;
			if (ninc < 1) ninc = 1;

			int nsteps = state1 - state0 + 1;
			if (nsteps / ninc > 32) nsteps = 32 * ninc;

			for (int i = state0; i < state0 + nsteps; i += ninc)
			{
				CPlotData* plot = nextData();
				plot->setLabel(QString("%1").arg(fem.GetState(i)->m_time));
			}

			for (int i = 0; i < (int)sel.size(); i++)
			{
				FSNode& node = mesh.Node(sel[i]);
				if (node.IsSelected())
				{
					// evaluate x-field
					TrackNodeHistory(sel[i], &xdata[0], m_dataX, state0, state0 + nsteps - 1);

					// evaluate y-field
					TrackNodeHistory(sel[i], &ydata[0], m_dataY, state0, state0 + nsteps - 1);

					int m = 0;
					for (int j = 0; j < nsteps; j += ninc)
					{
						CPlotData& p = GetPlotWidget()->getPlotData(m++);
						p.addPoint(xdata[j], ydata[j]);
					}
				}
			}

			// sort the plots 
			int nplots = GetPlotWidget()->plots();
			for (int i = 0; i < nplots; ++i)
			{
				CPlotData& data = GetPlotWidget()->getPlotData(i);
				data.sort();
			}
		}
	}
	break;
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedEdges()
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();
	FSMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected nodes
	int NL = mesh.Edges();
	for (int i = 0; i<NL; i++)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.IsSelected())
		{
			// evaluate x-field
			switch (m_xtype)
			{
			case 0:
				for (int j = 0; j<nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;
				break;
			case 1:
				for (int j = 0; j<nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;
				break;
			default:
				TrackEdgeHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);
			}

			// evaluate y-field
			TrackEdgeHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

			CPlotData* plot = nextData();
			plot->setLabel(QString("L%1").arg(i + 1));
			for (int j = 0; j<nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
		}
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedFaces()
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();
	FSMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected faces
	int NF = mesh.Faces();
	switch (m_xtype)
	{
	case 0:
		for (int i = 0; i < NF; ++i)
		{
			FSFace& f = mesh.Face(i);
			if (f.IsSelected())
			{
				// evaluate x-field
				for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

				// evaluate y-field
				TrackFaceHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				plot->setLabel(QString("F%1").arg(i + 1));
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			}
		}
		break;
	case 1:
		for (int i = 0; i < NF; ++i)
		{
			FSFace& f = mesh.Face(i);
			if (f.IsSelected())
			{
				for (int j = 0; j < nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;

				// evaluate y-field
				TrackFaceHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				plot->setLabel(QString("F%1").arg(i + 1));
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			}
		}
		break;
	case 2:
		for (int i = 0; i < NF; ++i)
		{
			FSFace& f = mesh.Face(i);
			if (f.IsSelected())
			{
				// evaluate x-field
				TrackFaceHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);

				// evaluate y-field
				TrackFaceHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				plot->setLabel(QString("F%1").arg(i + 1));
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
			}
		}
		break;
	case 3:	// time-scatter
	{
		vector<int> sel;
		for (int i = 0; i < NF; i++)
		{
			FSFace& face = mesh.Face(i);
			if (face.IsSelected()) sel.push_back(i);
		}

		if (sel.empty() == false)
		{
			int nsteps = m_lastState - m_firstState + 1;

			int ninc = m_incState;
			if (ninc < 1) ninc = 1;

			if (nsteps / ninc > 32) nsteps = 32*ninc;

			for (int i = m_firstState; i < m_firstState + nsteps; i += ninc)
			{
				CPlotData* plot = nextData();
				plot->setLabel(QString("%1").arg(fem.GetState(i)->m_time));
			}

			for (int i = 0; i < (int)sel.size(); i++)
			{
				FSFace& face = mesh.Face(sel[i]);

				// evaluate x-field
				TrackFaceHistory(sel[i], &xdata[0], m_dataX, m_firstState, m_lastState);

				// evaluate y-field
				TrackFaceHistory(sel[i], &ydata[0], m_dataY, m_firstState, m_lastState);

				int m = 0;
				for (int j = 0; j < nsteps; ++j)
				{
					CPlotData& p = GetPlotWidget()->getPlotData(m++);
					p.addPoint(xdata[j], ydata[j]);
				}
			}

			// sort the plots 
			CPlotWidget* w = GetPlotWidget();
			int nplots = w->plots();
			for (int i = 0; i < nplots; ++i)
			{
				CPlotData& data = GetPlotWidget()->getPlotData(i);
				data.sort();
			}

			if (w->autoRangeUpdate())
				w->fitToData(false);
		}
	}
	break;
	}
}

//-----------------------------------------------------------------------------
void CModelGraphWindow::addSelectedElems()
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();
	FSMesh& mesh = *fem.GetFEMesh(0);

	int nsteps = m_lastState - m_firstState + 1;
	vector<float> xdata(nsteps);
	vector<float> ydata(nsteps);

	// get the selected elements
	int NE = mesh.Elements();
	switch (m_xtype)
	{
	case 0:
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected())
			{
				// evaluate x-field
				for (int j = 0; j < nsteps; j++) xdata[j] = fem.GetState(j + m_firstState)->m_time;

				// evaluate y-field
				TrackElementHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				plot->setLabel(QString("E%1").arg(e.GetID()));
			}
		}
		break;
	case 1:
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected())
			{
				// evaluate x-field
				for (int j = 0; j < nsteps; j++) xdata[j] = (float)j + 1.f + m_firstState;

				// evaluate y-field
				TrackElementHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				plot->setLabel(QString("E%1").arg(e.GetID()));
			}
		}
		break;
	case 2:
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected())
			{
				// evaluate x-field
				TrackElementHistory(i, &xdata[0], m_dataX, m_firstState, m_lastState);

				// evaluate y-field
				TrackElementHistory(i, &ydata[0], m_dataY, m_firstState, m_lastState);

				CPlotData* plot = nextData();
				for (int j = 0; j < nsteps; ++j) plot->addPoint(xdata[j], ydata[j]);
				plot->setLabel(QString("E%1").arg(e.GetID()));
			}
		}
		break;
	case 3:	// time-scatter
	{
		vector<int> sel;
		for (int i = 0; i < NE; i++)
		{
			FEElement_& e = mesh.ElementRef(i);
			if (e.IsSelected()) sel.push_back(i);
		}

		if (sel.empty() == false)
		{
			int ninc = m_incState;
			if (ninc < 1) ninc = 1;

			int nsteps = m_lastState - m_firstState + 1;
			if (nsteps / ninc > 32) nsteps = 32 * ninc;

			for (int i = m_firstState; i < m_firstState + nsteps; i += ninc)
			{
				CPlotData* plot = nextData();
				plot->setLabel(QString("%1").arg(fem.GetState(i)->m_time));
			}

			for (int i = 0; i < (int)sel.size(); i++)
			{
				FEElement_& e = mesh.ElementRef(sel[i]);
				if (e.IsSelected())
				{
					// evaluate x-field
					TrackElementHistory(sel[i], &xdata[0], m_dataX, m_firstState, m_lastState);

					// evaluate y-field
					TrackElementHistory(sel[i], &ydata[0], m_dataY, m_firstState, m_lastState);

					int m = 0;
					for (int j = 0; j < nsteps; j += ninc)
					{
						CPlotData& p = GetPlotWidget()->getPlotData(m++);
						p.addPoint(xdata[j], ydata[j]);
					}
				}
			}

			// sort the plots 
			CPlotWidget* w = GetPlotWidget();
			int nplots = w->plots();
			for (int i = 0; i < nplots; ++i)
			{
				CPlotData& data = GetPlotWidget()->getPlotData(i);
				data.sort();
			}

			if (w->autoRangeUpdate())
				w->fitToData(false);
		}
	}
	break;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of a node

void CModelGraphWindow::TrackNodeHistory(int node, float* pval, int nfield, int nmin, int nmax)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	Post::NODEDATA nd;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateNode(node, n + nmin, nfield, nd);
		pval[n] = nd.m_val;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of a edge
void CModelGraphWindow::TrackEdgeHistory(int edge, float* pval, int nfield, int nmin, int nmax)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	Post::EDGEDATA nd;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateEdge(edge, n + nmin, nfield, nd);
		pval[n] = nd.m_val;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of a face
void CModelGraphWindow::TrackFaceHistory(int nface, float* pval, int nfield, int nmin, int nmax)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	float data[FSFace::MAX_NODES], val;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateFace(nface, n + nmin, nfield, data, val);
		pval[n] = val;
	}
}

//-----------------------------------------------------------------------------
// Calculate time history of an element
void CModelGraphWindow::TrackElementHistory(int nelem, float* pval, int nfield, int nmin, int nmax)
{
	CPostDocument* doc = GetPostDoc();
	Post::FEPostModel& fem = *doc->GetFSModel();

	int nsteps = fem.GetStates();
	if (nmin <       0) nmin = 0;
	if (nmax == -1) nmax = nsteps - 1;
	if (nmax >= nsteps) nmax = nsteps - 1;
	if (nmax <    nmin) nmax = nmin;
	int nn = nmax - nmin + 1;

	float data[FSElement::MAX_NODES] = { 0.f }, val;
	for (int n = 0; n<nn; n++)
	{
		fem.EvaluateElement(nelem, n + nmin, nfield, data, val);
		pval[n] = val;
	}
}
