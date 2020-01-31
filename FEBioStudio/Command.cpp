#include "stdafx.h"
#include "Command.h"
#include "Document.h"
#include "GLView.h"
#include <FEMLib/FEAnalysisStep.h>
#include <GeomLib/GMultiBox.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/FEMesher.h>

CDocument* CCommand::m_pDoc = 0;

//////////////////////////////////////////////////////////////////////
// CCmdAddMesh
//////////////////////////////////////////////////////////////////////

CCommand::CCommand(const string& name)
{
	m_name = name;
	if (m_pDoc) m_state = m_pDoc->GetViewState();
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

VIEW_STATE CCommand::GetViewState() 
{ 
	return m_state; 
}
