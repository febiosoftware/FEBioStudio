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
#include "GLTensorPlot.h"
#include "PostLib/constants.h"
#include "GLModel.h"
#include <stdlib.h>
#include <GLLib/glx.h>
#include <FSCore/ClassDescriptor.h>
#include <FSCore/ColorMapManager.h>
using namespace Post;

REGISTER_CLASS(GLTensorPlot, CLASS_PLOT, "tensor", 0);

GLTensorPlot::GLTensorPlot()
{
	SetTypeString("tensor");

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "TensorPlot.%02d", n++);
	SetName(szname);

	AddIntParam(0, "data_field")->SetEnumNames("@data_mat3");
	AddIntParam(0, "calculate")->SetEnumNames("eigenvectors\0columns\0rows\0");
	AddIntParam(0, "color_map")->SetEnumNames("@color_map");
	AddIntParam(10, "range_divisions")->SetIntRange(1, 50);
	AddBoolParam(true, "allow_clipping");
	AddBoolParam(true, "show_hidden");
	AddDoubleParam(0.0, "scale");
	AddDoubleParam(0.0, "density")->SetFloatRange(0.0, 1.0, 0.0001);
	AddIntParam(0, "glyph")->SetEnumNames("arrow\0line\0sphere\0box\0");
	AddIntParam(0, "glyph_color")->SetEnumNames("solid\0norm\0fractional anisotropy\0");
	AddColorParam(GLColor::White(), "solid_color");
	AddBoolParam(true, "auto-scale");
	AddBoolParam(true, "normalize" );
	AddChoiceParam(0, "max_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(1.0, "user_max");
	AddChoiceParam(0, "min_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0.0, "user_min");

	m_scale = 1;
	m_dens = 1;

	m_ndivs = 10;

	m_ntime = -1;
	m_ntensor = 0;

	m_nglyph = Glyph_Arrow;

	m_lastCol = -1;
	m_ncol = Glyph_Col_Solid;

	m_gcl.r = 255;
	m_gcl.g = 255;
	m_gcl.b = 255;
	m_gcl.a = 255;

	m_bautoscale = true;
	m_bshowHidden = true;
	m_bnormalize = false;

	m_lastTime = -1;
	m_lastDt = 0.f;

	m_seed = rand();

	m_nmethod = VEC_EIGEN;

	m_range.maxtype = RANGE_DYNAMIC;
	m_range.mintype = RANGE_DYNAMIC;
	m_range.valid = false;

	UpdateData(false);
}

bool GLTensorPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		int noldcol = m_ncol;
		int noldmeth = m_nmethod;

		m_ntensor = GetIntValue(DATA_FIELD);
		m_nmethod = GetIntValue(METHOD);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		AllowClipping(GetBoolValue(CLIP));
		m_bshowHidden = GetBoolValue(SHOW_HIDDEN);
		m_scale = GetFloatValue(SCALE);
		m_dens = GetFloatValue(DENSITY);
		m_nglyph = GetIntValue(GLYPH);
		m_ncol = GetIntValue(GLYPH_COLOR);
		m_gcl = GetColorValue(SOLID_COLOR);
		m_bautoscale = GetBoolValue(AUTO_SCALE);
		m_bnormalize = GetBoolValue(NORMALIZE);
		m_ndivs = GetIntValue(RANGE_DIVS);

		m_range.maxtype = GetIntValue(MAX_RANGE_TYPE);
		m_range.mintype = GetIntValue(MIN_RANGE_TYPE);

		if (noldcol != m_ncol)
		{
			m_map.SetTags(-1);
		}

		if(m_range.maxtype == RANGE_USER)
		{
			m_range.max = GetFloatValue(USER_MAX);
		}

		if(m_range.mintype == RANGE_USER)
		{
			m_range.min = GetFloatValue(USER_MIN);
		}

		if (m_range.maxtype == RANGE_USER || m_range.mintype == RANGE_USER)
		{
			m_range.valid = true;
		}
		else m_range.valid = false;

		if (noldmeth != m_nmethod)
		{
			for (int i = 0; i<m_map.States(); ++i) m_map.SetTag(i, -1);
		}
	}
	else
	{
		SetIntValue(DATA_FIELD, m_ntensor);
		SetIntValue(METHOD, m_nmethod);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetBoolValue(CLIP, AllowClipping());
		SetBoolValue(SHOW_HIDDEN, m_bshowHidden);
		SetFloatValue(SCALE, m_scale);
		SetFloatValue(DENSITY, m_dens);
		SetIntValue(GLYPH, m_nglyph);
		SetIntValue(GLYPH_COLOR, m_ncol);
		SetColorValue(SOLID_COLOR, m_gcl);
		SetBoolValue(AUTO_SCALE, m_bautoscale);
		SetBoolValue(NORMALIZE, m_bnormalize);
		SetIntValue(RANGE_DIVS, m_ndivs);
	}

	return false;
}

