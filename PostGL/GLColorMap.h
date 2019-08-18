#pragma once
#include "GLDataMap.h"
#include "GLWLib/GLWidget.h"
#include "PostLib/GLTexture1D.h"
#include "PostLib/ColorMap.h"

namespace Post {

class CGLModel;

//-----------------------------------------------------------------------------

class CGLColorMap : public CGLDataMap
{
public:
	enum Range_Type {
		RANGE_DYNA	= 0,
		RANGE_STAT  = 1,
		RANGE_USER	= 2
	};

public:
	struct RANGE
	{
		float min;
		float max;
		int	ntype;
	};

public:
	CGLColorMap(CGLModel* po);
	~CGLColorMap();

	Post::CColorTexture* GetColorMap() { return &m_Col; }

	void Update(int ntime, float dt, bool breset);

	int GetEvalField() const { return m_nfield; }
	void SetEvalField(int n);

	void GetRange(float* pd) { pd[0] = m_range.min; pd[1] = m_range.max; }
	int GetRangeType() { return m_range.ntype; }

	void SetRange(float* pd) { m_range.min = pd[0]; m_range.max = pd[1]; }
	void SetRangeMax(float f) { m_range.max = f; }
	void SetRangeMin(float f) { m_range.min = f; }
	void SetRangeType(int n) { m_range.ntype = n; m_breset = true; }

	void DisplayNodalValues(bool b) { m_bDispNodeVals = b; }
	bool DisplayNodalValues() { return m_bDispNodeVals; }

	bool ShowLegend() { return m_pbar->visible(); }
	void ShowLegend(bool b) { if (b) m_pbar->show(); else m_pbar->hide(); }

	bool GetColorSmooth();
	void SetColorSmooth(bool b);

	void Activate(bool b) { CGLObject::Activate(b); ShowLegend(b); }

	CPropertyList* propertyList();

private:
	void UpdateState(int ntime, bool breset);

protected:
	int		m_nfield;
	bool	m_breset;	// reset the range when the field has changed
	RANGE	m_range;	// range for legend

public:
	bool	m_bDispNodeVals;	// render nodal values

	Post::CColorTexture	m_Col;	// colormap used for rendering

	GLLegendBar*	m_pbar;	// the legend bar
};
}
