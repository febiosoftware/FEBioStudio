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
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QMessageBox>
#include <FECore/MathObject.h>
#include "CommandProcessor.h"
#include "version.h"
#include "PropertyList.h"

CCommandLine::CCommandLine() {}

CCommandLine::CCommandLine(const char* szcmd) : m_cmd(szcmd) {}

CCommandLine::CCommandLine(const QString& cmd) : m_cmd(cmd) {}

CCommandLine::CCommandLine(const QString& cmd, const QString& arg1)
{
	m_cmd = cmd;
	AddArgument(arg1);
}

CCommandLine::CCommandLine(const QString& cmd, const QString& arg1, const QString& arg2)
{
	m_cmd = cmd;
	AddArgument(arg1);
	AddArgument(arg2);
}

CCommandLine::CCommandLine(std::initializer_list<QString> initList)
{
	CCommandLine& T = *this;
	for (const auto& t : initList)
	{
		T << t;
	}
}

CCommandLine& CCommandLine::operator << (const QString& s)
{
	if (m_cmd.isEmpty()) { m_cmd = s; return *this; }
	else return AddArgument(s);
}

CCommandLine& CCommandLine::operator << (const QStringList& args)
{
	for (const QString& s : args)
	{
		if (m_cmd.isEmpty()) m_cmd = s;
		else AddArgument(s);
	}
	return *this;
}

CCommandLine& CCommandLine::AddCommand(const QString& cmd)
{
	m_cmd += ";" + cmd;
	return *this;
}

CCommandLine& CCommandLine::AddArgument(const QString& arg)
{
	m_cmd += " ";
	if (arg.contains(' '))
		m_cmd += "\"" + arg + "\"";
	else
		m_cmd += arg;
	return *this;
}

CCommandWindow* CCommandLogger::m_wnd = nullptr;
void CCommandLogger::Log(const CCommandLine& cmd)
{
	assert(m_wnd);
	if (m_wnd) m_wnd->LogCommand(cmd);
}

QStringList Stringify(const ParamContainer& PL)
{
	QStringList out;
	int n = PL.Parameters();
	for (int i = 0; i < n; ++i)
	{
		const Param& p = PL.GetParam(i);
		switch (p.GetParamType())
		{
		case Param_INT   : out << QString::number(p.GetIntValue()); break;
		case Param_BOOL  : out << (p.GetBoolValue() ? "1" : "0"); break;
		case Param_FLOAT : out << QString::number(p.GetFloatValue()); break;
		case Param_VEC3D : out << Vec3dToString(p.GetVec3dValue()); break;
		case Param_MAT3D : out << Mat3dToString(p.GetMat3dValue()); break;
		case Param_MAT3DS: out << Mat3dsToString(p.GetMat3dsValue()); break;
		default:
			assert(false);
			break;
		}
	}
	return out;
}

class Ui::CCommandWindow
{
public:
	::CMainWindow* m_wnd;

	QLineEdit* input = nullptr;
	QPlainTextEdit* out = nullptr;
	QPlainTextEdit* log = nullptr;

	CommandProcessor* cmd = nullptr;

public:
	void setup(::CCommandWindow* w)
	{
		QHBoxLayout* h = new QHBoxLayout;

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(input = new QLineEdit);

		out = new QPlainTextEdit;
		out->setReadOnly(true);
		out->setFont(QFont("Courier", 11));
		out->setWordWrapMode(QTextOption::NoWrap);
		l->addWidget(out);
		h->addLayout(l);

		QHBoxLayout* tl = new QHBoxLayout;
		QToolButton* b1 = new QToolButton; b1->setIcon(QIcon(":/icons/save.png")); b1->setAutoRaise(true); b1->setObjectName("cmdlogSave"); b1->setToolTip("<font color=\"black\">Save log");
		QToolButton* b2 = new QToolButton; b2->setIcon(QIcon(":/icons/clear.png")); b2->setAutoRaise(true); b2->setObjectName("cmdlogClear"); b2->setToolTip("<font color=\"black\">Clear log");
		tl->addWidget(b1);
		tl->addWidget(b2);
		tl->addStretch();

		log = new QPlainTextEdit;
		log->setReadOnly(true);
		log->setFont(QFont("Courier", 11));
		log->setWordWrapMode(QTextOption::NoWrap);

		QVBoxLayout* rl = new QVBoxLayout;
		rl->addLayout(tl);
		rl->addWidget(log);

		h->addLayout(rl);

		w->setLayout(h);

		QObject::connect(input, &QLineEdit::returnPressed, w, &::CCommandWindow::OnEnter);
		QObject::connect(b1, &QToolButton::clicked, w, &::CCommandWindow::OnSave);
		QObject::connect(b2, &QToolButton::clicked, w, &::CCommandWindow::OnClear);
	}

