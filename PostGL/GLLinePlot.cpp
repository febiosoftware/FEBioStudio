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
#include "GLLinePlot.h"
#include "GLModel.h"
#include <PostLib/FEPostModel.h>
#include <GLLib/glx.h>
#include <FSCore/ClassDescriptor.h>
#include <FSCore/ColorMapManager.h>
#include <GLLib/GLRenderEngine.h>

using namespace Post;

REGISTER_CLASS(CGLLinePlot, CLASS_PLOT, "lines", 0);

//-----------------------------------------------------------------------------
CGLLinePlot::CGLLinePlot()
{
	SetTypeString("lines");

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "Line.%02d", n++);
	SetName(szname);

	AddIntParam(0, "data_field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "color_mode")->SetEnumNames("solid\0segments\0line data\0model data\0");
	AddColorParam(GLColor(255, 0, 0), "solid_color");
	AddIntParam(0, "color_map")->SetEnumNames("@color_map");
	AddIntParam(0, "render_mode")->SetEnumNames("lines\0lines 3D\0smooth lines 3D\0");
	AddDoubleParam(1.0, "line_width");
	AddIntParam(0, "max_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "user_max");
	AddIntParam(0, "min_range_type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "user_min");
	AddBoolParam(true, "show_on_hidden_elements");
	AddBoolParam(true, "show_legend");

	m_line = 4.f;
	m_nmode = 0;
	m_ncolor = COLOR_SOLID;
	m_col = GLColor(255, 0, 0);
	m_nfield = -1;
	m_show = true;
	m_showLegend = true;

	m_range.min = 0.0; m_range.max = 1.0;
	m_range.mintype = m_range.maxtype = RANGE_DYNAMIC;

	m_lineData = nullptr;

	UpdateData(false);
}

//-----------------------------------------------------------------------------
CGLLinePlot::~CGLLinePlot()
{
	delete m_lineData;
	m_lineData = nullptr;
}

void CGLLinePlot::SetLineDataModel(LineDataModel* lineData)
{
	if (m_lineData) delete m_lineData;
	m_lineData = lineData;
}

LineDataModel* CGLLinePlot::GetLineDataModel()
{
	return m_lineData;
}

bool CGLLinePlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_nfield = GetIntValue(DATA_FIELD);
		m_ncolor = GetIntValue(COLOR_MODE);
		m_col = GetColorValue(SOLID_COLOR);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		m_nmode = GetIntValue(RENDER_MODE);
		m_line = GetFloatValue(LINE_WIDTH);
		m_show = GetBoolValue(SHOW_ALWAYS);
		m_showLegend = GetBoolValue(SHOW_LEGEND);

		m_range.maxtype = GetIntValue(MAX_RANGE_TYPE);
		m_range.mintype = GetIntValue(MIN_RANGE_TYPE);
		if (m_range.maxtype == RANGE_USER) m_range.max = GetFloatValue(USER_MAX);
		if (m_range.mintype == RANGE_USER) m_range.min = GetFloatValue(USER_MIN);

		Update(GetModel()->CurrentTimeIndex(), 0.0, false);
	}
	else
	{
		SetIntValue(DATA_FIELD, m_nfield);
		SetIntValue(COLOR_MODE, m_ncolor);
		SetColorValue(SOLID_COLOR, m_col);
		SetIntValue(COLOR_MAP, m_Col.GetColorMap());
		SetIntValue(RENDER_MODE, m_nmode);
		SetFloatValue(LINE_WIDTH, m_line);
		SetBoolValue(SHOW_ALWAYS, m_show);
		SetBoolValue(SHOW_LEGEND, m_showLegend);
		SetIntValue(MAX_RANGE_TYPE, m_range.maxtype);
		SetIntValue(MIN_RANGE_TYPE, m_range.mintype);
		SetFloatValue(USER_MAX, m_range.max);
		SetFloatValue(USER_MIN, m_range.min);
	}

	return false;
}

