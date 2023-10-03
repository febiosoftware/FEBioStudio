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
#include "GLCurveProbe.h"
#include "GLModel.h"
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshLib/FENodeNodeList.h>
#include <GLLib/glx.h>
#include <sstream>
#include <fstream>
using namespace Post;

REGISTER_CLASS(GLCurveProbe, CLASS_PLOT, "curve_probe", 0);

GLCurveProbe::GLCurveProbe()
{
	SetTypeString("curve_probe");

	static int n = 1;
	char sz[256] = { 0 };
	sprintf(sz, "CurveProbe%d", n++);
	SetName(sz);

	m_col = GLColor(255, 0, 0);
	AddColorParam(m_col, "color");
}

void GLCurveProbe::Render(CGLContext& rc)
{
	if (m_path.size())
	{
		GLColor c = m_col;
		glColor3ub(c.r, c.g, c.b);
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < m_path.size(); ++i)
		{
			vec3d& r = m_path[i];
			glx::vertex3d(r);
		}
		glEnd();
		glBegin(GL_POINTS);
		glx::vertex3d(m_path[0]);
		glEnd();
		glPopAttrib();
	}
}

void GLCurveProbe::Update()
{

}

void GLCurveProbe::Update(int ntime, float dt, bool breset)
{

}

bool GLCurveProbe::UpdateData(bool bsave)
{
	return false;
}

GLColor GLCurveProbe::GetColor() const
{
	return m_col;
}

void GLCurveProbe::SetColor(const GLColor& c)
{
	m_col = c;
}

bool GLCurveProbe::ImportPoints(const std::string& fileName)
{
	m_path.clear();
	m_curve.clear();
	std::ifstream fp(fileName);
	string line;
	double s = 1000;
	while (std::getline(fp, line))
	{
		if (line.empty() == false)
		{
			if (line[0] != '#')
			{
				double x, y, z;
				int n = sscanf(line.c_str(), "%lg %lg %lg", &x, &y, &z);
				m_path.push_back(vec3d(s*x, s*y, s*z));
			}
		}
	}
	double L = 0.0;
	for (int i = 1; i < m_path.size(); ++i)
	{
		vec3d r0 = m_path[i - 1];
		vec3d r1 = m_path[i];
		double li = (r1 - r0).Length();
		L += li;
	}
	if (L == 0.0) L = 1.0;
	double l = 0.0;
	m_curve.assign(m_path.size(), 0.0);
	for (int i = 1; i < m_path.size(); ++i)
	{
		vec3d r0 = m_path[i - 1];
		vec3d r1 = m_path[i];
		double li = (r1 - r0).Length();
		l += li;
		m_curve[i] = l / L;
	}

	return true;
}

vec2d GLCurveProbe::GetPointValue(int i)
{
	vec2d val(0, 0);

	Post::CGLModel* mdl = GetModel();
	if (mdl == nullptr) return val;
	FEPostModel& fem = *mdl->GetFSModel();
	if ((i < 0) || (i >= Points())) return val;

	int nstep = fem.CurrentTimeIndex();

	return GetPointValue(i, nstep);
}

vec2d GLCurveProbe::GetPointValue(int i, int nstep)
{
	vec2d val(0, 0);

	Post::CGLModel* mdl = GetModel();
	if (mdl == nullptr) return val;
	FEPostModel& fem = *mdl->GetFSModel();
	if ((i < 0) || (i >= Points())) return val;

	Post::CGLColorMap* cmap = mdl->GetColorMap();
	if (cmap == nullptr) return val;
	int nfield = cmap->GetEvalField();
	if (nfield == 0) return val;

	vec3f r0 = to_vec3f(m_path[i]);
	Post::NODEDATA data;
	fem.EvaluateNode(r0, nstep, nfield, data);
	val.x() = m_curve[i];
	val.y() = data.m_val;
	return val;
}
