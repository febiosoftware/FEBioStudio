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
#include "Command.h"

//////////////////////////////////////////////////////////////////////
// CCmdAddMesh
//////////////////////////////////////////////////////////////////////

CCommand::CCommand(const std::string& name)
{
	m_name = name;
	m_flags = MODIFIES_DOC;
}

CCommand::~CCommand() 
{

}

const char* CCommand::GetName() const 
{ 
	return m_name.c_str(); 
}

void CCommand::SetName(const std::string& name)
{ 
	m_name = name; 
}

void CCommand::SetViewState(VIEW_STATE state)
{
	m_state = state;
}

VIEW_STATE CCommand::GetViewState() 
{ 
	return m_state; 
}

//=============================================================================

CCmdGroup::CCmdGroup() : CCommand("Group") {}

CCmdGroup::CCmdGroup(const std::string& cmd) : CCommand(cmd) {}


CCmdGroup::~CCmdGroup() { for (int i = 0; i<(int)m_Cmd.size(); ++i) delete m_Cmd[i]; }

void CCmdGroup::AddCommand(CCommand* pcmd) { m_Cmd.push_back(pcmd); }

void CCmdGroup::Execute()
{
	int N = (int)m_Cmd.size();
	for (int i = 0; i<N; i++) m_Cmd[i]->Execute();
}

void CCmdGroup::UnExecute()
{
	int N = (int)m_Cmd.size();
	for (int i = N - 1; i >= 0; i--) m_Cmd[i]->UnExecute();
}

int CCmdGroup::GetCount() const { return (int)m_Cmd.size(); }

void CCmdGroup::SetViewState(VIEW_STATE state)
{
	CCommand::SetViewState(state);
	for (int i = 0; i < m_Cmd.size(); i++) m_Cmd[i]->SetViewState(state);
}
