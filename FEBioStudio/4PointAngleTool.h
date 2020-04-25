#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class C4PointAngleDecoration;

//-----------------------------------------------------------------------------
// This tool measures the angle between four consecutively selected nodes
class C4PointAngleTool : public CBasicTool
{
public:
	// constructor
	C4PointAngleTool(CMainWindow* wnd);

	// update
	void Update() override;

private:
	void UpdateAngle();
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	int		m_node[4];
	double	m_angle;

	friend class Props;
};
