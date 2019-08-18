#pragma once
#include "GLPlot.h"

namespace Post {

class CGLParticleFlowPlot : public CGLPlot
{
	class FlowParticle
	{
	public:
		FlowParticle(){}
		FlowParticle(const FlowParticle& p)
		{
			m_r = p.m_r;
			m_v = p.m_v;
			m_col = p.m_col;
			m_balive = p.m_balive;
			m_ndeath = p.m_ndeath;
			m_pos = p.m_pos;
			m_vel = p.m_vel;
		}
		void operator =(const FlowParticle& p)
		{
			m_r = p.m_r;
			m_v = p.m_v;
			m_col = p.m_col;
			m_balive = p.m_balive;
			m_ndeath = p.m_ndeath;
			m_pos = p.m_pos;
			m_vel = p.m_vel;
		}

	public:
		vec3f			m_r;		// current particle position
		vec3f			m_v;		// current particle velocity
		GLColor			m_col;		// particles color
		bool			m_balive;	// is particle alive at current position?
		int				m_ndeath;	// time of death
		vector<vec3f>	m_pos;		// particle position at various times
		vector<vec3f>	m_vel;		// particle velocity at various times
	};

public:
	CGLParticleFlowPlot(CGLModel* mdl);

	void Update(int ntime, float dt, bool breset);

	void Render(CGLContext& rc);

	CPropertyList* propertyList();

public:
	int GetVectorType() { return m_nvec; }
	void SetVectorType(int ntype);

	CColorTexture* GetColorMap() { return &m_Col; }

	float StepSize() const { return m_dt; }
	void SetStepSize(float v);

	int SeedTime() const { return m_seedTime; }
	void SetSeedTime(int n);

	bool ShowPath() const { return m_showPath; }
	void ShowPath(bool b) { m_showPath = b; }

	float Threshold() const { return m_vtol; }
	void SetThreshold(float v);

	float Density() const { return m_density; }
	void SetDensity(float v);

protected:
	void UpdateParticles(int ntime);

	void SeedParticles();

	void AdvanceParticles(int t0, int t1);

	vec3f Velocity(const vec3f& r, int ntime, float dt, bool& ok);

	void UpdateParticleState(int ntime);

public:
	void UpdateParticleColors();

private:
	int		m_nvec;	// vector field
	float	m_dt;	// time increment
	float	m_vtol;	// seeding velocity tolerance
	float	m_density;	// seeding density
	CColorTexture	m_Col;	// color map

	bool	m_showPath;

	vector<vec2f>	m_rng;	// nodal ranges
	DataMap<vec3f>	m_map;	// nodal values map
	vec2f			m_crng;	// current range

	int				m_seedTime;	// time the particles begin to flow
	int				m_maxtime;	// the time to which we have evaluated the flow field

	FEFindElement	m_find;

	vector<FlowParticle>	m_particles;
};
}
