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

#include "geom.h"
#include <assert.h>
#include <FSCore/math3d.h>

//-----------------------------------------------------------------------------
// solves the quadratic: a*x^2 + b*x + c = 0
//
int quadratic(double a, double b, double c, double x[2])
{
	double D = b*b - 4.0*a*c;
	if (D < 0) return 0;
	if (D == 0)
	{
		x[0] = x[1] = -b/(2.0*a);
		return 1;
	}
	else
	{
		if (b < 0)
		{
			x[0] = (-b + sqrt(D))/(2.0*a);
			x[1] = c/(a*x[0]);
		}
		else
		{
			x[1] = (-b - sqrt(D))/(2.0*a);
			x[0] = c/(a*x[1]);
		}
		return 2;
	}
}

//=============================================================================
// GM_LINE
//-----------------------------------------------------------------------------
vec2d GM_LINE::Point(double l)
{
	return m_r0*(1.0 - l) + m_r1*l;
}

//-----------------------------------------------------------------------------
vec2d GM_LINE::Tangent(double l)
{
	vec2d t = m_r1 - m_r0;
	t.unit();
	return t;
}

//-----------------------------------------------------------------------------
// This function returns the parametric coordinate along the line and if the 
// projection lies on the line, the value will be between zero and one.
double GM_LINE::Project(vec2d r, double& D)
{
	vec2d ab = m_r1 - m_r0;
	double d2 = ab*ab;
	assert(d2 != 0.0);
	if (d2 != 0.0)
	{
		double l = ((r - m_r0)*ab)/d2;
		vec2d p = m_r0*(1.0 - l) + m_r1*l;
		D = (r - p).norm();
		return l;
	}
	else
	{
		D = (r - m_r0).norm();
		return 0;
	}
}

//-----------------------------------------------------------------------------
bool GM_LINE::Intersect(GM_LINE& e, double& w)
{
	vec2d a0 = m_r0;
	vec2d a1 = m_r1;
	vec2d b0 = e.m_r0;
	vec2d b1 = e.m_r1;

	double Q[2][2], B[2];
	B[0] = b0.x() - a0.x();
	B[1] = b0.y() - a0.y();
	Q[0][0] = a1.x() - a0.x(); Q[0][1] = b0.x() - b1.x(); 
	Q[1][0] = a1.y() - a0.y(); Q[1][1] = b0.y() - b1.y();
	double D = Q[0][0]*Q[1][1] - Q[0][1]*Q[1][0];
	if (D == 0) return false;

	double wa, wb;
	wa = ( Q[1][1]*B[0] - Q[0][1]*B[1])/D;
	wb = (-Q[1][0]*B[0] + Q[0][0]*B[1])/D;
	if ((wa>0)&&(wa<1)&&(wb>0)&&(wb<1))
	{
		w = wa;
		return true;
	}
	else return false;
}

//-----------------------------------------------------------------------------
bool GM_LINE::Intersect(GM_CIRCLE_ARC &e, double &w)
{
	vec2d r0 = m_r0;
	vec2d r1 = m_r1;
	vec2d c = e.m_c;
	double R = e.m_R;

	double ax = r0.x() - c.x();
	double ay = r0.y() - c.y();
	double bx = r1.x() - r0.x();
	double by = r1.y() - r0.y();

	double a2 = bx*bx + by*by;
	double a1 = 2.0*(ax*bx + ay*by);
	double a0 = ax*ax + ay*ay - R*R;

	double l[2];
	if (quadratic(a2,a1,a0,l) == 0) return false;

	w = l[0];
	if ((w<=0)||(w>=1)) w = l[1];

	if ((w>0)&&(w<1))
	{
		vec2d q = Point(w);
		double lc = e.Coord(q);
		if ((lc>0)&&(lc<1)) return true;
		else return false;
	}
	else return false;
}

//=============================================================================
// GM_CIRCLE_ARC
//-----------------------------------------------------------------------------
GM_CIRCLE_ARC::GM_CIRCLE_ARC(vec2d c, double R, double w0, double w1)
{
	m_c = c;
	m_R = R;
	m_w0 = w0;
	m_w1 = w1;
}

//-----------------------------------------------------------------------------
GM_CIRCLE_ARC::GM_CIRCLE_ARC(vec2d c, vec2d a, vec2d b, int nw)
{
	vec2d e1 = a - c;
	vec2d e2 = b - c;

	m_R = e1.unit();
	e2.unit();

	m_c = c;
	m_w0 = acos(e1.x());
	m_w1 = acos(e2.x());

	if (e1.y() < 0) m_w0 = -m_w0;
	if (e2.y() < 0) m_w1 = -m_w1;

	if (nw == 1)
	{
		if (m_w1 <= m_w0) m_w1 += 2.0*PI;
	}
	else
	{
		if (m_w1 >= m_w0) m_w1 -= 2.0*PI;
	}
}

