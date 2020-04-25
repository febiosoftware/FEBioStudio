#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CReadCurveTool : public CBasicTool
{
public:
	// constructor
	CReadCurveTool(CMainWindow* wnd);

	bool OnApply();

private:
	bool	m_bcheck;
	QString	m_file;
};
