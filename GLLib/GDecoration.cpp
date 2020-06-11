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

#include "GDecoration.h"
#ifdef WIN32
#include <Windows.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/GLU.h>
#endif
#ifdef LINUX
#include <GL/gl.h>
#include <GL/glu.h>
#endif

void GPointDecoration::render()
{
	glColor3ub(m_col.r, m_col.g, m_col.b);
	glBegin(GL_POINTS);
	{
		glVertex3f(pos.x, pos.y, pos.z);
	}
	glEnd();
}

GLineDecoration::GLineDecoration(const vec3f& a, const vec3f& b)
{
	m_del = true;
	p1 = new GPointDecoration(a);
	p2 = new GPointDecoration(b);
}

GLineDecoration::~GLineDecoration()
{
	if (m_del)
	{
		delete p1;
		delete p2;
	}
}

void GLineDecoration::render()
{
	if (p1 && p2)
	{
		vec3f& r1 = p1->position();
		vec3f& r2 = p2->position();
		glColor3ub(m_col.r, m_col.g, m_col.b);
		glBegin(GL_LINES);
		{
			glVertex3f(r1.x, r1.y, r1.z);
			glVertex3f(r2.x, r2.y, r2.z);
		}
		glEnd();
	}
}

void GTriangleDecoration::render()
{
	vec3f& r1 = p1->position();
	vec3f& r2 = p2->position();
	vec3f& r3 = p3->position();
	glColor4ub(m_col.r, m_col.g, m_col.b, m_col.a);
	glBegin(GL_TRIANGLES);
	glVertex3f(r1.x, r1.y, r1.z);
	glVertex3f(r2.x, r2.y, r2.z);
	glVertex3f(r3.x, r3.y, r3.z);
	glEnd();
}

GArcDecoration::GArcDecoration(const vec3f& c, const vec3f& p0, const vec3f& p1, int ndivs, double scale)
{
	m_c = c;
	m_e0 = p0 - c; double l0 = m_e0.Length(); m_e0.Normalize();
	m_e1 = p1 - c; double l1 = m_e1.Length(); m_e1.Normalize();

	double lmin = (l0 <= l1 ? l0 : l1);
	lmin *= scale;

	m_scale = lmin;

	if (lmin <= 0.0)
	{
		m_divs = 0;
	}
	else m_divs = ndivs;
}

void GArcDecoration::render()
{
	if (m_divs == 0) return;

	quatd Q0(0.0, vec3d(0, 0, 1));
	quatd Q1(m_e0, m_e1);

	vec3d c(m_c);
	vec3d p0 = c + m_e0*m_scale;

	glColor3ub(m_col.r, m_col.g, m_col.b);
	glBegin(GL_LINES);
	for (int i = 0; i <= m_divs; ++i)
	{
		double t = i / (double)m_divs;
		quatd Q = quatd::lerp(Q0, Q1, t);

		vec3d rt = m_e0;
		Q.RotateVector(rt);

		vec3d p1 = c + rt*m_scale;

		glVertex3d(p0.x, p0.y, p0.z);
		glVertex3d(p1.x, p1.y, p1.z);

		p0 = p1;
	}
	glEnd();
}

GSphereDecoration::GSphereDecoration(const vec3f& a, double R)
{
	m_c = a;
	m_R = R;
	m_col = GLColor(255, 255, 0, 64);
}

void GSphereDecoration::render()
{
	GLUquadric* glq = gluNewQuadric();
	glColor4ub(m_col.r, m_col.g, m_col.b, m_col.a);
	glTranslatef(m_c.x, m_c.y, m_c.z);
	gluSphere(glq, m_R, 64, 32);
	glTranslatef(-m_c.x, -m_c.y, -m_c.z);
	gluDeleteQuadric(glq);
}

GCompositeDecoration::GCompositeDecoration()
{

}

GCompositeDecoration::~GCompositeDecoration()
{
	for (int i = 0; i < m_deco.size(); ++i) delete m_deco[i];
}

void GCompositeDecoration::AddDecoration(GDecoration* deco)
{
	m_deco.push_back(deco);
}

void GCompositeDecoration::render()
{
	for (int i = 0; i < m_deco.size(); ++i) m_deco[i]->render();
}
