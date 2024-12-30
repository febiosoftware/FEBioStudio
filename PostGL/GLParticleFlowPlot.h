/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include "GLPlot.h"
#include <PostLib/FEPostMesh.h>
#include <MeshLib/FEFindElement.h>

namespace Post {

class CGLParticleFlowPlot : public CGLPlot
{
	enum { DATA_FIELD, COLOR_MAP, CLIP, SEED_STEP, THRESHOLD, DENSITY, STEP_SIZE, PATH_LINES, PATH_LENGTH };

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
	CGLParticleFlowPlot();

	void Update(int ntime, float dt, bool breset) override;

	void Render(GLRenderEngine& re, GLContext& rc) override;

	bool UpdateData(bool bsave = true) override;

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
	int		m_pathLength;

	vector<vec2f>	m_rng;	// nodal ranges
	DataMap<vec3f>	m_map;	// nodal values map
	vec2f			m_crng;	// current range

	int				m_seedTime;	// time the particles begin to flow
	int				m_maxtime;	// the time to which we have evaluated the flow field

	float	m_lastTime;
	float	m_lastDt;

	FEFindElement*	m_find;

	vector<FlowParticle>	m_particles;
};
}
