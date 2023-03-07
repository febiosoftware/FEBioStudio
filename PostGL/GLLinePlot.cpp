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
#include <PostGL/GLModel.h>
#include <PostLib/FEPostModel.h>
#include <MeshLib/FECurveMesh.h>
#include <GLLib/glx.h>
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

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 120, 500);
	bar->align(GLW_ALIGN_LEFT | GLW_ALIGN_VCENTER);
	bar->copy_label(szname);
	bar->ShowTitle(true);
	bar->hide();
	SetLegendBar(bar);

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

		if (GetLegendBar())
		{
			bool b = (m_showLegend && (m_ncolor != 0));
			if (b) GetLegendBar()->show(); else GetLegendBar()->hide();
		}
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
void CGLLinePlot::Render(CGLContext& rc)
{
	if (m_lineData == nullptr) return;

	CGLModel& glm = *GetModel();
	FEPostModel& fem = *glm.GetFSModel();
	int ns = glm.CurrentTimeIndex();

	GLfloat zero[4] = { 0.f };
	GLfloat one[4] = { 1.f, 1.f, 1.f, 1.f };
	GLfloat col[4] = { (GLfloat)m_col.r, (GLfloat)m_col.g, (GLfloat)m_col.b, 1.f};
	GLfloat amb[4] = { 0.1f, 0.1f, 0.1f, 1.f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 64);

	GLfloat line_old;
	glGetFloatv(GL_LINE_WIDTH, &line_old);
	glLineWidth(m_line);
	if ((ns >= 0) && (ns <fem.GetStates()))
	{
		FEState& s = *fem.GetState(ns);
		Post::LineData& lineData = m_lineData->GetLineData(ns);
		int NL = lineData.Lines();
		if (NL > 0)
		{
			glPushAttrib(GL_ENABLE_BIT);
			{
				switch (m_nmode)
				{
				case 0: RenderLines(s, ns); break;
				case 1: Render3DLines(s, ns); break;
				case 2: Render3DSmoothLines(s, ns); break;
				}
			}
			glPopAttrib();
		}
	}
	glLineWidth(line_old);
}

//-----------------------------------------------------------------------------
int randomize(int n, int nmax)
{
	int a = nmax / 3; if (a == 0) a = 1;
	int b = nmax / 2;
	return (a * n + b) % nmax;
}

//-----------------------------------------------------------------------------
void CGLLinePlot::RenderLines(FEState& s, int ntime)
{
	glDisable(GL_LIGHTING);
	Post::LineData& lineData = m_lineData->GetLineData(ntime);
	if (m_ncolor == COLOR_SOLID)
	{
		glColor3ub(m_col.r, m_col.g, m_col.b);
		glBegin(GL_LINES);
		{
			int NL = lineData.Lines();
			for (int i = 0; i < NL; ++i)
			{
				LINESEGMENT& l = lineData.Line(i);
				if (m_show || ShowLine(l, s))
				{
					glx::line(l.m_r0, l.m_r1);
				}
			}
		}
		glEnd();
	}
	else if (m_ncolor == COLOR_SEGMENT)
	{
		CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

		int maxseg = 0;
		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			if (lineData.Line(i).m_segId > maxseg) maxseg = lineData.Line(i).m_segId;
		}
		if (maxseg == 0) maxseg = 1;

		glBegin(GL_LINES);
		{
			for (int i = 0; i < NL; ++i)
			{
				LINESEGMENT& l = lineData.Line(i);

				int n = l.m_segId;// randomize(l.m_segId, maxseg);
				float f = (float)n / (float)maxseg;
				GLColor c = map.map(f);
				glx::glcolor(c);
				
				if (m_show || ShowLine(l, s))
				{
					glx::line(l.m_r0, l.m_r1);
				}
			}
		}
		glEnd();
	}
	else
	{
		CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

		float vmin = m_range.min;
		float vmax = m_range.max;
		if (vmin == vmax) vmax++;

		Post::LineData& lineData = m_lineData->GetLineData(ntime);
		glBegin(GL_LINES);
		{
			int NL = lineData.Lines();
			for (int i = 0; i < NL; ++i)
			{
				LINESEGMENT& l = lineData.Line(i);
				if (m_show || ShowLine(l, s))
				{
					float f0 = (l.m_val[0] - vmin) / (vmax - vmin);
					float f1 = (l.m_val[1] - vmin) / (vmax - vmin);

					GLColor c0 = map.map(f0);
					GLColor c1 = map.map(f1);
					glx::line(l.m_r0, l.m_r1, c0, c1);
				}
			}
		}
		glEnd();

	}
}

