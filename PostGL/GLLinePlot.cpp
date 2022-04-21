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

	AddIntParam(0, "Data field")->SetEnumNames("@data_scalar");
	AddIntParam(0, "Color mode")->SetEnumNames("Solid\0Segments\0Line Data\0Model Data\0");
	AddColorParam(GLColor(255, 0, 0), "Solid color");
	AddIntParam(0, "Color map")->SetEnumNames("@color_map");
	AddIntParam(0, "render mode")->SetEnumNames("lines\0lines 3D\0smooth lines 3D\0");
	AddDoubleParam(1.0, "line width");
	AddIntParam(0, "Max Range type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "User max");
	AddIntParam(0, "Min Range type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "User min");
	AddBoolParam(true, "Show on hidden elements");
	AddBoolParam(true, "Show legend");

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
				LINESEGMENT& l = lineData.Line(i);

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

//=============================================================================
PointDataModel::PointDataModel(FEPostModel* fem) : m_fem(fem)
{
	int ns = fem->GetStates();
	if (ns > 0) m_point.resize(ns);
}

PointDataModel::~PointDataModel()
{
	delete m_src;
}

void PointDataModel::Clear()
{
	m_point.clear();
	int ns = m_fem->GetStates();
	if (ns > 0) m_point.resize(ns);
}

void PointDataModel::AddDataField(const std::string& dataName)
{
	m_dataNames.push_back(dataName);
}

bool PointDataModel::Reload()
{
	if (m_src) return m_src->Reload();
	else return false;
}

//-----------------------------------------------------------------------------
void PointData::AddPoint(vec3f a, int nlabel)
{
	PointData::POINT p;
	p.m_r = a;
	p.nlabel = nlabel;
	m_pt.push_back(p);
}

//-----------------------------------------------------------------------------
void PointData::AddPoint(vec3f a, const std::vector<float>& data, int nlabel)
{
	PointData::POINT p;
	p.m_r = a;
	p.nlabel = nlabel;
	int n = data.size();
	if (n > MAX_POINT_DATA_FIELDS) n = MAX_POINT_DATA_FIELDS;
	for (int i = 0; i < n; ++i) p.val[i] = data[i];
	m_pt.push_back(p);
}

//=============================================================================

REGISTER_CLASS(CGLPointPlot, CLASS_PLOT, "points", 0);

//-----------------------------------------------------------------------------
CGLPointPlot::CGLPointPlot()
{
	SetTypeString("points");

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
	AddIntParam(0, "Max Range type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "User max");
	AddIntParam(0, "Min Range type")->SetEnumNames("dynamic\0static\0user\0");
	AddDoubleParam(0, "User min");

	m_pointSize = 8.f;
	m_renderMode = 0;
	m_colorMode = 0;
	m_solidColor = GLColor(0, 0, 255);
	m_showLegend = true;

	m_range.min = 0.0; m_range.max = 1.0;
	m_range.mintype = m_range.maxtype = RANGE_DYNAMIC;

	GLLegendBar* bar = new GLLegendBar(&m_Col, 0, 0, 120, 500);
	bar->align(GLW_ALIGN_LEFT | GLW_ALIGN_VCENTER);
	bar->copy_label(szname);
	bar->ShowTitle(true);
	bar->hide();
	SetLegendBar(bar);

	UpdateData(false);
}

void CGLPointPlot::SetPointDataModel(PointDataModel* pointData)
{
	m_pointData = pointData;
	std::vector<std::string> dataNames = pointData->GetDataNames();

	int nsize = 0;
	for (int i = 0; i < dataNames.size(); ++i)
	{
		nsize += dataNames[i].length();
		nsize += 1; // null character
	}
	nsize++; // final null character

	char* buf = new char[nsize];
	char* c = buf;
	for (int i = 0; i < dataNames.size(); ++i)
	{
		string& s = dataNames[i];
		strcpy(c, s.c_str());
		c += s.length();
		*c++ = 0;
	}
	*c++ = 0;

	Param& p = GetParam(DATA_FIELD);
	p.CopyEnumNames(buf);

	delete buf;

}

void CGLPointPlot::Reload()
{
	m_pointData->Reload();
}

//-----------------------------------------------------------------------------
CGLPointPlot::~CGLPointPlot()
{
	delete m_pointData;
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

		m_range.maxtype = GetIntValue(MAX_RANGE_TYPE);
		m_range.mintype = GetIntValue(MIN_RANGE_TYPE);
		if (m_range.maxtype == RANGE_USER) m_range.max = GetFloatValue(USER_MAX);
		if (m_range.mintype == RANGE_USER) m_range.min = GetFloatValue(USER_MIN);

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
		SetIntValue(MAX_RANGE_TYPE, m_range.maxtype);
		SetIntValue(MIN_RANGE_TYPE, m_range.mintype);
		SetFloatValue(USER_MAX, m_range.max);
		SetFloatValue(USER_MIN, m_range.min);
	}

	return false;
}

//-----------------------------------------------------------------------------
void CGLPointPlot::Render(CGLContext& rc)
{
	FEPostModel& fem = *GetModel()->GetFSModel();
	int ns = GetModel()->CurrentTimeIndex();
	if ((ns < 0) || (ns >= fem.GetStates())) return;
	if (m_pointData->GetPointData(ns).Points() <= 0) return;

	switch (m_renderMode)
	{
	case 0: RenderPoints(); break;
	case 1: RenderSpheres(); break;
	}
}

void CGLPointPlot::RenderPoints()
{
	FEPostModel& fem = *GetModel()->GetFSModel();
	int ns = fem.CurrentTimeIndex();
	
	PointData& pd = m_pointData->GetPointData(ns);

	int ndata = GetIntValue(DATA_FIELD);
	if (ndata < 0) ndata = 0;

	// evaluate the range
	float fmin = 1e34f, fmax = -1e34f;
	int NP = pd.Points();
	for (int i = 0; i < NP; ++i)
	{
		PointData::POINT& p = pd.Point(i);
		float v = p.val[ndata];

		if (v < fmin) fmin = v;
		if (v > fmax) fmax = v;
	}
	if (fmax == fmin) fmax++;

	switch (m_range.mintype)
	{
	case 1: if (fmin > m_range.min) fmin = m_range.min; break;
	case 2: fmin = m_range.min; break;
	}

	switch (m_range.maxtype)
	{
	case 1: if (fmax < m_range.max) fmax = m_range.max; break;
	case 2: fmax = m_range.max; break;
	}

	m_range.min = fmin;
	m_range.max = fmax;

	if (GetLegendBar())
	{
		GetLegendBar()->SetRange(fmin, fmax);
	}

	const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

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
				PointData::POINT& p = pd.Point(i);

				if (m_colorMode == 1)
				{
					double w = (p.val[ndata] - fmin) / (fmax - fmin);
					GLColor c = map.map(w);
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
	FEPostModel& fem = *GetModel()->GetFSModel();
	int ns = fem.CurrentTimeIndex();
	PointData& pd = m_pointData->GetPointData(ns);

	int ndata = 0;
	int colorMode = m_colorMode;
	if (colorMode == 1)
	{
		if (m_pointData->GetDataFields() == 0) colorMode = 0;
		else ndata = GetIntValue(DATA_FIELD);
		if (ndata < 0) {
			colorMode = 0; ndata = 0;
		}
	}

	// evaluate the range
	float fmin = 1e34f, fmax = -1e34f;
	int NP = pd.Points();
	for (int i = 0; i < NP; ++i)
	{
		PointData::POINT& p = pd.Point(i);
		float v = p.val[ndata];

		if (v < fmin) fmin = v;
		if (v > fmax) fmax = v;
	}
	if (fmax == fmin) fmax++;

	switch (m_range.mintype)
	{
	case 1: if (fmin > m_range.min) fmin = m_range.min; break;
	case 2: fmin = m_range.min; break;
	}

	switch (m_range.maxtype)
	{
	case 1: if (fmax < m_range.max) fmax = m_range.max; break;
	case 2: fmax = m_range.max; break;
	}

	m_range.min = fmin;
	m_range.max = fmax;

	if (GetLegendBar())
	{
		GetLegendBar()->SetRange(fmin, fmax);
	}

	const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

	glColor3ub(m_solidColor.r, m_solidColor.g, m_solidColor.b);

	GLUquadricObj* pobj = gluNewQuadric();

	for (int i = 0; i < pd.Points(); ++i)
	{
		PointData::POINT& p = pd.Point(i);
		vec3f& c = p.m_r;

		if (m_colorMode == 1)
		{
			double w = (p.val[ndata] - fmin) / (fmax - fmin);
			GLColor c = map.map(w);
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

//-----------------------------------------------------------------------------
class OctreeBox
{
public:
	OctreeBox(BOX box, int levels) : m_box(box), m_level(levels)
	{
		if (levels == 0)
		{
			for (int i = 0; i < 8; ++i) m_child[i] = nullptr;
			return;
		}

		double R = box.Radius();

		double x0 = box.x0, x1 = box.x1;
		double y0 = box.y0, y1 = box.y1;
		double z0 = box.z0, z1 = box.z1;
		int n = 0;
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
				{
					double xa = x0 + i * (x1 - x0) * 0.5;
					double ya = y0 + j * (y1 - y0) * 0.5;
					double za = z0 + k * (z1 - z0) * 0.5;
					double xb = x0 + (i + 1.0) * (x1 - x0) * 0.5;
					double yb = y0 + (j + 1.0) * (y1 - y0) * 0.5;
					double zb = z0 + (k + 1.0) * (z1 - z0) * 0.5;
					BOX boxi(xa, ya, za, xb, yb, zb);
					boxi.Inflate(R * 1e-7);
					m_child[n++] = new OctreeBox(boxi, levels - 1);
				}
	}
	~OctreeBox() { for (int i = 0; i < 8; ++i) delete m_child[i]; }

	int addNode(vector<vec3d>& points, const vec3d& r)
	{
		if (m_box.IsInside(r) == false) return -1;

		if (m_level == 0)
		{
			const double eps = 1e-12;
			for (int i = 0; i < m_nodes.size(); ++i)
			{
				vec3d& ri = points[m_nodes[i]];
				if ((ri - r).SqrLength() <= eps)
				{
					// node is already in list
					return m_nodes[i];
				}
			}

			// if we get here, the node is in this box, 
			// but not in the points array yet, so add it
			points.push_back(r);
			m_nodes.push_back(points.size() - 1);
			return points.size() - 1;
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				int n = m_child[i]->addNode(points, r);
				if (n >= 0) return n;
			}
			return -1;
		}
	}

private:
	int			m_level;
	BOX			m_box;
	OctreeBox* m_child[8];
	vector<int>	m_nodes;
};

class Segment
{
public:
	struct LINE
	{
		int	m_index;	// index into mesh' edge array
		int	m_ln[2];	// local node indices in segment's node array
		int m_orient;	// 1 or -1, depending on the orientation in the segment
	};

public:
	Segment(FECurveMesh* mesh) : m_mesh(mesh) {}
	Segment(const Segment& s) { m_mesh = s.m_mesh; m_pt = s.m_pt; m_seg = s.m_seg; }
	void operator = (const Segment& s) { m_mesh = s.m_mesh; m_pt = s.m_pt; m_seg = s.m_seg; }

	void Add(int n)
	{
		LINE l;
		l.m_index = n;
		l.m_orient = 0;
		l.m_ln[0] = l.m_ln[1] = -1;
		m_seg.push_back(l);
	}

	int Points() const { return m_pt.size(); }
	vec3d& Point(int n) { return m_mesh->Node(m_pt[n]).r; }

	void Build()
	{
		m_pt.clear();
		for (int i = 0; i < m_seg.size(); ++i)
		{
			FSEdge& edge = m_mesh->Edge(m_seg[i].m_index);
			m_seg[i].m_ln[0] = addPoint(edge.n[0]); assert(m_seg[i].m_ln[0] >= 0);
			m_seg[i].m_ln[1] = addPoint(edge.n[1]); assert(m_seg[i].m_ln[1] >= 0);
		}
		assert((m_pt.size() == m_seg.size() + 1) || (m_pt.size() == m_seg.size()));

		Sort();
	}

	int LineSegments() const { return m_seg.size(); }
	LINE& GetLineSegment(int n) { return m_seg[n]; }

private:
	int addPoint(int n)
	{
		for (int j = 0; j < m_pt.size(); ++j)
		{
			if (m_pt[j] == n) return j;
		}
		m_pt.push_back(n);
		return m_pt.size() - 1;
	}

	void Sort()
	{
		int pts = m_pt.size();
		vector<int> tag(pts, 0);
		for (int i = 0; i < m_seg.size(); ++i)
		{
			tag[m_seg[i].m_ln[0]]++;
			tag[m_seg[i].m_ln[1]]++;
		}

		// find one end
		vector<int> NLT; NLT.reserve(pts);
		for (int i = 0; i < pts; ++i)
		{
			assert((tag[i] == 1) || (tag[i] == 2));
			if (tag[i] == 1)
			{
				NLT.push_back(i);
				break;
			}
		}

		int nltsize = pts;
		if (NLT.empty())
		{
			// this is probably a loop, so just pick the first point
			NLT.push_back(0);
			nltsize++;
		}
		assert(NLT.size() == 1);

		int nsegs = m_seg.size();
		int seg0 = 0;
		while (NLT.size() != nltsize)
		{
			int n0 = NLT.back();

			// find a segment with this node
			for (int i = seg0; i < nsegs; ++i)
			{
				int m0 = m_seg[i].m_ln[0];
				int m1 = m_seg[i].m_ln[1];
				if ((m0 == n0) || (m1 == n0))
				{
					if (m0 == n0)
					{
						NLT.push_back(m1);
						m_seg[i].m_orient = 1;
					}
					else
					{
						NLT.push_back(m0);
						m_seg[i].m_orient = -1;
					}
					swapSegments(i, seg0);
					seg0++;
					break;
				}
			}
		}

		// reorder nodes
		vector<int> NLTi(pts);
		vector<int> newpt(pts);
		for (int i = 0; i < pts; ++i)
		{
			int ni = NLT[i];
			NLTi[ni] = i;
			newpt[i] = m_pt[ni];
		}
		m_pt = newpt;

		// reorder segments
		for (int i = 0; i < m_seg.size(); ++i)
		{
			LINE& li = m_seg[i];
			li.m_ln[0] = NLTi[li.m_ln[0]];
			li.m_ln[1] = NLTi[li.m_ln[1]];
		}
	}

	void swapSegments(int a, int b)
	{
		if (a == b) return;
		LINE tmp = m_seg[a];
		m_seg[a] = m_seg[b];
		m_seg[b] = tmp;
	}

private:
	FECurveMesh* m_mesh;
	vector<LINE>	m_seg;
	vector<int>		m_pt;
};

LineDataSource::LineDataSource(LineDataModel* mdl) : m_mdl(mdl)
{
	if (mdl) mdl->SetLineDataSource(this);
}

LineDataModel::LineDataModel(FEPostModel* fem) : m_fem(fem)
{
	m_src = nullptr;
	int ns = fem->GetStates();
	if (ns != 0) m_line.resize(ns);
}

LineDataModel::~LineDataModel()
{
	delete m_src;
}

void LineDataModel::Clear()
{
	m_line.clear();
	int ns = m_fem->GetStates();
	if (ns != 0) m_line.resize(ns);
}

void LineData::processLines()
{
	int lines = Lines();

	// find the bounding box
	BOX box;
	for (int i = 0; i < lines; ++i)
	{
		Post::LINESEGMENT& line = Line(i);
		box += to_vec3d(line.m_r0);
		box += to_vec3d(line.m_r1);
	}

	// inflate a little
	double R = box.GetMaxExtent(); if (R == 0.0) R = 1.0;
	box.Inflate(R * 1e-5);

	// start adding points
	int l = (int)(log(lines) / log(8.0));
	if (l < 3) l = 3;
	if (l > 8) l = 8;
	OctreeBox ocb(box, l);
	vector<pair<int, int> > ptIndex; ptIndex.resize(lines, pair<int, int>(-1, -1));
	vector<vec3d> points; points.reserve(lines * 2);
	for (int i = 0; i < lines; ++i)
	{
		Post::LINESEGMENT& line = Line(i);
		ptIndex[i].first = ocb.addNode(points, to_vec3d(line.m_r0)); assert(ptIndex[i].first >= 0);
		ptIndex[i].second = ocb.addNode(points, to_vec3d(line.m_r1)); assert(ptIndex[i].second >= 0);
		assert(ptIndex[i].first != ptIndex[i].second);
	}

	// now build a curve mesh
	// (this is used to find the segments, i.e. the connected lines)
	FECurveMesh mesh;
	mesh.Create(points.size(), lines);
	for (int i = 0; i < points.size(); ++i)
	{
		FSNode& node = mesh.Node(i);
		node.r = points[i];
	}
	for (int i = 0; i < lines; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		edge.n[0] = ptIndex[i].first;
		edge.n[1] = ptIndex[i].second;
	}
	mesh.BuildMesh();

	// figure out the segments
	int nsegs = mesh.Segments(); assert(nsegs >= 1);
	vector<Segment> segment(nsegs, Segment(&mesh));
	for (int i = 0; i < lines; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		segment[edge.m_gid].Add(i);
		m_Line[i].m_segId = edge.m_gid;
	}

	// process each segment
	for (int n = 0; n < nsegs; ++n)
	{
		Segment& segn = segment[n];
		segn.Build();

		int pts = segn.Points();
		vector<vec3d> t(pts, vec3d(0, 0, 0));
		for (int i = 0; i < pts - 1; ++i)
		{
			vec3d a = segn.Point(i);
			vec3d b = segn.Point(i + 1);

			vec3d ti = b - a;
			t[i] += ti;
			t[i + 1] += ti;
		}

		// normalize tangents
		for (int i = 0; i < pts; ++i) t[i].Normalize();

		// assign the tangents
		int nseg = segn.LineSegments();
		for (int i = 0; i < nseg; ++i)
		{
			Segment::LINE& seg = segn.GetLineSegment(i);

			Post::LINESEGMENT& line = Line(seg.m_index);

			if (i == 0)
			{
				if (seg.m_orient > 0) line.m_end[0] = 1;
				if (seg.m_orient < 0) line.m_end[1] = 1;
			}
			else if (i == nseg - 1)
			{
				if (seg.m_orient > 0) line.m_end[1] = 1;
				if (seg.m_orient < 0) line.m_end[0] = 1;
			}

			line.m_t0 = t[seg.m_ln[0]];
			line.m_t1 = t[seg.m_ln[1]];

			assert(seg.m_orient != 0);
			if (seg.m_orient < 0)
			{
				line.m_t0 = -line.m_t0;
				line.m_t1 = -line.m_t1;
			}
		}
	}
}
