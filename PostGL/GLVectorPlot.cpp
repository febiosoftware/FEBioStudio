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
#include "GLVectorPlot.h"
#include "PostLib/ColorMap.h"
#include "PostLib/constants.h"
#include "GLWLib/GLWidgetManager.h"
#include "GLModel.h"
#include <GLLib/glx.h>
#include <FSCore/ClassDescriptor.h>
using namespace Post;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

REGISTER_CLASS(CGLVectorPlot, CLASS_PLOT, "vector", 0);

CGLVectorPlot::CGLVectorPlot()
{
	SetTypeString("vector");

	static int n = 1;
	char szname[128] = {0};
	sprintf(szname, "VectorPlot.%02d", n++);
	SetName(szname);

	AddIntParam(0, "data_field")->SetEnumNames("@data_vec3");
	AddIntParam(0, "color_map")->SetEnumNames("@color_map");
	AddBoolParam(true, "allow_clipping");
	AddBoolParam(true, "show_hidden"   );
	AddDoubleParam(0., "density")->SetFloatRange(0.0, 1.0, 0.0001);
	AddIntParam(0, "glyph"         )->SetEnumNames("arrow\0cone\0cylinder\0sphere\0box\0line\0");
	AddIntParam(0, "glyph_color"   )->SetEnumNames("solid\0length\0orientation\0");
	AddColorParam(GLColor::White(), "solid_color");
	AddBoolParam(true, "normalize" );
	AddBoolParam(true, "auto-scale");
	AddDoubleParam(0., "scale"     );
	AddDoubleParam(1., "glyph_aspect_ratio");
	AddIntParam(0, "range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(1., "user_max"  );
	AddDoubleParam(0., "user_min"  );

	m_scale = 1;
	m_dens = 1;
	m_ar = 1;

	m_ntime = -1;
	m_nvec = -1;

	m_lastTime = -1;
	m_lastDt = 0.f;

	m_nglyph = GLYPH_ARROW;

	m_ncol = GLYPH_COL_SOLID;

	m_gcl.r = 255;
	m_gcl.g = 255;
	m_gcl.b = 255;
	m_gcl.a = 255;

	m_bnorm = false;
	m_bautoscale = false;
	m_bshowHidden = true;

	m_seed = rand();

	m_rngType = 0;
	m_usr[0] = 0.0;
	m_usr[1] = 1.0;

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 120, 500);
	bar->align(GLW_ALIGN_BOTTOM | GLW_ALIGN_HCENTER);
	bar->SetOrientation(GLLegendBar::ORIENT_HORIZONTAL);
	bar->copy_label(szname);
	SetLegendBar(bar);

	bar->hide();
	bar->ShowTitle(true);

	UpdateData(false);
}

bool CGLVectorPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		int oldvec = m_nvec;
		m_nvec = GetIntValue(DATA_FIELD);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		AllowClipping(GetBoolValue(CLIP));
		m_bshowHidden = GetBoolValue(SHOW_HIDDEN);
		m_dens = GetFloatValue(DENSITY);
		m_nglyph = GetIntValue(GLYPH);
		m_ncol = GetIntValue(GLYPH_COLOR);
		m_gcl = GetColorValue(SOLID_COLOR);
		m_bnorm = GetBoolValue(NORMALIZE);
		m_bautoscale = GetBoolValue(AUTO_SCALE);
		m_scale = GetFloatValue(SCALE);
		m_ar = GetFloatValue(ASPECT_RATIO);
		m_rngType = GetIntValue(RANGE_TYPE);
		m_usr[1] = GetFloatValue(USER_MAX);
		m_usr[0] = GetFloatValue(USER_MIN);

		GLLegendBar* bar = GetLegendBar();
		if ((m_ncol == 0) || !IsActive()) bar->hide();
		else
		{
			bar->SetRange(m_crng.x, m_crng.y);
			bar->show();
		}

		Update();
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nvec);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(CLIP, AllowClipping());
		SetBoolValue(SHOW_HIDDEN, m_bshowHidden);
		SetFloatValue(DENSITY, m_dens);
		SetIntValue(GLYPH, m_nglyph);
		SetIntValue(GLYPH_COLOR, m_ncol);
		SetColorValue(SOLID_COLOR, m_gcl);
		SetBoolValue(NORMALIZE, m_bnorm);
		SetBoolValue(AUTO_SCALE, m_bautoscale);
		SetFloatValue(SCALE, m_scale);
		SetFloatValue(ASPECT_RATIO, m_ar);
		SetIntValue(RANGE_TYPE, m_rngType);
		SetFloatValue(USER_MAX, m_usr[1]);
		SetFloatValue(USER_MIN, m_usr[0]);
	}

	return false;
}

