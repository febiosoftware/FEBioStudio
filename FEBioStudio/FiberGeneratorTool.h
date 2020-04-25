#pragma once
#include "Tool.h"

class CFiberGeneratorTool : public CBasicTool
{
public:
	CFiberGeneratorTool(CMainWindow* wnd);

	bool OnApply();

private:
	int	m_ndata;
	int	m_niter;
};
