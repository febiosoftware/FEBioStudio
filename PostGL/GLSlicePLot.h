#pragma once
#include "GLPlot.h"
#include "GLWLib/GLWidget.h"
#include "PostLib/DataMap.h"

namespace Post {

class CGLSlicePlot : public CGLLegendPlot  
{
	enum { DATA_FIELD, COLOR_MAP, CLIP, SHOW_LEGEND, SLICES, SLICE_OFFSET, RANGE, RANGE_MAX, RANGE_MIN, NORMAL_X, NORMAL_Y, NORMAL_Z };

public:
	CGLSlicePlot(CGLModel* po);

	void SetBoundingBox(BOX box) { m_box = box; }

	int GetSlices();
	void SetSlices(int nslices);

	vec3f GetPlaneNormal() { return m_norm; }
	void SetPlaneNormal(vec3f& n) { m_norm = n; }

	void Render(CGLContext& rc) override;

	int GetEvalField() { return m_nfield; }
	void SetEvalField(int n);

	CColorTexture* GetColorMap() { return &m_Col; }

	void Update(int ntime, float dt, bool breset) override;

	void SetRangeType(int ntype) { m_nrange = ntype; }
	int GetRangeType() const { return m_nrange; }

	float GetUserRangeMax() const { return m_fmax; }
	void SetUserRangeMax(float f) { m_fmax = f; }
	float GetUserRangeMin() const { return m_fmin; }
	void SetUserRangeMin(float f) { m_fmin = f; }

	float GetSliceOffset() const { return m_offset; }
	void SetSliceOffset(float f);

	void UpdateTexture() override { m_Col.UpdateTexture(); }

	void UpdateData(bool bsave = true) override;

	void Update() override;

protected:
	void RenderSlice(float ref);

protected:
	int			m_nslices;	// nr. of iso surface slices
	BOX			m_box;		// box to use for slices	
	vec3f		m_norm;

	int			m_nrange;		//!< range option (0=dynamic, 1=user)
	float		m_fmin, m_fmax;	//!< user-defined range 
	float		m_offset;

	int				m_nfield;	// data field
	CColorTexture	m_Col;		// colormap

	vector<vec2f>	m_rng;	// value range
	DataMap<float>	m_map;	// nodal values map
	vector<float>	m_val;	// current nodal values
	vec2f			m_crng;	// current range

	int m_lastTime;
	float	m_lastDt;
};
}