static double frand() { return (double) rand() / (double) RAND_MAX; }

void CGLVectorPlot::Render(GLRenderEngine& re, GLContext& rc)
{
	if (m_nvec == -1) return;

	// store attributes
	re.pushState();

	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();

	srand(m_seed);

	FEPostModel* pfem = mdl->GetFSModel();
	FSMesh* pm = mdl->GetActiveMesh();

	// calculate scale factor for rendering
	m_fscale = 0.02f*m_scale*pfem->GetBoundingBox().Radius();

	// calculate auto-scale factor
	if (m_bautoscale)
	{
		float autoscale = 1.f;
		float Lmax = 0.f;
		for (int i = 0; i<(int)m_val.size(); ++i)
		{
			float L = m_val[i].Length();
			if (L > Lmax) Lmax = L;
		}
		if (Lmax == 0.f) Lmax = 1.f;
		autoscale = 1.f / Lmax;

		m_fscale *= autoscale;
	}

	if (m_nglyph == GLYPH_LINE)
	{
		re.setMaterial(GLMaterial::CONSTANT, m_gcl);
	}
	else
	{
		re.setMaterial(GLMaterial::PLASTIC, m_gcl);
	}

	if (IS_ELEM_FIELD(m_nvec))
	{
		pm->TagAllElements(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement_& e = pm->ElementRef(i);
			Material* mat = ps->GetMaterial(e.m_MatID);
			if (mat->benable && (m_bshowHidden || mat->visible()))
			{
				e.m_ntag = 1;
			}
		}

		if (m_bshowHidden == false)
		{
			// make sure no vector is drawn for hidden elements
			for (int i = 0; i < pm->Elements(); ++i)
			{
				FEElement_& elem = pm->ElementRef(i);
				if (elem.IsVisible() == false) elem.m_ntag = 0;
			}
		}

		// render the vectors at the elements' centers
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement_& elem = pm->ElementRef(i);
			if ((frand() <= m_dens) && elem.m_ntag)
			{
				vec3f r = to_vec3f(pm->ElementCenter(elem));
				vec3f v = m_val[i];
				RenderVector(re, r, v);
			}
		}
	}
	else if (IS_FACE_FIELD(m_nvec))
	{
		pm->TagAllFaces(0);
		for (int i = 0; i < pm->Faces(); ++i)
		{
			FSFace& f = pm->Face(i);
			FEElement_* pe = pm->ElementPtr(f.m_elem[0].eid);
			if (pe)
			{
				Material* mat = ps->GetMaterial(pe->m_MatID);
				if (mat->benable && (m_bshowHidden || mat->visible()))
				{
					f.m_ntag = 1;
				}
			}
		}

		if (m_bshowHidden == false)
		{
			// make sure no vector is drawn for hidden faces
			for (int i = 0; i < pm->Faces(); ++i)
			{
				FSFace& face = pm->Face(i);
				if (face.IsVisible() == false) face.m_ntag = 0;
			}
		}

		// render the vectors at the face's center
		for (int i = 0; i < pm->Faces(); ++i)
		{
			FSFace& face = pm->Face(i);
			if ((frand() <= m_dens) && face.m_ntag)
			{
				vec3f r = to_vec3f(pm->FaceCenter(face));
				vec3f v = m_val[i];
				RenderVector(re, r, v);
			}
		}
	}
	else if (IS_NODE_FIELD(m_nvec))
	{
		pm->TagAllNodes(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FEElement_& e = pm->ElementRef(i);
			Material* mat = ps->GetMaterial(e.m_MatID);
			if (mat->benable && (m_bshowHidden || mat->visible()))
			{
				int n = e.Nodes();
				for (int j = 0; j < n; ++j) pm->Node(e.m_node[j]).m_ntag = 1;
			}
		}

		if (m_bshowHidden == false)
		{
			// make sure no vector is drawn for hidden nodes
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				FSNode& node = pm->Node(i);
				if (node.IsVisible() == false) node.m_ntag = 0;
			}
		}

		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			if ((frand() <= m_dens) && node.m_ntag)
			{
				vec3f r = to_vec3f(node.r);

				vec3f v = m_val[i];

				RenderVector(re, r, v);
			}
		}
	}

	// restore attributes
	re.popState();
}

