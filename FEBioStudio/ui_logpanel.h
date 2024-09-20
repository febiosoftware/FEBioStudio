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
#include "LogPanel.h"
#include <QBoxLayout>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QStackedWidget>
#include <QComboBox>
#include <QLabel>

class Ui::CLogPanel
{
public:
	QComboBox* combo;
	QStackedWidget* stack;
	QPlainTextEdit*	txt[4];
	QTextCharFormat defaultTextCharFormat;

public:
	void setupUi(QWidget* parent)
	{
		stack = new QStackedWidget;
		for (int i = 0; i < 4; ++i)
		{
			txt[i] = new QPlainTextEdit;
			txt[i]->setReadOnly(true);
			txt[i]->setFont(QFont("Courier", 11));
			txt[i]->setWordWrapMode(QTextOption::NoWrap);
			stack->addWidget(txt[i]);
		}

		QVBoxLayout* pl = new QVBoxLayout;
		pl->setContentsMargins(0,0,0,0);
		pl->setSpacing(0);

		combo = new QComboBox;
		combo->setMinimumWidth(200);
		combo->addItem("Log");
		combo->addItem("FEBio");
		combo->addItem("Build");
		combo->addItem("Python");
		combo->setObjectName("combo");

		QToolButton* b1 = new QToolButton; b1->setIcon(QIcon(":/icons/save.png")); b1->setAutoRaise(true); b1->setObjectName("logSave"); b1->setToolTip("<font color=\"black\">Save log");
		QToolButton* b2 = new QToolButton; b2->setIcon(QIcon(":/icons/clear.png")); b2->setAutoRaise(true); b2->setObjectName("logClear"); b2->setToolTip("<font color=\"black\">Clear log");
		QHBoxLayout* pv = new QHBoxLayout;
		pv->addWidget(new QLabel("Show output from:"));
		pv->addWidget(combo);
		pv->addWidget(b1);
		pv->addWidget(b2);
		pv->addStretch();

		pl->addLayout(pv);
		pl->addWidget(stack);
		parent->setLayout(pl);

		QMetaObject::connectSlotsByName(parent);

		QTextDocument * document = txt[0]->document();
		QTextCursor cursor(document);
		defaultTextCharFormat = cursor.charFormat();
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
		if (combo->currentIndex() != n) combo->setCurrentIndex(n);
	}
};
