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

// onchoidFit.cpp: implementation of the ConchoidFit class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ConchoidFit.h"
#include "SphereFit.h"
#include <FECore/tools.h>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ConchoidFit::ConchoidFit()
{

}

ConchoidFit::~ConchoidFit()
{

}

ConchoidFit* pfitter;
double func(double l)
{
	ConchoidFit& fit = *pfitter;
	return fit.ObjFunc(l);
}

vec3d ConchoidFit::GetOrientation(const vector<vec3d>& y)
{
	vec3d o;
	int N = (int)y.size();
	for (int i=0; i<N; ++i)
	{
		vec3d r = y[i] - m_rc;
		o += r;
	}
	o.Normalize();

	return o;
}

void ConchoidFit::FindMinimum()
{
	// bracket the minimum
	pfitter = this;
	double la = m_a;
	double lb = m_b;
	double lc = 0;
	double fa, fb, fc;
	mnbrak(&la, &lb, &lc, &fa, &fb, &fc, func);

	// find the minimum using Brent's method
	double lmin;
	double Fmin = brent(la, lb, lc, func, 0.01, &lmin);

	// we have found the minimum, so set the new center
	if (lmin != 0)
	{
		vec3d p(lmin,0,0);
		m_q.RotateVector(p);

		m_rc = m_rc + p;
	}
	printf("Center of conchoid: (%lg, %lg, %lg)\n", m_rc.x, m_rc.y, m_rc.z);
}

// complete fit algorithm
void ConchoidFit::Fit(const vector<vec3d>& y, int maxiter)
{
	m_y = y;

	// fit a sphere to the data
	SphereFit s;
	s.Fit(y);

	// the center of the sphere is our starting center for the conchoid
	m_rc = s.m_rc;

	// the orientation is the average orientation to this center
	vec3d o = GetOrientation(y);

	// set up the quaternions
	m_q = quatd(vec3d(1,0,0), o);
	m_qi = m_q.Inverse();

	// find the minimum
	m_a = -s.m_R*0.5;
	m_b = 0;
	FindMinimum();

	// determine the final conchoid parameters
	Regression(y, m_rc, m_qi, m_a, m_b);
}

void ConchoidFit::Apply(vector<vec3d>& y)
{
	int N = (int) y.size();
	for (int i=0; i<N; ++i)
	{
		vec3d r = y[i];
		Transform(m_rc, m_q, r);
		double l = m_a + m_b*r.x;

		vec3d q = y[i] - m_rc;
		q.Normalize();
		y[i] = m_rc + q*l;
	}
}

void ConchoidFit::Regression(const vector<vec3d>& y, vec3d& rc, quatd& q, double& a, double& b)
{
	int i;
	double x, r;
	quatd qi = m_q.Inverse();

	double sx = 0, sy = 0;
	double sx2 = 0, sxy = 0;

	int m = 0;

	int N = (int) y.size();
	for (i=0; i<N; ++i)
	{
		// calculate the polar coordinates of the point
		vec3d p = Transform(rc, q, y[i]);
		r = p.Length();	// = r
		p.Normalize();
		x = p.x;		// = cos(theta)

		// do the summations
		sx += x;
		sy += r;
		sx2 += x*x;
		sxy += x*r;
		++m;
	}

	// calculate the least square solution
	b = (m*sxy - sx*sy) / (m*sx2 - sx*sx);
	a = (sy - b*sx)/m;
}

double ConchoidFit::ObjFunc(double l)
{
	// calculate a new center
	vec3d rc;
	if (l != 0)
	{
		vec3d p(l,0,0);
		m_q.RotateVector(p);

		rc = m_rc + p;
	}
	else rc = m_rc;

	double a, b;
	Regression(m_y, rc, m_qi, a, b);

	// calculate the objective function
	double F = 0, df;
	int m = 0;
	double x, y;
	int N = (int) m_y.size();
	for (int i=0; i<N; ++i)
	{
		vec3d p = Transform(rc, m_qi, m_y[i]);
		y = p.Length();
		p.Normalize();
		x = p.x;

		df = y - a - b*x;

		F += df*df;
		++m;
	}

	return F / m;
}
