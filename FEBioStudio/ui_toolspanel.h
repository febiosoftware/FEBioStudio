#pragma once
#include "ToolsPanel.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include "Tool.h"
#include "ToolBox.h"

class Ui::CToolsPanel
{
public:
	QStackedWidget* stack;

public:
	void setupUi(::CToolsPanel* parent)
	{
		QList<CAbstractTool*>& tools = parent->tools;

		QVBoxLayout* pg = new QVBoxLayout(parent);
		pg->setMargin(1);

		QWidget* box = new QWidget;

		QGridLayout* grid = new QGridLayout;
		box->setLayout(grid);
		grid->setSpacing(2);

		QButtonGroup* group = new QButtonGroup(box);
		group->setObjectName("buttons");

		int ntools = tools.size();
		QList<CAbstractTool*>::Iterator it = tools.begin();
		for (int i = 0; i<ntools; ++i, ++it)
		{
			CAbstractTool* tool = *it;
			QPushButton* but = new QPushButton(tool->name());
			but->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
			but->setCheckable(true);

			grid->addWidget(but, i / 2, i % 2);
			group->addButton(but); group->setId(but, i + 1);
		}

		stack = new QStackedWidget;
		QLabel* label = new QLabel("(No tool selected)");
		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		stack->addWidget(label);

		it = tools.begin();
		for (int i = 0; i<ntools; ++i, ++it)
		{
			CAbstractTool* tool = *it;
			QWidget* pw = tool->createUi();
			if (pw == 0)
			{
				QLabel* pl = new QLabel("(no properties)");
				pl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
				stack->addWidget(pl);
			}
			else stack->addWidget(pw);
		}

		// create the toolbox
		CToolBox* tool = new CToolBox;
		tool->addTool("Tools", box);
		tool->addTool("Parameters", stack);

		pg->addWidget(tool);

		QMetaObject::connectSlotsByName(parent);
	}
};
