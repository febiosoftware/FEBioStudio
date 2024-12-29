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
#include "GLMirrorPlane.h"
#include "GLModel.h"
#include <FSCore/ClassDescriptor.h>
#include "../FEBioStudio/GLRenderEngine.h"
using namespace Post;

REGISTER_CLASS(CGLMirrorPlane, CLASS_PLOT, "mirror", 0);

CGLMirrorPlane* CGLMirrorPlane::m_mirrors[MAX_MIRROR_PLANES] = { nullptr };

CGLMirrorPlane* CGLMirrorPlane::GetMirrorPlane(int n)
{
	assert(n < MAX_MIRROR_PLANES);
	return m_mirrors[n];
}

CGLMirrorPlane::CGLMirrorPlane()
{
	SetTypeString("mirror");

	static int n = 1;
	m_id = n;
	char szname[128] = { 0 };
	sprintf(szname, "MirrorPlane.%02d", n++);
	SetName(szname);

	AddIntParam(0, "mirror_plane")->SetEnumNames("x\0y\0z\0");
	AddBoolParam(true, "show_plane");
	AddDoubleParam(0.25, "transparency")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.f, "offset");
	AddBoolParam(true, "recursion");

	m_plane = 0;
	m_showPlane = true;
	m_transparency = 0.25f;
	m_offset = 0.f;
	m_recursive = true;

	UpdateData(false);

	m_render_id = -1;
	AllocRenderID();
}

CGLMirrorPlane::~CGLMirrorPlane()
{
	DeallocRenderID();
}

void CGLMirrorPlane::DeallocRenderID()
{
	assert(m_render_id >= 0);
	assert(m_mirrors[m_render_id] == this);
	m_mirrors[m_render_id] = nullptr;
	m_render_id = -1;
}

void CGLMirrorPlane::AllocRenderID()
{
	m_render_id = -1;
	for (int i = 0; i < MAX_MIRROR_PLANES; ++i)
	{
		if (m_mirrors[i] == nullptr)
		{
			m_mirrors[i] = this;
			m_render_id = i;
			break;
		}
	}
	assert(m_render_id != -1);
}

bool CGLMirrorPlane::UpdateData(bool bsave)
{
	if (bsave)
	{
		m_plane = GetIntValue(PLANE);
		m_showPlane = GetBoolValue(SHOW_PLANE);
		m_transparency = GetFloatValue(TRANSPARENCY);
		m_offset = GetFloatValue(OFFSET);
		m_recursive = GetBoolValue(RECURSION);
	}
	else
	{ 
		SetIntValue(PLANE, m_plane);
		SetBoolValue(SHOW_PLANE, m_showPlane);
		SetFloatValue(TRANSPARENCY, m_transparency);
		SetFloatValue(OFFSET, m_offset);
		SetBoolValue(RECURSION, m_recursive);
	}

	return false;
}

void CGLMirrorPlane::Render(GLRenderEngine& re, CGLContext& rc)
{
	// render the plane
	if (m_showPlane) RenderPlane(re);
}

//-----------------------------------------------------------------------------
void CGLMirrorPlane::RenderPlane(GLRenderEngine& re)
{
	CGLModel* mdl = GetModel();

	BOX box = mdl->GetFSModel()->GetBoundingBox();

	// plane center
	vec3d rc = box.Center();
	switch (m_plane)
	{
	case 0: rc.x = 0.f; break;
	case 1: rc.y = 0.f; break;
	case 2: rc.z = 0.f; break;
	}

	vec3f norm;
	switch (m_plane)
	{
	case 0: norm = vec3f(1.f, 0.f, 0.f); break;
	case 1: norm = vec3f(0.f, 1.f, 0.f); break;
	case 2: norm = vec3f(0.f, 0.f, 1.f); break;
	}

	re.pushTransform();
	re.translate(rc);

	vec3d offset(-0.5f*m_offset*norm.x, -0.5f*m_offset*norm.y, -0.5f*m_offset*norm.z);
	re.translate(offset);

	quatd q = quatd(vec3d(0, 0, 1), to_vec3d(norm));
	re.rotate(q);

	float R = 2*box.Radius();

	// store attributes
	re.pushState();

	GLdouble r = fabs(norm.x);
	GLdouble g = fabs(norm.y);
	GLdouble b = fabs(norm.z);
	re.setMaterial(GLMaterial::CONSTANT, GLColor::FromRGBf(r, g, b, m_transparency), GLMaterial::NONE, false);

	glDepthMask(false);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f(R, -R, 0);
		glVertex3f(R, R, 0);
		glVertex3f(-R, R, 0);
	}
	glEnd();

	re.setColor(GLColor(255, 255, 0));
	glDepthMask(true);
	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f(R, -R, 0);
		glVertex3f(R, R, 0);
		glVertex3f(-R, R, 0);
	}
	glEnd();

	re.popState();
	re.popTransform();
}
