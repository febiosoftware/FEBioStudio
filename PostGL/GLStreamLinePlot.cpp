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
#include "GLStreamLinePlot.h"
#include "GLWLib/GLWidgetManager.h"
#include "GLModel.h"
#include <MeshLib/MeshTools.h>
#include <FSCore/ClassDescriptor.h>
using namespace Post;

//=================================================================================================

REGISTER_CLASS(CGLStreamLinePlot, CLASS_PLOT, "streamlines", 0);

CGLStreamLinePlot::CGLStreamLinePlot()
{
	SetTypeString("streamlines");

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "StreamLines.%02d", n++);
	SetName(szname);

	AddIntParam(0, "data_field")->SetEnumNames("@data_vec3");
	AddIntParam(0, "color_map" )->SetEnumNames("@color_map");
	AddBoolParam(true, "allow_clipping");
	AddDoubleParam(0, "step_size");
	AddDoubleParam(0, "density")->SetFloatRange(0.0, 1.0, 0.01);
	AddDoubleParam(0, "velocity_threshold");
	AddIntParam(0, "range_type")->SetEnumNames("Dynamic\0Static\0User\0");
	AddIntParam(0, "range_divisions")->SetIntRange(1, 100);
	AddDoubleParam(0., "user_range_max");
	AddDoubleParam(0., "user_range_min");

	m_find = nullptr;

	m_density = 1.f;
	m_vtol = 1e-5f;

	m_nvec = -1;
	m_inc = 0.01f;

	m_rangeType = RNG_DYNAMIC;
	m_userMin = 0.;
	m_userMax = 1.;
	m_rngMin = 0.;
	m_rngMax = 1.;

	m_lastTime = 0;
	m_lastdt = 1.f;

	m_Col.SetDivisions(10);

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 600, 100, GLLegendBar::ORIENT_HORIZONTAL);
	bar->align(GLW_ALIGN_BOTTOM | GLW_ALIGN_HCENTER);
	bar->SetType(GLLegendBar::GRADIENT);
	bar->copy_label(szname);
	bar->ShowTitle(true);
	SetLegendBar(bar);

	UpdateData(false);
}

void CGLStreamLinePlot::Update()
{
	Update(m_lastTime, m_lastdt, true);
}

bool CGLStreamLinePlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_nvec = GetIntValue(DATA_FIELD);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		AllowClipping(GetBoolValue(CLIP));
		m_inc = GetFloatValue(STEP_SIZE);
		m_density = GetFloatValue(DENSITY);
		m_vtol = GetFloatValue(THRESHOLD);
		m_rangeType = GetIntValue(RANGE);
		m_Col.SetDivisions(GetIntValue(DIVS));
		m_userMax = GetFloatValue(USER_MAX);
		m_userMin = GetFloatValue(USER_MIN);
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nvec);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(CLIP, AllowClipping());
		SetFloatValue(STEP_SIZE, m_inc);
		SetFloatValue(DENSITY, m_density);
		SetFloatValue(THRESHOLD, m_vtol);
		SetIntValue(RANGE, m_rangeType);
		SetIntValue(DIVS, m_Col.GetDivisions());
		SetFloatValue(USER_MAX, m_userMax);
		SetFloatValue(USER_MIN, m_userMin);
	}

	return false;
}

void CGLStreamLinePlot::SetVectorType(int ntype)
{
	m_nvec = ntype;
	Update();
}

void CGLStreamLinePlot::Render(CGLContext& rc)
{
	int NSL = (int)m_streamLines.size();
	if (NSL == 0) return;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_1D);

	m_mesh.Render(GLMesh::FLAG_COLOR);

	glPopAttrib();
}

static float frand()
{
	return rand() / (float)RAND_MAX;
}

