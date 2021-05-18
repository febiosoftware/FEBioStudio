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

#pragma once
#include "CurveEditor.h"
#include <QTreeWidget>
#include <QBoxLayout>
#include "PlotWidget.h"
#include <FSCore/LoadCurve.h>
#include <QHeaderView>
#include <QComboBox>
#include <QToolBar>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QSplitter>
#include "MainWindow.h"

class CCurvePlotWidget : public CPlotWidget
{
public:
	CCurvePlotWidget(QWidget* parent = nullptr) : CPlotWidget(parent) 
	{ 
		m_lc = nullptr; 
		setLineSmoothing(true);
	}

	void DrawPlotData(QPainter& p, CPlotData& data) override;

	void SetLoadCurve(FELoadCurve* lc) { m_lc = lc; }

private:
	FELoadCurve*	m_lc;
};

class CCurveEditorItem : public QTreeWidgetItem
{
public:
	CCurveEditorItem(QTreeWidget* tree) : QTreeWidgetItem(tree) { m_plc = 0; m_pp = 0; }
	CCurveEditorItem(QTreeWidgetItem* item) : QTreeWidgetItem(item) { m_plc = 0; m_pp = 0; }

	void SetLoadCurve(FELoadCurve* plc) { m_plc = plc; }
	FELoadCurve* GetLoadCurve() { return m_plc; }

	void SetParam(Param* pp) { m_pp = pp; }
	Param* GetParam() { return m_pp; }

private:
	Param*			m_pp;
	FELoadCurve*	m_plc;
};

class Ui::CCurveEdior
{
public:
	QComboBox*		filter;
	QTreeWidget*	tree;
	CCurvePlotWidget*	plot;
	QLineEdit*		xval;
	QLineEdit*		yval;
	QToolButton*	addPoint;
	QToolButton*	snap2grid;
	QHBoxLayout*	pltbutton;
	
	QComboBox* lineType;
	QComboBox* extendMode;

	QAction* undo;
	QAction* redo;

