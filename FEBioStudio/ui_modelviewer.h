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
#include "ModelViewer.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QListWidget>
#include <QToolButton>
#include <QScrollArea>
#include <QComboBox>
#include <QLabel>
#include "ModelTree.h"
#include "ModelPropsPanel.h"
#include "ModelSearch.h"
#include "MainWindow.h"
#include "IconProvider.h"

class WarningLabel : public QToolButton
{
public:
	WarningLabel(QWidget* parent = nullptr) : QToolButton(parent)
	{
		setIcon(QIcon(":/icons/warning.png"));
		m_warnings = 0;
		setWarnings(0);
	}

	void setWarnings(int n)
	{
		m_warnings = n;
		setText(QString("(%1)").arg(n));
		setToolTip(QString("%1 warnings").arg(n));
		if (n == 0) hide(); else show();
	}

	void increase()
	{
		setWarnings(m_warnings + 1);
	}

private:
	int		m_warnings;
};

class Ui::CModelViewer
{
public:
	QStackedWidget*		m_stack;
	CModelSearch*		m_search;

	CModelTree*			tree;
	::CModelPropsPanel*	props;

	QComboBox*	m_filter;
	WarningLabel* m_errs;

	QToolButton* srcButton;
	QToolButton* highlightButton;

	bool	m_blockUpdate;

public:
	void setupUi(::CMainWindow* mainWnd, ::CModelViewer* wnd)
	{
		m_blockUpdate = false;

		QToolButton* selectButton = new QToolButton; 
		selectButton->setIcon(QIcon(":/icons/select.png")); 
		selectButton->setObjectName("selectButton"); 
		selectButton->setAutoRaise(true);
		selectButton->setToolTip("<font color=\"black\">Select in Graphics View");

		QToolButton* deleteButton = new QToolButton; 
		deleteButton->setIcon(CIconProvider::GetIcon("delete"));
		deleteButton->setObjectName("deleteButton"); 
		deleteButton->setAutoRaise(true);
		deleteButton->setToolTip("<font color=\"black\">Delete item");

		srcButton = new QToolButton; 
		srcButton->setIcon(QIcon(":/icons/search.png")); 
		srcButton->setObjectName("searchButton"); 
		srcButton->setAutoRaise(true);
		srcButton->setToolTip("<font color=\"black\">Toggle search panel");
		srcButton->setCheckable(true);

		QToolButton* syncButton = new QToolButton; 
		syncButton->setIcon(QIcon(":/icons/sync.png")); 
		syncButton->setObjectName("syncButton"); 
		syncButton->setAutoRaise(true);
		syncButton->setToolTip("<font color=\"black\">Sync selection");

		QToolButton* refreshButton = new QToolButton;
		refreshButton->setIcon(QIcon(":/icons/refresh.png"));
		refreshButton->setObjectName("refreshButton");
		refreshButton->setAutoRaise(true);
		refreshButton->setToolTip("<font color=\"black\">Refresh");

		highlightButton = new QToolButton;
		highlightButton->setIcon(QIcon(":/icons/select_highlight.png"));
		highlightButton->setObjectName("highlightButton");
		highlightButton->setAutoRaise(true);
		highlightButton->setToolTip("<font color=\"black\">Toggle selection highlighting");
		highlightButton->setCheckable(true);
		highlightButton->setChecked(false);

		QToolButton* helpButton = new QToolButton;
		helpButton->setIcon(QIcon(":/icons/help.png"));
		helpButton->setObjectName("helpButton");
		helpButton->setAutoRaise(true);
		helpButton->setToolTip("<font color=\"black\">Help");

		// filter box
		m_filter = new QComboBox;
		m_filter->addItems(QStringList() << "All items" << "Geometry" << "Materials" << "Physics" << "Steps" << "Jobs" << "Studies" << "Images");
		m_filter->setObjectName("filter");
		m_filter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		m_errs = new WarningLabel; m_errs->setObjectName("warnings");

		QHBoxLayout* hf = new QHBoxLayout;
		hf->setContentsMargins(0,0,0,0);
		QLabel* l = new QLabel("Filter:");
		l->setBuddy(m_filter);
		hf->addWidget(l);
		hf->addWidget(m_filter);
		hf->addWidget(m_errs);
		m_errs->hide();

		// model tree
		tree = new CModelTree(wnd);
		tree->setObjectName("modelTree");

		// search widget
		m_search = new CModelSearch(wnd, tree);
		m_search->setObjectName("modelSearch");

		// stacked widget
		m_stack = new QStackedWidget;

		QVBoxLayout* th = new QVBoxLayout;
		th->setContentsMargins(0,0,0,0);
		th->addLayout(hf);
		th->addWidget(tree);

		QWidget* dummy = new QWidget;
		dummy->setLayout(th);

		m_stack->addWidget(dummy);
		m_stack->addWidget(m_search);

		// props panel
		props = new ::CModelPropsPanel(mainWnd);
		props->setObjectName("props");

		QSplitter* splitter = new QSplitter(Qt::Vertical);
		splitter->addWidget(m_stack);
		splitter->addWidget(props);
		splitter->setContentsMargins(0, 0, 0, 0);

		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addWidget(selectButton);
		buttonLayout->addWidget(highlightButton);
		buttonLayout->addWidget(deleteButton);
		buttonLayout->addWidget(srcButton);
		buttonLayout->addWidget(syncButton);
		buttonLayout->addWidget(refreshButton);
		buttonLayout->addWidget(helpButton);
		buttonLayout->addStretch();
		buttonLayout->setContentsMargins(0,0,0,0);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(buttonLayout);
		mainLayout->addWidget(splitter);
		mainLayout->setContentsMargins(0,0,0,0);

		wnd->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(wnd);
	}

	void showSearchPanel(bool b)
	{
		m_stack->setCurrentIndex(b ? 1 : 0);
	}

	void unCheckSearch()
	{
		if (srcButton->isChecked())
		{
			srcButton->setChecked(false);
		}
	}

	void setWarningCount(int n)
	{
		m_errs->setWarnings(n);
	}
};
