#pragma once
#include "Tool.h"

class CMaterialMapTool : public CBasicTool
{
public:
	CMaterialMapTool(CMainWindow* wnd);

	bool OnApply();

private:
	QString	m_file;
	int		m_nmat;
};
