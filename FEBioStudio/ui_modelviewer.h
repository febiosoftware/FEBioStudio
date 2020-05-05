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
#include "ModelTree.h"
#include "ModelPropsPanel.h"
#include "ModelSearch.h"
#include "MainWindow.h"

class Ui::CModelViewer
{
public:
	QStackedWidget*		m_stack;
	CModelSearch*		m_search;

	CModelTree*			tree;
	::CModelPropsPanel*	props;

	QComboBox*	m_filter;

	QToolButton* srcButton;

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
		deleteButton->setIcon(CResource::Icon("delete")); 
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

		// filter box
		m_filter = new QComboBox;
		m_filter->addItems(QStringList() << "All items" << "Materials" << "Physics" << "Steps");
		m_filter->setObjectName("filter");

		// model tree
		tree = new CModelTree(wnd);
		tree->setObjectName("modelTree");

		// search widget
		m_search = new CModelSearch(wnd, tree);

		// stacked widget
		m_stack = new QStackedWidget;

		QVBoxLayout* th = new QVBoxLayout;
		th->setMargin(0);
		th->addWidget(m_filter);
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
		buttonLayout->addWidget(deleteButton);
		buttonLayout->addWidget(srcButton);
		buttonLayout->addWidget(syncButton);
		buttonLayout->addStretch();
		buttonLayout->setMargin(0);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(buttonLayout);
		mainLayout->addWidget(splitter);
		mainLayout->setMargin(0);

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
};
