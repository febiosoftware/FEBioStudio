// CommandManager.cpp: implementation of the CCommandManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "Document.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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

//////////////////////////////////////////////////////////////////////
// CCommandManager
//////////////////////////////////////////////////////////////////////

CCommandManager::CCommandManager(CDocument* pdoc)
{
	m_pDoc = pdoc;

	if (pdoc) CCommand::m_pDoc = pdoc;
}

CCommandManager::~CCommandManager()
{
	m_pDoc = 0;
}

bool CCommandManager::DoCommand(CCommand* pcmd)
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
	if (m_pDoc) m_pDoc->SetViewState(pcmd->GetViewState());

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
	if (m_pDoc) m_pDoc->SetViewState(pcmd->GetViewState());

	// execute it
	pcmd->Execute();

	// push it on the undo stack
	m_Undo.push(pcmd);
}
