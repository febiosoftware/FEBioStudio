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

#include "stdafx.h"
#include "GLParticleFlowPlot.h"
#include "GLModel.h"
#include <FSCore/ClassDescriptor.h>
#include <GLLib/GLMesh.h>
using namespace Post;

REGISTER_CLASS(CGLParticleFlowPlot, CLASS_PLOT, "particle-flow", 0);

CGLParticleFlowPlot::CGLParticleFlowPlot()
{
	SetTypeString("particle-flow");

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "ParticleFlow.%02d", n++);
	SetName(szname);

	AddIntParam(0, "data_field")->SetEnumNames("@data_vec3");
	AddIntParam(0, "color_map")->SetEnumNames("@color_map");
	AddBoolParam(true, "allow_clipping");
	AddIntParam(0, "seed_step");
	AddDoubleParam(0, "velocity_threshold");
	AddDoubleParam(0, "seeding_density")->SetFloatRange(0.0, 1.0, 0.01);
	AddDoubleParam(0, "step_size");
	AddBoolParam(false, "show_path_lines");
	AddIntParam(0, "path_line_length");

	m_find = nullptr;

	m_nvec = -1;
	m_showPath = false;
	m_pathLength = 0; // 0 means all time steps
	m_vtol = 1e-5f;
	m_density = 1.f;

	m_maxtime = -1;
	m_seedTime = 1;
	m_dt = 0.01f;

	m_lastTime = 0.f;
	m_lastDt = 1.f;

	UpdateData(false);
}

bool CGLParticleFlowPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		// store some parameters that require rebuilding the entire plot
		int oldvec = m_nvec;
		int oldSeedtime = m_seedTime;
		float oldDt = m_dt;
		float oldVtol = m_vtol;
		float oldDensity = m_density;

		m_nvec = GetIntValue(DATA_FIELD);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		AllowClipping(GetBoolValue(CLIP));
		m_seedTime = GetIntValue(SEED_STEP);
		m_vtol = GetFloatValue(THRESHOLD);
		m_density = GetFloatValue(DENSITY);
		m_dt = GetFloatValue(STEP_SIZE);
		m_showPath = GetBoolValue(PATH_LINES);
		m_pathLength = GetIntValue(PATH_LENGTH);

		bool update = false;
		if (oldvec != m_nvec) update = true;
		if (oldSeedtime != m_seedTime) update = true;
		if (oldVtol != m_vtol) update = true;
		if (oldDt != m_dt) update = true;
		if (oldDensity != m_density) update = true;

		if (update)
		{
			Update(m_lastTime, m_lastDt, true);
		}
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nvec);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(CLIP, AllowClipping());
		SetIntValue(SEED_STEP, m_seedTime);
		SetFloatValue(THRESHOLD, m_vtol);
		SetFloatValue(DENSITY, m_density);
		SetFloatValue(STEP_SIZE, m_dt);
		SetBoolValue(PATH_LINES, m_showPath);
		SetIntValue(PATH_LENGTH, m_pathLength);
	}

	return false;
}

void CGLParticleFlowPlot::SetVectorType(int ntype)
{
	m_nvec = ntype;
	Update(GetModel()->CurrentTimeIndex(), 0.0, true);
}

void CGLParticleFlowPlot::SetStepSize(float v)
{
	m_dt = v; 
	Update(GetModel()->CurrentTimeIndex(), 0.0, true);
}

void CGLParticleFlowPlot::SetSeedTime(int n)
{
	if (n < 0) n = 0;
	m_seedTime = n;
	Update(GetModel()->CurrentTimeIndex(), 0.0, true);
}

void CGLParticleFlowPlot::SetThreshold(float v)
{
	m_vtol = v;
	Update(GetModel()->CurrentTimeIndex(), 0.0, true);
}

void CGLParticleFlowPlot::SetDensity(float v)
{
	m_density = v;
	Update(GetModel()->CurrentTimeIndex(), 0.0, true);
}

