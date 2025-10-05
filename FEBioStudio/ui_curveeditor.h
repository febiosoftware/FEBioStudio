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

#pragma once
#include "CurveEditor.h"
#include <QTreeWidget>
#include <QBoxLayout>
#include "CurveEditWidget.h"
#include "MathEditWidget.h"
#include <FSCore/LoadCurve.h>
#include <QHeaderView>
#include <QComboBox>
#include <QToolBar>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QSplitter>
#include <QPushButton>
#include <QStackedWidget>
#include "MainWindow.h"
#include "FEClassPropsView.h"

class CCurveEditorItem : public QTreeWidgetItem
{
public:
	CCurveEditorItem(QTreeWidget* tree) : QTreeWidgetItem(tree) { m_pp = 0; }
	CCurveEditorItem(QTreeWidgetItem* item) : QTreeWidgetItem(item) { m_pp = 0; }

	void SetParam(Param* pp) { m_pp = pp; }
	Param* GetParam() { return m_pp; }

private:
	Param*			m_pp;
};

class Ui::CCurveEdior
{
public:
	QComboBox*			filter;
	QTreeWidget*		tree;
	CCurveEditWidget*	plot;
	QStackedWidget*		stack;
	QWidget*			lcWidget;
	QComboBox*			selectLC;
	CMathEditWidget*	math;
	CMathEditWidget*	math2;
	FEClassPropsView*	props;

public:
	void setupUi(QMainWindow* wnd)
	{
		filter = new QComboBox;
		filter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		filter->setObjectName("filter");
		filter->addItem("All");
		filter->addItem("Geometry");
		filter->addItem("Model Data");
		filter->addItem("Materials");
		filter->addItem("Boundary Conditions");
		filter->addItem("Loads");
		filter->addItem("Contact");
		filter->addItem("Nonlinear Constraints");
		filter->addItem("Rigid Constraints");
		filter->addItem("Rigid Connectors");
		filter->addItem("Discrete Materials");
		filter->addItem("Steps");
		filter->addItem("Loadcurves");

		tree = new QTreeWidget;
		tree->header()->close();
		tree->setObjectName("tree");

		QHBoxLayout* fl = new QHBoxLayout;
		fl->addWidget(new QLabel("Filter:"));
		fl->addWidget(filter);

		plot = new CCurveEditWidget();
		plot->setObjectName("plot");

		math = new CMathEditWidget;
		math->SetOrdinate("t");
		math->setObjectName("math");

		math2 = new CMathEditWidget;
		math2->SetOrdinate("t");
		math2->setObjectName("math2");
		math2->showRangeOptions(true);
		math2->setMinMaxRange(0.0, 1.0);

		props = new FEClassPropsView;
		props->setObjectName("props");

		QVBoxLayout* treeLayout = new QVBoxLayout;
		treeLayout->addLayout(fl);
		treeLayout->addWidget(tree);
		treeLayout->setContentsMargins(2,2,0,2);

		QWidget* treeWidget = new QWidget;
		treeWidget->setLayout(treeLayout);

		QHBoxLayout* lcLayout = new QHBoxLayout;
		lcLayout->addWidget(new QLabel("Select load controller"));
		selectLC = new QComboBox; selectLC->setObjectName("selectLC");
		selectLC->setMinimumWidth(200);
		lcLayout->addWidget(selectLC);

		QPushButton* newLC = new QPushButton("New...");
		newLC->setObjectName("newLC");
		lcLayout->addWidget(newLC);
		lcLayout->addStretch();

		lcWidget = new QWidget;
		lcWidget->setLayout(lcLayout);

		stack = new QStackedWidget;
		stack->addWidget(new QLabel("(no controller)"));
		stack->addWidget(plot);
		stack->addWidget(math);
		stack->addWidget(math2);
		stack->addWidget(props);

		QVBoxLayout* plotLayout = new QVBoxLayout;
		plotLayout->setContentsMargins(0, 0, 0, 0);
		plotLayout->addWidget(lcWidget);
		plotLayout->addWidget(stack);

		QWidget* plotWidget = new QWidget;
		plotWidget->setLayout(plotLayout);

		QSplitter* splitter = new QSplitter;
		splitter->addWidget(treeWidget);
		splitter->addWidget(plotWidget);

		wnd->setCentralWidget(splitter);

		QMetaObject::connectSlotsByName(wnd);
	}

	QTreeWidgetItem* addTreeItem(QTreeWidgetItem* item, const QString& txt, Param* pp = 0)
	{
		CCurveEditorItem* child = new CCurveEditorItem(item);
		if (pp && (pp->GetLoadCurveID() > 0)) {
			QFont f = child->font(0); f.setBold(true); child->setFont(0, f);
		}
		child->SetParam(pp);
		child->setText(0, txt);
		return child;
	}

	void setCurrentLC(int lcid)
	{
		lcWidget->setEnabled(true);
		if (lcid < 0)
		{
			selectLC->setCurrentIndex(0);
		}
		else
		{
			int n = selectLC->findData(lcid); assert(n > 0);
			selectLC->setCurrentIndex(n);
		}
	}

	void deactivate()
	{
		lcWidget->setEnabled(false);
		stack->setCurrentIndex(0);
	}
};
