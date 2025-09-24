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
#include "Document.h"
#include "Command.h"
#include "CommandManager.h"
#include <QDateTime>

class ChangeLog
{
public:
	class Entry {
	public:
		QString txt;
		QDateTime time;
	};

public:
	size_t size() const { return m_entry.size(); }
	const Entry& entry(size_t n) const { return m_entry[n]; }

	void append(const QString& s);

public:
	QString toJson() const;

	void fromJson(const QString& json);

private:
	std::vector<Entry>	m_entry;
};

// Base class for documents that use an undo stack
class CUndoDocument : public CDocument
{
	Q_OBJECT

public:
	CUndoDocument(CMainWindow* wnd);
	~CUndoDocument();

	void Clear() override;

	// --- Command history functions ---
	bool CanUndo();
	bool CanRedo();
	void AddCommand(CCommand* pcmd, const std::string& s = "");
	bool DoCommand(CCommand* pcmd, const std::string& s = "");
	void UndoCommand();
	void RedoCommand();
	const char* GetUndoCmdName();
	const char* GetRedoCmdName();
	void ClearCommandStack();
	const std::string& GetCommandErrorString() const;

public:
	//! Get the change log
	const ChangeLog& GetChangeLog();

	//! Add a string to the change log
	void AppendChangeLog(const QString& s);

protected:
	void SetChangeLog(const ChangeLog& log);

protected:
	// The command manager
	CCommandManager* m_pCmd;		// the command manager
	ChangeLog			m_changeLog;
};
