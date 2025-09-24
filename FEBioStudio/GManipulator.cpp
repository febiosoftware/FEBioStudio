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
#include "GManipulator.h"
#include <GLLib/glx.h>
#include "GLView.h"
#include "GLViewTransform.h"

GManipulator::GManipulator(CGLView* view) : m_view(view)
{
	m_scale = 1.0;
}

GManipulator::~GManipulator(void)
{
}

void GTranslator::Render(GLRenderEngine& re, int npivot, bool bactive)
{
	double d = m_scale;
	double l = 0.1*d;
	double r = 0.3*d;

	re.pushState();
	re.setMaterial(GLMaterial::OVERLAY, GLColor::White());

	GLColor g(128, 128, 128);

	// X-axis
	re.pushTransform();
	{
		re.rotate(90, 0, 1, 0);
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_X) re.setColor(GLColor(255,255,0));
			else re.setColor(GLColor(255, 0, 0));
		}
		else re.setColor(g);
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d));

		re.translate(vec3d(0, 0, d));
		glx::drawCone(re, 0.5 * l, l, 12);
	}
	re.popTransform();

	// Y-axis
	re.pushTransform();
	{
		re.rotate(90, -1, 0, 0);
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_Y) re.setColor(GLColor(255,255,0));
			else re.setColor(GLColor(0,255,0));
		}
		else re.setColor(g);
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d));

		re.translate(vec3d(0, 0, d));
		glx::drawCone(re, 0.5 * l, l, 12);
	}
	re.popTransform();

	// Z-axis
	re.pushTransform();
	{
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_Z) re.setColor(GLColor(255,255,0));
			else re.setColor(GLColor(0,0,255));
		}
		else re.setColor(g);
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d));

		re.translate(vec3d(0, 0, d));
		glx::drawCone(re, 0.5 * l, l, 12);
	}
	re.popTransform();

	if (bactive)
	{
		// XY-plane
		if (npivot == PIVOT_SELECTION_MODE::SELECT_XY) re.setColor(GLColor(255, 255, 0, 128));
		else re.setColor(GLColor(164, 164, 0, 90));
		re.renderRect(0,0,r,r);
		re.setColor(GLColor(255,255,0, 128));
		re.renderLine(vec3d(r, 0, 0), vec3d(r, r, 0), vec3d(0, r, 0));

		// YZ-plane
		if (npivot == PIVOT_SELECTION_MODE::SELECT_YZ) re.setColor(GLColor(255, 255, 0, 128));
		else re.setColor(GLColor(164, 164, 0, 90));
		re.pushTransform();
		{
			re.rotate(-90, 0, 1, 0);
			re.renderRect(0,0,r,r);

			re.setColor(GLColor(255,255,0,128));
			re.renderLine(vec3d(r, 0, 0), vec3d(r, r, 0), vec3d(0, r, 0));
		}
		re.popTransform();

		// XZ-plane
		if (npivot == PIVOT_SELECTION_MODE::SELECT_XZ) re.setColor(GLColor(255, 255, 0, 128));
		else re.setColor(GLColor(164, 164, 0, 90));
		re.pushTransform();
		{
			re.rotate(90, 1, 0, 0);
			re.renderRect(0,0,r,r);

			re.setColor(GLColor(255,255,0,128));
			re.renderLine(vec3d(r, 0, 0), vec3d(r, r, 0), vec3d(0, r, 0));
		}
		re.popTransform();
	}

	re.popState();
}

