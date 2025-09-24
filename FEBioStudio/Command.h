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
#include <string>
#include <vector>

class CCommandManager;

//-----------------------------------------------------------------------------
// view state
// This stores the variables that define the state of the UI 
struct VIEW_STATE
{
	int		nselect;	// selection mode
	int		nstyle;		// selection style
	int		ntrans;		// transform mode
	int		nitem;		// modify mode
};

//----------------------------------------------------------------
class CCommand;

//----------------------------------------------------------------
// exception thrown a command is executed
class CCmdFailed
{
public:
	CCmdFailed(CCommand* pcmd, const std::string& err) : m_pcmd(pcmd), m_err(err) {}
	CCommand* GetCommand() { return m_pcmd; }
	std::string GetErrorString() { return m_err; }

private:
	CCommand* m_pcmd;
	std::string	m_err;
};

//----------------------------------------------------------------
// CCommand
// Base class for other commands
//
class CCommand  
{
public:
	CCommand(const std::string& name);
	virtual ~CCommand();

	virtual void Execute  () = 0;
	virtual void UnExecute() = 0;

	const char* GetName() const;
	void SetName(const std::string& name);

	virtual void SetViewState(VIEW_STATE state);
	VIEW_STATE GetViewState();

protected:
	// doc/view state variables
	VIEW_STATE	m_state;

	std::string m_name;	// command name

	friend class CCommandManager;
};

typedef std::vector<CCommand*> CCmdPtrArray;

//----------------------------------------------------------------
// CCmdGroup
// Command that groups other commands
//
class CCmdGroup : public CCommand
{
public:
	CCmdGroup();
	CCmdGroup(const std::string& cmd);
	virtual ~CCmdGroup();

	void AddCommand(CCommand* pcmd);

	void Execute() override;

	void UnExecute() override;

	int GetCount() const;

	void SetViewState(VIEW_STATE state) override;

protected:
	CCmdPtrArray	m_Cmd;	// array of pointer to commands
};
