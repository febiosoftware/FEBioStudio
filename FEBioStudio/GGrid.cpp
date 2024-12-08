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
#include "GGrid.h"
#include <GLLib/GLCamera.h>
#include <GLLib/GLContext.h>
#include <GLLib/glx.h>

GGrid::GGrid() : m_o(0,0,0), m_q(0, vec3d(0,0,1))
{
	m_scale = .1f; 
}

vec3d GGrid::Intersect(vec3d r, vec3d t, bool bsnap)
{
	vec3d q = m_o - r;
	vec3d n = m_q*vec3d(0,0,1);

	double nt = n * t;
	double nq = n * q;

	double a = 0;

	if (nt != 0) a = nq / nt;

	vec3d x = r + t*a;

	if (bsnap) x = Snap(x);

	return x;
}

inline double sign(double x) { return (x >= 0.0 ? 1.0 : -1.0); }

vec3d GGrid::Snap(vec3d r)
{
	double s = 1/ (m_scale == 0 ? 1 : m_scale);

	r.x = m_scale*((int) (r.x*s + sign(r.x)*0.5));
	r.y = m_scale*((int) (r.y*s + sign(r.y)*0.5));
	r.z = m_scale*((int) (r.z*s + sign(r.z)*0.5));

	return r;
}

void GGrid::Render(CGLContext& renderContext)
{
	// store attributes
	glPushAttrib(GL_ENABLE_BIT);

	// turn off lighting
	glDisable(GL_LIGHTING);

	// get the camera
	CGLCamera& cam = *renderContext.m_cam;

	// store modelview matrix
	glPushMatrix();

	// orient the grid
	glx::rotate(m_q);

	// determine the scale
	double scale = 1.f*(double) cam.GetTargetDistance();
	double l = (double)((int) log10(scale));
	double s = m_scale = (double) pow(10, l-1);

	// determine the colors for the major axis
	GLColor cx, cy;
	View_Mode view = renderContext.m_settings.m_nview;
	switch (view)
	{
	case VIEW_USER:
    case VIEW_ISOMETRIC:
	case VIEW_TOP: 
	case VIEW_BOTTOM: cx = GLColor(200, 0, 0); cy = GLColor(0, 200, 0); break;
	case VIEW_RIGHT: 
	case VIEW_LEFT:  cx = GLColor(0, 200, 0); cy = GLColor(0, 0, 255); break;
	case VIEW_FRONT:
	case VIEW_BACK:  cx = GLColor(200, 0, 0); cy = GLColor(0, 0, 255); break;
	}

	// get the camera position
	vec3d rc = cam.GlobalPosition();

	int n = 2*(int)(scale/s);

	int nx = (int)(rc.x/s);
	int ny = (int)(rc.y/s);

	int i;

	int i0 = -n+nx;
	int i1 =  n+nx;

	// render the major axis
	glx::drawLine(0, (-n + ny)*s, 0, (n + ny)*s, 0, 1, cy, 2 * n + 1);
	glx::drawLine((-n + nx)*s, 0, (n + nx)*s, 0, 0, 1, cx, 2 * n + 1);

	// grid lines color
	GLColor c1(0,0,0);

	// render the x-lines
	double f, g, a;
	for (i=i0; i<=i1; ++i)
	{
		f = (double) (i-i0) / (double) (i1 - i0);
		g = 1.0 - f;
		a = 4.0*f*g;

		a = 0.35*a*a;
		if (abs(i)%10 == 0) a *= 2;

		if (i != 0) glx::drawLine(i*s, (-n + ny)*s, i*s, (n + ny)*s, 0, a, c1, 2 * n + 1);
	}

	// render the y-lines
	i0 = -n+ny;
	i1 = n+ny;
	for (i=i0; i<=i1; ++i)
	{
		f = (double) (i-i0) / (double) (i1 - i0);
		g = 1.0 - f;
		a = 4.0*f*g;

		a = 0.35*a*a;
		if (i != 0) a *= 0.25;
		if (abs(i)%10 == 0) a *= 2;

		if (i != 0) glx::drawLine((-n + nx)*s, i*s, (n + nx)*s, i*s, 0, a, c1, 2 * n + 1);
	}

	// restore modelview matrix
	glPopMatrix();

	// restore attribs
	glPopAttrib();
}
