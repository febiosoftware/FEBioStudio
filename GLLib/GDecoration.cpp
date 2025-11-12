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
#include <FSCore/math3d.h>
#include "GLRenderEngine.h"
#include "glx.h"

void GPointDecoration::render(GLRenderEngine& re)
{
	vec3d p = to_vec3d(m_pos);
	if (m_renderAura)
	{
		float s0 = re.pointSize();

		re.setPointSize(s0 + 2);
		re.setColor(m_col2);
		re.renderPoint(p);
		re.setPointSize(s0);
	}

	re.setColor(m_col);
	re.renderPoint(p);
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

void GLineDecoration::render(GLRenderEngine& re)
{
	if (p1 && p2)
	{
		vec3d r1 = to_vec3d(p1->position());
		vec3d r2 = to_vec3d(p2->position());
		re.setColor(m_col);
		re.renderLine(r1, r2);
	}
}

void GTriangleDecoration::render(GLRenderEngine& re)
{
	vec3d r1 = to_vec3d(p1->position());
	vec3d r2 = to_vec3d(p2->position());
	vec3d r3 = to_vec3d(p3->position());

	re.setColor(m_col);
	re.setMaterial(GLMaterial::GLASS, m_col, GLMaterial::NONE, false);
	re.begin(GLRenderEngine::TRIANGLES);
	{
		re.vertex(r1);
		re.vertex(r2);
		re.vertex(r3);
	}
	re.end();

	re.setMaterial(GLMaterial::OVERLAY, m_col, GLMaterial::NONE, false);
	re.begin(GLRenderEngine::LINELOOP);
	{
		re.vertex(r1);
		re.vertex(r2);
		re.vertex(r3);
	}
	re.end();
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

void GArcDecoration::render(GLRenderEngine& re)
{
	if (m_divs == 0) return;

	quatd Q0(0.0, vec3d(0, 0, 1));
	quatd Q1(to_vec3d(m_e0), to_vec3d(m_e1));

	vec3d c(to_vec3d(m_c));
	vec3d p0 = c + to_vec3d(m_e0)*m_scale;

	re.setColor(m_col);
	re.begin(GLRenderEngine::LINES);
	for (int i = 0; i <= m_divs; ++i)
	{
		double t = i / (double)m_divs;
		quatd Q = quatd::lerp(Q0, Q1, t);

		vec3d rt = to_vec3d(m_e0);
		Q.RotateVector(rt);

		vec3d p1 = c + rt*m_scale;

		re.vertex(p0);
		re.vertex(p1);

		p0 = p1;
	}
	re.end();
}

GSphereDecoration::GSphereDecoration(const vec3f& a, double R)
{
	m_c = to_vec3d(a);
	m_R = R;
	m_col = GLColor(255, 255, 0, 64);
}

void GSphereDecoration::render(GLRenderEngine& re)
{
	re.setColor(m_col);
	re.translate(m_c);
	glx::drawSphere(re, m_R);
	re.translate(-m_c);
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

void GCompositeDecoration::render(GLRenderEngine& re)
{
	for (int i = 0; i < m_deco.size(); ++i) m_deco[i]->render(re);
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

void GPlaneCutDecoration::render(GLRenderEngine& re)
{
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
	vec3d n(m_a[0], m_a[1], m_a[2]);

	float ev[8];	// element nodal values
	vec3d ex[8];	// element nodal positions
	for (int k = 0; k < 8; ++k)
	{
		ex[k] = r[k];
		ev[k] = ex[k]*n - m_a[3];
	}

	double ref = 0.0;

	int ncase = 0;
	for (int k = 0; k < 8; ++k)
		if (ev[k] <= ref) ncase |= (1 << k);

	// Draw lines first
	box.Scale(0.9999);
	int* pf = LUT[ncase];
	re.setColor(m_col2);
	vec3d rc(0, 0, 0);
	double lc = 0;
	for (int l = 0; l < 5; l++)
	{
		if (*pf == -1) break;

		// calculate nodal positions
		vec3d r[3], vn[3];
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

			vec3d rk = (r[k] + r[k1])*0.5;
			if (box.IsInside(rk) == false)
			{
				vec3d ek = r[k1] - r[k];
				rc += rk*ek.Length(); lc += ek.Length();

				re.renderLine(r[k], r[k1]);
			}
		}

		pf += 3;
	}

	// next, draw faces
	re.setColor(m_col);
	pf = LUT[ncase];
	vec3d Nc;
	for (int l = 0; l < 5; l++)
	{
		if (*pf == -1) break;

		// calculate nodal positions
		vec3d r[3], vn[3];
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
		re.begin(GLRenderEngine::TRIANGLES);
		{
			re.normal(vn[0]); re.vertex(r[0]);
			re.normal(vn[1]); re.vertex(r[1]);
			re.normal(vn[2]); re.vertex(r[2]);
		}
		re.end();

		pf += 3;
	}

	// draw the normal
	re.setColor(m_col2);
	double R = 0.25*box.GetMaxExtent();
	double R2 = R * 0.15;
	if (lc > 0)
	{
		quatd q(vec3d(0, 0, 1), Nc);
		vec3d e1( R2*0.6, 0, -R2);
		vec3d e2(-R2*0.6, 0, -R2);
		q.RotateVector(e1);
		q.RotateVector(e2);

		rc /= (float)lc;
		vec3d r2 = rc + Nc * R;
		vec3d a1 = r2 + e1;
		vec3d a2 = r2 + e2;
		re.begin(GLRenderEngine::LINES);
		{
			re.vertex(rc); re.vertex(r2);
			re.vertex(r2); re.vertex(a1);
			re.vertex(r2); re.vertex(a2);
		}
		re.end();
	}
}