void CGLLinePlot::SetColorMode(int m) 
{
	m_ncolor = m;
	Update(GetModel()->CurrentTimeIndex(), 0.0, false);
}

void CGLLinePlot::SetDataField(int n)
{ 
	m_nfield = n; 
	Update(GetModel()->CurrentTimeIndex(), 0.0, false);
}

void CGLLinePlot::Reload()
{
	if (m_lineData) m_lineData->Reload();
}

//-----------------------------------------------------------------------------
void CGLLinePlot::Render(GLRenderEngine& re, GLContext& rc)
{
	if (m_lineData == nullptr) return;

	CGLModel& glm = *GetModel();
	FEPostModel& fem = *glm.GetFSModel();
	int ns = glm.CurrentTimeIndex();

	if ((ns >= 0) && (ns <fem.GetStates()))
	{
		FEState& s = *fem.GetState(ns);
		Post::LineData& lineData = m_lineData->GetLineData(ns);
		int NL = lineData.Lines();
		if (NL > 0)
		{
			switch (m_nmode)
			{
			case 0: RenderLines(re); break;
			case 1: Render3DLines(re); break;
			case 2: Render3DSmoothLines(re); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
int randomize(int n, int nmax)
{
	int a = nmax / 3; if (a == 0) a = 1;
	int b = nmax / 2;
	return (a * n + b) % nmax;
}

//-----------------------------------------------------------------------------
void CGLLinePlot::RenderLines(GLRenderEngine& re)
{
	re.setMaterial(GLMaterial::CONSTANT, m_col);
	float line_old = re.lineWidth();
	re.setLineWidth(m_line);
	re.renderGMeshEdges(m_mesh, false);
	re.setLineWidth(line_old);
}

//-----------------------------------------------------------------------------
bool CGLLinePlot::ShowLine(LINESEGMENT& l, FEState& s)
{
	if ((l.m_elem[0] == -1) || (l.m_elem[1] == -1)) return true;

	FSMesh* m = s.GetFEMesh();
	FSElement& e0 = m->Element(l.m_elem[0]);
	FSElement& e1 = m->Element(l.m_elem[1]);

	return (e0.IsVisible() || e1.IsVisible());
}

//-----------------------------------------------------------------------------
void CGLLinePlot::Render3DLines(GLRenderEngine& re)
{
	if (m_ncolor == COLOR_SOLID)
	{
		re.setMaterial(GLMaterial::PLASTIC, m_col);
	}
	else
	{
		re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::TEXTURE_1D, false);
		re.setTexture(m_Col.GetTexture());
	}
	re.renderGMesh(m_mesh, false);
}

//-----------------------------------------------------------------------------
void CGLLinePlot::Render3DSmoothLines(GLRenderEngine& re)
{
	if (m_ncolor == COLOR_SOLID)
	{
		re.setMaterial(GLMaterial::PLASTIC, m_col);
	}
	else if (m_ncolor == COLOR_SEGMENT)
	{
		re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::VERTEX_COLOR, false);
	}
	else
	{
		re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::TEXTURE_1D, false);
		re.setTexture(m_Col.GetTexture());
	}
	re.renderGMesh(m_mesh, false);
}

void CGLLinePlot::Update(int ntime, float dt, bool breset)
{
	if (m_lineData == nullptr) { m_mesh.Clear(); return; }

	CGLModel& glm = *GetModel();
	FEPostModel& fem = *glm.GetFSModel();
	FEState& s = *fem.GetState(ntime);

	float vmax = -1e20f;
	float vmin = 1e20f;
	if (m_ncolor == COLOR_LINE_DATA)
	{
		CGLModel& glm = *GetModel();
		FEPostModel& fem = *glm.GetFSModel();

		FEState& s = *fem.GetState(ntime);
		Post::LineData& lineData = m_lineData->GetLineData(ntime);
		int NL = lineData.Lines();

		for (int i = 0; i < NL; ++i)
		{
			LINESEGMENT& line = lineData.Line(i);

			line.m_val[0] = line.m_user_data[0];
			line.m_val[1] = line.m_user_data[1];

			if (line.m_val[0] > vmax) vmax = line.m_val[0];
			if (line.m_val[0] < vmin) vmin = line.m_val[0];

			if (line.m_val[1] > vmax) vmax = line.m_val[1];
			if (line.m_val[1] < vmin) vmin = line.m_val[1];
		}
	}
	else if ((m_ncolor == COLOR_MODEL_DATA) && (m_nfield != -1))
	{
		Post::LineData& lineData = m_lineData->GetLineData(ntime);
		int NL = lineData.Lines();

		NODEDATA nd1, nd2;
		for (int i = 0; i < NL; ++i)
		{
			LINESEGMENT& line = lineData.Line(i);

			fem.EvaluateNode(line.m_r0, ntime, m_nfield, nd1);
			fem.EvaluateNode(line.m_r1, ntime, m_nfield, nd2);
			line.m_val[0] = nd1.m_val;
			line.m_val[1] = nd2.m_val;

			if (line.m_val[0] > vmax) vmax = line.m_val[0];
			if (line.m_val[0] < vmin) vmin = line.m_val[0];

			if (line.m_val[1] > vmax) vmax = line.m_val[1];
			if (line.m_val[1] < vmin) vmin = line.m_val[1];
		}
	}
	else
	{
		vmin = vmax = 0.f;
	}
	if (vmin == vmax) vmax++;

	switch (m_range.mintype)
	{
	case 1:	if (vmin > m_range.min) vmin = m_range.min; break;
	case 2: vmin = m_range.min; break;
	}

	switch (m_range.maxtype)
	{
	case 1:	if (vmax < m_range.max) vmax = m_range.max; break;
	case 2: vmax = m_range.max; break;
	}

	m_range.min = vmin;
	m_range.max = vmax;

	// update the meshes
	switch (m_nmode)
	{
	case 0: UpdateLineMesh(s, ntime); break;
	case 1: Update3DLines(s, ntime); break;
	case 2: UpdateSmooth3DLines(s, ntime); break;
	}
}

void CGLLinePlot::UpdateLineMesh(FEState& s, int ntime)
{
	Post::LineData& lineData = m_lineData->GetLineData(ntime);

	int NL = lineData.Lines();
	if (NL == 0) { m_mesh.Clear(); return; }

	int maxseg = 0;
	for (int i = 0; i < NL; ++i)
	{
		if (lineData.Line(i).m_segId > maxseg) maxseg = lineData.Line(i).m_segId;
	}
	if (maxseg == 0) maxseg = 1;

	m_mesh.Clear();

	if (m_ncolor == COLOR_SOLID)
	{
		for (int i = 0; i < NL; ++i)
		{
			LINESEGMENT& l = lineData.Line(i);
			if (m_show || ShowLine(l, s))
			{
				vec3f vr[2];
				vr[0] = l.m_r0;
				vr[1] = l.m_r1;
				m_mesh.AddEdge(vr, 2);
			}
		}
	}
	else if (m_ncolor == COLOR_SEGMENT)
	{
		const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

		for (int i = 0; i < NL; ++i)
		{
			LINESEGMENT& l = lineData.Line(i);

			int n = l.m_segId;// randomize(l.m_segId, maxseg);
			float f = (float)n / (float)maxseg;
			GLColor c = map.map(f);

			if (m_show || ShowLine(l, s))
			{
				vec3f vr[2];
				vr[0] = l.m_r0;
				vr[1] = l.m_r1;
				m_mesh.AddEdge(vr, c);
			}
		}
	}
	else
	{
		const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

		float vmin = m_range.min;
		float vmax = m_range.max;
		if (vmin == vmax) vmax++;

		{
			int NL = lineData.Lines();
			for (int i = 0; i < NL; ++i)
			{
				LINESEGMENT& l = lineData.Line(i);
				if (m_show || ShowLine(l, s))
				{
					vec3f vr[2];
					vr[0] = l.m_r0;
					vr[1] = l.m_r1;

					float f0 = (l.m_val[0] - vmin) / (vmax - vmin);
					float f1 = (l.m_val[1] - vmin) / (vmax - vmin);

					GLColor c[2];
					c[0] = map.map(f0);
					c[1] = map.map(f1);
					m_mesh.AddEdge(vr, c);
				}
			}
		}
	}
}

void CGLLinePlot::Update3DLines(FEState& s, int ntime)
{
	Post::LineData& lineData = m_lineData->GetLineData(ntime);
	if (lineData.Lines() == 0) { m_mesh.Clear(); return; }

	const int NSEG = 8;
	int NL = lineData.Lines();

	int quads = NL * NSEG;
	m_mesh.Clear();

	for (int i = 0; i < NL; ++i)
	{
		LINESEGMENT& l = lineData.Line(i);
		if (m_show || ShowLine(l, s))
		{
			vec3d ra = to_vec3d(l.m_r0);
			vec3d rb = to_vec3d(l.m_r1);
			vec3d n = rb - ra;
			double L = n.Length();
			n.Normalize();

			float vmin = m_range.min;
			float vmax = m_range.max;
			if (vmin == vmax) vmax++;

			float f0 = (l.m_val[0] - vmin) / (vmax - vmin);
			float f1 = (l.m_val[1] - vmin) / (vmax - vmin);

			quatd q(vec3d(0, 0, 1), n);

			double R = m_line;

			// generate a cylinder
			for (int i = 0; i < NSEG; ++i)
			{
				double w0 = 2 * PI * i / (double)NSEG;
				double w1 = 2 * PI * (i + 1) / (double)NSEG;
				double x0 = cos(w0), x1 = cos(w1);
				double y0 = sin(w0), y1 = sin(w1);

				vec3d r[4] = {
					vec3d(R*x0, R*y0, 0),
					vec3d(R*x1, R*y1, 0),
					vec3d(R*x1, R*y1, L),
					vec3d(R*x0, R*y0, L)
				};
				double tex[4] = { f0, f1, f1, f0 };

				vec3f vr[4], vn[4];
				for (int j = 0; j < 4; ++j)
				{
					vec3d rj = r[j];
					q.RotateVector(rj);
					rj += ra;
					vr[j] = to_vec3f(rj);

					vec3d nj = r[j]; nj.z = 0; nj /= R;
					q.RotateVector(nj);
					vn[j] = to_vec3f(nj);
				}

				vec3f ra[3], rb[3];
				ra[0] = vr[0]; ra[1] = vr[1]; ra[2] = vr[2];
				rb[0] = vr[2]; ra[1] = vr[3]; ra[2] = vr[0];
				m_mesh.AddFace(ra);
				m_mesh.AddFace(rb);
			}
		}
	}
	m_mesh.Update();
}


//-----------------------------------------------------------------------------
void tesselateSmoothPath(GLMesh& mesh, const vec3d& r0, const vec3d& r1, float R, const vec3d& n0, const vec3d& n1, float t0, float t1, GLColor c, int nsegs, int ndivs)
{
	quatd q0(vec3d(0, 0, 1), n0);
	quatd q1(vec3d(0, 0, 1), n1);

	double L = (r1 - r0).Length();
	vec3d m0 = n0 * L;
	vec3d m1 = n1 * L;

	int M = nsegs;
	if (M < 2) M = 2;

	const int MAX_DIVS = 16;
	if (ndivs > MAX_DIVS) ndivs = MAX_DIVS;
	double x[MAX_DIVS+1], y[MAX_DIVS+1];
	for (int i = 0; i <= ndivs; ++i)
	{
		double w = 2 * PI * i / (double)ndivs;
		x[i] = cos(w);
		y[i] = sin(w);
	}

	const int N = ndivs;
	for (int j = 0; j < M; ++j)
	{
		quatd qa = quatd::slerp(q0, q1, (double)j / M);
		quatd qb = quatd::slerp(q0, q1, (double)(j + 1) / M);

		vec3d ra = glx::interpolate(r0, r1, m0, m1, (double)j / M);
		vec3d rb = glx::interpolate(r0, r1, m0, m1, (double)(j + 1.0) / M);

		float ta = t0 + j * (t1 - t0) / M;
		float tb = t0 + (j + 1) * (t1 - t0) / M;

		double tex[4] = { ta, tb, tb, ta };

		vec3d r[4], n[4];
		for (int i = 0; i <= N; ++i)
		{
			n[1] = vec3d(x[i], y[i], 0); qa.RotateVector(n[1]);
			n[2] = vec3d(x[i], y[i], 0); qb.RotateVector(n[2]);

			r[1] = ra + n[1]*R;
			r[2] = rb + n[2]*R;

			if (i > 0)
			{
				int L[2][3] = { {0,1,2}, {2,3,0} };
				vec3f vr[3], vn[3];
				float vt[3];
				for (int k = 0; k < 2; ++k)
				{
					vr[0] = to_vec3f(r[L[k][0]]); vn[0] = to_vec3f(n[L[k][0]]); vt[0] = tex[L[k][0]];
					vr[1] = to_vec3f(r[L[k][1]]); vn[1] = to_vec3f(n[L[k][1]]); vt[1] = tex[L[k][1]];
					vr[2] = to_vec3f(r[L[k][2]]); vn[2] = to_vec3f(n[L[k][2]]); vt[2] = tex[L[k][2]];
					mesh.AddFace(vr, vn, vt, c);
				}
			}

			r[0] = r[1]; r[3] = r[2];
			n[0] = n[1]; n[3] = n[2];
		}
	}
}

void tesselateHalfSphere(GLMesh& mesh, const vec3d& r0, float R, const vec3d& n0, float tex, GLColor c, int ndivs, int nsecs)
{
	quatd q0(vec3d(0, 0, 1), n0);
	const int M = nsecs;
	const int N = ndivs;
	for (int j = 0; j < M; ++j)
	{
		double th0 = 0.5 * PI * j / (double)M;
		double th1 = 0.5 * PI * (j + 1) / (double)M;
		double z1 = sin(th0);
		double z2 = sin(th1);

		double ct0 = cos(th0);
		double ct1 = cos(th1);

		double r1 = R * ct0;
		double r2 = R * ct1;

		if (j < M - 1)
		{
			for (int i = 0; i < N; ++i)
			{
				double w0 = 2 * PI * i / (double)N;
				double w1 = 2 * PI * (i+1) / (double)N;
				double x0 = cos(w0), x1 = cos(w1);
				double y0 = sin(w0), y1 = sin(w1);

				vec3d r[4] = {
					vec3d(r1 * x0, r1 * y0, R * z1),
					vec3d(r1 * x1, r1 * y1, R * z1),
					vec3d(r2 * x1, r2 * y1, R * z2),
					vec3d(r2 * x0, r2 * y0, R * z2),
				};

				vec3d n[4] = {
					vec3d(ct0 * x0, ct0 * y0, z1),
					vec3d(ct0 * x1, ct0 * y1, z1),
					vec3d(ct1 * x1, ct1 * y1, z2),
					vec3d(ct1 * x0, ct1 * y0, z2),
				};

				for (int k = 0; k < 4; ++k)
				{
					q0.RotateVector(r[k]); r[k] += r0;
					q0.RotateVector(n[k]);
				}

				int L[2][3] = { {0,1,2}, {2,3,0} };
				vec3f vr[3], vn[3];
				for (int k = 0; k < 2; ++k)
				{
					vr[0] = to_vec3f(r[L[k][0]]); vn[0] = to_vec3f(n[L[k][0]]);
					vr[1] = to_vec3f(r[L[k][1]]); vn[1] = to_vec3f(n[L[k][1]]);
					vr[2] = to_vec3f(r[L[k][2]]); vn[2] = to_vec3f(n[L[k][2]]);

					mesh.AddFace(vr, vn, tex, c);
				}
			}
		}
		else
		{
			vec3d ri1(0, 0, R); q0.RotateVector(ri1);
			vec3d rb = r0 + ri1;
			vec3d nb(0, 0, 1); q0.RotateVector(nb);

			for (int i = 0; i < N; ++i)
			{
				double w0 = 2 * PI * i / (double)N;
				double w1 = 2 * PI * (i+1) / (double)N;
				double x0 = cos(w0), x1 = cos(w1);
				double y0 = sin(w0), y1 = sin(w1);

				vec3d r[2] = {
					vec3d(r1 * x0, r1 * y0, R * z1),
					vec3d(r1 * x1, r1 * y1, R * z1)
				};

				vec3d n[2] = {
					vec3d(ct0 * x0, ct0 * y0, z1),
					vec3d(ct0 * x1, ct0 * y1, z1)
				};

				q0.RotateVector(r[0]); r[0] += r0;
				q0.RotateVector(r[1]); r[1] += r0;
				q0.RotateVector(n[0]);
				q0.RotateVector(n[1]);

				vec3f vr[3], vn[3];
				vr[0] = to_vec3f(  rb); vn[0] = to_vec3f(  nb);
				vr[1] = to_vec3f(r[0]); vn[1] = to_vec3f(n[0]);
				vr[2] = to_vec3f(r[1]); vn[2] = to_vec3f(n[1]);

				mesh.AddFace(vr, vn, tex, c);
			}
		}
	}
}


void CGLLinePlot::UpdateSmooth3DLines(FEState& s, int ntime)
{
	Post::LineData& lineData = m_lineData->GetLineData(ntime);

	int NL = lineData.Lines();
	if (NL == 0) { m_mesh.Clear(); return; }

	int maxseg = 0;
	for (int i = 0; i < NL; ++i)
	{
		if (lineData.Line(i).m_segId > maxseg) maxseg = lineData.Line(i).m_segId;
	}
	if (maxseg == 0) maxseg = 1;

	float vmin = m_range.min;
	float vmax = m_range.max;
	if (vmin == vmax) vmax++;

	const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

	const int NSEG = 12; // length segments of smooth path
	const int NDIV = 8; // radial divisions
	const int NSEC = 4;  // spherical sections for half-sphere

	// allocate mesh
	int trisPerLine = NSEG * NDIV * 2 + 2 * ((NSEC - 1) * NDIV * 2 + NDIV);
	int totalTris = NL * trisPerLine;
	m_mesh.Clear();

	for (int i = 0; i < NL; ++i)
	{
		LINESEGMENT& l = lineData.Line(i);
		if (m_show || ShowLine(l, s))
		{
			vec3f n = l.m_r1 - l.m_r0;
			float L = n.Length();
			n.Normalize();

			vec3d e1 = l.m_t0; e1.Normalize();
			vec3d e2 = l.m_t1; e2.Normalize();

			float f0 = (l.m_val[0] - vmin) / (vmax - vmin);
			float f1 = (l.m_val[1] - vmin) / (vmax - vmin);

			GLColor c(255, 255, 255);
			if (m_ncolor == COLOR_SOLID) c = m_col;
			else if (m_ncolor == COLOR_SEGMENT)
			{
				int nid = l.m_segId;// randomize(l.m_segId, maxseg);
				float f = (float)nid / (float)maxseg;
//				f = fmod(536.f * f + 0.38756, 1.f);
				c = map.map(f);
			}

			// render cylinder
			tesselateSmoothPath(m_mesh, to_vec3d(l.m_r0), to_vec3d(l.m_r1), m_line, e1, e2, f0, f1, c, NSEG, NDIV);

			// render caps
			if (l.m_end[0] == 1) tesselateHalfSphere(m_mesh, to_vec3d(l.m_r0), m_line, e1, f0, c, NDIV, NSEC);
			if (l.m_end[1] == 1) tesselateHalfSphere(m_mesh, to_vec3d(l.m_r1), m_line, e2, f1, c, NDIV, NSEC);
		}
	}
}