int GTranslator::Pick(int x, int  y)
{
	double r = 0.3*m_scale;
	vec3d o = m_view->GetPivotPosition();
	quatd Q = m_view->GetPivotRotation();
	vec3d ax(r, 0, 0); Q.RotateVector(ax);
	vec3d ay(0, r, 0); Q.RotateVector(ay);
	vec3d az(0, 0, r); Q.RotateVector(az);

	// convert the point to a ray
	GLViewTransform transform(m_view);
	Ray ray = transform.PointToRay(x, y);

	// check the combo-transforms first
	// In this case we need to find the closes point

	Intersection intersect;
	int pivot = PIVOT_SELECTION_MODE::SELECT_NONE;
	double dmin = 1e99;
	Quad qxy = {o, o+ax, o+ax+ay, o+ay};
	if (FastIntersectQuad(ray, qxy, intersect))
	{
		pivot = PIVOT_SELECTION_MODE::SELECT_XY;
		dmin = ray.direction*(intersect.point - ray.origin);
	}

	Quad qyz = { o, o + ay, o + ay + az, o + az };
	if (FastIntersectQuad(ray, qyz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_SELECTION_MODE::SELECT_YZ;
			dmin = D;
		}
	}

	Quad qxz = { o, o + ax, o + ax + az, o + az };
	if (FastIntersectQuad(ray, qxz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_SELECTION_MODE::SELECT_XZ;
			dmin = D;
		}
	}

	if (pivot != PIVOT_SELECTION_MODE::SELECT_NONE) return pivot;

	// now we'll check the individual axes
	int S = 4;
	QRect rt(x - S, y - S, 2 * S, 2 * S);

	vec3d ex(m_scale, 0, 0); Q.RotateVector(ex);
	vec3d ey(0, m_scale, 0); Q.RotateVector(ey);
	vec3d ez(0, 0, m_scale); Q.RotateVector(ez);

	vec3d p0 = transform.WorldToScreen(o);
	vec3d p1 = transform.WorldToScreen(o + ex);
	vec3d p2 = transform.WorldToScreen(o + ey);
	vec3d p3 = transform.WorldToScreen(o + ez);

	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_SELECTION_MODE::SELECT_X;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p2.x, (int)p2.y), rt)) return PIVOT_SELECTION_MODE::SELECT_Y;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p3.x, (int)p3.y), rt)) return PIVOT_SELECTION_MODE::SELECT_Z;

	return PIVOT_SELECTION_MODE::SELECT_NONE;
}

void GRotator::Render(GLRenderEngine& re, int npivot, bool bactive)
{
	double d = m_scale;
	double l = 0.1*d;
	const int N = 50;

	re.pushState();
	re.setMaterial(GLMaterial::OVERLAY, GLColor::White());

	// X-axis
	re.pushTransform();
	{
		re.rotate(90, 0, 1, 0);
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_X)
			{
				re.setColor(GLColor(200,0,0,64));
				glx::drawDisk(re, d, N);

				re.setColor(GLColor(255,255,0));
				re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d + l));
				glx::drawCircle(re, d, N);

				re.setColor(GLColor(255,255,0));
				re.translate(vec3d(0, 0, d+l));
				glx::drawCone(re, 0.5 * l, l);
			}
			else
			{
				re.setColor(GLColor(255,0,0));
				glx::drawCircle(re, d, N);
			}
		}
		else
		{
			re.setColor(GLColor(128,128,128));
			glx::drawCircle(re, d, N);
		}
	}
	re.popTransform();

	// Y-axis
	re.pushTransform();
	{
		re.rotate(90.f, -1.f, 0.f, 0.f);
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_Y)
			{
				re.setColor(GLColor(0,200,0,64));
				glx::drawDisk(re, d, N);

				re.setColor(GLColor(255,255,0));
				re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d + l));
				glx::drawCircle(re, d, N);

				re.setColor(GLColor(255,255,0));
				re.translate(vec3d(0, 0, d+l));
				glx::drawCone(re, 0.5 * l, l);
			}
			else
			{
				re.setColor(GLColor(0,255,0));
				glx::drawCircle(re, d, N);
			}
		}
		else
		{
			re.setColor(GLColor(128,128,128));
			glx::drawCircle(re, d, N);
		}
	}
	re.popTransform();

	// Z-axis
	re.pushTransform();
	{
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_Z)
			{
				re.setColor(GLColor(0,0,255,64));
				glx::drawDisk(re, d, N);

				re.setColor(GLColor(255,255,0));
				re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d + l));
				glx::drawCircle(re, d, N);

				re.setColor(GLColor(255,255,0));
				re.translate(vec3d(0, 0, d+l));
				glx::drawCone(re, 0.5 * l, l);
			}
			else
			{
				re.setColor(GLColor(0,0,255));
				glx::drawCircle(re, d, N);
			}
		}
		else
		{
			re.setColor(GLColor(128,128,128));
			glx::drawCircle(re, d, N);
		}
	}
	re.popTransform();

	re.popState();
}

