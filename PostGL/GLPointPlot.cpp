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
#include "GLPointPlot.h"
#include "GLModel.h"
#include <PostLib/FEPostModel.h>
#include <FSCore/ClassDescriptor.h>
#include "../FEBioStudio/GLRenderEngine.h"

using namespace Post;

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
	AddChoiceParam(0, "point size source");
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

int stringlist_to_szstring(std::vector<std::string>& sl, char* sz)
{
	int nsize = 0;
	for (int i = 0; i < sl.size(); ++i)
	{
		nsize += sl[i].length();
		nsize += 1; // null character
	}
	nsize++; // final null character

	if (sz == nullptr) return nsize;

	char* c = sz;
	for (int i = 0; i < sl.size(); ++i)
	{
		string& s = sl[i];
		strcpy(c, s.c_str());
		c += s.length();
		*c++ = 0;
	}
	*c++ = 0;

	return nsize;
}

void CGLPointPlot::SetPointDataModel(PointDataModel* pointData)
{
	m_pointData = pointData;

	// get all data field names
	std::vector<std::string> dataNames = pointData->GetDataNames();

	// convert to zero-terminated string
	int nsize = stringlist_to_szstring(dataNames, nullptr);
	char* buf = new char[nsize];
	stringlist_to_szstring(dataNames, buf);

	// copy enums
	Param& dataField = GetParam(DATA_FIELD);
	dataField.CopyEnumNames(buf);
	delete buf;

	// for point size data, we do the same, but append
	dataNames.insert(dataNames.begin(), "(default)");
	nsize = stringlist_to_szstring(dataNames, nullptr);
	buf = new char[nsize];
	stringlist_to_szstring(dataNames, buf);

	// copy enums
	Param& pointSizeSrc = GetParam(POINT_SIZE_SOURCE);
	pointSizeSrc.CopyEnumNames(buf);
	pointSizeSrc.SetIntValue(0);
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
void CGLPointPlot::Render(GLRenderEngine& re, CGLContext& rc)
{
	FEPostModel& fem = *GetModel()->GetFSModel();
	int ns = GetModel()->CurrentTimeIndex();
	if ((ns < 0) || (ns >= fem.GetStates())) return;
	if (m_pointData->GetPointData(ns).Points() <= 0) return;

	switch (m_renderMode)
	{
	case 0: RenderPoints(re); break;
	case 1: RenderSpheres(re); break;
	}
}

void CGLPointPlot::RenderPoints(GLRenderEngine& re)
{
	re.setMaterial(GLMaterial::CONSTANT, GLColor::White(), GLMaterial::VERTEX_COLOR);
	GLfloat size_old;
	glGetFloatv(GL_POINT_SIZE, &size_old);
	re.setPointSize(m_pointSize);

	re.renderGMeshNodes(m_pointMesh, false);
	re.setPointSize(size_old);
}

void CGLPointPlot::RenderSpheres(GLRenderEngine& re)
{
	const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

	glColor3ub(m_solidColor.r, m_solidColor.g, m_solidColor.b);

	GLUquadricObj* pobj = gluNewQuadric();

	double pointSize = m_pointSize;

	int pointSizeSource = GetIntValue(POINT_SIZE_SOURCE);

	FEPostModel& fem = *GetModel()->GetFSModel();
	int ns = fem.CurrentTimeIndex();
	PointData& pd = m_pointData->GetPointData(ns);

	for (int i = 0; i < pd.Points(); ++i)
	{
		PointData::POINT& p = pd.Point(i);
		vec3d c = to_vec3d(p.m_r);

		if (m_colorMode == 1)
		{
			GLColor c = map.map(p.tex);
			glColor3ub(c.r, c.g, c.b);
		}

		if (pointSizeSource > 0)
		{
			pointSize = m_pointSize*fabs(p.val[pointSizeSource - 1]);
		}

		if (pointSize > 0)
		{
			re.pushTransform();
			{
				re.translate(c);
				gluSphere(pobj, pointSize, 32, 32);
			}
			re.popTransform();
		}
	}
	gluDeleteQuadric(pobj);
}

void CGLPointPlot::Update(int ntime, float dt, bool breset)
{
	m_pointMesh.Clear();

	FEPostModel& fem = *GetModel()->GetFSModel();
	int ns = GetModel()->CurrentTimeIndex();
	if ((ns < 0) || (ns >= fem.GetStates())) return;
	if (m_pointData->GetPointData(ns).Points() <= 0) return;

	// update range (this also evaluates the "texture" coordinates
	UpdateRange();

	switch (m_renderMode)
	{
	case 0: UpdatePointMesh(); break;
	case 1: UpdateTriMesh(); break;
	}
}

void CGLPointPlot::UpdateRange()
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

	for (int i = 0; i < pd.Points(); ++i)
	{
		PointData::POINT& p = pd.Point(i);
		p.tex = (p.val[ndata] - fmin) / (fmax - fmin);
	}
}

void CGLPointPlot::UpdatePointMesh()
{
	m_pointMesh.Clear();

	FEPostModel& fem = *GetModel()->GetFSModel();
	int ns = fem.CurrentTimeIndex();
	PointData& pd = m_pointData->GetPointData(ns);

	int NP = pd.Points(); 
	m_pointMesh.Clear();

	const CColorMap& map = ColorMapManager::GetColorMap(m_Col.GetColorMap());

	GLColor c = m_solidColor;
	for (int i = 0; i < NP; ++i)
	{
		PointData::POINT& p = pd.Point(i);
		if (m_colorMode == 1) c = map.map(p.tex);
		m_pointMesh.AddNode(p.m_r, c);
	}
}

void CGLPointPlot::UpdateTriMesh()
{

}
