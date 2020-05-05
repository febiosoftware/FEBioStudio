#include "stdafx.h"
#include "Command.h"
#include "Document.h"
#include "GLView.h"
#include <FEMLib/FEAnalysisStep.h>
#include <GeomLib/GMultiBox.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/FEMesher.h>

//////////////////////////////////////////////////////////////////////
// CCmdAddMesh
//////////////////////////////////////////////////////////////////////

CCommand::CCommand(const string& name)
{
	m_name = name;
}

CCommand::~CCommand() 
{

}

const char* CCommand::GetName() const 
{ 
	return m_name.c_str(); 
}

void CCommand::SetName(const string& name) 
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