int GRotator::Pick(int x, int y)
{
	GLViewTransform transform(m_view);

	int S = 4;
	QRect rt(x - S, y - S, 2 * S, 2 * S);
	vec3d o = m_view->GetPivotPosition();
	quatd Q = m_view->GetPivotRotation();

	const int N = 50;
	vector<double> cw(N + 1), sw(N + 1);
	for (int i=0; i<=N; ++i)
	{	
		double w = 2.0*i*PI/N;
		cw[i] = m_scale*cos(w);
		sw[i] = m_scale*sin(w);
	}

	vec3d r0, r1, p0, p1;

	// X-rotation
	for (int i = 0; i<N; ++i)
	{
		r0.x = 0.0;
		r0.y = cw[i];
		r0.z = sw[i];

		r1.x = 0.0;
		r1.y = cw[i+1];
		r1.z = sw[i+1];

		Q.RotateVector(r0);
		Q.RotateVector(r1);

		p0 = transform.WorldToScreen(o + r0);
		p1 = transform.WorldToScreen(o + r1);

		if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_SELECTION_MODE::SELECT_X;
	}


	// Y-rotation
	for (int i = 0; i<N; ++i)
	{
		r0.x = cw[i];
		r0.y = 0.0;
		r0.z = sw[i];

		r1.x = cw[i + 1];
		r1.y = 0.0;
		r1.z = sw[i + 1];

		Q.RotateVector(r0);
		Q.RotateVector(r1);

		p0 = transform.WorldToScreen(o + r0);
		p1 = transform.WorldToScreen(o + r1);

		if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_SELECTION_MODE::SELECT_Y;
	}

	// Z-rotation
	for (int i = 0; i<N; ++i)
	{
		r0.x = cw[i];
		r0.y = sw[i];
		r0.z = 0.0;

		r1.x = cw[i + 1];
		r1.y = sw[i + 1];
		r1.z = 0.0;

		p0 = transform.WorldToScreen(o + r0);
		p1 = transform.WorldToScreen(o + r1);

		if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_SELECTION_MODE::SELECT_Z;
	}


	return PIVOT_SELECTION_MODE::SELECT_NONE;
}

void GScalor::Render(GLRenderEngine& re, int npivot, bool bactive)
{
	double d = m_scale;
	double l = 0.1*d;
	double r = 0.3*d;

	re.pushState();
	re.setMaterial(GLMaterial::OVERLAY, GLColor::White());

	GLColor g(128, 128, 128);

	// X-axis
	re.pushTransform();
	{
		re.rotate(90, 0, 1, 0);
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_X) re.setColor(GLColor(255,255,0));
			else re.setColor(GLColor(255,0,0));
		}
		else re.setColor(GLColor(g.r, g.g, g.b));
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d));

		re.translate(vec3d(0, 0, d));
		glx::drawCappedCylinder(re, 0.5 * l, l);
	}
	re.popTransform();

	// Y-axis
	re.pushTransform();
	{
		re.rotate(90, -1, 0, 0);
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_Y) re.setColor(GLColor(255,255,0));
			else re.setColor(GLColor(0,255,0));
		}
		else re.setColor(GLColor(g.r, g.g, g.b));
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d));

		re.translate(vec3d(0, 0, d));
		glx::drawCappedCylinder(re, 0.5 * l, l);
	}
	re.popTransform();

	// Z-axis
	re.pushTransform();
	{
		if (bactive)
		{
			if (npivot == PIVOT_SELECTION_MODE::SELECT_Z) re.setColor(GLColor(255,255,0));
			else re.setColor(GLColor(0,0,255));
		}
		else re.setColor(GLColor(g.r, g.g, g.b));
		re.renderLine(vec3d(0, 0, 0), vec3d(0, 0, d));

		re.translate(vec3d(0, 0, d));
		glx::drawCappedCylinder(re, 0.5 * l, l);
	}
	re.popTransform();

	if (bactive)
	{
		// XY-plane
		if (npivot == PIVOT_SELECTION_MODE::SELECT_XY) re.setColor(GLColor(255, 255, 0, 128));
		else re.setColor(GLColor(164, 164, 0, 90));
		re.renderRect(0,0,r,r);
		re.setColor(GLColor(255,255,0, 128));
		re.renderLine(vec3d(r, 0, 0), vec3d(r, r, 0), vec3d(0, r, 0));

		// YZ-plane
		if (npivot == PIVOT_SELECTION_MODE::SELECT_YZ) re.setColor(GLColor(255, 255, 0, 128));
		else re.setColor(GLColor(164, 164, 0, 90));
		re.pushTransform();
		{
			re.rotate(-90, 0, 1, 0);
			re.renderRect(0,0,r,r);

			re.setColor(GLColor(255,255,0,128));
			re.renderLine(vec3d(r, 0, 0), vec3d(r, r, 0), vec3d(0, r, 0));
		}
		re.popTransform();

		// XZ-plane
		if (npivot == PIVOT_SELECTION_MODE::SELECT_XZ) re.setColor(GLColor(255, 255, 0, 128));
		else re.setColor(GLColor(164, 164, 0, 90));
		re.pushTransform();
		{
			re.rotate(90, 1, 0, 0);
			re.renderRect(0,0,r,r);

			re.setColor(GLColor(255,255,0,128));
			re.renderLine(vec3d(r, 0, 0), vec3d(r, r, 0), vec3d(0, r, 0));
		}
		re.popTransform();
	}

	re.popState();
}

