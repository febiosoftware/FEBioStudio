#pragma once
#include "Tool.h"
#include <MathLib/math3d.h>

//-----------------------------------------------------------------------------
class CDocument;

//-----------------------------------------------------------------------------
// This tool measures the angle between three consecutively selected nodes
class CAddPointTool : public CBasicTool
{
public:
	// constructor
	CAddPointTool(CMainWindow* wnd);

	// update
	void Update() override;

private:
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	vec3f	m_pos;

	friend class Props;
};
