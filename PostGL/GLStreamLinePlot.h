#pragma once
#include "GLPlot.h"
#include "GLWLib/GLWidget.h"

namespace Post {

class CGLStreamLinePlot : public CGLPlot
{
public:
	enum RANGE_TYPE {
		RNG_DYNAMIC,
		RNG_STATIC,
		RNG_USER
	};

	struct StreamPoint
	{
		vec3f		r;
		float		v;
		GLColor		c;
	};

	class StreamLine
	{
	public:
		StreamLine(){}
		StreamLine(const StreamLine& sl)
		{
			m_pt = sl.m_pt;
		}
		void operator = (const StreamLine& sl)
		{
			m_pt = sl.m_pt;
		}

		void Add(const vec3f& r, float w) { StreamPoint p = {r, w, GLColor(0,0,0)}; m_pt.push_back(p); }

		int Points() const { return (int) m_pt.size(); }
		StreamPoint& operator [] (int i) { return m_pt[i]; }

		vector<StreamPoint>	m_pt;
	};

public:
	CGLStreamLinePlot(CGLModel* fem);
	~CGLStreamLinePlot();

	void Render(CGLContext& rc);

	void Update();
	void Update(int ntime, float dt, bool breset);

	CPropertyList* propertyList();

	void UpdateStreamLines();

	void ColorStreamLines();

public:
	int GetVectorType() { return m_nvec; }
	void SetVectorType(int ntype);

	CColorTexture* GetColorMap() { return &m_Col; }

	float StepSize() const { return m_inc; }
	void SetStepSize(float v) { m_inc = v; }

	float Density() const { return m_density; }
	void SetDensity(float v) { m_density = v; }

	float Threshold() const { return m_vtol; }
	void SetThreshold(float v) { m_vtol = v; }

	void SetRangeType(int n) { m_rangeType = n; }
	int GetRangeType() const { return m_rangeType; }

	void SetUserRangeMin(double rangeMin) { m_userMin = rangeMin; }
	double GetUserRangeMin() const { return m_userMin; }

	void SetUserRangeMax(double rangeMax) { m_userMax = rangeMax; }
	double GetUserRangeMax() const { return m_userMax; }

protected:

	vec3f Velocity(const vec3f& r, bool& ok);

private:
	int	m_nvec;	// vector field

	float	m_inc;
	float	m_density;
	float	m_vtol;	// seeding velocity tolerance

	int		m_lastTime;
	float	m_lastdt;

	CColorTexture	m_Col;	// color map

	vector<vec2f>	m_rng;	// nodal ranges
	DataMap<vec3f>	m_map;	// nodal values map

	int				m_ntime;	// current time at which this plot is evaluated
	vector<vec3f>	m_val;	// current nodal values
	vec2f			m_crng;	// current range

	vector<StreamLine>	m_streamLines;
	vector<float>		m_prob;

	FEFindElement	m_find;

	int		m_rangeType;				//!< dynamic, static, or user-defined
	double	m_userMin, m_userMax;		//!< range for user-defined range
	double	m_rngMin, m_rngMax;
	GLLegendBar*	m_pbar;
};
}
