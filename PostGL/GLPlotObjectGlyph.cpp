/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include "GLPlotObjectGlyph.h"
#include <PostGL/GLModel.h>
#include <GLLib/glx.h>

Post::GLPlotObjectVector::GLPlotObjectVector(Post::FEPostModel::PointObject* po) : GLPlotObjectGlyph(po) 
{
	SetTypeString("vector-glyph");

	static int n = 1;
	char szname[128] = { 0 };
	sprintf(szname, "VectorGlyph.%02d", n++);
	SetName(szname);

	std::vector<std::string> nameList;
	size_t l = 0;
	for (int i = 0; i < po->DataCount(); ++i)
	{
		Post::PlotObjectData* pd = po->GetData(i);
		if (pd->Type() == DATA_VEC3)
		{
			std::string name = pd->GetName();
			nameList.push_back(name);
			l += name.size() + 1;
		}
	}
	char* sztmp = new char[l + 1];
	char* c = sztmp;
	for (int i = 0; i < nameList.size(); ++i)
	{
		string& name = nameList[i];
		strcpy(c, name.c_str());
		c += name.size();
		*c++ = 0;
	}
	*c++ = 0;

	AddIntParam(0, "data_field")->CopyEnumNames(sztmp);
	AddBoolParam(true, "allow_clipping");
	AddBoolParam(true, "show_hidden");
	AddIntParam(0, "glyph")->SetEnumNames("arrow\0cone\0cylinder\0sphere\0box\0line\0");
	AddIntParam(0, "glyph_color")->SetEnumNames("solid\0orientation\0");
	AddColorParam(GLColor(200, 200, 200), "solid_color");
	AddBoolParam(true, "auto-scale");
	AddDoubleParam(0., "scale");
	AddDoubleParam(1., "glyph_aspect_ratio");

	delete[] sztmp;
}

void Post::GLPlotObjectVector::Update(int ntime, float dt, bool breset)
{
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();

	Post::FEState* state = ps->GetState(ntime);
	Post::OBJ_POINT_DATA& data = state->m_objPt[m_po->m_id];

	int nvec = GetIntValue(DATA_FIELD);
	for (int i = 0, n = 0; i < m_po->DataCount(); ++i)
	{
		Post::PlotObjectData* pd = m_po->GetData(i);
		if (pd->Type() == DATA_VEC3)
		{
			if (nvec == n)
			{
				nvec = i;
				break;
			}
			n++;
		}
	}

	m_val = data.data->get<vec3f>(nvec);
}

void Post::GLPlotObjectVector::Render(GLRenderEngine& re, GLContext& rc)
{
	if (m_po == nullptr) return;

	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();

	bool autoScale = GetBoolValue(AUTO_SCALE);
	double scale = GetFloatValue(SCALE);
	int glyph = GetIntValue(GLYPH);
	GLColor solidColor = GetColorValue(SOLID_COLOR);

	// calculate scale factor for rendering
	double fscale = (autoScale ? 0.2f * scale * ps->GetBoundingBox().Radius() : scale);

	if (glyph == GLYPH_LINE)
	{
		re.setMaterial(GLMaterial::CONSTANT, solidColor);
	}
	else
	{
		re.setMaterial(GLMaterial::PLASTIC, solidColor);
	}

	vec3d r = m_po->m_pos;
	RenderVector(re, r, m_val);
}

void Post::GLPlotObjectVector::RenderVector(GLRenderEngine& re, const vec3d& r, vec3f v)
{
	CGLModel* mdl = GetModel();
	FEPostModel* ps = mdl->GetFSModel();

	float L = v.Length();
	if (L == 0.f) return;

	GLColor solidCol = GetColorValue(SOLID_COLOR);
	int col = GetIntValue(GLYPH_COLOR);
	switch (col)
	{
	case GLYPH_COL_ORIENT:
	{
		vec3f vn = v;
		vn.Normalize();
		double r = fabs(vn.x);
		double g = fabs(vn.y);
		double b = fabs(vn.z);
		re.setColor(GLColor::FromRGBf(r, g, b));
	}
	break;
	case GLYPH_COL_SOLID:
	default:
		re.setColor(solidCol);
	}

	bool autoScale = GetBoolValue(AUTO_SCALE);
	double scale = GetFloatValue(SCALE);
	double ar = GetFloatValue(ASPECT_RATIO);

	// calculate scale factor for rendering
	double fscale = (autoScale ? 0.2f * scale * ps->GetBoundingBox().Radius() : scale);

	L *= fscale;
	float l0 = L * .9;
	float l1 = L * .2;
	float r0 = L * 0.05 * ar;
	float r1 = L * 0.15 * ar;

	re.pushTransform();

	re.translate(r);
	quatd q;
	vec3d V = to_vec3d(v);
	if (V * vec3d(0, 0, 1) == -1.0) q = quatd(PI, vec3d(1, 0, 0));
	else q = quatd(vec3d(0, 0, 1), to_vec3d(v));
	re.rotate(q);

	int glyph = GetIntValue(GLYPH);
	switch (glyph)
	{
	case GLYPH_ARROW:
		glx::drawCylinder(re, r0, l0, 5);
		re.translate(vec3d(0, 0, l0 * 0.9));
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
