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

#pragma once
#include <FSCore/math3d.h>

//-----------------------------------------------------------------------------
class GM_LINE;
class GM_CIRCLE_ARC;

//-----------------------------------------------------------------------------
class GM_LINE
{
public:
	GM_LINE(){}
	GM_LINE(vec2d a, vec2d b) : m_r0(a), m_r1(b) {}

	double Length() { return (m_r1 - m_r0).norm(); }
	double Project(vec2d r, double& D);
	vec2d Point(double l);
	vec2d Tangent(double l);

	bool Intersect(GM_LINE& e, double& w);
	bool Intersect(GM_CIRCLE_ARC& e, double& w);

public:
	vec2d	m_r0, m_r1;
};

//-----------------------------------------------------------------------------
// circular arc in 2D
class GM_CIRCLE_ARC
{
public:
	GM_CIRCLE_ARC() { m_R = 0; m_w0 = m_w1 = 0; }
	GM_CIRCLE_ARC(vec2d c, double R, double w0, double w1);
	GM_CIRCLE_ARC(vec2d c, vec2d a, vec2d b, int nw = 1);

	double Coord(vec2d& q);
	double Length();
	double Project(vec2d r, double& D);
	vec2d Point(double l);
	vec2d Tangent(double l);

	bool Intersect(GM_LINE& e, double& w);
	bool Intersect(GM_CIRCLE_ARC& e, double& w);

public:
	vec2d	m_c;
	double	m_R;
	double	m_w0, m_w1;
};

//-----------------------------------------------------------------------------
// Ellipsoidal arc (aligned with x,y coord system)
class GM_ARC
{
public:
	GM_ARC() { m_R0 = m_R1 = 0; m_w0 = m_w1 = 0; }
	GM_ARC(vec2d c, double R0, double R1, double w0, double w1);
	GM_ARC(vec2d c, vec2d a, vec2d b, int nw = 1);

	double Coord(vec2d& q);
	double Length();
	double Project(vec2d r, double& D);
	vec2d Point(double l);
	vec2d Tangent(double l);

	bool Intersect(GM_LINE& e, double& w);
	bool Intersect(GM_CIRCLE_ARC& e, double& w);

public:
	vec2d	m_c;			// center point
	double	m_R0, m_R1;		// radii of ellipsoid
	double	m_w0, m_w1;		// start/end parameter
};

//-----------------------------------------------------------------------------
// circular arc in 3D
// c = center
// a,b = two points defining the circular arc
class GM_CIRCLE_3P_ARC
{
public:
	GM_CIRCLE_3P_ARC(const vec3d& c, const vec3d& a, const vec3d& b, int nw = 1);

	vec3d Point(double l);

public:
	vec3d	m_c, m_a, m_b;
	int		m_winding;
};

class GM_BEZIER
{
public:
	GM_BEZIER(const std::vector<vec3d>& P) : m_P(P) {}

	vec3d Point(double l);

private:
	std::vector<vec3d> m_P;
};