void CGLParticleFlowPlot::Render(CGLContext& rc)
{
	int NP = (int) m_particles.size();
	if (NP == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);

	// build a point mesh
	GLPointMesh mesh(NP, GLMesh::FLAG_COLOR);
	mesh.BeginMesh();
	for (int i=0; i<NP; ++i)
	{
		FlowParticle& p = m_particles[i];
		if (p.m_balive) mesh.AddVertex(p.m_r, p.m_col);
	}
	mesh.EndMesh();

	// render the points
	mesh.Render();

	if (m_showPath)
	{
		int ntime = GetModel()->CurrentTimeIndex();
		if (ntime >= m_seedTime + 1)
		{
			// count line segments
			int lines = 0;
			for (int i = 0; i < NP; ++i)
			{
				FlowParticle& p = m_particles[i];

				int tend = ntime;
				if (tend > p.m_ndeath) tend = p.m_ndeath;

				int n0 = m_seedTime;
				if (m_pathLength > 0)
				{
					n0 = ntime - m_pathLength;
					if (n0 < m_seedTime) n0 = m_seedTime;
					if (n0 > tend) n0 = tend;
				}

				lines += tend - n0 + 1;
			}

			// allocate line mesh
			GLLineMesh lineMesh(lines);

			glColor3ub(0,0,255);
			lineMesh.BeginMesh();
			for (int i = 0; i<NP; ++i)
			{
				FlowParticle& p = m_particles[i];

				int tend = ntime;
				if (tend > p.m_ndeath) tend = p.m_ndeath;

				int n0 = m_seedTime;
				if (m_pathLength > 0)
				{
					n0 = ntime - m_pathLength;
					if (n0 < m_seedTime) n0 = m_seedTime;
					if (n0 > tend) n0 = tend;
				}

				for (int n=n0; n<tend; ++n)
				{
					vec3f& r0 = p.m_pos[n];
					vec3f& r1 = p.m_pos[n+1];
					lineMesh.AddVertex(r0);
					lineMesh.AddVertex(r1);
				}					
			}
			lineMesh.EndMesh();

			// render the lines
			lineMesh.Render();
		}
	}

	glPopAttrib();
}

void CGLParticleFlowPlot::Update(int ntime, float dt, bool breset)
{
	m_lastTime = ntime;
	m_lastDt = dt;

	if (breset) { m_map.Clear(); m_rng.clear(); m_maxtime = -1; m_particles.clear(); }
	if (m_nvec == -1) return;

	CGLModel* mdl = GetModel();

	// see if we need to revaluate the FEFindElement object
	// We evaluate it when the plot needs to be reset, or when the model has a displacement map
	bool bdisp = mdl->HasDisplacementMap();
	if (breset || bdisp)
	{
		if (m_find == nullptr) m_find = new FEFindElement(*mdl->GetActiveMesh());
		// choose reference frame or current frame, depending on whether we have a displacement map
		m_find->Init(bdisp ? 1 : 0);
	}

	FSMeshBase* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	if (m_map.States() == 0)
	{
		int NS = pfem->GetStates();
		int NN = pm->Nodes();

		m_map.Create(NS, NN, vec3f(0, 0, 0), -1);
		m_rng.resize(NS);
	}

	// check the tag
	int ntag = m_map.GetTag(ntime);

	// see if we need to update
	if (ntag != m_nvec)
	{
		// check all states up until this time
		for (int n=0; n<=ntime; ++n)
		{
			ntag = m_map.GetTag(n);
			if (ntag != m_nvec)
			{
				m_map.SetTag(n, m_nvec);

				// get the state we are interested in
				vector<vec3f>& val = m_map.State(n);

				vec2f& rng = m_rng[n];
				rng.x = rng.y = 0;

				float L;

				for (int i = 0; i<pm->Nodes(); ++i)
				{
					val[i] = pfem->EvaluateNodeVector(i, n, m_nvec);
					L = val[i].Length();
					if (L > rng.y) rng.y = L;
				}

				if (rng.y == rng.x) ++rng.y;
			}
		}
	}

	// copy nodal values
	m_crng = m_rng[ntime];

	// update particles
	UpdateParticles(ntime);
}

void CGLParticleFlowPlot::UpdateParticles(int ntime)
{
	if (ntime < m_seedTime)
	{
		// deactivate all particles
		for (int i=0; i<m_particles.size(); ++i) m_particles[i].m_balive = false;
		return;
	}

	if (ntime > m_maxtime)
	{
		// perform particle integration
		if (m_maxtime < m_seedTime)
		{
			// seed the particles
			SeedParticles();
			m_maxtime = m_seedTime;
		}

		// advance the particles from maxtime to this time
		AdvanceParticles(m_maxtime, ntime);
		m_maxtime = ntime;
	}

	// update current state of the particles
	UpdateParticleState(ntime);
}

void CGLParticleFlowPlot::UpdateParticleState(int ntime)
{
	int NP = (int) m_particles.size();
	for (int i=0; i<NP; ++i)
	{
		FlowParticle& p = m_particles[i];

		if (ntime >= p.m_ndeath)
		{
			p.m_balive = false;
		}
		else
		{
			p.m_balive = true;
			p.m_r = p.m_pos[ntime];
			p.m_v = p.m_vel[ntime];
		}
	}

	UpdateParticleColors();
}

void CGLParticleFlowPlot::UpdateParticleColors()
{
	float vmin = m_crng.x;
	float vmax = m_crng.y;
	if (vmax == vmin) vmax++;

	int ncol = m_Col.GetColorMap();
	CColorMap& col = ColorMapManager::GetColorMap(ncol);

	int NP = (int)m_particles.size();
	for (int i = 0; i<NP; ++i)
	{
		FlowParticle& p = m_particles[i];
		if (p.m_balive)
		{
			vec3f& v = p.m_v;
			float V = v.Length();
			float w = (V - vmin) / (vmax - vmin);
			p.m_col = col.map(w);
		}
	}
}

