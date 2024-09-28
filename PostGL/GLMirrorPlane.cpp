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
#include <PostLib/GLModel.h>
#include <FSCore/ClassDescriptor.h>
using namespace Post;

REGISTER_CLASS(CGLMirrorPlane, CLASS_PLOT, "mirror", 0);

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

int CGLMirrorPlane::m_render_id = -1;

void CGLMirrorPlane::Render(CGLContext& rc)
{
	// need to make sure we are not calling this recursively
	if (m_recursive)
	{
		if ((m_render_id != -1) && (m_id >= m_render_id)) return;
	}
	else
	{
		if (m_render_id != -1) return;
	}

	// plane normal
	vec3f scl;
	switch (m_plane)
	{
	case 0: m_norm = vec3f(1.f, 0.f, 0.f); scl = vec3f(-1.f, 1.f, 1.f); break;
	case 1: m_norm = vec3f(0.f, 1.f, 0.f); scl = vec3f(1.f, -1.f, 1.f); break;
	case 2: m_norm = vec3f(0.f, 0.f, 1.f); scl = vec3f(1.f, 1.f, -1.f); break;
	}

	// render the flipped model
	CGLModel* m = GetModel();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(-m_offset*m_norm.x, -m_offset*m_norm.y, -m_offset*m_norm.z);
	glScalef(scl.x, scl.y, scl.z);

	int old_id = m_render_id;
	m_render_id = m_id;
	int frontFace;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);
	glFrontFace(frontFace == GL_CW ? GL_CCW : GL_CW);
	m->Render(rc);
	glFrontFace(frontFace == GL_CW ? GL_CW : GL_CCW);
	m_render_id = old_id;

	glPopMatrix();

	// render the plane
	if (m_showPlane) RenderPlane();
}

//-----------------------------------------------------------------------------
void CGLMirrorPlane::RenderPlane()
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

	glPushMatrix();

	glTranslatef(rc.x, rc.y, rc.z);
	glTranslatef(-0.5f*m_offset*m_norm.x, -0.5f*m_offset*m_norm.y, -0.5f*m_offset*m_norm.z);

	quatd q = quatd(vec3d(0, 0, 1), to_vec3d(m_norm));
	float w = q.GetAngle();
	if (w != 0)
	{
		vec3d v = q.GetVector();
		glRotated(w * 180 / PI, v.x, v.y, v.z);
	}

	float R = box.Radius();

	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	GLdouble r = fabs(m_norm.x);
	GLdouble g = fabs(m_norm.y);
	GLdouble b = fabs(m_norm.z);

	glColor4d(r, g, b, m_transparency);
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
	glDepthMask(true);

	glColor3ub(255, 255, 0);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINE_LOOP);
	{
		glVertex3f(-R, -R, 0);
		glVertex3f(R, -R, 0);
		glVertex3f(R, R, 0);
		glVertex3f(-R, R, 0);
	}
	glEnd();

	glPopMatrix();

	// restore attributes
	glPopAttrib();
}
