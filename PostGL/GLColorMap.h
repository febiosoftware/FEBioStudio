#pragma once
#include "GLDataMap.h"
#include <GLWLib/GLWidget.h>
#include <GLLib/GLTexture1D.h>
#include <PostLib/ColorMap.h>

namespace Post {

class CGLModel;

//-----------------------------------------------------------------------------

class CGLColorMap : public CGLDataMap
{
	enum { DATA_FIELD, DATA_SMOOTH, COLOR_MAP, NODAL_VALS, RANGE_TYPE, RANGE_DIVS, SHOW_LEGEND, LEGEND_ORIENT, USER_MAX, USER_MIN };

public:
	CGLColorMap(CGLModel* po);
	~CGLColorMap();

	Post::CColorTexture* GetColorMap() { return &m_Col; }

	void Update(int ntime, float dt, bool breset) override;

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

	void Activate(bool b) override { CGLObject::Activate(b); ShowLegend(b); }

private:
	void UpdateState(int ntime, bool breset);

	void UpdateData(bool bsave = true) override;

	void Update() override;

protected:
	int		m_nfield;
	bool	m_breset;	// reset the range when the field has changed
	DATA_RANGE	m_range;	// range for legend

public:
	bool	m_bDispNodeVals;	// render nodal values

	Post::CColorTexture	m_Col;	// colormap used for rendering

	GLLegendBar*	m_pbar;	// the legend bar
};
}
