#pragma once
#include "Tool.h"
#include <vector>

//-----------------------------------------------------------------------------
class FEFace;

//-----------------------------------------------------------------------------
// This tool measures the angle between three consecutively selected nodes
class CMeasureAreaTool : public CBasicTool
{
public:
	// constructor
	CMeasureAreaTool(CMainWindow* wnd);

	// Apply button
	void Update() override;

private:
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	int		m_nsel;		// selected faces
	double	m_area;		// area of selection
	friend class Props;
};
