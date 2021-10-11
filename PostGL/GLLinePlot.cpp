/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <GLLib/glx.h>
using namespace Post;

//-----------------------------------------------------------------------------
CGLLinePlot::CGLLinePlot(CGLModel* po) : CGLLegendPlot(po)
{
	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "Line.%02d", n++);
	SetName(szname);

	AddIntParam(0, "Data field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "Color mode")->SetEnumNames("Solid\0Segments\0Line Data\0Model Data\0");
	AddColorParam(GLColor(255, 0, 0), "Solid color");
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddIntParam(0, "render mode")->SetEnumNames("lines\0lines 3D\0smooth lines 3D\0");
	AddDoubleParam(1.0, "line width");
	AddIntParam(0, "Max Range type")->SetEnumNames("dynamic\0static\0");
	AddBoolParam(true, "Show on hidden elements");
	AddBoolParam(true, "Show legend");

	m_line = 4.f;
	m_nmode = 0;
	m_ncolor = COLOR_SOLID;
	m_col = GLColor(255, 0, 0);
	m_nfield = -1;
	m_show = true;
	m_rangeMode = 0;
	m_showLegend = true;

	m_rng.x = 0.f;
	m_rng.y = 1.f;

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 120, 500);
	bar->align(GLW_ALIGN_LEFT | GLW_ALIGN_VCENTER);
	bar->copy_label(szname);
	bar->hide();
	SetLegendBar(bar);

	UpdateData(false);
}