//-----------------------------------------------------------------------------
double GM_CIRCLE_ARC::Coord(vec2d& q)
{
	vec2d e = q - m_c; e.unit();
	double w = acos(e.x());
	if (e.y() < 0) w = -w;
	if (w < m_w0) w += 2.0*PI;

	assert(m_w1 != m_w0);
	double l = 0;
	if (m_w1 != m_w0) l = (w - m_w0)/(m_w1 - m_w0);
	
	return l;
}

//-----------------------------------------------------------------------------
double GM_CIRCLE_ARC::Length()
{
	return m_R*(m_w1 - m_w0);
}

//-----------------------------------------------------------------------------
vec2d GM_CIRCLE_ARC::Point(double l)
{
	double w = m_w0 + l*(m_w1 - m_w0);
	return m_c + vec2d(m_R*cos(w), m_R*sin(w));
}

//-----------------------------------------------------------------------------
vec2d GM_CIRCLE_ARC::Tangent(double l)
{
	double w = m_w0 + l*(m_w1 - m_w0);
	vec2d t(-sin(w), cos(w));
	return t;
}

//-----------------------------------------------------------------------------
// This function returns the parametric coordinate along the line and if the 
// projection lies on the line, the value will be between zero and one.
double GM_CIRCLE_ARC::Project(vec2d r, double& D)
{
	double A = (r - m_c).norm();

	double l = Coord(r);
	if ((l>0)&&(l<1)) D = fabs(m_R - A);
	else
	{
		vec2d a = Point(0.0);
		vec2d b = Point(1.0);
		double La = (r - a).norm();
		double Lb = (r - b).norm();
		D = (La>Lb?Lb:La);
	}
	return l;
}

//-----------------------------------------------------------------------------
bool GM_CIRCLE_ARC::Intersect(GM_LINE &e, double &w)
{
	vec2d r0 = e.m_r0;
	vec2d r1 = e.m_r1;
	vec2d c = m_c;

	double ax = r0.x() - c.x();
	double ay = r0.y() - c.y();
	double bx = r1.x() - r0.x();
	double by = r1.y() - r0.y();

	double a2 = bx*bx + by*by;
	double a1 = 2.0*(ax*bx + ay*by);
	double a0 = ax*ax + ay*ay - m_R*m_R;

	double l[2];
	if (quadratic(a2, a1, a0, l) == 0) return false;

	double lm = l[0];
	if ((lm<=0)||(lm>=1)) lm = l[1];

	if ((lm>0)&&(lm<1))
	{
		vec2d q = e.Point(lm);
		w = Coord(q);
		if ((w>0)&&(w<1)) return true;
		else return false;
	}
	else return false;
}

//-----------------------------------------------------------------------------
bool GM_CIRCLE_ARC::Intersect(GM_CIRCLE_ARC &e, double &w)
{
	double R0 = m_R;
	double R1 = e.m_R;
	vec2d c0 = m_c;
	vec2d c1 = e.m_c;

	// let's begin by eliminating cases
	if (c0 == c1) return false;
	double D = (c0 - c1).norm();
	if (D > R0 + R1) return false;
	if (D + R0 < R1) return false;
	if (D + R1 < R0) return false;

	double a0 = c0.x(), a1 = c0.y();
	double b0 = c1.x(), b1 = c1.y();
	double a2 = a0*a0 + a1*a1;
	double b2 = b0*b0 + b1*b1;

	double ab0 = fabs(a0 - b0);
	double ab1 = fabs(a1 - b1);
	if ((ab0 == 0)&&(ab1==0)) return false;

	vec2d r[2];
	if (ab0 > ab1)
	{
		double g0 = -((R0*R0 - R1*R1)-(a2 - b2))/(2.0*(a0 - b0));
		double g1 = -((a1 - b1)/(a0 - b0));

		double h2 = 1 + g1*g1;
		double h1 = 2.0*(g0*g1 - a1 - a0*g1);
		double h0 = g0*g0 + a2 - 2.0*a0*g0 - R0*R0;

		double y[2];
		if (quadratic(h2, h1, h0, y) == 0) return false;

		r[0].x() = g0 + g1*y[0];
		r[0].y() = y[0];

		r[1].x() = g0 + g1*y[1];
		r[1].y() = y[1];
	}
	else
	{
		double g0 = -((R0*R0 - R1*R1)-(a2 - b2))/(2.0*(a1 - b1));
		double g1 = -((a0 - b0)/(a1 - b1));

		double h2 = 1 + g1*g1;
		double h1 = 2.0*(g0*g1 - a0 - a1*g1);
		double h0 = g0*g0 + a2 - 2.0*a1*g0 - R0*R0;

		double x[2];
		if (quadratic(h2, h1, h0, x) == 0) return false;

		r[0].x() = x[0];
		r[0].y() = g0 + g1*x[0];

		r[1].x() = x[1];
		r[1].y() = g0 + g1*x[1];
	}

	// we found our intersection point(s) r
	for (int i=0; i<2; ++i)
	{
		double w0 = Coord(r[i]);
		double w1 = e.Coord(r[i]);
		if ((w0>0)&&(w0<1)&&(w1>0)&&(w1<1))
		{
			w = w0;
			return true;
		}
	}
	return false;
}

