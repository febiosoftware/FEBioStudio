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
#include <PostGL/GLModel.h>
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshLib/FENodeNodeList.h>
#include <GLLib/glx.h>
#include <FSCore/ClassDescriptor.h>
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
	m_scale = 1.0;
	AddColorParam(m_col, "color");
	AddDoubleParam(m_scale, "scale factor");
}

void GLCurveProbe::Render(GLRenderEngine& re, GLContext& rc)
{
	if (m_path.size())
	{
		re.setMaterial(GLMaterial::OVERLAY, m_col);
		
		re.begin(GLRenderEngine::LINESTRIP);
		{
			for (int i = 0; i < m_path.size(); ++i)
			{
				vec3d r = m_path[i] * m_scale;
				re.vertex(r);
			}
		}
		re.end();

		re.begin(GLRenderEngine::POINTS);
		{
			re.vertex(m_path[0] * m_scale);
		}
		re.end();
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
	if (bsave)
	{
		m_col = GetColorValue(0);
		m_scale = GetFloatValue(1);
	}
	else
	{
		SetColorValue(0, m_col);
		SetFloatValue(1, m_scale);
	}

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

bool GLCurveProbe::SetPoints(const std::vector<vec3d>& points)
{
	m_path = points;
	return (m_path.size() > 1);
}

vector<double> GLCurveProbe::SectionLenghts(bool normalized)
{
	vector<double> curve(m_path.size(), 0.0);

	// calculate normalize factor
	double L = 0.0;
	if (normalized)
	{
		for (int i = 1; i < m_path.size(); ++i)
		{
			vec3d r0 = m_path[i - 1] * m_scale;
			vec3d r1 = m_path[i] * m_scale;
			double li = (r1 - r0).Length();
			L += li;
		}
		if (L == 0.0) L = 1.0;
	}
	else L = 1.0;

	// calculate section lenghts
	double l = 0.0;
	for (int i = 1; i < m_path.size(); ++i)
	{
		vec3d r0 = m_path[i - 1] * m_scale;
		vec3d r1 = m_path[i    ] * m_scale;
		double li = (r1 - r0).Length();
		l += li;
		curve[i] = l / L;
	}

	return curve;
}

double GLCurveProbe::GetPointValue(int i)
{
	double val(0);

	Post::CGLModel* mdl = GetModel();
	if (mdl == nullptr) return val;
	FEPostModel& fem = *mdl->GetFSModel();
	if ((i < 0) || (i >= Points())) return val;

	int nstep = fem.CurrentTimeIndex();

	return GetPointValue(i, nstep);
}

double GLCurveProbe::GetPointValue(int i, int nstep)
{
	double val(0);

	Post::CGLModel* mdl = GetModel();
	if (mdl == nullptr) return val;
	FEPostModel& fem = *mdl->GetFSModel();
	if ((i < 0) || (i >= Points())) return val;

	Post::CGLColorMap* cmap = mdl->GetColorMap();
	if (cmap == nullptr) return val;
	int nfield = cmap->GetEvalField();
	if (nfield == 0) return val;

	vec3f r0 = to_vec3f(m_path[i])* m_scale;
	Post::NODEDATA data;
	// TODO: This will evaluate the point at the current time step coordinates!
	//       Not at the mesh coordinates of step "nstep"! If the mesh deforms,
	//       this will not give the correct answer!! 
	fem.EvaluateNode(r0, nstep, nfield, data);
	val = data.m_val;
	return val;
}
