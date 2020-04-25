#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CSphereFitTool : public CBasicTool
{
public:
	// constructor
	CSphereFitTool(CMainWindow* wnd);

	// method called when user presses Apply button (optional)
	bool OnApply();

private:
	bool	m_bsel;
	double	m_x, m_y, m_z;
	double	m_R;
	double	m_obj;
};