//-----------------------------------------------------------------------------
CGLLinePlot::~CGLLinePlot()
{
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
		m_rangeMode = GetIntValue(RANGE_MODE);
		m_show = GetBoolValue(SHOW_ALWAYS);
		m_showLegend = GetBoolValue(SHOW_LEGEND);

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
		SetIntValue(RANGE_MODE, m_rangeMode);
		SetBoolValue(SHOW_ALWAYS, m_show);
		SetBoolValue(SHOW_LEGEND, m_showLegend);
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

//-----------------------------------------------------------------------------
void CGLLinePlot::Render(CGLContext& rc)
{
	CGLModel& glm = *GetModel();
	FEPostModel& fem = *glm.GetFEModel();
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
		Post::LineData& lineData = s.GetLineData();
		int NL = lineData.Lines();
		if (NL > 0)
		{
			glPushAttrib(GL_ENABLE_BIT);
			{
				switch (m_nmode)
				{
				case 0: RenderLines(s); break;
				case 1: Render3DLines(s); break;
				case 2: Render3DSmoothLines(s); break;
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
void CGLLinePlot::RenderLines(FEState& s)
{
	glDisable(GL_LIGHTING);
	Post::LineData& lineData = s.GetLineData();
	if (m_ncolor == COLOR_SOLID)
	{
		glColor3ub(m_col.r, m_col.g, m_col.b);
		glBegin(GL_LINES);
		{
			int NL = lineData.Lines();
			for (int i = 0; i < NL; ++i)
			{
				LINEDATA& l = lineData.Line(i);
				if (m_show || ShowLine(l, s))
				{
					glVertex3f(l.m_r0.x, l.m_r0.y, l.m_r0.z);
					glVertex3f(l.m_r1.x, l.m_r1.y, l.m_r1.z);
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
				LINEDATA& l = lineData.Line(i);

				int n = l.m_segId;// randomize(l.m_segId, maxseg);
				float f = (float)n / (float)maxseg;
				GLColor c = map.map(f);
				glColor3ub(c.r, c.g, c.b);
				
				if (m_show || ShowLine(l, s))
				{
					glVertex3f(l.m_r0.x, l.m_r0.y, l.m_r0.z);
					glVertex3f(l.m_r1.x, l.m_r1.y, l.m_r1.z);
				}
			}
		}
		glEnd();
	}
	else
	{
		CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

		float vmin = m_rng.x;
		float vmax = m_rng.y;
		if (vmin == vmax) vmax++;

		Post::LineData& lineData = s.GetLineData();
		glBegin(GL_LINES);
		{
			int NL = lineData.Lines();
			for (int i = 0; i < NL; ++i)
			{
				LINEDATA& l = lineData.Line(i);
				if (m_show || ShowLine(l, s))
				{
					float f0 = (l.m_val[0] - vmin) / (vmax - vmin);
					float f1 = (l.m_val[1] - vmin) / (vmax - vmin);

					GLColor c0 = map.map(f0);
					GLColor c1 = map.map(f1);

					glColor3ub(c0.r, c0.g, c0.b);
					glVertex3f(l.m_r0.x, l.m_r0.y, l.m_r0.z);
					glColor3ub(c1.r, c1.g, c1.b);
					glVertex3f(l.m_r1.x, l.m_r1.y, l.m_r1.z);
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
bool CGLLinePlot::ShowLine(LINEDATA& l, FEState& s)
{
	if ((l.m_elem[0] == -1) || (l.m_elem[1] == -1)) return true;

	Post::FEPostMesh* m = s.GetFEMesh();
	FEElement& e0 = m->Element(l.m_elem[0]);
	FEElement& e1 = m->Element(l.m_elem[1]);

	return (e0.IsVisible() || e1.IsVisible());
}

//-----------------------------------------------------------------------------
void CGLLinePlot::Render3DLines(FEState& s)
{
	Post::LineData& lineData = s.GetLineData();
	if (m_ncolor == COLOR_SOLID)
	{
		glColor3ub(m_col.r, m_col.g, m_col.b);
		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			LINEDATA& l = lineData.Line(i);
			if (m_show || ShowLine(l, s))
			{
				vec3f n = l.m_r1 - l.m_r0;
				float L = n.Length();
				n.Normalize();

				glPushMatrix();
				{
					glTranslatef(l.m_r0.x, l.m_r0.y, l.m_r0.z);

					quatd q(vec3f(0, 0, 1), n);
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

		float vmin = m_rng.x;
		float vmax = m_rng.y;
		if (vmin == vmax) vmax++;

		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			LINEDATA& l = lineData.Line(i);
			if (m_show || ShowLine(l, s))
			{
				vec3f n = l.m_r1 - l.m_r0;
				float L = n.Length();
				n.Normalize();

				glPushMatrix();
				{
					glTranslatef(l.m_r0.x, l.m_r0.y, l.m_r0.z);

					quatd q(vec3f(0, 0, 1), n);
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
void CGLLinePlot::Render3DSmoothLines(FEState& s)
{
	Post::LineData& lineData = s.GetLineData();
	if (m_ncolor == COLOR_SOLID)
	{
		glColor3ub(m_col.r, m_col.g, m_col.b);
		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			LINEDATA& l = lineData.Line(i);
			if (m_show || ShowLine(l, s))
			{
				vec3f n = l.m_r1 - l.m_r0;
				float L = n.Length();
				n.Normalize();

				vec3d e1 = l.m_t0; e1.Normalize();
				vec3d e2 = l.m_t1; e2.Normalize();

				// render cylinder
				glx::drawSmoothPath(l.m_r0, l.m_r1, m_line, e1, e2);

				// render caps
				if (l.m_end[0] == 1) glx::drawSphere(l.m_r0, m_line, e1); 
				if (l.m_end[1] == 1) glx::drawSphere(l.m_r1, m_line, e2);
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
			LINEDATA& l = lineData.Line(i);
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
				glx::drawSmoothPath(l.m_r0, l.m_r1, m_line, e1, e2);

				// render caps
				if (l.m_end[0] == 1) glx::drawSphere(l.m_r0, m_line, e1);
				if (l.m_end[1] == 1) glx::drawSphere(l.m_r1, m_line, e2);
			}
		}
	}
	else
	{
		glColor3ub(255, 255, 255);

		glPushAttrib(GL_ENABLE_BIT);
		glEnable(GL_TEXTURE_1D);
		m_Col.GetTexture().MakeCurrent();

		float vmin = m_rng.x;
		float vmax = m_rng.y;
		if (vmin == vmax) vmax++;

		int NL = lineData.Lines();
		for (int i = 0; i < NL; ++i)
		{
			LINEDATA& l = lineData.Line(i);
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
				glx::drawSmoothPath(l.m_r0, l.m_r1, m_line, e1, e2, f0, f1);

				// render caps
				if (l.m_end[0] == 1) glx::drawSphere(l.m_r0, m_line, e1, f0);
				if (l.m_end[1] == 1) glx::drawSphere(l.m_r1, m_line, e2, f1);
			}
		}
		glPopAttrib();
	}
}

void CGLLinePlot::Update(int ntime, float dt, bool breset)
{
	if ((m_ncolor == COLOR_SOLID) || ((m_ncolor==COLOR_MODEL_DATA)&&(m_nfield == -1))) return;

	float vmax = -1e20f;
	float vmin = 1e20f;
	if (m_ncolor == COLOR_LINE_DATA)
	{
		CGLModel& glm = *GetModel();
		FEPostModel& fem = *glm.GetFEModel();

		FEState& s = *fem.GetState(ntime);
		Post::LineData& lineData = s.GetLineData();
		int NL = lineData.Lines();

		for (int i = 0; i < NL; ++i)
		{
			LINEDATA& line = lineData.Line(i);

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
		FEPostModel& fem = *glm.GetFEModel();

		FEState& s = *fem.GetState(ntime);
		Post::LineData& lineData = s.GetLineData();
		int NL = lineData.Lines();

		NODEDATA nd1, nd2;
		for (int i = 0; i < NL; ++i)
		{
			LINEDATA& line = lineData.Line(i);

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

	if (breset || (m_rangeMode == 0))
	{
		m_rng.x = vmin;
		m_rng.y = vmax;
	}
	else
	{
		if (vmin < m_rng.x) m_rng.x = vmin;
		if (vmax > m_rng.y) m_rng.y = vmax;
	}

	if (GetLegendBar())
	{
		GetLegendBar()->SetRange(m_rng.x, m_rng.y);
	}
}

//=============================================================================

//-----------------------------------------------------------------------------
CGLPointPlot::CGLPointPlot(CGLModel* po) : CGLLegendPlot(po)
{
	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "Points.%02d", n++);
	SetName(szname);

	AddDoubleParam(8.0, "point size");
	AddIntParam(0, "render mode")->SetEnumNames("points\0sphere\0");
	AddIntParam(0, "color mode")->SetEnumNames("solid\0color map\0");
	AddColorParam(GLColor::White(), "solid color");
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddChoiceParam(0, "Data field");
	AddBoolParam(true, "Show legend");

	m_pointSize = 8.f;
	m_renderMode = 0;
	m_colorMode = 0;
	m_solidColor = GLColor(0, 0, 255);
	m_showLegend = true;

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 120, 500);
	bar->align(GLW_ALIGN_LEFT | GLW_ALIGN_VCENTER);
	bar->copy_label(szname);
	bar->hide();
	SetLegendBar(bar);

	UpdateData(false);
}

void CGLPointPlot::AddDataField(const std::string& dataName)
{
	m_dataNames.push_back(dataName);

	int nsize = 0;
	for (int i = 0; i < m_dataNames.size(); ++i)
	{
		nsize += m_dataNames[i].length();
		nsize += 1; // null character
	}
	nsize++; // final null character

	char* buf = new char[nsize];
	char* c = buf;
	for (int i = 0; i < m_dataNames.size(); ++i)
	{
		string& s = m_dataNames[i];
		strcpy(c, s.c_str());
		c += s.length();
		*c++ = 0;
	}
	*c++ = 0;

	Param& p = GetParam(DATA_FIELD);
	p.CopyEnumNames(buf);

	delete buf;
}

//-----------------------------------------------------------------------------
CGLPointPlot::~CGLPointPlot()
{
	FEPostModel* fem = GetModel()->GetFEModel();
	if (fem)
	{
		for (int i = 0; i < fem->GetStates(); ++i)
		{
			FEState& state = *fem->GetState(i);
			state.ClearPoints();
		}
	}
}

//-----------------------------------------------------------------------------
bool CGLPointPlot::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_pointSize  = GetFloatValue(POINT_SIZE);
		m_renderMode = GetIntValue(RENDER_MODE);
		m_colorMode  = GetIntValue(COLOR_MODE);
		m_solidColor = GetColorValue(SOLID_COLOR);
		m_Col.SetColorMap(GetIntValue(COLOR_MAP));
		m_showLegend = GetBoolValue(SHOW_LEGEND);

		if (GetLegendBar())
		{
			bool b = (m_showLegend && (m_colorMode != 0));
			if (b) GetLegendBar()->show(); else GetLegendBar()->hide();
		}
	}
	else
	{
		SetFloatValue(POINT_SIZE , m_pointSize );
		SetIntValue  (RENDER_MODE, m_renderMode);
		SetIntValue  (COLOR_MODE , m_colorMode );
		SetColorValue(SOLID_COLOR, m_solidColor);
		SetIntValue  (COLOR_MAP  , m_Col.GetColorMap());
	}

	return false;
}

//-----------------------------------------------------------------------------
void CGLPointPlot::Render(CGLContext& rc)
{
	FEPostModel& fem = *GetModel()->GetFEModel();
	int ns = GetModel()->CurrentTimeIndex();
	if ((ns < 0) || (ns >= fem.GetStates())) return;
	FEState& s = *fem.GetState(ns);
	int NP = s.Points();
	if (NP <= 0) return;

	switch (m_renderMode)
	{
	case 0: RenderPoints(); break;
	case 1: RenderSpheres(); break;
	}
}

void CGLPointPlot::RenderPoints()
{
	FEPostModel& fem = *GetModel()->GetFEModel();
	int ns = fem.CurrentTimeIndex();
	FEState& s = *fem.GetState(ns);

	int ndata = GetIntValue(DATA_FIELD);
	if (ndata < 0) ndata = 0;

	// evaluate the range
	float fmin = 1e99, fmax = -1e99;
	int NP = s.Points();
	for (int i = 0; i < NP; ++i)
	{
		POINTDATA& p = s.Point(i);
		float v = p.val[ndata];

		if (v < fmin) fmin = v;
		if (v > fmax) fmax = v;
	}
	if (fmax == fmin) fmax++;

	if (GetLegendBar())
	{
		GetLegendBar()->SetRange(fmin, fmax);
	}

	CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());
	map.SetRange(fmin, fmax);

	GLfloat size_old;
	glGetFloatv(GL_POINT_SIZE, &size_old);
	glPushAttrib(GL_ENABLE_BIT);
	{
		glColor3ub(m_solidColor.r, m_solidColor.g, m_solidColor.b);

		glPointSize(m_pointSize);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);

		glBegin(GL_POINTS);
		{
			for (int i = 0; i < NP; ++i)
			{
				POINTDATA& p = s.Point(i);

				if (m_colorMode == 1)
				{
					GLColor c = map.map(p.val[ndata]);
					glColor3ub(c.r, c.g, c.b);
				}

				glVertex3f(p.m_r.x, p.m_r.y, p.m_r.z);
			}
		}
		glEnd();
	}
	glPopAttrib();
	glPointSize(size_old);
}

void CGLPointPlot::RenderSpheres()
{
	FEPostModel& fem = *GetModel()->GetFEModel();
	int ns = fem.CurrentTimeIndex();
	FEState& s = *fem.GetState(ns);

	int ndata = 0;
	int colorMode = m_colorMode;
	if (colorMode == 1)
	{
		if (m_dataNames.empty()) colorMode = 0;
		else ndata = GetIntValue(DATA_FIELD);
		if (ndata < 0) {
			colorMode = 0; ndata = 0;
		}
	}

	// evaluate the range
	float fmin = 1e99, fmax = -1e99;
	int NP = s.Points();
	for (int i = 0; i < NP; ++i)
	{
		POINTDATA& p = s.Point(i);
		float v = p.val[ndata];

		if (v < fmin) fmin = v;
		if (v > fmax) fmax = v;
	}
	if (fmax == fmin) fmax++;

	CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());
	map.SetRange(fmin, fmax);

	glColor3ub(m_solidColor.r, m_solidColor.g, m_solidColor.b);

	GLUquadricObj* pobj = gluNewQuadric();

	for (int i = 0; i < s.Points(); ++i)
	{
		POINTDATA& p = s.Point(i);
		vec3f& c = p.m_r;

		if (m_colorMode == 1)
		{
			GLColor c = map.map(p.val[ndata]);
			glColor3ub(c.r, c.g, c.b);
		}

		glPushMatrix();
		{
			glTranslatef(c.x, c.y, c.z);
			gluSphere(pobj, m_pointSize, 32, 32);
		}
		glPopMatrix();
	}
	gluDeleteQuadric(pobj);
}
