#pragma once
#include <string>
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

typedef vector<CCommand*> CCmdPtrArray;

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