void GLTensorPlot::SetTensorField(int nfield)
{
	m_ntensor = nfield;
	Update(GetModel()->CurrentTimeIndex(), 0.0, false);
}

void GLTensorPlot::SetVectorMethod(int m)
{ 
	m_nmethod = m; 
	for (int i=0; i<m_map.States(); ++i) m_map.SetTag(i, -1);
	Update(GetModel()->CurrentTimeIndex(), 0.0, false);
}

CColorTexture* GLTensorPlot::GetColorMap()
{
	return &m_Col;
}

void GLTensorPlot::Update()
{
	Update(m_lastTime, m_lastDt, false);
}

void GLTensorPlot::Update(int ntime, float dt, bool breset)
{
	if (m_lastCol != m_ncol) breset = true;
	m_lastCol = m_ncol;

	if (breset) { m_map.Clear(); m_val.clear(); m_range.valid = false; }

	m_lastTime = ntime;
	m_lastDt = dt;

	CGLModel* mdl = GetModel();
	FSMesh* pm = mdl->GetActiveMesh();
	FEPostModel* pfem = mdl->GetFSModel();

	if (m_map.States() == 0)
	{
		int NS = pfem->GetStates();

		// pick the max of nodes and elements
		int NN = pm->Nodes();
		int NE = pm->Elements();
		int NM = (NN > NE ? NN : NE);

		TENSOR dummy;
		m_map.Create(NS, NM, dummy, -1);
		m_val.resize(NM);
	}

	// check the tag
	int ntag = m_map.GetTag(ntime);

	// get the state we are interested in
	vector<TENSOR>& val = m_map.State(ntime);

	// see if we need to update
	int items = 0;
	if (IS_ELEM_FIELD(m_ntensor)) items = pm->Elements(); else items = pm->Nodes();
	if (ntag != m_ntensor)
	{
		m_map.SetTag(ntime, m_ntensor);

		TENSOR t;
		if (IS_ELEM_FIELD(m_ntensor))
		{
			items = pm->Elements();
			for (int i = 0; i < pm->Elements(); ++i)
			{
				switch (m_nmethod)
				{
				case VEC_EIGEN:
				{
					mat3f m = pfem->EvaluateElemTensor(i, ntime, m_ntensor, DATA_MAT3S);

					mat3fs s = m.sym();
					s.eigen(t.r, t.l);

					vec3f n = t.r[0] ^ t.r[1];

					// NOTE: I can't do this, because the eigenvalues are sorted
					//       so there is no guarantee that the eigenvectors are orderd orthogonally
/*					if (n*t.r[2] < 0.f)
					{
						t.r[2] = -t.r[2];
						t.l[2] = -t.l[2];
					}
*/
					t.f = 0.f;

					if (m_ncol == Glyph_Col_Norm)
					{
						t.f = s.von_mises();
					}

					if (m_ncol == Glyph_Col_Anisotropy)
					{
						double la = (t.l[0] + t.l[1] + t.l[2]) / 3.0;
						double D = sqrt(t.l[0] * t.l[0] + t.l[1] * t.l[1] + t.l[2] * t.l[2]);
						double fa = 0.0;
						if (D != 0) fa = sqrt(3.0 / 2.0)*sqrt((t.l[0] - la)*(t.l[0] - la) + (t.l[1] - la)*(t.l[1] - la) + (t.l[2] - la)*(t.l[2] - la)) / D;
						t.f = fa;
					}

					val[i] = t;
				}
				break;
				case VEC_COLUMN:
				{
					mat3f m = pfem->EvaluateElemTensor(i, ntime, m_ntensor);
					t.r[0] = m.col(0);
					t.r[1] = m.col(1);
					t.r[2] = m.col(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				case VEC_ROW:
				{
					mat3f m = pfem->EvaluateElemTensor(i, ntime, m_ntensor);
					t.r[0] = m.row(0);
					t.r[1] = m.row(1);
					t.r[2] = m.row(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				}
			}
		}
		else
		{
			items = pm->Nodes();
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				switch (m_nmethod)
				{
				case VEC_EIGEN:
				{
					mat3f m = pfem->EvaluateNodeTensor(i, ntime, m_ntensor, DATA_MAT3S);

					mat3fs s = m.sym();
					s.eigen(t.r, t.l);

					vec3f n = t.r[0] ^ t.r[1];
					if (n*t.r[2] < 0.f)
					{
						t.r[2] = -t.r[2];
						t.l[2] = -t.l[2];
					}
					t.f = s.von_mises();

					val[i] = t;
				}
				break;
				case VEC_COLUMN:
				{
					mat3f m = pfem->EvaluateNodeTensor(i, ntime, m_ntensor);
					t.r[0] = m.col(0);
					t.r[1] = m.col(1);
					t.r[2] = m.col(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				case VEC_ROW:
				{
					mat3f m = pfem->EvaluateNodeTensor(i, ntime, m_ntensor);
					t.r[0] = m.row(0);
					t.r[1] = m.row(1);
					t.r[2] = m.row(2);

					t.l[0] = t.r[0].Length();
					t.l[1] = t.r[1].Length();
					t.l[2] = t.r[2].Length();

					t.f = 0.f;

					val[i] = t;
				}
				break;
				}
			}
		}
	}


	// evaluate range
	float fmax, fmin;
	fmax = fmin = val[0].f;
	for (int i = 0; i < items; ++i)
	{
		float v = val[i].f;
		if (v > fmax) fmax = v;
		if (v < fmin) fmin = v;
	}

	if (m_range.valid == false)
	{
		if (m_range.maxtype != RANGE_USER) m_range.max = fmax;
		if (m_range.mintype != RANGE_USER) m_range.min = fmin;
		m_range.valid = true;
	}
	else
	{
		if (m_range.maxtype == RANGE_DYNAMIC)
		{
			m_range.max = fmax;
		}
		else if (m_range.maxtype == RANGE_STATIC)
		{
			if (fmax > m_range.max) m_range.max = fmax;
		}

		if (m_range.mintype == RANGE_DYNAMIC)
		{
			m_range.min = fmin;
		}
		else if (m_range.mintype == RANGE_STATIC)
		{
			if (fmin < m_range.min) m_range.min = fmin;
		}
	}

	// copy nodal values
	m_val = m_map.State(ntime);
}

static double frand() { return (double)rand() / (double)RAND_MAX; }

void GLTensorPlot::Render(GLRenderEngine& re, GLContext& rc)
{
	if (m_ntensor == 0) return;

	// store attributes
	re.pushState();

	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();

	srand(m_seed);

	FEPostModel* pfem = mdl->GetFSModel();
	FSMesh* pm = mdl->GetActiveMesh();

	float scale = 0.02f*m_scale*pfem->GetBoundingBox().Radius();

	if (m_nglyph == Glyph_Line)
	{
		re.setMaterial(GLMaterial::CONSTANT, m_gcl);
	}
	else
	{
		re.setMaterial(GLMaterial::PLASTIC, m_gcl);
	}

	if (IS_ELEM_FIELD(m_ntensor))
	{
		pm->TagAllElements(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement_& e = pm->ElementRef(i);
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
				FSElement_& elem = pm->ElementRef(i);
				if (elem.IsVisible() == false) elem.m_ntag = 0;
			}
		}

		float auto_scale = 1.f;
		if (m_bautoscale)
		{
			float Lmax = 0.f;
			for (int i = 0; i < pm->Elements(); ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					float L = fabs(m_val[i].l[j]);
					if (L > Lmax) Lmax = L;
				}
			}
			if (Lmax == 0.f) Lmax = 1.f;
			auto_scale = 1.f / Lmax;
		}

		const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());
		float fmax = 1.f, fmin = 0.f;
		if (m_ncol != Glyph_Col_Solid)
		{
			fmax = m_range.max;
			fmin = m_range.min;
		}

		if (fmax == fmin) fmax++;

		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement_& elem = pm->ElementRef(i);
			if ((frand() <= m_dens) && elem.m_ntag)
			{
				vec3d r = pm->ElementCenter(elem);

				TENSOR& t = m_val[i];

				if (m_ncol != Glyph_Col_Solid)
				{
					float w = (t.f - fmin) / (fmax - fmin);
					GLColor c = map.map(w);
					re.setColor(c);
				}

				re.translate(r);
				RenderGlyphs(re, t, scale*auto_scale);
				re.translate(-r);
			}
		}
	}
	else
	{
		pm->TagAllNodes(0);
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement_& e = pm->ElementRef(i);
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

		float auto_scale = 1.f;
		if (m_bautoscale)
		{
			float Lmax = 0.f;
			for (int i = 0; i < pm->Nodes(); ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					float L = fabs(m_val[i].l[j]);
					if (L > Lmax) Lmax = L;
				}
			}
			if (Lmax == 0.f) Lmax = 1.f;
			auto_scale = 1.f / Lmax;
		}

		const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());
		float fmax = 1.f, fmin = 0.f;
		if (m_ncol != Glyph_Col_Solid)
		{
			fmax = m_range.max;
			fmin = m_range.min;
		}

		if (fmax == fmin) fmax++;

		re.setColor(m_gcl);

		for (int i = 0; i < pm->Nodes(); ++i)
		{
			FSNode& node = pm->Node(i);
			if ((frand() <= m_dens) && node.m_ntag)
			{
				vec3d r = node.r;

				TENSOR& t = m_val[i];

				if (m_ncol != Glyph_Col_Solid)
				{
					float w = (t.f  - fmin)/ (fmax - fmin);
					re.setColor(map.map(w));
				}

				re.translate(r);
				RenderGlyphs(re, t, scale*auto_scale);
				re.translate(-r);
			}
		}
	}

	// restore attributes
	re.popState();
}

