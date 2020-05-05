#pragma once
#include <stack>
#include <string>

class CCommand;
class CDocument;

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
	CCommandManager(CDocument* pdoc);
	virtual ~CCommandManager();

	void AddCommand(CCommand* pcmd) override;

	bool DoCommand(CCommand* pcmd) override;

	void UndoCommand() override;

	void RedoCommand() override;

protected:
	CDocument* m_pDoc; // pointer to the current document
};
