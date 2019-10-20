#pragma once
#include "LogPanel.h"
#include <QBoxLayout>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QStackedWidget>
#include <QComboBox>

class Ui::CLogPanel
{
public:
	QComboBox* combo;
	QStackedWidget* stack;
	QPlainTextEdit*	txt[2];

public:
	void setupUi(QWidget* parent)
	{
		txt[0] = new QPlainTextEdit;
		txt[0]->setReadOnly(true);
		txt[0]->setFont(QFont("Courier", 11));
		txt[0]->setWordWrapMode(QTextOption::NoWrap);

		txt[1] = new QPlainTextEdit;
		txt[1]->setReadOnly(true);
		txt[1]->setFont(QFont("Courier", 11));
		txt[1]->setWordWrapMode(QTextOption::NoWrap);

		stack = new QStackedWidget;
		stack->addWidget(txt[0]);
		stack->addWidget(txt[1]);

		QVBoxLayout* pl = new QVBoxLayout;
		pl->setMargin(0);
		pl->setSpacing(0);

		combo = new QComboBox;
		combo->addItem("Log");
		combo->addItem("Output");
		combo->setObjectName("combo");

		QToolButton* b1 = new QToolButton; b1->setIcon(QIcon(":/icons/save.png")); b1->setAutoRaise(true); b1->setObjectName("logSave"); b1->setToolTip("<font color=\"black\">Save log");
		QToolButton* b2 = new QToolButton; b2->setIcon(QIcon(":/icons/clear.png")); b2->setAutoRaise(true); b2->setObjectName("logClear"); b2->setToolTip("<font color=\"black\">Clear log");
		QHBoxLayout* pv = new QHBoxLayout;
		pv->addWidget(b1);
		pv->addWidget(b2);
		pv->addWidget(combo);
		pv->addStretch();

		pl->addLayout(pv);
		pl->addWidget(stack);
		parent->setLayout(pl);

		QMetaObject::connectSlotsByName(parent);
	}

	void setOutput(int n)
	{
		stack->setCurrentIndex(n);
	}

	QPlainTextEdit* currentTxt()
	{
		int n = stack->currentIndex();
		return txt[n];
	}

	void clearLog(int n)
	{
		txt[n]->clear();
	}

	void showTxt(int n)
	{
		if (combo->currentIndex() != 1) combo->setCurrentIndex(1);
	}
};
