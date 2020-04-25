#pragma once
#include "Tool.h"

class CScalarFieldTool : public CBasicTool
{
public:
	CScalarFieldTool(CMainWindow* wnd);

	bool OnApply();

private:
	QString		m_name;
	int			m_ntype;
	int			m_ngen[2];
	double		m_weight[2];
};
