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
	C4PointAngleTool();

	// activate the tool
	void activate(CMainWindow* wnd);

	// deactive the tool
	void deactivate();

	void update(bool breset);

private:
	void UpdateAngle();
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	int		m_node[4];
	double	m_angle;

	C4PointAngleDecoration*	m_deco;

	friend class Props;
};