void CGLVectorPlot::RenderVector(GLRenderEngine& re, const vec3f& r, vec3f v)
{
	float L = v.Length();
	if (L == 0.f) return;

	CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

	float fmin = m_crng.x;
	float fmax = m_crng.y;

	float f = (L - fmin) / (fmax - fmin);
	GLColor col = map.map(f);
	v.Normalize();

	switch (m_ncol)
	{
	case GLYPH_COL_LENGTH:
		re.setColor(col);
		break;
	case GLYPH_COL_ORIENT:
	{
		double r = fabs(v.x);
		double g = fabs(v.y);
		double b = fabs(v.z);
		re.setColor(GLColor::FromRGBf(r,g,b));
	}
	break;
	case GLYPH_COL_SOLID:
	default:
		re.setColor(m_gcl);
	}

	if (m_bnorm) L = 1;

	L *= m_fscale;
	float l0 = L*.9;
	float l1 = L*.2;
	float r0 = L*0.05*m_ar;
	float r1 = L*0.15*m_ar;

	re.pushTransform();

	re.translate(to_vec3d(r));
	quatd q;
	vec3d V = to_vec3d(v);
	if (V * vec3d(0, 0, 1) == -1.0) q = quatd(PI, vec3d(1, 0, 0));
	else q = quatd(vec3d(0, 0, 1), to_vec3d(v));
	re.rotate(q);

	switch (m_nglyph)
	{
	case GLYPH_ARROW:
		glx::drawCylinder(re, r0, l0, 5);
		re.translate(vec3d(0, 0, l0*0.9));
		glx::drawCone(re, r1, l1, 10);
		break;
	case GLYPH_CONE:
		glx::drawCone(re, r1, l0, 10);
		break;
	case GLYPH_CYLINDER:
		glx::drawCylinder(re, r1, l0, 10);
		break;
	case GLYPH_SPHERE:
		glx::drawSphere(re, r1);
		break;
	case GLYPH_BOX:
		glx::drawBox(re, r0, r0, r0);
		break;
	case GLYPH_LINE:
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, L));
	}

	re.popTransform();
}

void CGLVectorPlot::SetVectorField(int ntype) 
{ 
	m_nvec = ntype; 
	Update(GetModel()->CurrentTimeIndex(), 0.0, true);
}

void CGLVectorPlot::Update()
{
	Update(m_lastTime, m_lastDt, false);
}

void CGLVectorPlot::Activate(bool b)
{
	CGLLegendPlot::Activate(b);
	GLLegendBar* bar = GetLegendBar();
	if ((m_ncol == 0) || !IsActive()) bar->hide();
	else
	{
		bar->SetRange(m_crng.x, m_crng.y);
		bar->show();
	}
}

