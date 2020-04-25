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
	C3PointAngleTool(CMainWindow* wnd);

	// update
	void Update();

private:
	QVariant GetPropertyValue(int i) override;
	void SetPropertyValue(int i, const QVariant& v) override;
	void addPoint(int n);

private:
	int		m_node[3];
	double	m_angle;

	friend class Props;
};
