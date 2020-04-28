#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class CPointCongruencyTool : public CBasicTool
{
public:
	// constructor
	CPointCongruencyTool(CMainWindow* wnd);

	bool OnApply() override;

private:
	int		m_node;
	double	m_smooth;
	int		m_face;

	double	m_H1, m_H2;
	double	m_G1, m_G2;
	double	m_alpha, m_delta;
	double	m_Kemin, m_Kemax, m_cong;
};
