#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CTetOverlapTool : public CBasicTool
{
public:
	// constructor
	CTetOverlapTool(CMainWindow* wnd);

	// method called when user presses Apply button (optional)
	bool OnApply();

private:
	int	m_ncount;
};

