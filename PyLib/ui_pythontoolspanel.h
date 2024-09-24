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
#include "PythonToolsPanel.h"
#include <QBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QGroupBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QStackedWidget>
#include <QProgressBar>
#include <FEBioStudio/IconProvider.h>
#include <QLabel>
#include <FEBioStudio/Tool.h>
#include <FEBioStudio/ToolBox.h>
#include "PythonTool.h"
#include "PythonThread.h"

class CPythonToolBar : public QToolBar
{
public:
	CPythonToolBar(QWidget* parent = nullptr) : QToolBar(nullptr)
	{
		QAction* importScript = new QAction(CIconProvider::GetIcon("open"), "Import Script", parent);
		importScript->setObjectName("importScript");
		importScript->setIconVisibleInMenu(false);
		addAction(importScript);

		// add empty widget so that the refresh button can be aligned on the right
		QWidget* empty = new QWidget();
		empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		addWidget(empty);

		QAction* refresh = new QAction(CIconProvider::GetIcon("refresh"), "Refresh", parent);
		refresh->setObjectName("refresh");
		refresh->setIconVisibleInMenu(false);
		addAction(refresh);
	}
};

class CPythonButtonBox : public QWidget
{
public:
	CPythonButtonBox(QWidget* parent = nullptr) : QWidget(parent)
	{
		m_grid = new QGridLayout;
		setLayout(m_grid);
		m_grid->setSpacing(2);

		m_buttonGroup = new QButtonGroup(this);
		m_buttonGroup->setObjectName("buttons");
	}

	void addButton(const QString& label)
	{
		QPushButton* but = new QPushButton(label);
		but->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
		but->setCheckable(true);

		int i = m_buttons++;
		m_grid->addWidget(but, i / 2, i % 2);
		m_buttonGroup->addButton(but, i + 1); 
	}

	void setButtonText(int id, const QString& txt)
	{
		QAbstractButton* pb = m_buttonGroup->button(id);
		if (pb) pb->setText(txt);
	}

	void pushButton(int id)
	{
		QAbstractButton* pb = m_buttonGroup->button(id);
		if (pb) pb->click();
	}

private:
	int m_buttons = 0;
	QGridLayout* m_grid;
	QButtonGroup* m_buttonGroup;
};

class CPythonParameterPanel : public QStackedWidget
{
public:
	CPythonParameterPanel(QWidget* parent = nullptr) : QStackedWidget(parent)
	{
		QLabel* label = new QLabel("(No tool selected)");

		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		addWidget(label);
	}

	void AddPanel(QWidget* w)
	{
		if (w == nullptr)
		{
			QLabel* pl = new QLabel("(no properties)");
			pl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
			addWidget(pl);
		}
		else addWidget(w);
	}
};

class CPythonRunningPane : public QStackedWidget
{
private:
	QLabel* m_text;
	QProgressBar* m_progress;

public:
	CPythonRunningPane(QWidget* parent = nullptr) : QStackedWidget(parent)
	{
		QWidget* w1 = new QWidget;
		QVBoxLayout* l1 = new QVBoxLayout(w1);
		QPushButton* runButton = new QPushButton("run");
		runButton->setObjectName("run");
		l1->addWidget(runButton);
		l1->addStretch();
		addWidget(w1);

		QWidget* w2 = new QWidget;
		QVBoxLayout* l2 = new QVBoxLayout(w2);
		l2->addWidget(m_text = new QLabel());
		l2->addWidget(m_progress = new QProgressBar());
		l2->addStretch();
		addWidget(w2);

		m_progress->setMinimum(0);
		m_progress->setMaximum(100);
	}

	void startRunning(const QString& msg)
	{
		setCurrentIndex(1);
		m_text->setText(msg);
		m_progress->setValue(0);
	}

	void stopRunning()
	{
		setCurrentIndex(0);
	}

	void setProgressText(const QString& txt)
	{
		m_text->setText(txt);
	}

	void setProgress(int n)
	{
		m_progress->setValue(n);
	}
};

class Ui::CPythonToolsPanel
{
public:
	CPythonButtonBox* buttons = nullptr;
	CPythonParameterPanel* paramStack = nullptr;
	CPythonRunningPane* runPane = nullptr;

	::CPythonToolsPanel* parent = nullptr;

public:
	CPyThread* m_pythonThread = nullptr;

	CPythonTool* m_activeTool = nullptr;
	std::vector<CPythonTool*>	tools;

public:
	void setupUi(::CPythonToolsPanel* parent)
	{
		this->parent = parent;

		// build the toolbar
		QToolBar* toolbar = new CPythonToolBar(parent);

		// build the button box
		buttons = new CPythonButtonBox();

		// create the parameters pane
		QWidget* paramWidget = new QWidget;
		QVBoxLayout* paramLayout = new QVBoxLayout(paramWidget);
		paramStack = new CPythonParameterPanel();
		paramLayout->addWidget(paramStack);

		// create the running page (shown when a tool is running)
		runPane = new CPythonRunningPane();
		runPane->hide();
		paramLayout->addWidget(runPane);

		// create the toolbox
		CToolBox* tool = new CToolBox;
		tool->addTool("Tools", buttons);
		tool->addTool("Parameters", paramWidget);

		// build the main layout
		QVBoxLayout* mainLayout = new QVBoxLayout(parent);
		mainLayout->setContentsMargins(1, 1, 1, 1);
		mainLayout->addWidget(toolbar);
		mainLayout->addWidget(tool);

		QMetaObject::connectSlotsByName(parent);
	}

	void addTool(CPythonTool* tool)
	{
		buttons->addButton(tool->name());
		paramStack->AddPanel(tool->createUi());
		tool->SetID((int)tools.size());
		tools.push_back(tool);
		// trigger a button push so the new button becomes the active tool.
		buttons->pushButton(tool->GetID() + 1);
	}

	CPythonTool* findTool(const QString& filepath)
	{
		for (auto tool : tools)
		{
			if (tool->GetFilePath() == filepath) return tool;
		}
		return nullptr;
	}

	void setToolName(CPythonTool* tool, const QString& toolName)
	{
		tool->setName(toolName);
		for (int i = 0; i < (int)tools.size(); ++i)
		{
			if (tools[i] == tool)
			{
				buttons->setButtonText(i + 1, toolName);
				return;
			}
		}
	}
};
