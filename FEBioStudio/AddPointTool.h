#pragma once
#include "Tool.h"
#include <MathLib/math3d.h>

//-----------------------------------------------------------------------------
class CDocument;
class GPointDecoration;

//-----------------------------------------------------------------------------
// This tool measures the angle between three consecutively selected nodes
class CAddPointTool : public CBasicTool
{
public:
	// constructor
	CAddPointTool();

	// activate the tool
	void activate(CMainWindow* wnd);

	// deactive the tool
	void deactivate();

private:
	void UpdateNode();
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	vec3f	m_pos;

	GPointDecoration*	m_deco;

	friend class Props;
};
