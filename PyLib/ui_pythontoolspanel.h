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
#include <FEBioStudio/IconProvider.h>
#include <QLabel>
#include <FEBioStudio/Tool.h>
#include <FEBioStudio/ToolBox.h>
#include <QPlainTextEdit>

class Ui::CPythonToolsPanel
{
public:
	QStackedWidget* parentStack;
	QStackedWidget* stack;
	QGridLayout* grid;
	QButtonGroup* group;
	
	QAction* importScript;

	QPlainTextEdit*	txt;

public:
	void setupUi(::CPythonToolsPanel* parent)
	{
		this->parent = parent;

		QList<CPythonTool*>& tools = parent->tools;
		
		QVBoxLayout* parentLayout = new QVBoxLayout(parent);
		parentLayout->addWidget(parentStack = new QStackedWidget);

		QWidget* mainPage = new QWidget;

		QVBoxLayout* pg = new QVBoxLayout(mainPage);
		pg->setContentsMargins(1,1,1,1);

		QToolBar* toolbar = new QToolBar();

		importScript = new QAction(CIconProvider::GetIcon("open"), "Import Script", parent);
		importScript->setObjectName("importScript");
		importScript->setIconVisibleInMenu(false);
		toolbar->addAction(importScript);

		pg->addWidget(toolbar);

		QWidget* box = new QWidget;

		grid = new QGridLayout;
		box->setLayout(grid);
		grid->setSpacing(2);

		group = new QButtonGroup(box);
		group->setObjectName("buttons");

		stack = new QStackedWidget;
		QLabel* label = new QLabel("(No tool selected)");
		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		stack->addWidget(label);

		// create the toolbox
		CToolBox* tool = new CToolBox;
		tool->addTool("Tools", box);
		tool->addTool("Parameters", stack);

		pg->addWidget(tool);

		txt = new QPlainTextEdit;
		txt->setReadOnly(true);
		txt->setFont(QFont("Courier", 11));
		txt->setWordWrapMode(QTextOption::NoWrap);

		pg->addWidget(txt);

		parentStack->addWidget(mainPage);

		QMetaObject::connectSlotsByName(parent);
	}


	void addTool(CAbstractTool* tool)
	{
		int i = parent->tools.size() - 1;

		QPushButton* but = new QPushButton(tool->name());
		but->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
		but->setCheckable(true);

		grid->addWidget(but, i / 2, i % 2);
		group->addButton(but); group->setId(but, i + 1);

		QWidget* pw = tool->createUi();
		if (pw == 0)
		{
			QLabel* pl = new QLabel("(no properties)");
			pl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
			stack->addWidget(pl);
		}
		else stack->addWidget(pw);
	}

	void addPage(QWidget* page)
	{
		parentStack->addWidget(page);
		parentStack->setCurrentIndex(1);
	}

	void removePage()
	{
		parentStack->setCurrentIndex(0);
		parentStack->removeWidget(parentStack->widget(1));
	}

private:
	::CPythonToolsPanel* parent;
};
