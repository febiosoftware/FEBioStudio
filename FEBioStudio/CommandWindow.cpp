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
#include "Commands.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <FEBioLink/FEBioModule.h>
#include <GeomLib/GPrimitive.h>
#include <FECore/MathObject.h>
#include <sstream>

class CommandProcessor
{
private:
	struct CCommandDescriptor
	{
		QString name;	// name of command
		QString brief;	// brief description
		bool(CommandProcessor::*f)(QStringList);
	};

	std::vector<CCommandDescriptor> m_cmds;

public:
	CommandProcessor(CMainWindow* wnd) : m_wnd(wnd) 
	{
		m_cmds.push_back({ "close" , "closes the current model", &CommandProcessor::RunCloseCmd });
		m_cmds.push_back({ "cmd"   , "run a command script", &CommandProcessor::RunCmdCmd });
		m_cmds.push_back({ "create", "add a primitive to the current model", &CommandProcessor::RunCreateCmd });
		m_cmds.push_back({ "exit"  , "closes FEBio Studio", &CommandProcessor::RunExitCmd });
		m_cmds.push_back({ "job"   , "run the model in FEBio", &CommandProcessor::RunJobCmd });
		m_cmds.push_back({ "help"  , "show help", &CommandProcessor::RunHelpCmd });
		m_cmds.push_back({ "new"   , "create a new model", &CommandProcessor::RunNewCmd });
		m_cmds.push_back({ "open"  , "open a file", &CommandProcessor::RunOpenCmd });
		m_cmds.push_back({ "save"  , "save the current model", &CommandProcessor::RunSaveCmd });
	}

	QString GetErrorString() { return m_error; }

	QString GetCommandOutput() { return m_output; }

	bool ProcessCommandLine(QString cmdLine)
	{
		QStringList cmdAndOps = ParseCommandLine(cmdLine);
		if (cmdAndOps.empty()) return false;
		QString cmd = cmdAndOps[0];
		QStringList ops = cmdAndOps; ops.pop_front();
		return RunCommand(cmd, ops);
	}

	bool RunCommand(QString cmd, QStringList ops)
	{
		m_error.clear();
		m_output.clear();
		for (auto& entry : m_cmds)
		{
			if (cmd == entry.name)
			{
				return (*this.*(entry.f))(ops);
			}
		}
		return Error(QString("Unknown command: %1").arg(cmd));
	}

public: // command functions
	bool RunJobCmd(QStringList ops)
	{
		if (ops.empty())
		{
			m_wnd->on_actionFEBioRun_triggered();
			return true;
		}
		return Error("Invalid number of arguments.");
	}

	bool RunCloseCmd(QStringList ops)
	{
		m_wnd->CloseView(m_wnd->GetDocument());
		return true;
	}

	bool RunHelpCmd(QStringList ops)
	{
		m_output = "available commands:\n";
		for (auto& entry : m_cmds)
		{
			m_output += QString("  %1 - %2\n").arg(entry.name, -7).arg(entry.brief);
		}
		return true;
	}

