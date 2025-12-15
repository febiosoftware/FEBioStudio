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
#include "UndoDocument.h"
#include <FSCore/FSLogger.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
using namespace std;

void ChangeLog::append(const QString& s)
{
	QDateTime current = QDateTime::currentDateTime();
	m_entry.push_back({ s, current });
}

QString ChangeLog::toJson() const
{
	QJsonArray json;
	for (const Entry& e : m_entry)
	{
		QJsonObject o;
		o["txt"] = e.txt;
		o["time"] = e.time.toString();
		json.push_back(o);
	}

	QJsonDocument jdoc(json);
	QString s = jdoc.toJson(QJsonDocument::JsonFormat::Compact);
	return s;
}

void ChangeLog::fromJson(const QString& json)
{
	QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8());
	QJsonArray ja = jdoc.array();
	m_entry.clear();
	for (QJsonArray::Iterator it = ja.begin(); it != ja.end(); it++)
	{
		QJsonValue v = *it;
		QJsonObject o = v.toObject();
		QString txt = o["txt"].toString();
		QString time = o["time"].toString();
		m_entry.push_back({ txt, QDateTime::fromString(time) });
	}
}

CUndoDocument::CUndoDocument(CMainWindow* wnd) : CDocument(wnd)
{
	m_pCmd = new CCommandManager(this);
}

CUndoDocument::~CUndoDocument()
{
	delete m_pCmd;
}

void CUndoDocument::Clear()
{
	CDocument::Clear();

	// Clear the command history
	m_pCmd->Clear();
}

bool CUndoDocument::CanUndo() { return m_pCmd->CanUndo(); }

bool CUndoDocument::CanRedo() { return m_pCmd->CanRedo(); }

void CUndoDocument::AddCommand(CCommand* pcmd, const std::string& s)
{
	assert(pcmd);
	if (pcmd == nullptr) return;

	const char* szname = pcmd->GetName();
	string msg = (szname ? szname : "<unknown>");
	if (s.empty() == false) msg += " (" + s + ")";
	AppendChangeLog(QString::fromStdString(msg));
	FSLogger::Write("Executing command: " + msg + "\n");

	m_pCmd->AddCommand(pcmd);
	if (pcmd->HasFlag(CCommand::MODIFIES_DOC)) SetModifiedFlag();
	Update();
}

const char* CUndoDocument::GetUndoCmdName() { return m_pCmd->GetUndoCmdName(); }

const char* CUndoDocument::GetRedoCmdName() { return m_pCmd->GetRedoCmdName(); }

bool CUndoDocument::DoCommand(CCommand* pcmd, const std::string& s)
{
	assert(pcmd);
	if (pcmd == nullptr) return false;

	const char* szname = pcmd->GetName();
	string msg = (szname ? szname : "<unknown>");
	if (s.empty() == false) msg += " (" + s + ")";
	AppendChangeLog(QString::fromStdString(msg));
	FSLogger::Write("Executing command: " + msg + "\n");

	bool ret = m_pCmd->DoCommand(pcmd);
	if (ret && pcmd->HasFlag(CCommand::MODIFIES_DOC))
	{
		SetModifiedFlag();
	}
	Update();
	return ret;
}

const std::string& CUndoDocument::GetCommandErrorString() const
{
	return m_pCmd->GetErrorString();
}

void CUndoDocument::UndoCommand()
{
	string cmdName = m_pCmd->GetUndoCmdName();
	m_pCmd->UndoCommand();
	SetModifiedFlag();
	Update();

	string msg = "Undo last command (" + cmdName + ")\n";
	FSLogger::Write(msg);
	AppendChangeLog(QString::fromStdString(msg));
}

void CUndoDocument::RedoCommand()
{
	string cmdName = m_pCmd->GetRedoCmdName();
	m_pCmd->RedoCommand();
	SetModifiedFlag();
	Update();

	string msg = "Redo last command (" + cmdName + ")\n";
	FSLogger::Write(msg);
	AppendChangeLog(QString::fromStdString(msg));
}

void CUndoDocument::ClearCommandStack()
{
	m_pCmd->Clear();
}

void CUndoDocument::AppendChangeLog(const QString& s)
{
	m_changeLog.append(s.simplified());
}

void CUndoDocument::SetChangeLog(const ChangeLog& log)
{
	m_changeLog = log;
}

const ChangeLog& CUndoDocument::GetChangeLog()
{
	return m_changeLog;
}