	QString getCommand() { return input->text(); }

	void Output(QString msg, int level = 0)
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

	void Log(QString msg)
	{
		QTextDocument* document = log->document();
		QTextCursor cursor(document);
		cursor.movePosition(QTextCursor::End);
		cursor.insertText(msg + "\n");
	}

	void RunCalculator(QString str)
	{
		MSimpleExpression m;
		std::string sstr = str.toStdString();
		if (m.Create(sstr) == false)
		{
			Output("syntax error", 1);
		}
		else
		{
			double v = m.value();
			QString ans = QString("%1 = %2").arg(QString::fromStdString(sstr)).arg(v, 0, 'g', 15);
			Output(ans);
			input->clear();
		}
	}
};

CCommandWindow::CCommandWindow(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new Ui::CCommandWindow)
{
	ui->m_wnd = wnd;
	ui->cmd = new CommandProcessor(wnd);
	ui->setup(this);
	CCommandLogger::SetCommandWindow(this);
}

CCommandWindow::~CCommandWindow()
{
	
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
	if (str.isEmpty()) return;

	if (str[0] == '=')
	{
		str.remove(0, 1); // remove '='
		ui->RunCalculator(str);
		return;
	}

	QStringList cmdAndOps = ui->cmd->ParseCommandLine(str);
	if (!cmdAndOps.empty())
	{
		QString cmd = cmdAndOps[0];
		QStringList ops = cmdAndOps; ops.pop_front();

		CMD_RETURN_CODE returnCode = ui->cmd->RunCommand(cmd, ops);
		QString msg = ui->cmd->GetCommandOutput();
		if (returnCode != CMD_RETURN_CODE::CMD_ERROR)
		{
			if (msg.isEmpty()) msg = str;
			ui->Output(msg);
			ui->input->clear();
			if (returnCode != CMD_RETURN_CODE::CMD_IGNORE)
				ui->Log(str);
		}
		else
		{
			msg = "ERROR: " + msg;
			ui->Output(msg, 1);
		}
	}
	else
	{
		QString msg = ui->cmd->GetCommandOutput();
		if (!msg.isEmpty())
		{
			msg = "ERROR: " + msg;
			ui->Output(msg, 1);
		}
		else
		{
			ui->Output(str);
			ui->input->clear();
		}
	}
}

void CCommandWindow::LogCommand(const CCommandLine& cmd)
{
	QString str = cmd.GetCommandString();
	// TODO: Do some minimal validation perhaps?
	QStringList commands = str.split(";");
	for (QString commandString : commands)
		ui->Log(commandString);
}

void CCommandWindow::OnSave()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "FEBio Studio Command Files (*.fsc)");
	if (fileName.isEmpty() == false)
	{
		// convert to const char*
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();

		// open the file
		FILE* fp = fopen(szfile, "wb");
		if (fp == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed saving command file");
			return;
		}

		// add a header comment
		fprintf(fp, "# Created by FEBio Studio %d.%d.%d\n", FBS_VERSION, FBS_SUBVERSION, FBS_SUBSUBVERSION);

		// convert data to string
		QString txt = ui->log->toPlainText();
		std::string s = txt.toStdString();
		size_t len = s.length();
		size_t nwritten = fwrite(s.c_str(), sizeof(char), len, fp);

		// close the file
		fclose(fp);
	}
}

void CCommandWindow::OnClear()
{
	if (QMessageBox::question(this, "Command Window", "Are you sure you want to clear the command history?"))
	{
		ui->log->clear();
	}
}