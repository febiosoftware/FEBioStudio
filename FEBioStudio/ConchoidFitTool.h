#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CConchoidFitTool : public CBasicTool
{
public:
	// constructor
	CConchoidFitTool(CMainWindow* wnd);

	bool OnApply();

private:
	bool	m_bsel;
	double	m_x, m_y, m_z;
	double	m_A, m_B;
	double	m_obj;
};
