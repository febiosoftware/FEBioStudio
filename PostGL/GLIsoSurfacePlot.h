#pragma once
#include "GLPlot.h"
#include "GLWLib/GLWidget.h"
#include "PostLib/DataMap.h"

namespace Post {

class CGLIsoSurfacePlot : public CGLLegendPlot
{
	enum { DATA_FIELD, COLOR_MAP, CLIP, HIDDEN, SLICES, LEGEND, SMOOTH, RANGE_TYPE, USER_MAX, USER_MIN };

public:
	enum RANGE_TYPE {
		RNG_DYNAMIC,
		RNG_STATIC,
		RNG_USER
	};

public:
	CGLIsoSurfacePlot(CGLModel* po);

	int GetSlices();
	void SetSlices(int nslices);

	void Render(CGLContext& rc);

	void Update(int ntime, float dt, bool breset);

	bool RenderSmooth() { return m_bsmooth; }
	void RenderSmooth(bool b) { m_bsmooth = b; }

	int GetEvalField() { return m_nfield; }
	void SetEvalField(int n);

	CColorTexture* GetColorMap() { return &m_Col; }

	bool CutHidden() { return m_bcut_hidden; }
	void CutHidden(bool b) { m_bcut_hidden = b; }

	void UpdateTexture() { m_Col.UpdateTexture(); }

	void SetRangeType(int n) { m_rangeType = n; }
	int GetRangeType() const { return m_rangeType; }

	void SetUserRangeMin(double rangeMin) { m_userMin = rangeMin; }
	double GetUserRangeMin() const { return m_userMin; }

	void SetUserRangeMax(double rangeMax) { m_userMax = rangeMax; }
	double GetUserRangeMax() const { return m_userMax; }

	void Update();

	void UpdateData(bool bsave = true);

protected:
	void RenderSlice(float ref, GLColor col);

protected:
	int		m_nslices;		// nr. of iso surface slices
	bool	m_bsmooth;		// render smooth or not
	bool	m_bcut_hidden;	//!< cut hidden materials or not

	int		m_rangeType;				//!< dynamic, static, or user-defined
	double	m_userMin, m_userMax;		//!< range for user-defined range

	double	m_rngMin, m_rngMax;

	int				m_nfield;	// data field
	CColorTexture	m_Col;		// colormap

	vector<vec2f>	m_rng;	// value range
	DataMap<float>	m_map;	// nodal values map
	VectorMap		m_GMap;	// nodal gradient values map

	vec2f			m_crng;
	vector<float>	m_val;	// current nodal values
	vector<vec3f>	m_grd;	// current gradient values

	int		m_lastTime;
	float	m_lastdt;
};
}