	bool RunNewCmd(QStringList ops)
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
				else return Error("Failed creating new model.");
			}
			else return Error(QString("Don't know module \"%1\"").arg(ops[0]));
		}
		return true;
	}

	bool RunOpenCmd(QStringList ops)
	{
		if (ops.empty())
			m_wnd->on_actionOpen_triggered();
		else
		{
			m_wnd->OpenFile(ops[0]);
		}
		return true;
	}

	bool RunSaveCmd(QStringList ops)
	{
		m_wnd->on_actionSave_triggered();
		return true;
	}

	bool RunExitCmd(QStringList ops)
	{
		m_wnd->on_actionExit_triggered();
		return true;
	}

	bool RunCmdCmd(QStringList ops)
	{
		QString cmdFile;
		if (ops.empty())
		{
			QStringList filters; filters << "FEBio Studio Command File (*.fsc)";

			QFileDialog dlg(m_wnd, "Open");
			dlg.setFileMode(QFileDialog::ExistingFile);
			dlg.setAcceptMode(QFileDialog::AcceptOpen);
			dlg.setDirectory(m_wnd->CurrentWorkingDirectory());
			dlg.setNameFilters(filters);
			if (dlg.exec())
			{
				// get the file name
				QStringList files = dlg.selectedFiles();
				cmdFile = files.first();
			}
			else return true;
		}
		else cmdFile = ops[0];
		if (!cmdFile.isEmpty())
		{
			return RunCommandFile(cmdFile);
		}
		return Error("Failed to run command file.");
	}

	bool RunCreateCmd(QStringList ops)
	{
		if (ops.empty()) return Error("Missing command arguments.");

		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No active model.");

		QString type = ops[0];
		GObject* po = nullptr;
		if (type == "box") po = new GBox(); 
		if (po == nullptr) return Error(QString("Can create %1").arg(type));

		// set default name
		std::stringstream ss;
		ss << "Object" << po->GetID();
		po->SetName(ss.str());

		// apply parameters
		if (ops.size() > 1)
		{
			int N = ops.size() - 1;
			if (N > po->Parameters())
			{
				delete po;
				return Error("Invalid number of arguments.");
			}

			for (int i = 1; i < ops.size(); ++i)
			{
				Param& pp = po->GetParam(i - 1);
				switch (pp.GetParamType())
				{
				case Param_INT  : pp.SetIntValue(ops[i].toInt()); break;
				case Param_FLOAT: pp.SetFloatValue(ops[i].toDouble()); break;
				}
			}
			po->Update();
		}

		doc->DoCommand(new CCmdAddAndSelectObject(doc->GetGModel(), po), po->GetNameAndType());
		m_wnd->UpdateModel(po);
		return true;
	}

	bool RunCommandFile(QString cmdFile)
	{
		QFile file(cmdFile);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
			return Error(QString("Failed to open command file: %1").arg(cmdFile));

		int lineCount = 0;
		while (!file.atEnd())
		{
			QByteArray line = file.readLine();
			lineCount++;
			string s = line.toStdString();
			if (s[0] != '#')
			{
				if (s.rfind('\n') != string::npos) s.pop_back();
				QString cmdLine = QString::fromStdString(s);
				if (ProcessCommandLine(cmdLine) == false)
				{
					QString msg = QString("Error at line %1:\n%2").arg(lineCount).arg(m_error);
					return Error(msg);
				}
			}
		}
		return Success(QString("run %1").arg(cmdFile));
	}

private:
	bool Error(QString msg) { m_error = msg; return false; }
	bool Success(QString msg) { m_output = msg; return true; }

	QStringList ParseCommandLine(QString cmd)
	{
		return cmd.split(" ", Qt::SkipEmptyParts);
	}

private:
	CMainWindow* m_wnd;
	QString m_error;
	QString m_output;
};

class Ui::CCommandWindow
{
public:
	::CMainWindow* m_wnd;

	QLineEdit* input = nullptr;
	QPlainTextEdit* out = nullptr;

	CommandProcessor* cmd = nullptr;

public:
	void setup(::CCommandWindow* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(input = new QLineEdit);

		out = new QPlainTextEdit;
		out->setReadOnly(true);
		out->setFont(QFont("Courier", 11));
		out->setWordWrapMode(QTextOption::NoWrap);
		l->addWidget(out);
		w->setLayout(l);

		QObject::connect(input, &QLineEdit::returnPressed, w, &::CCommandWindow::OnEnter);
	}

	QString getCommand() { return input->text(); }

	void Log(QString msg, int level = 0)
	{
		out->clear();
		QTextDocument* document = out->document();
		QTextCursor cursor(document);
		QTextCharFormat fmt = cursor.charFormat();
		switch (level)
		{
		case 0: fmt.setForeground(Qt::black); break;
		case 1: fmt.setForeground(Qt::red); break;
		}
		cursor.movePosition(QTextCursor::End);
		cursor.insertText(msg, fmt);
	}
};

CCommandWindow::CCommandWindow(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new Ui::CCommandWindow)
{
	ui->m_wnd = wnd;
	ui->cmd = new CommandProcessor(wnd);
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
	if (str[0] == '=')
	{
		MSimpleExpression m;
		str.remove(0, 1);
		std::string sstr = str.toStdString();
		if (m.Create(sstr) == false)
		{
			ui->Log("syntax error", 1);
		}
		else
		{
			double v = m.value();
			QString ans = QString("%1 = %2").arg(QString::fromStdString(sstr)).arg(v, 0, 'g', 15);
			ui->Log(ans);
			ui->input->clear();
		}
		return;
	}

	bool b = ui->cmd->ProcessCommandLine(str);
	if (b)
	{
		QString msg = ui->cmd->GetCommandOutput();
		if (msg.isEmpty()) msg = str;
		ui->Log(msg);
		ui->input->clear();
	}
	else
	{
		ui->Log(ui->cmd->GetErrorString(), 1);
	}
}