void GLTensorPlot::RenderGlyphs(GLRenderEngine& re, TENSOR& t, float scale)
{
	switch (m_nglyph)
	{
	case Glyph_Arrow : RenderArrows(re, t, scale); break;
	case Glyph_Line  : RenderLines (re, t, scale); break;
	case Glyph_Sphere: RenderSphere(re, t, scale); break;
	case Glyph_Box   : RenderBox   (re, t, scale); break;
	}
}

void GLTensorPlot::RenderArrows(GLRenderEngine& re, GLTensorPlot::TENSOR& t, float scale)
{
	GLColor c[3];
	c[0] = GLColor(255, 0, 0);
	c[1] = GLColor(0, 255, 0);
	c[2] = GLColor(0, 0, 255);

	for (int i = 0; i<3; ++i)
	{
		re.pushTransform();

		float L = (m_bnormalize ? scale : scale*t.l[i]);
		vec3f v = t.r[i];
		if (L < 0) { v = -v; L = -L; }

		float l0 = L * .9;
		float l1 = L * .2;
		float r0 = L * 0.05;
		float r1 = L * 0.15;

		quatd q = quatd(vec3d(0,0,1), to_vec3d(v));
		re.rotate(q);

		re.setColor(c[i]);
		glx::drawCylinder(re, r0, l0, 5);
		re.translate(vec3d(0, 0, l0*0.9));
		glx::drawCone(re, r1, l1, 10);

		re.popTransform();
	}
}

