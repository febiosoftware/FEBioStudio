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
#include "stdafx.h"
#include "CommandWindow.h"
#include "MainWindow.h"
#include <QBoxLayout>
#include <QLineEdit>

class Ui::CCommandWindow
{
public:
	::CMainWindow* m_wnd;

	QLineEdit* cmd;

public:
	void setup(::CCommandWindow* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(cmd = new QLineEdit);
		l->addStretch();
		w->setLayout(l);

		QObject::connect(cmd, &QLineEdit::returnPressed, w, &::CCommandWindow::OnEnter);
	}

	QString getCommand() { return cmd->text(); }
};

CCommandWindow::CCommandWindow(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new Ui::CCommandWindow)
{
	ui->m_wnd = wnd;
	ui->setup(this);
}

void CCommandWindow::OnEnter()
{
	QString str = ui->getCommand();
	QStringList cmd = str.split(" ", Qt::SkipEmptyParts);
	if (cmd[0] == "new")
	{
		ui->m_wnd->on_actionNewModel_triggered();
	}
	else if (cmd[0] == "open")
	{
		ui->m_wnd->on_actionOpen_triggered();
	}
	ui->cmd->clear();
}