	QPointF	m_dragPt;
	int		m_dragIndex;
	vector<QPointF>	m_p0;	// used by point dragging

public:
	void setupUi(QMainWindow* wnd)
	{
		filter = new QComboBox;
		filter->setObjectName("filter");
		filter->addItem("All");
		filter->addItem("Geometry");
		filter->addItem("Materials");
		filter->addItem("Boundary Conditions");
		filter->addItem("Loads");
		filter->addItem("Contact");
		filter->addItem("Rigid Constraints");
		filter->addItem("Rigid Connectors");
		filter->addItem("Discrete Materials");
		filter->addItem("Steps");
		filter->addItem("Loadcurves");

		tree = new QTreeWidget;
		tree->header()->close();
		tree->setObjectName("tree");

		lineType = new QComboBox; lineType->setObjectName("lineType");
		lineType->addItem("Linear");
		lineType->addItem("Step");
		lineType->addItem("Smooth");
        lineType->addItem("Cubic spline");
        lineType->addItem("Control points");
        lineType->addItem("Approximation");

		extendMode = new QComboBox; extendMode->setObjectName("extendMode");
		extendMode->addItem("Constant");
		extendMode->addItem("Extrapolate");
		extendMode->addItem("Repeat");
		extendMode->addItem("Repeat Offset");

		QHBoxLayout* curveLayout = new QHBoxLayout;
		curveLayout->addWidget(new QLabel("Type:"));
		curveLayout->addWidget(lineType);
		curveLayout->addWidget(new QLabel("Extend:"));
		curveLayout->addWidget(extendMode);
		curveLayout->addStretch();
//		curveLayout->setSpacing(2);

		plot = new CCurvePlotWidget();
		plot->setObjectName("plot");
		plot->showLegend(false);
		plot->showToolTip(false);
		plot->scaleAxisLabels(false);
		plot->setFullScreenMode(true);
		plot->setXAxisLabelAlignment(ALIGN_LABEL_TOP);
		plot->setYAxisLabelAlignment(ALIGN_LABEL_RIGHT);
		plot->setBackgroundColor(QColor(48, 48, 48));
		plot->setGridColor(QColor(128, 128, 128));
		plot->setXAxisColor(QColor(255, 255, 255));
		plot->setYAxisColor(QColor(255, 255, 255));
		plot->setSelectionColor(QColor(255, 255, 192));

		xval = new QLineEdit; xval->setObjectName("xval");
		yval = new QLineEdit; yval->setObjectName("yval");

		addPoint = new QToolButton;
		addPoint->setAutoRaise(true);
		addPoint->setIcon(QIcon(":/icons/newpoint.png"));
		addPoint->setCheckable(true);
		addPoint->setChecked(false);
		addPoint->setToolTip("<font color=\"black\">Add a new point");

		QToolButton* delPoint = new QToolButton; delPoint->setObjectName("deletePoint");
		delPoint->setAutoRaise(true);
		delPoint->setIcon(QIcon(":/icons/deletepoint.png"));
		delPoint->setToolTip("<font color=\"black\">Delete a point");

		snap2grid = new QToolButton;
		snap2grid->setAutoRaise(true);
		snap2grid->setIcon(QIcon(":/icons/snaptogrid.png"));
		snap2grid->setCheckable(true);
		snap2grid->setChecked(false);
		snap2grid->setToolTip("<font color=\"black\">Snap to grid");

		QToolButton* zoom = new QToolButton; zoom->setObjectName("zoomToFit");
		zoom->setAutoRaise(true);
		zoom->setIcon(QIcon(":/icons/zoom_fit.png"));
		zoom->setToolTip("<font color=\"black\">Zoom to fit");

		QToolButton* zoomx = new QToolButton; zoomx->setObjectName("zoomX");
		zoomx->setAutoRaise(true);
		zoomx->setIcon(QIcon(":/icons/zoom_x.png"));
		zoomx->setToolTip("<font color=\"black\">Zoom X extents");

		QToolButton* zoomy = new QToolButton; zoomy->setObjectName("zoomY");
		zoomy->setAutoRaise(true);
		zoomy->setIcon(QIcon(":/icons/zoom_y.png"));
		zoomy->setToolTip("<font color=\"black\">Zoom Y extents");

		QToolButton* map = new QToolButton; map->setObjectName("map");
		map->setAutoRaise(true);
		map->setIcon(QIcon(":/icons/zoom-fit-best-2.png"));
		map->setToolTip("<font color=\"black\">Map to rectangle");

		QVBoxLayout* treeLayout = new QVBoxLayout;
		treeLayout->addWidget(filter);
		treeLayout->addWidget(tree);
		treeLayout->setContentsMargins(0,0,0,0);

		QWidget* treeWidget = new QWidget;
		treeWidget->setLayout(treeLayout);

		pltbutton = new QHBoxLayout;
		pltbutton->addWidget(xval);
		pltbutton->addWidget(yval);
		pltbutton->addWidget(addPoint);
		pltbutton->addWidget(delPoint);
		pltbutton->addWidget(snap2grid);
		pltbutton->addWidget(zoomx);
		pltbutton->addWidget(zoomy);
		pltbutton->addWidget(zoom);
		pltbutton->addWidget(map);
		pltbutton->addStretch();
		pltbutton->setSpacing(2);

		QVBoxLayout* plotLayout = new QVBoxLayout;
		plotLayout->addLayout(curveLayout);
		plotLayout->addWidget(plot);
		plotLayout->addLayout(pltbutton);
		plotLayout->setContentsMargins(0,0,0,0);

		QWidget* plotWidget = new QWidget;
		plotWidget->setLayout(plotLayout);

		QSplitter* splitter = new QSplitter;
		splitter->addWidget(treeWidget);
		splitter->addWidget(plotWidget);

		wnd->setCentralWidget(splitter);

		QToolBar* toolBar = new QToolBar(wnd);
		toolBar->setObjectName(QStringLiteral("toolBar"));
		wnd->addToolBar(Qt::TopToolBarArea, toolBar);

		QAction* open  = toolBar->addAction(QIcon(":/icons/open.png"), "Open"); open->setObjectName("open");
		QAction* save  = toolBar->addAction(QIcon(":/icons/save.png"), "Save"); save->setObjectName("save");
		QAction* clip  = toolBar->addAction(QIcon(":/icons/clipboard.png"), "Copy to clipboard"); clip->setObjectName("clip");
		toolBar->addSeparator();
		QAction* copy  = toolBar->addAction(QIcon(":/icons/copy.png" ), "Copy Curve" ); copy->setObjectName("copy");
		QAction* paste = toolBar->addAction(QIcon(":/icons/paste.png"), "Paste Curve"); paste->setObjectName("paste");
		QAction* del   = toolBar->addAction(QIcon(":/icons/clear.png"), "Delete Curve"); del->setObjectName("delete");
		toolBar->addSeparator();
		undo = toolBar->addAction(QIcon(":/icons/undo.png"), "Undo"); undo->setObjectName("undo");
		redo = toolBar->addAction(QIcon(":/icons/redo.png"), "Redo"); redo->setObjectName("redo");
		toolBar->addSeparator();
		QAction* math = toolBar->addAction(CResource::Icon("formula"), "Formula"); math->setObjectName("math");

		QMetaObject::connectSlotsByName(wnd);
	}

	void enablePointEdit(bool b)
	{
		if (xval->isEnabled() != b)
		{
			if (b == false) xval->setText("");
			xval->setEnabled(b);
		}

		if (yval->isEnabled() != b)
		{
			if (b == false) yval->setText("");
			yval->setEnabled(b);
		}
	}

	QTreeWidgetItem* addTreeItem(QTreeWidgetItem* item, const QString& txt, FELoadCurve* plc = 0, Param* pp = 0)
	{
		CCurveEditorItem* child = new CCurveEditorItem(item);
		child->SetLoadCurve(plc);
		child->SetParam(pp);
		child->setText(0, txt);
		return child;
	}

	void setPointValues(double x, double y)
	{
		xval->setText(QString("%1").arg(x));
		yval->setText(QString("%1").arg(y));
	}

	bool isAddPointChecked()
	{
		return addPoint->isChecked();
	}

	bool isSnapToGrid()
	{
		return snap2grid->isChecked();
	}

	QPointF getPointValue()
	{
		return QPointF(xval->text().toDouble(), yval->text().toDouble());
	}

	void setCmdNames(const QString& undotxt, const QString& redotxt)
	{
		undo->setToolTip(QString("<font color=\"black\">") + undotxt);
		redo->setToolTip(QString("<font color=\"black\">") + redotxt);
	}

	void setCurveType(int line, int extend)
	{
		lineType->setCurrentIndex(line);
		extendMode->setCurrentIndex(extend);
	}
};
