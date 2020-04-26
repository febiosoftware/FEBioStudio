#pragma once
#include "Tool.h"

class CElementVolumeTool : public CBasicTool
{
public:
	// constructor
	CElementVolumeTool(CMainWindow* wnd);

	// Apply button
	void Update() override;

private:
	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	int		m_nsel;		// selected elements
	double	m_vol;		// volume of selection
	friend class Props;
};