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
#include "CommandManager.h"
#include "Command.h"
#include "GLDocument.h"
#include <GeomLib/GObject.h>

std::string CBasicCmdManager::m_err;

CBasicCmdManager::CBasicCmdManager()
{
}

CBasicCmdManager::~CBasicCmdManager()
{
	Clear();
}

void CBasicCmdManager::AddCommand(CCommand* pcmd)
{
	// push the command
	m_Undo.push(pcmd);

	// clear the redo stack
	int N = (int)m_Redo.size();
	for (int i = 0; i<N; i++) { delete m_Redo.top(); m_Redo.pop(); }
}

bool CBasicCmdManager::DoCommand(CCommand* pcmd)
{
	m_err.clear();

	// execute the command
	try
	{
		pcmd->Execute();
	}
	catch (CCmdFailed e)
	{
		CCommand* pc = e.GetCommand();
		std::string err = e.GetErrorString();
		if (err.empty()) err = "(unknown)";

		SetErrorString(err);

		// TODO: should I clear redo stack?
		return false;
	}
	catch (...)
	{
		SetErrorString("An unknown error has occurred.");
		return false;
	}

	// add it to the undo stack
	m_Undo.push(pcmd);

	// clear the redo stack
	int N = (int)m_Redo.size();
	for (int i = 0; i<N; i++) { delete m_Redo.top(); m_Redo.pop(); }

	return true;
}

void CBasicCmdManager::UndoCommand()
{
	if (m_Undo.empty() == false)
	{
		// pop the command from the undo stack
		CCommand* pcmd = m_Undo.top(); m_Undo.pop();

		// unexecute it
		pcmd->UnExecute();

		// push it on the redo stack
		m_Redo.push(pcmd);
	}
}

void CBasicCmdManager::RedoCommand()
{
	if (m_Redo.empty() == false)
	{
		// pop the command from the redo stack
		CCommand* pcmd = m_Redo.top(); m_Redo.pop();

		// execute it
		pcmd->Execute();

		// push it on the undo stack
		m_Undo.push(pcmd);
	}
}

void CBasicCmdManager::Clear()
{
	// clear undo stack
	int N = (int)m_Undo.size();
	for (int i = 0; i<N; i++) { delete m_Undo.top(); m_Undo.pop(); }

	// clear redo stack
	N = (int)m_Redo.size();
	for (int i = 0; i<N; i++) { delete m_Redo.top(); m_Redo.pop(); }
}

const char* CBasicCmdManager::GetUndoCmdName() { return (m_Undo.size() ? m_Undo.top()->GetName() : 0); }
const char* CBasicCmdManager::GetRedoCmdName() { return (m_Redo.size() ? m_Redo.top()->GetName() : 0); }

//////////////////////////////////////////////////////////////////////
// CCommandManager
//////////////////////////////////////////////////////////////////////

CCommandManager::CCommandManager(CUndoDocument* pdoc)
{
	m_pDoc = pdoc;
}

CCommandManager::~CCommandManager()
{
	m_pDoc = 0;
}

void CCommandManager::AddCommand(CCommand* pcmd)
{
    CGLDocument* glDoc = dynamic_cast<CGLDocument*>(m_pDoc);
    if(glDoc)
    {
        pcmd->SetViewState(glDoc->GetViewState());
    }
	
	CBasicCmdManager::AddCommand(pcmd);
}

bool CCommandManager::DoCommand(CCommand* pcmd)
{
	CGLDocument* glDoc = dynamic_cast<CGLDocument*>(m_pDoc);
    if(glDoc)
    {
        pcmd->SetViewState(glDoc->GetViewState());
    }

	m_err.clear();

	// execute the command
	try
	{
		pcmd->Execute();
	}
	catch (CCmdFailed e)
	{
		CCommand* pc = e.GetCommand();
		std::string err = e.GetErrorString();
		if (err.empty()) err = "(unknown)";

		SetErrorString(err);

		// TODO: should I clear redo stack?
		return false;
	}
	catch (GObjectException e)
	{
		GObject* po = e.GetGObject();
		if (po)
		{
			po->Update(true);
		}

		SetErrorString("Object exception has occurred");

		return false;
	}
	catch (...)
	{
		SetErrorString("Object exception has occurred");
		return false;
	}
		
	// add it to the undo stack
	m_Undo.push(pcmd);

	// clear the redo stack
	int N = (int)m_Redo.size();
	for (int i=0; i<N; i++) { delete m_Redo.top(); m_Redo.pop(); }

	return true;
}

void CCommandManager::UndoCommand()
{
	// pop the command from the undo stack
	CCommand* pcmd = m_Undo.top(); m_Undo.pop();

	// reset the view state
    CGLDocument* glDoc = dynamic_cast<CGLDocument*>(m_pDoc);
    if(glDoc)
    {
        glDoc->SetViewState(pcmd->GetViewState());
    }

	// unexecute it
	pcmd->UnExecute();

	// push it on the redo stack
	m_Redo.push(pcmd);
}

void CCommandManager::RedoCommand()
{
	// pop the command from the redo stack
	CCommand* pcmd = m_Redo.top(); m_Redo.pop();

	// reset the view state
	CGLDocument* glDoc = dynamic_cast<CGLDocument*>(m_pDoc);
    if(glDoc)
    {
        glDoc->SetViewState(pcmd->GetViewState());
    }

	// execute it
	pcmd->Execute();

	// push it on the undo stack
	m_Undo.push(pcmd);
}
