#pragma once
#include "Tool.h"
#include <MathLib/mat3d.h>

class CPlaneCutTool : public CBasicTool
{
public:
	CPlaneCutTool(CMainWindow* wnd);

	bool OnApply();

private:
	vec3d	m_r0, m_r1, m_r2;
};