//-----------------------------------------------------------------------------
void glxCylinder(float H, float R, float t0 = 0.f, float t1 = 1.f)
{
	glBegin(GL_QUAD_STRIP);
	const int N = 8;
	for (int i=0; i<=N; ++i)
	{
		double w = 2*PI*i/(double)N;
		double x = cos(w);
		double y = sin(w);
		glNormal3d(x, y, 0.0);
		glTexCoord1d(t1); glVertex3d(R*x, R*y, H);
		glTexCoord1d(t0); glVertex3d(R*x, R*y, 0);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
bool CGLLinePlot::ShowLine(LINESEGMENT& l, FEState& s)
{
	if ((l.m_elem[0] == -1) || (l.m_elem[1] == -1)) return true;

	Post::FEPostMesh* m = s.GetFEMesh();
	FSElement& e0 = m->Element(l.m_elem[0]);
	FSElement& e1 = m->Element(l.m_elem[1]);

	return (e0.IsVisible() || e1.IsVisible());
}

//-----------------------------------------------------------------------------
void CGLLinePlot::Render3DLines(FEState& s, int ntime)
{
	Post::LineData& lineData = m_lineData->GetLineData(ntime);
	if (m_ncolor == COLOR_SOLID)
	{
		glColor3ub(m_col.r, m_col.g, m_col.b);
		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			LINESEGMENT& l = lineData.Line(i);
			if (m_show || ShowLine(l, s))
			{
				vec3f n = l.m_r1 - l.m_r0;
				float L = n.Length();
				n.Normalize();

				glPushMatrix();
				{
					glTranslatef(l.m_r0.x, l.m_r0.y, l.m_r0.z);

					quatd q(vec3d(0, 0, 1), to_vec3d(n));
					vec3d r = q.GetVector();
					double angle = 180 * q.GetAngle() / PI;
					if ((angle != 0.0) && (r.Length() > 0))
						glRotated(angle, r.x, r.y, r.z);

					// render cylinder
					glxCylinder(L, m_line);
				}
				glPopMatrix();
			}
		}
	}
	else
	{
		glColor3ub(255, 255, 255);

		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_TEXTURE_1D);
		m_Col.GetTexture().MakeCurrent();

		float vmin = m_range.min;
		float vmax = m_range.max;
		if (vmin == vmax) vmax++;

		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			LINESEGMENT& l = lineData.Line(i);
			if (m_show || ShowLine(l, s))
			{
				vec3f n = l.m_r1 - l.m_r0;
				float L = n.Length();
				n.Normalize();

				glPushMatrix();
				{
					glTranslatef(l.m_r0.x, l.m_r0.y, l.m_r0.z);

					quatd q(vec3d(0, 0, 1), to_vec3d(n));
					vec3d r = q.GetVector();
					double angle = 180 * q.GetAngle() / PI;
					if ((angle != 0.0) && (r.Length() > 0))
						glRotated(angle, r.x, r.y, r.z);

					float f0 = (l.m_val[0] - vmin) / (vmax - vmin);
					float f1 = (l.m_val[1] - vmin) / (vmax - vmin);

					// render cylinder
					glxCylinder(L, m_line, f0, f1);
				}
				glPopMatrix();
			}
		}

		glPopAttrib();
	}
}

//-----------------------------------------------------------------------------
void CGLLinePlot::Render3DSmoothLines(FEState& s, int ntime)
{
	Post::LineData& lineData = m_lineData->GetLineData(ntime);
	if (m_ncolor == COLOR_SOLID)
	{
		glColor3ub(m_col.r, m_col.g, m_col.b);
		int NL = lineData.Lines();
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

				// render cylinder
				glx::drawSmoothPath(to_vec3d(l.m_r0), to_vec3d(l.m_r1), m_line, e1, e2);

				// render caps
				if (l.m_end[0] == 1) glx::drawHalfSphere(to_vec3d(l.m_r0), m_line, e1);
				if (l.m_end[1] == 1) glx::drawHalfSphere(to_vec3d(l.m_r1), m_line, e2);
			}
		}
	}
	else if (m_ncolor == COLOR_SEGMENT)
	{
		CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

		int maxseg = 0;
		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			if (lineData.Line(i).m_segId > maxseg) maxseg = lineData.Line(i).m_segId;
		}
		if (maxseg == 0) maxseg = 1;

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

				int nid = l.m_segId;// randomize(l.m_segId, maxseg);
				float f = (float)nid / (float)maxseg;
				GLColor c = map.map(f);
				glColor3ub(c.r, c.g, c.b);

				// render cylinder
				glx::drawSmoothPath(to_vec3d(l.m_r0), to_vec3d(l.m_r1), m_line, e1, e2);

				// render caps
				if (l.m_end[0] == 1) glx::drawHalfSphere(to_vec3d(l.m_r0), m_line, e1);
				if (l.m_end[1] == 1) glx::drawHalfSphere(to_vec3d(l.m_r1), m_line, e2);
			}
		}
	}
	else
	{
		glColor3ub(255, 255, 255);

		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_TEXTURE_1D);
		m_Col.GetTexture().MakeCurrent();

		float vmin = m_range.min;
		float vmax = m_range.max;
		if (vmin == vmax) vmax++;

		int NL = lineData.Lines();
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

				// render cylinder
				glx::drawSmoothPath(to_vec3d(l.m_r0), to_vec3d(l.m_r1), m_line, e1, e2, f0, f1);

				// render caps
				if (l.m_end[0] == 1) glx::drawHalfSphere(to_vec3d(l.m_r0), m_line, e1, f0);
				if (l.m_end[1] == 1) glx::drawHalfSphere(to_vec3d(l.m_r1), m_line, e2, f1);
			}
		}
		glPopAttrib();
	}
}

void CGLLinePlot::Update(int ntime, float dt, bool breset)
{
	if (m_lineData == nullptr) return;

	if ((m_ncolor == COLOR_SOLID) || ((m_ncolor==COLOR_MODEL_DATA)&&(m_nfield == -1))) return;

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
	else if (m_ncolor == COLOR_MODEL_DATA)
	{
		CGLModel& glm = *GetModel();
		FEPostModel& fem = *glm.GetFSModel();

		FEState& s = *fem.GetState(ntime);
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

	if (GetLegendBar())
	{
		GetLegendBar()->SetRange(m_range.min, m_range.max);
	}
}
