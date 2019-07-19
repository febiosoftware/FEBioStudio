#pragma once
#include "Tool.h"

class CPlaneCutTool : public CBasicTool
{
public:
	CPlaneCutTool();

	bool OnApply();

private:
	vec3d	m_r0, m_r1, m_r2;
};