vec3f CGLParticleFlowPlot::Velocity(const vec3f& r, int ntime, float w, bool& ok)
{
	vec3f v(0.f, 0.f, 0.f);
	vec3f ve0[FSElement::MAX_NODES];
	vec3f ve1[FSElement::MAX_NODES];
	FEPostMesh& mesh = *GetModel()->GetActiveMesh();

	vector<vec3f>& val0 = m_map.State(ntime    );
	vector<vec3f>& val1 = m_map.State(ntime + 1);

	int nelem;
	double q[3];
	if (m_find->FindElement(r, nelem, q))
	{
		ok = true;
		FEElement_& el = mesh.ElementRef(nelem);

		int ne = el.Nodes();
		for (int i = 0; i<ne; ++i)
		{
			ve0[i] = val0[el.m_node[i]];
			ve1[i] = val1[el.m_node[i]];
		}

		vec3f v0 = el.eval(ve0, q[0], q[1], q[2]);
		vec3f v1 = el.eval(ve1, q[0], q[1], q[2]);

		v = v0*(1.f - w) + v1*w;
	}
	else ok = false;

	return v;
}

void CGLParticleFlowPlot::AdvanceParticles(int n0, int n1)
{
	// get the model
	CGLModel* mdl = GetModel();
	if (mdl == 0) return;
	FEPostModel& fem = *mdl->GetFSModel();

	// get the mesh
	FSMeshBase& mesh = *mdl->GetActiveMesh();

	BOX box = m_find->BoundingBox();
	float R = box.GetMaxExtent();
	float dt = m_dt;
	if (dt <= 0.f) return;

	for (int ntime=n0; ntime<n1; ++ntime)
	{
		float t0 = fem.GetState(ntime    )->m_time;
		float t1 = fem.GetState(ntime + 1)->m_time;
		if (t1 < t0) t1 = t0;

		int NP = (int)m_particles.size();
#pragma omp parallel for shared (NP)
		for (int i = 0; i<NP; ++i)
		{
			FlowParticle& p = m_particles[i];
			p.m_pos[ntime+1] = p.m_pos[ntime];
			p.m_vel[ntime+1] = p.m_vel[ntime];
		}

		float t = t0;
		while (t < t1)
		{
			t += dt;
			if (t > t1) t = t1;
			float w = (t - t0) / (t1 - t0);

			int NP = (int) m_particles.size();
#pragma omp parallel for shared (NP)
			for (int i=0; i<NP; ++i)
			{
				FlowParticle& p = m_particles[i];

				if (p.m_ndeath > ntime)
				{
					vec3f r0 = p.m_pos[ntime + 1];
					vec3f v0 = p.m_vel[ntime + 1];

					vec3f r1 = r0 + v0*dt;

					bool ok = true;
					vec3f v1 = Velocity(r1, ntime, w, ok);
					if (ok == false)
					{
						p.m_ndeath = ntime + 1;
					}
					else
					{
						p.m_pos[ntime + 1] = r1;
						p.m_vel[ntime + 1] = v1;
					}
				}
			}
		}
	}
}

static float frand()
{
	return rand() / (float) RAND_MAX;
}

void CGLParticleFlowPlot::SeedParticles()
{
	// clear current particles, if any
	m_particles.clear();

	// get the model
	CGLModel* mdl = GetModel();
	if (mdl == 0) return;

	// get the number of states
	FEPostModel* fem = mdl->GetFSModel();
	int NS = fem->GetStates();

	// make sure there is a valid seed time
	if ((m_seedTime < 0) || (m_seedTime >= NS)) return;

	// get the mesh
	FSMeshBase& mesh = *mdl->GetActiveMesh();

	vector<vec3f>& val = m_map.State(m_seedTime);

	// make sure vtol is positive
	float vtol = fabs(m_vtol);

	// loop over all the surface facts
	int NF = mesh.Faces();
#pragma omp parallel for shared (NF)
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);

		// evaluate the average velocity at this face
		int nf = f.Nodes();
		vec3f vf(0.f, 0.f, 0.f);
		for (int j = 0; j<nf; ++j) vf += val[f.n[j]];
		vf /= nf;

		// generate random number
		float w = frand();

		// see if this is a valid candidate for a seed
		vec3f fn = f.m_fn;
		if ((fn*vf < -vtol) && (w <= m_density))
		{
			// calculate the face center, this will be the seed
			// NOTE: We are using reference coordinates, therefore we assume that the mesh is not deforming!!
			vec3d cf(0.f, 0.f, 0.f);
			for (int j = 0; j<nf; ++j) cf += mesh.Node(f.n[j]).r;
			cf /= nf;

			// create a particle here
			FlowParticle p;
			p.m_pos.resize(NS);
			p.m_vel.resize(NS);
			p.m_balive = true;
			p.m_ndeath = NS;	// assume the particle will live the entire time

			// set initial position and velocity
			p.m_pos[m_seedTime] = to_vec3f(cf);
			p.m_vel[m_seedTime] = vf;

			// add it to the pile
#pragma omp critical
            {
                m_particles.push_back(p);
            }
		}
	}
}
