#pragma once
#include "Tool.h"
#include <vector>

//-----------------------------------------------------------------------------
class CDocument;
class FEFace;

namespace Post {
	class FEState;
}

//-----------------------------------------------------------------------------
// This tool measures the angle between three consecutively selected nodes
class CMeasureAreaTool : public CBasicTool
{
public:
	// constructor
	CMeasureAreaTool();

	// Apply button
	bool OnApply() override;

private:
	double getValue(Post::FEState* state, const std::vector<FEFace*>& selection);

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	int		m_nsel;		// selected faces
	double	m_area;		// area of selection

	bool	m_bfilter;
	double	m_minFilter;
	double	m_maxFilter;
	bool	m_allSteps;

	friend class Props;
};
