#pragma once
#include "Tool.h"

//-----------------------------------------------------------------------------
class C3PointAngleDecoration;

//-----------------------------------------------------------------------------
// This tool measures the angle between three consecutively selected nodes
class C3PointAngleTool : public CBasicTool
{
public:
	// constructor
	C3PointAngleTool();

	// activate the tool
	void activate(CMainWindow* wnd) override;

	// deactive the tool
	void deactivate();

	void update(bool reset);

private:
	void UpdateAngle();
	QVariant GetPropertyValue(int i) override;
	void SetPropertyValue(int i, const QVariant& v) override;

private:
	int		m_node[3];
	double	m_angle;

	C3PointAngleDecoration*	m_deco;

	friend class Props;
};