//=============================================================================
// GM_ARC
//-----------------------------------------------------------------------------
GM_ARC::GM_ARC(vec2d c, double R0, double R1, double w0, double w1)
{
	m_c = c;
	m_R0 = R0;
	m_R1 = R1;
	m_w0 = w0;
	m_w1 = w1;
}

//-----------------------------------------------------------------------------
// constructs the ellips from points a,b,c, where
// c is the center and a, b are two points on the ellips
GM_ARC::GM_ARC(vec2d c, vec2d a, vec2d b, int nw)
{
	vec2d A = a - c;
	vec2d B = b - c;

	double ua = A.x()*A.x(), va = A.y()*A.y();
	double ub = B.x()*B.x(), vb = B.y()*B.y();
	double N = vb*ua-va*ub;
	double r0 = N / (vb - va);
	double r1 = N / (ua - ub);

	m_R0 = sqrt(r0);
	m_R1 = sqrt(r1);
	m_c = c;

	m_w0 = atan2(A.y()/m_R1, A.x()/m_R0);
	m_w1 = atan2(B.y()/m_R1, B.x()/m_R0);

	if (nw == 1)
	{
		if (m_w1 <= m_w0) m_w1 += 2.0*PI;
	}
	else
	{
		if (m_w1 >= m_w0) m_w1 -= 2.0*PI;
	}
}

//-----------------------------------------------------------------------------
// returns the curve parameter scaled to [0..1] between [w0,w1].
double GM_ARC::Coord(vec2d& q)
{
	vec2d e = q - m_c;
	double w = atan2(e.y()/m_R1, e.x() /m_R0);
	if (e.y() < 0) w = -w;
	if (w < m_w0) w += 2.0*PI;

	assert(m_w1 != m_w0);
	double l = 0;
	if (m_w1 != m_w0) l = (w - m_w0)/(m_w1 - m_w0);
	
	return l;
}

//-----------------------------------------------------------------------------
double GM_ARC::Length()
{
	assert(false);
	return 0.0;
}

//-----------------------------------------------------------------------------
// evaluates the curve at point l = [0..1] between [w0,w1]
vec2d GM_ARC::Point(double l)
{
	double w = m_w0 + l*(m_w1 - m_w0);
	return m_c + vec2d(m_R0*cos(w), m_R1*sin(w));
}

//-----------------------------------------------------------------------------
// evaluates the tangent to the curve at point l = [0..1] between [w0,w1].
vec2d GM_ARC::Tangent(double l)
{
	double w = m_w0 + l*(m_w1 - m_w0);
	vec2d t(-m_R0*sin(w), m_R1*cos(w));
	t.unit();
	return t;
}

//-----------------------------------------------------------------------------
// This function returns the parametric coordinate along the line and if the 
// projection lies on the line, the value will be between zero and one.
double GM_ARC::Project(vec2d r, double& D)
{
	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------
bool GM_ARC::Intersect(GM_LINE &e, double &w)
{
	assert(false);
	return false;
}

//-----------------------------------------------------------------------------
bool GM_ARC::Intersect(GM_CIRCLE_ARC &e, double &w)
{
	assert(false);
	return false;
}

//===============================================================================
GM_CIRCLE_3P_ARC::GM_CIRCLE_3P_ARC(const vec3d& c, const vec3d& a, const vec3d& b, int nw)
{
	m_c = c;
	m_a = a;
	m_b = b;
	m_winding = nw;
}

vec3d GM_CIRCLE_3P_ARC::Point(double l)
{
	vec3d r1 = m_a - m_c;
	vec3d r2 = m_b - m_c;
	vec3d n = r1 ^ r2; n.Normalize();
	quatd q(n, vec3d(0, 0, 1)), qi = q.Inverse();
	q.RotateVector(r1);
	q.RotateVector(r2);
	GM_CIRCLE_ARC c(vec2d(0, 0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y), m_winding);
	vec2d a = c.Point(l);
	vec3d p(a.x(), a.y(), 0);
	qi.RotateVector(p);
	vec3d r = p + m_c;
	return r;
}

vec3d GM_BEZIER::Point(double u)
{
	std::vector<vec3d> Q(m_P);
	int n = Q.size() - 1;
	for (int k = 1; k <= n; ++k)
		for (int i = 0; i <= n - k; ++i)
			Q[i] = Q[i] * (1.0 - u) + Q[i + 1] * u;

	return Q[0];
}