void CGLStreamLinePlot::Update(int ntime, float dt, bool breset)
{
	m_lastTime = ntime;
	m_lastdt = dt;

	CGLModel* mdl = GetModel();
	FSMeshBase* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	if (breset) { m_map.Clear(); m_rng.clear(); m_val.clear(); m_prob.clear(); }

	// see if we need to revaluate the FEFindElement object
	// We evaluate it when the plot needs to be reset, or when the model has a displacement map
	bool bdisp = mdl->HasDisplacementMap();
	if (breset || bdisp)
	{
		if (m_find == nullptr) m_find = new FEFindElement(*mdl->GetActiveMesh());
		// choose reference frame or current frame, depending on whether we have a displacement map
		m_find->Init(bdisp ? 1 : 0);
	}

	if (m_map.States() == 0)
	{
		int NS = pfem->GetStates();
		int NN = pm->Nodes();

		m_map.Create(NS, NN, vec3f(0, 0, 0), -1);
		m_rng.resize(NS);
		m_val.resize(NN);
	}

	if (m_prob.empty())
	{
		FSMeshBase& mesh = *pfem->GetFEMesh(0);
		int NF = mesh.Faces();
		m_prob.resize(NF);
		for (int i=0; i<NF; ++i) m_prob[i] = frand();
	}

	// check the tag
	int ntag = m_map.GetTag(ntime);

	// see if we need to update
	if (ntag != m_nvec)
	{
		m_map.SetTag(ntime, m_nvec);

		// get the state we are interested in
		vector<vec3f>& val = m_map.State(ntime);

		vec2f& rng = m_rng[ntime];
		rng.x = rng.y = 0;

		float L;

		for (int i = 0; i<pm->Nodes(); ++i)
		{
			val[i] = pfem->EvaluateNodeVector(i, ntime, m_nvec);
			L = val[i].Length();
			if (L > rng.y) rng.y = L;
		}

		if (rng.y == rng.x) ++rng.y;
	}

	// copy nodal values
	m_val = m_map.State(ntime);
	m_crng = m_rng[ntime];

	// update static range
	if (breset) { m_rngMin = m_crng.x; m_rngMax = m_crng.y; }
	else
	{
		if (m_crng.x < m_rngMin) m_rngMin = m_crng.x;
		if (m_crng.y > m_rngMax) m_rngMax = m_crng.y;
	}

	// set range for color map
	switch (m_rangeType)
	{
	case RNG_DYNAMIC:
		m_crng = m_rng[ntime];
		break;
	case RNG_STATIC:
		m_crng = vec2f((float)m_rngMin, (float)m_rngMax);
		break;
	case RNG_USER:
		m_crng = vec2f((float)m_userMin, (float)m_userMax);
		break;
	}

	GetLegendBar()->SetRange(m_crng.x, m_crng.y);

	// update stream lines
	UpdateStreamLines();

	// update the mesh
	UpdateMesh();
}

vec3f CGLStreamLinePlot::Velocity(const vec3f& r, bool& ok)
{
	vec3f v(0.f, 0.f, 0.f);
	vec3f ve[FSElement::MAX_NODES];
	FEPostMesh& mesh = *GetModel()->GetActiveMesh();
	int nelem;
	double q[3];
	if (m_find->FindElement(r, nelem, q))
	{
		ok = true;
		FEElement_& el = mesh.ElementRef(nelem);

		int ne = el.Nodes();
		for (int i=0; i<ne; ++i) ve[i] = m_val[el.m_node[i]];

		v = el.eval(ve, q[0], q[1], q[2]);
	}
	else ok = false;

	return v;
}

