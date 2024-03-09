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
#include "DocManager.h"
#include "ModelDocument.h"
#include "units.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <FEBioLink/FEBioModule.h>

class Ui::CCommandWindow
{
public:
	::CMainWindow* m_wnd;

	QLineEdit* input;

public:
	void setup(::CCommandWindow* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(input = new QLineEdit);
		l->addStretch();
		w->setLayout(l);

		QObject::connect(input, &QLineEdit::returnPressed, w, &::CCommandWindow::OnEnter);
	}

	QString getCommand() { return input->text(); }

	void ProcessCommand(QStringList cmdAndOps)
	{
		QString cmd = cmdAndOps[0];
		QStringList ops = cmdAndOps; ops.pop_front();
		if      (cmd == "new" ) RunNewCmd(ops);
		else if (cmd == "open") RunOpenCmd(ops);
		else if (cmd == "save") RunSaveCmd(ops);
		else if (cmd == "exit") RunExitCmd(ops);
		input->clear();
	}

	void RunNewCmd(QStringList ops)
	{
		if (ops.empty())
			m_wnd->on_actionNewModel_triggered();
		else
		{
			CDocManager* dm = m_wnd->GetDocManager();
			int moduleID = FEBio::GetModuleId(ops[0].toStdString());
			if (moduleID != -1)
			{
				CModelDocument* doc = dm->CreateNewDocument(moduleID);
				if (doc)
				{
					int units = doc->GetUnitSystem();
					Units::SetUnitSystem(units);
					m_wnd->AddDocument(doc);
				}
			}
		}
	}

	void RunOpenCmd(QStringList ops)
	{
		if (ops.empty())
			m_wnd->on_actionOpen_triggered();
		else
		{
			m_wnd->OpenFile(ops[0]);
		}
	}

	void RunSaveCmd(QStringList ops)
	{
		m_wnd->on_actionSave_triggered();
	}

	void RunExitCmd(QStringList ops)
	{
		m_wnd->on_actionExit_triggered();
	}
};

CCommandWindow::CCommandWindow(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new Ui::CCommandWindow)
{
	ui->m_wnd = wnd;
	ui->setup(this);
}

void CCommandWindow::Show()
{
	parentWidget()->show();
	parentWidget()->raise();
	ui->input->setFocus();
}

void CCommandWindow::showEvent(QShowEvent* ev)
{
	ui->input->setFocus();
}

void CCommandWindow::OnEnter()
{
	QString str = ui->getCommand();
	QStringList cmd = str.split(" ", Qt::SkipEmptyParts);
	ui->ProcessCommand(cmd);
}
