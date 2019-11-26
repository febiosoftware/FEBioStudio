#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CTetOverlapTool : public CBasicTool
{
public:
	// constructor
	CTetOverlapTool();

	// method called when user presses Apply button (optional)
	bool OnApply();

private:
	int	m_ncount;
};

