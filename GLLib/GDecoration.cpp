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
#include <FSCore/math3d.h>

void GPointDecoration::render()
{
	if (m_renderAura)
	{
		GLfloat s0;
		glGetFloatv(GL_POINT_SIZE, &s0);

		glPointSize(s0 + 2);
		glColor3ub(m_col2.r, m_col2.g, m_col2.b);
		glBegin(GL_POINTS);
		{
			glVertex3f(m_pos.x, m_pos.y, m_pos.z);
		}
		glEnd();
		glPointSize(s0);
	}

	glColor3ub(m_col.r, m_col.g, m_col.b);
	glBegin(GL_POINTS);
	{
		glVertex3f(m_pos.x, m_pos.y, m_pos.z);
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
	glEnable(GL_POLYGON_STIPPLE);
	glBegin(GL_TRIANGLES);
	glVertex3f(r1.x, r1.y, r1.z);
	glVertex3f(r2.x, r2.y, r2.z);
	glVertex3f(r3.x, r3.y, r3.z);
	glEnd();
	glDisable(GL_POLYGON_STIPPLE);
	glBegin(GL_LINE_LOOP);
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
	quatd Q1(to_vec3d(m_e0), to_vec3d(m_e1));

	vec3d c(to_vec3d(m_c));
	vec3d p0 = c + to_vec3d(m_e0)*m_scale;

	glColor3ub(m_col.r, m_col.g, m_col.b);
	glBegin(GL_LINES);
	for (int i = 0; i <= m_divs; ++i)
	{
		double t = i / (double)m_divs;
		quatd Q = quatd::lerp(Q0, Q1, t);

		vec3d rt = to_vec3d(m_e0);
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
	m_c = to_vec3d(a);
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

//=================================================================================================

// in MeshTools\lut.cpp
extern int LUT[256][15];
extern int ET_HEX[12][2];

GPlaneCutDecoration::GPlaneCutDecoration()
{
	m_box = BOX(vec3d(0, 0, 0), vec3d(1, 1, 1));
	m_a[0] = 0;
	m_a[1] = 0;
	m_a[2] = 0;
	m_a[3] = 0;
}

GPlaneCutDecoration::~GPlaneCutDecoration()
{

}

void GPlaneCutDecoration::setBoundingBox(BOX box)
{
	m_box = box;
}

void GPlaneCutDecoration::setPlane(double n0, double n1, double n2, double d)
{
	m_a[0] = n0;
	m_a[1] = n1;
	m_a[2] = n2;
	m_a[3] = d;
}

void GPlaneCutDecoration::render()
{
	glEnable(GL_DEPTH_TEST);

	// get the nodal values
	BOX box = m_box;
	box.Scale(1.05);
	vec3d a = box.r0();
	vec3d b = box.r1();
	vec3d r[8] = {
		vec3d(a.x, a.y, a.z),
		vec3d(b.x, a.y, a.z),
		vec3d(b.x, b.y, a.z),
		vec3d(a.x, b.y, a.z),
		vec3d(a.x, a.y, b.z),
		vec3d(b.x, a.y, b.z),
		vec3d(b.x, b.y, b.z),
		vec3d(a.x, b.y, b.z)
	};
	vec3f n(m_a[0], m_a[1], m_a[2]);

	float ev[8];	// element nodal values
	vec3f ex[8];	// element nodal positions
	for (int k = 0; k < 8; ++k)
	{
		ex[k] = to_vec3f(r[k]);
		ev[k] = ex[k]*n - m_a[3];
	}

	float ref = 0.0f;

	int ncase = 0;
	for (int k = 0; k < 8; ++k)
		if (ev[k] <= ref) ncase |= (1 << k);

	// Draw lines first
	box.Scale(0.9999f);
	int* pf = LUT[ncase];
	glColor4ub(m_col2.r, m_col2.g, m_col2.b, m_col2.a);
	vec3f rc(0.f, 0.f, 0.f);
	double lc = 0;
	for (int l = 0; l < 5; l++)
	{
		if (*pf == -1) break;

		// calculate nodal positions
		vec3f r[3], vn[3];
		for (int k = 0; k < 3; k++)
		{
			int n1 = ET_HEX[pf[k]][0];
			int n2 = ET_HEX[pf[k]][1];
			double w = (ref - ev[n1]) / (ev[n2] - ev[n1]);
			r[k] = ex[n1] * (1 - w) + ex[n2] * w;
		}

		for (int k = 0; k < 3; k++)
		{
			int k1 = (k + 1) % 3;

			vec3f rk = (r[k] + r[k1])*0.5f;
			if (box.IsInside(to_vec3d(rk)) == false)
			{
				vec3f ek = r[k1] - r[k];
				rc += rk*ek.Length(); lc += ek.Length();
				glBegin(GL_LINES);
				{
					glVertex3f(r[k].x, r[k].y, r[k].z);
					glVertex3f(r[k1].x, r[k1].y, r[k1].z);
				}
				glEnd();
			}
		}

		pf += 3;
	}

	// next, draw faces
	glColor4ub(m_col.r, m_col.g, m_col.b, m_col.a);
	pf = LUT[ncase];
	vec3f Nc;
	for (int l = 0; l < 5; l++)
	{
		if (*pf == -1) break;

		// calculate nodal positions
		vec3f r[3], vn[3];
		for (int k = 0; k < 3; k++)
		{
			int n1 = ET_HEX[pf[k]][0];
			int n2 = ET_HEX[pf[k]][1];

			double w = (ref - ev[n1]) / (ev[n2] - ev[n1]);

			r[k] = ex[n1] * (1 - w) + ex[n2] * w;
		}

		for (int k = 0; k < 3; k++)
		{
			int kp1 = (k + 1) % 3;
			int km1 = (k + 2) % 3;
			vn[k] = (r[kp1] - r[k]) ^ (r[km1] - r[k]);
			vn[k].Normalize();

			Nc = -vn[k];
		}

		// render the face
		glBegin(GL_TRIANGLES);
		{
			glNormal3f(vn[0].x, vn[0].y, vn[0].z); glVertex3f(r[0].x, r[0].y, r[0].z);
			glNormal3f(vn[1].x, vn[1].y, vn[1].z); glVertex3f(r[1].x, r[1].y, r[1].z);
			glNormal3f(vn[2].x, vn[2].y, vn[2].z); glVertex3f(r[2].x, r[2].y, r[2].z);
		}
		glEnd();

		pf += 3;
	}

	glDisable(GL_DEPTH_TEST);

	// draw the normal
	glColor4ub(m_col2.r, m_col2.g, m_col2.b, m_col2.a);
	double R = 0.25*box.GetMaxExtent();
	double R2 = R * 0.15;
	if (lc > 0)
	{
		quatd q(vec3d(0, 0, 1), to_vec3d(Nc));
		vec3d e1( R2*0.6, 0, -R2);
		vec3d e2(-R2*0.6, 0, -R2);
		q.RotateVector(e1);
		q.RotateVector(e2);

		rc /= (float)lc;
		vec3f r2 = rc + Nc * R;
		vec3f a1 = r2 + to_vec3f(e1);
		vec3f a2 = r2 + to_vec3f(e2);
		glBegin(GL_LINES);
		{
			glVertex3f(rc.x, rc.y, rc.z); glVertex3f(r2.x, r2.y, r2.z);
			glVertex3f(r2.x, r2.y, r2.z); glVertex3f(a1.x, a1.y, a1.z);
			glVertex3f(r2.x, r2.y, r2.z); glVertex3f(a2.x, a2.y, a2.z);
		}
		glEnd();
	}
}
