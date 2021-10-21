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
#include <stack>
#include <string>

class CCommand;
class CGLDocument;

typedef std::stack<CCommand*> CCmdStack;

class CBasicCmdManager
{
public:
	CBasicCmdManager();
	virtual ~CBasicCmdManager();

	bool CanUndo() { return (m_Undo.size() > 0); }
	bool CanRedo() { return (m_Redo.size() > 0); }

	virtual void AddCommand(CCommand* pcmd);

	virtual bool DoCommand(CCommand* pcmd);

	virtual void UndoCommand();

	virtual void RedoCommand();

	void Clear();

	const char* GetUndoCmdName();
	const char* GetRedoCmdName();

protected:
	CCmdStack	m_Undo;	// the undo stack
	CCmdStack	m_Redo;	// the redo stack

public:
	static const std::string& GetErrorString() { return m_err; }
	void SetErrorString(const std::string& err) { m_err = err; }
	static std::string	m_err;	// error string
};

class CCommandManager  : public CBasicCmdManager
{
public:
	CCommandManager(CGLDocument* pdoc);
	virtual ~CCommandManager();

	void AddCommand(CCommand* pcmd) override;

	bool DoCommand(CCommand* pcmd) override;

	void UndoCommand() override;

	void RedoCommand() override;

protected:
	CGLDocument* m_pDoc; // pointer to the current document
};