void GLTensorPlot::RenderLines(GLRenderEngine& re, GLTensorPlot::TENSOR& t, float scale)
{
	GLColor c[3];
	c[0] = GLColor(255, 0, 0);
	c[1] = GLColor(0, 255, 0);
	c[2] = GLColor(0, 0, 255);

	for (int i = 0; i<3; ++i)
	{
		re.pushTransform();

		float L = (m_bnormalize ? scale : scale*t.l[i]);

		vec3f v = t.r[i];
		quatd q = quatd(vec3d(0, 0, 1), to_vec3d(v));
		re.rotate(q);

		re.setColor(c[i]);
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, L));

		re.popTransform();
	}
}

void GLTensorPlot::RenderSphere(GLRenderEngine& re, TENSOR& t, float scale)
{
	if (scale <= 0.f) return;

	float smax = 0.f;
	float sx = fabs(t.l[0]); if (sx > smax) smax = sx;
	float sy = fabs(t.l[1]); if (sy > smax) smax = sy;
	float sz = fabs(t.l[2]); if (sz > smax) smax = sz;
	if (smax < 1e-7f) return;

	if (sx < 0.1*smax) sx = 0.1f*smax;
	if (sy < 0.1*smax) sy = 0.1f*smax;
	if (sz < 0.1*smax) sz = 0.1f*smax;

	re.pushTransform();
	vec3f* e = t.r;
	vec3f n = e[0] ^ e[1];
	if (n*e[2] < 0) e[2] = -e[2];
	double m[4][4] = {0};
	m[3][3] = 1.f;
	m[0][0] = e[0].x; m[0][1] = e[0].y; m[0][2] = e[0].z;
	m[1][0] = e[1].x; m[1][1] = e[1].y; m[1][2] = e[1].z;
	m[2][0] = e[2].x; m[2][1] = e[2].y; m[2][2] = e[2].z;
	re.multTransform(&m[0][0]);

	re.scale(scale*sx, scale*sy, scale*sz);
	glx::drawSphere(re, 1.f);
	re.popTransform();
}

