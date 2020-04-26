#include "stdafx.h"
#include "CommandPanel.h"
#include "MainWindow.h"

//-----------------------------------------------------------------------------
CCommandPanel::CCommandPanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent), m_wnd(wnd)
{
}

//-----------------------------------------------------------------------------
CDocument*	CCommandPanel::GetDocument()
{
	return m_wnd->GetDocument();
}

//-----------------------------------------------------------------------------
void CCommandPanel::Update(bool b)
{
	// by default, nothing to do
}
