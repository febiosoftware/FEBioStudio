#pragma once
#include <MeshTools/FEModifier.h>
#include <FEMLib/FEInterface.h>
#include <GeomLib/GObject.h>
#include <FEMLib/FEBoundaryCondition.h>
#include <FEMLib/FEConnector.h>
#include <FEMLib/FELoad.h>
#include <FSCore/ParamBlock.h>
#include <MeshTools/GGroup.h>
#include <MeshTools/GDiscreteObject.h>
#include <FEMLib/FERigidConstraint.h>
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GModifiedObject.h>
#include <MeshTools/FESurfaceModifier.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GLLib/GLCamera.h>
#include "ViewSettings.h"

class CDocument;
class CCommandManager;
class CGLView;
class FEAnalysisStep;

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
	CCommand(const string& name);
	virtual ~CCommand();

	virtual void Execute  () = 0;
	virtual void UnExecute() = 0;

	const char* GetName() const;
	void SetName(const string& name);

	VIEW_STATE GetViewState();

protected:
	static CDocument* m_pDoc;

	// doc/view state variables
	VIEW_STATE	m_state;

	string m_name;	// command name

	friend class CCommandManager;
};

typedef vector<CCommand*> CCmdPtrArray;

//----------------------------------------------------------------
// CCmdGroup
// Command that groups other commands
//
class CCmdGroup : public CCommand
{
public:
	CCmdGroup() : CCommand("Group") {}
	CCmdGroup(const std::string& cmd) : CCommand(cmd) {}
	virtual ~CCmdGroup() { for (int i=0; i<(int) m_Cmd.size(); ++i) delete m_Cmd[i]; }

	void AddCommand(CCommand* pcmd) { m_Cmd.push_back(pcmd); }

	virtual void Execute()
	{
		int N = (int) m_Cmd.size();
		for (int i=0; i<N; i++) m_Cmd[i]->Execute();
	}

	virtual void UnExecute()
	{
		int N = (int)m_Cmd.size();
		for (int i=N-1; i>=0; i--) m_Cmd[i]->UnExecute();
	}

	int GetCount() { return (int)m_Cmd.size(); }

protected:
	CCmdPtrArray	m_Cmd;	// array of pointer to commands
};