void GLTensorPlot::RenderBox(GLRenderEngine& re, TENSOR& t, float scale)
{
	if (scale <= 0.f) return;

	float smax = 0.f;
	float sx = fabs(t.l[0]); if (sx > smax) smax = sx;
	float sy = fabs(t.l[1]); if (sy > smax) smax = sy;
	float sz = fabs(t.l[2]); if (sz > smax) smax = sz;
	if (smax < 1e-7f) return;

	if (sx < 0.1*smax) sx = 0.1f*smax;
	if (sy < 0.1*smax) sy = 0.1f*smax;
	if (sz < 0.1*smax) sz = 0.1f*smax;

	re.pushTransform();
	vec3f* e = t.r;
	double m[4][4] = { 0 };
	m[3][3] = 1.f;
	m[0][0] = e[0].x; m[0][1] = e[0].y; m[0][2] = e[0].z;
	m[1][0] = e[1].x; m[1][1] = e[1].y; m[1][2] = e[1].z;
	m[2][0] = e[2].x; m[2][1] = e[2].y; m[2][2] = e[2].z;
	re.multTransform(&m[0][0]);

	re.scale(scale*sx, scale*sy, scale*sz);
	glx::drawBox(re, 0.5, 0.5, 0.5);
	re.popTransform();
}

LegendData GLTensorPlot::GetLegendData() const
{
	LegendData l;

	int glyphCol = GetIntValue(GLYPH_COLOR);
	if ((glyphCol >  0) && (m_range.valid))
	{
		l.discrete = false;
		l.ndivs = m_ndivs;
		l.vmin = m_range.min;
		l.vmax = m_range.max;
		l.smooth = true;
		l.colormap = GetIntValue(COLOR_MAP);
		l.title = GetName();
	}

	return l;
}