void CGLVectorPlot::Update(int ntime, float dt, bool breset)
{
	if (breset) { m_map.Clear(); m_rng.clear(); m_val.clear(); }

	m_lastTime = ntime;
	m_lastDt = dt;

	CGLModel* mdl = GetModel();
	FSMesh* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	int N = pfem->GetStates();
	if (N == 0) return;

	// allocate buffers if needed
	if (m_map.States() == 0)
	{
		// nr of states
		int NS = pfem->GetStates();

		// pick the max of nodes and elements
		int NM = 0;
		for (int i = 0; i < NS; ++i)
		{
			FSMesh* pmi = pfem->GetState(i)->GetFEMesh();
			int NN = pmi->Nodes();
			int NE = pmi->Elements();
			if (NN > NM) NM = NN;
			if (NE > NM) NM = NE;
		}

		// allocate buffers
		m_map.Create(NS, NM, vec3f(0,0,0), -1);
		m_rng.resize(NS);
		m_val.resize(NM);
	}

	// get the current states
	int n0 = ntime;
	int n1 = (ntime + 1 >= N ? ntime : ntime + 1);
	if (dt == 0.f) n1 = n0;

	// Update the states
	UpdateState(n0);
	if (n1 != n0) UpdateState(n1);

	// copy nodal values
	if (n1 == n0)
	{
		m_val = m_map.State(ntime);
	}
	else
	{
		// get the state
		FEState& s0 = *pfem->GetState(n0);
		FEState& s1 = *pfem->GetState(n1);

		float df = s1.m_time - s0.m_time;
		if (df == 0) df = 1.f;

		float w = dt / df;

		int ND = 0;
		if (IS_ELEM_FIELD(m_nvec)) ND = pm->Elements();
		if (IS_FACE_FIELD(m_nvec)) ND = pm->Faces();
		else ND = pm->Nodes();

		vector<vec3f>& data0 = m_map.State(n0);
		vector<vec3f>& data1 = m_map.State(n1);

		for (int i = 0; i < ND; ++i)
		{
			vec3f v0 = data0[i];
			vec3f v1 = data1[i];
			m_val[i] = v0*(1.f - w) + v1*w;
		}
	}

	// TODO: when n1 != n0 we need to update the range

	// update static range
	if (breset)
	{
		m_staticRange = m_rng[ntime];
	}
	else
	{
		vec2f& rng = m_rng[ntime];
		if (rng.x < m_staticRange.x) m_staticRange.x = rng.x;
		if (rng.y > m_staticRange.y) m_staticRange.y = rng.y;
	}

	// choose the range for rendering
	switch (m_rngType)
	{
	case 0: // dynamic
		m_crng = m_rng[ntime];
		break;
	case 1: // static
		m_crng = m_staticRange;
		break;
	case 2: // user
		m_crng.x = m_usr[0];
		m_crng.y = m_usr[1];
		break;
	}
	if (m_crng.x == m_crng.y) m_crng.y++;

	// update the color bar's range
	GLLegendBar* bar = GetLegendBar();
	bar->SetRange(m_crng.x, m_crng.y);
}

void CGLVectorPlot::UpdateState(int nstate)
{
	CGLModel* mdl = GetModel();
	FSMesh* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	// check the tag
	int ntag = m_map.GetTag(nstate);

	// see if we need to update
	if (ntag != m_nvec)
	{
		m_map.SetTag(nstate, m_nvec);

		// get the state we are interested in
		vector<vec3f>& val = m_map.State(nstate);

		vec2f& rng = m_rng[nstate];
		rng.x = rng.y = 0;

		if (IS_ELEM_FIELD(m_nvec))
		{
			for (int i = 0; i < pm->Elements(); ++i)
			{
				val[i] = pfem->EvaluateElemVector(i, nstate, m_nvec);
				float L = val[i].Length();
				if (L > rng.y) rng.y = L;
			}
		}
		else if (IS_FACE_FIELD(m_nvec))
		{
			for (int i = 0; i < pm->Faces(); ++i)
			{
				bool b = pfem->EvaluateFaceVector(i, nstate, m_nvec, val[i]);
				float L = val[i].Length();
				if (L > rng.y) rng.y = L;
			}
		}
		else
		{
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				val[i] = pfem->EvaluateNodeVector(i, nstate, m_nvec);
				float L = val[i].Length();
				if (L > rng.y) rng.y = L;
			}
		}

		if (rng.y == rng.x) ++rng.y;
	}
}
