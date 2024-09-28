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
#include "GLRuler.h"
#include <PostLib/GLModel.h>
#include <MeshLib/MeshTools.h>
#include <PostLib/constants.h>
#include <MeshLib/FENodeNodeList.h>
#include <GLLib/glx.h>
#include <FSCore/ClassDescriptor.h>
#include <sstream>
using namespace Post;

REGISTER_CLASS(GLRuler, CLASS_PLOT, "ruler", 0);

GLRuler::GLRuler()
{
	SetTypeString("ruler");

	static int n = 1;
	char sz[256] = { 0 };
	sprintf(sz, "Ruler%d", n++);
	SetName(sz);

	m_node[0] = -1;
	m_node[1] = -1;
	m_size = 0.05;
	m_col = GLColor::White();
	m_bfollow = true;

	m_lastTime = -1.0;
	m_lastDt = 0.0;
	m_R = 0.0;
	m_rt[0] = vec3d(0,0,0);
	m_rt[1] = vec3d(0, 0, 0);

	AddIntParam(m_node[0], "node0", "Node 1");
	AddIntParam(m_node[1], "node1", "Node 2");
	AddDoubleParam(m_size, "size", "Size scale factor");
	AddColorParam(m_col, "color", "Color");

	// set the render-order to 1, so this gets drawn after the model is drawn
	SetRenderOrder(1);
}

bool GLRuler::UpdateData(bool bsave)
{
	if (bsave)
	{
		int n0 = m_node[0];
		int n1 = m_node[1];

		m_node[0] = GetIntValue(NODE0);
		m_node[1] = GetIntValue(NODE1);
		m_size = GetFloatValue(SIZE);
		m_col = GetColorValue(COLOR);

		if (!(n0 == m_node[0]) || !(n1 == m_node[1])) Update();
	}
	else
	{
		SetIntValue(NODE0, m_node[0]);
		SetIntValue(NODE1, m_node[1]);
		SetFloatValue(SIZE, m_size);
		SetColorValue(COLOR, m_col);
	}

	return false;
}

void GLRuler::Render(CGLContext& rc)
{
	if ((m_node[0] <= 0) || (m_node[1] <= 0)) return;

	vec3d ra = m_rt[0];
	vec3d rb = m_rt[1];

	vec3d t = rb - ra;
	double H = t.Length(); t.Normalize();
	double R = m_R * m_size;

	glColor3ub(m_col.r, m_col.g, m_col.b);
	GLUquadricObj* pobj = gluNewQuadric();
	glPushMatrix();
	{
		glTranslated(ra.x, ra.y, ra.z);
		quatd q(vec3d(0, 0, 1), t);
		if (q.GetAngle() != 0)
		{
			double w = q.GetAngle() * RAD2DEG;
			vec3d v = q.GetVector();
			glRotated(w, v.x, v.y, v.z);
		}
		gluCylinder(pobj, R, R, H, 12, 1);
	}
	glPopMatrix();

	glPushMatrix();
	{
		glTranslated(ra.x, ra.y, ra.z);
		gluSphere(pobj, R, 12, 12);
		glTranslated(-ra.x, -ra.y, -ra.z);
		glTranslated(rb.x, rb.y, rb.z);
		gluSphere(pobj, R, 12, 12);
	}
	glPopMatrix();
	gluDeleteQuadric(pobj);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	{
		glVertex3d(ra.x, ra.y, ra.z);
		glVertex3d(rb.x, rb.y, rb.z);
	}
	glEnd();
	glPopAttrib();

}

void GLRuler::Update()
{
	Update(m_lastTime, m_lastDt, true);
}

void GLRuler::Update(int ntime, float dt, bool breset)
{
	if ((breset == false) && (ntime == m_lastTime) && (dt == m_lastDt)) return;

	m_lastTime = ntime;
	m_lastDt = dt;

	if ((m_node[0] <= 0) || (m_node[1] <= 0)) return;

	CGLModel* mdl = GetModel();
	if (mdl == nullptr) return;

	Post::FEPostModel& fem = *mdl->GetFSModel();

	// update the size of the probe
	BOX box = fem.GetBoundingBox();
	m_R = 0.05 * box.GetMaxExtent();

	m_rt[0] = to_vec3d(fem.NodePosition(m_node[0]-1, ntime));
	m_rt[1] = to_vec3d(fem.NodePosition(m_node[1]-1, ntime));
}

GLColor GLRuler::GetColor() const
{
	return m_col;
}

void GLRuler::SetColor(const GLColor& c)
{
	m_col = c;
}

double GLRuler::DataValue(int nfield, int nstep)
{
	if ((m_node[0] <= 0) || (m_node[1] <= 0)) return 0.0;

	CGLModel* mdl = GetModel();
	if (mdl == nullptr) return 0.0;

	Post::FEPostModel& fem = *mdl->GetFSModel();
	vec3d ra = to_vec3d(fem.NodePosition(m_node[0] - 1, nstep));
	vec3d rb = to_vec3d(fem.NodePosition(m_node[1] - 1, nstep));

	double val = 0.0;
	vec3d t = rb - ra;
	switch (nfield)
	{
	case 1: val = t.x; break;
	case 2: val = t.y; break;
	case 3: val = t.z; break;
	case 4: val = t.Length(); break;
	}
	return val;
}
