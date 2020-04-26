#pragma once
#include <QWidget>

//-----------------------------------------------------------------------------
// forward declarations
class CMainWindow;
class CDocument;

//-----------------------------------------------------------------------------
//! Base class for command panels.
//! A command panel provides tools for displaying and modifying parts of the FE 
//! model.  
class CCommandPanel : public QWidget
{
public:
	//! constructor
	CCommandPanel(CMainWindow* wnd, QWidget* parent = 0);

	//! get the main window
	CMainWindow* GetMainWindow() { return m_wnd; }

	//! Get the main document
	CDocument*	GetDocument();

	//! Update the command panel, since the model has changed
	virtual void Update(bool breset = true);

	//! Process Esc key event (return true if processed)
	virtual bool OnEscapeEvent() { return false; }

	//! Process Del key event (return true if processed)
	virtual bool OnDeleteEvent() { return false; }

	//! Mechanism for programmatically apply a command tool
	virtual void Apply() {}

private:
	CMainWindow*	m_wnd;	//!< pointer to main window
};