int GScalor::Pick(int x, int y)
{
	double r = 0.3*m_scale;
	vec3d o = m_view->GetPivotPosition();
	quatd Q = m_view->GetPivotRotation();
	vec3d ax(r, 0, 0); Q.RotateVector(ax);
	vec3d ay(0, r, 0); Q.RotateVector(ay);
	vec3d az(0, 0, r); Q.RotateVector(az);

	// convert the point to a ray
	GLViewTransform transform(m_view);
	Ray ray = transform.PointToRay(x, y);

	// check the combo-transforms first
	// In this case we need to find the closes point

	Intersection intersect;
	int pivot = PIVOT_SELECTION_MODE::SELECT_NONE;
	double dmin = 1e99;
	Quad qxy = { o, o + ax, o + ax + ay, o + ay };
	if (FastIntersectQuad(ray, qxy, intersect))
	{
		pivot = PIVOT_SELECTION_MODE::SELECT_XY;
		dmin = ray.direction*(intersect.point - ray.origin);
	}

	Quad qyz = { o, o + ay, o + ay + az, o + az };
	if (FastIntersectQuad(ray, qyz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_SELECTION_MODE::SELECT_YZ;
			dmin = D;
		}
	}

	Quad qxz = { o, o + ax, o + ax + az, o + az };
	if (FastIntersectQuad(ray, qxz, intersect))
	{
		double D = ray.direction*(intersect.point - ray.origin);
		if (D < dmin)
		{
			pivot = PIVOT_SELECTION_MODE::SELECT_XZ;
			dmin = D;
		}
	}

	if (pivot != PIVOT_SELECTION_MODE::SELECT_NONE) return pivot;

	// now we'll check the individual axes
	int S = 4;
	QRect rt(x - S, y - S, 2 * S, 2 * S);

	vec3d ex(m_scale, 0, 0); Q.RotateVector(ex);
	vec3d ey(0, m_scale, 0); Q.RotateVector(ey);
	vec3d ez(0, 0, m_scale); Q.RotateVector(ez);

	vec3d p0 = transform.WorldToScreen(o);
	vec3d p1 = transform.WorldToScreen(o + ex);
	vec3d p2 = transform.WorldToScreen(o + ey);
	vec3d p3 = transform.WorldToScreen(o + ez);

	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p1.x, (int)p1.y), rt)) return PIVOT_SELECTION_MODE::SELECT_X;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p2.x, (int)p2.y), rt)) return PIVOT_SELECTION_MODE::SELECT_Y;
	if (intersectsRect(QPoint((int)p0.x, (int)p0.y), QPoint((int)p3.x, (int)p3.y), rt)) return PIVOT_SELECTION_MODE::SELECT_Z;

	return PIVOT_SELECTION_MODE::SELECT_NONE;
}