void CGLStreamLinePlot::UpdateStreamLines()
{
	// clear current stream lines
	m_streamLines.clear();

	// make sure we have a valid vector field
	if (m_nvec == -1) return;

	// get the model
	CGLModel* mdl = GetModel();
	if (mdl == 0) return;

	if (m_inc < 1e-6) return;

	int MAX_POINTS = 2*(int)(1.f / m_inc);
	if (MAX_POINTS > 10000) MAX_POINTS = 10000;

	// make sure vtol is positive
	float vtol = fabs(m_vtol);

	// get the mesh
	FEPostMesh& mesh = *mdl->GetActiveMesh();

	BOX box = m_find->BoundingBox();
	float R = box.GetMaxExtent();
	float maxStep = m_inc*R;

	// tag all elements
	mesh.TagAllElements(0);

	// use the same seed
	srand(0);

	// loop over all the surface facts
	double q[3];
	int NF = mesh.Faces();
#pragma omp parallel for shared (NF)
	for (int i=0; i<NF; ++i)
	{
		FSFace& f = mesh.Face(i);

		// evaluate the average velocity at this face
		int nf = f.Nodes();
		vec3f vf(0.f, 0.f, 0.f);
		for (int j=0; j<nf; ++j) vf += m_val[f.n[j]];
		vf /= nf;

		// see if this is a valid candidate for a seed
		vec3f fn = f.m_fn;
		if ((fn*vf < -vtol) && (m_prob[i] <= m_density))
		{
			// calculate the face center, this will be the seed
			// NOTE: We are using reference coordinates, therefore we assume that te mesh is not deforming!!
			vec3f cf(0.f, 0.f, 0.f);
			for (int j = 0; j<nf; ++j) cf += to_vec3f(mesh.Node(f.n[j]).r);
			cf /= nf;

			// project the seed into the adjacent solid element
			int nelem = f.m_elem[0].eid;
			FEElement_* el = &mesh.ElementRef(nelem);
			el->m_ntag = 1;
			ProjectInsideReferenceElement(mesh, *el, cf, q);

			double V = vf.Length();

			// now, propagate the seed and form the stream line
			StreamLine l;
			l.Add(cf, V);

			vec3f vc = vf;

			vec3f v[FSElement::MAX_NODES];
			bool ok;
			do
			{
				// make sure the velocity is not zero, otherwise we'll be stuck
				double V = vc.Length();
				if (V < 1e-5f) break;

				// "time" increment
				float dt =  maxStep / V;

				// propagate the seed
				// Euler's method
//				cf += vc*dt;

				// RK2 method
/*				vec3f p = cf + vc*dt;
				vec3f vp = Velocity(p, ok);
				if (ok == false) break;
				cf += (vc + vp)*(dt*0.5f);
*/
				// RK4
				vec3f dr(0.f, 0.f, 0.f);
				do
				{
					vec3f a = vc*dt;
					vec3f b = Velocity(cf + a*0.5f, ok)*dt; if (ok == false) break;
					vec3f c = Velocity(cf + b*0.5f, ok)*dt; if (ok == false) break;
					vec3f d = Velocity(cf + c     , ok)*dt; if (ok == false) break;

					dr = (a + b*2.f + c*2.f + d) / 6.0;
					float DR = dr.Length();
					if (DR > 2.0f*maxStep) dt *= 0.5f; else break;
				}
				while (1);
				if (ok == false) break;

				cf += dr;

				// add it to the stream line
				l.Add(cf, V);

				// if for some reason we're stuck, we'll set a max nr of points
				if (l.Points() > MAX_POINTS) break;

				// get velocity at new point
				vc = Velocity(cf, ok);
				if (ok == false) break;
			}
			while (1);

			if (l.Points() > 2)
#pragma omp critical
			{
				m_streamLines.push_back(l);
			}
		}
	}

	// evaluate the color of stream lines
	ColorStreamLines();
}

void CGLStreamLinePlot::ColorStreamLines()
{
	// get the range
	float Vmin = m_crng.x;
	float Vmax = m_crng.y;
	if (Vmax == Vmin) Vmax++;

	int ncol = m_Col.GetColorMap();
	CColorMap& col = ColorMapManager::GetColorMap(ncol);

	int NSL = (int)m_streamLines.size();
	for (int i=0; i<NSL; ++i)
	{
		StreamLine& sl = m_streamLines[i];
		int NP = sl.Points();
		for (int j=0; j<NP; ++j)
		{
			StreamPoint& pt = sl[j];

			float V = pt.v;
			float w = (V - Vmin) / (Vmax - Vmin);
			pt.c = col.map(w);
		}
	}
}

void CGLStreamLinePlot::UpdateMesh()
{
	int NSL = (int)m_streamLines.size();

	// count vertices
	int verts = 0;
	for (int i = 0; i < NSL; ++i)
	{
		StreamLine& sl = m_streamLines[i];
		if (sl.Points() > 1)
			verts += 2*(sl.Points() - 2) + 2;
	}

	// allocate mesh
	m_mesh.Create(verts / 2, GLMesh::FLAG_COLOR);

	// build mesh
	m_mesh.BeginMesh();
	for (int i = 0; i < NSL; ++i)
	{
		StreamLine& sl = m_streamLines[i];
		int NP = sl.Points();
		if (NP > 1)
		{
			for (int j = 0; j < NP - 1; ++j)
			{
				StreamPoint& p0 = sl[j];
				StreamPoint& p1 = sl[j+1];
				m_mesh.AddVertex(p0.r, p0.c);
				m_mesh.AddVertex(p1.r, p1.c);
			}
		}
	}
	m_mesh.EndMesh();
}
