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

// SphereFit.cpp: implementation of the SphereFit class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <stdio.h>
#include "SphereFit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define SQR(x) ((x)*(x))

SphereFit::SphereFit()
{
	m_R = 0;
}

SphereFit::~SphereFit()
{

}

void SphereFit::Apply(vector<vec3d>& y)
{
	int N = (int) y.size();
	for (int i=0; i<N; ++i)
	{
		vec3d r = y[i] - m_rc;
		r.Normalize();
		y[i] = m_rc + r*m_R;
	}
}

bool SphereFit::Fit(const vector<vec3d>& y, int maxiter)
{
	const double TOL = 0.001;
	int i;

	int N = (int) y.size();
	if (N == 0) return false;

	// Get the average coordindates
	vec3d ra = y[0];	
	for (i=1; i<N; i++) ra += y[i];
	ra /= N;

	vec3d rc = ra, rcp = rc, drc;
	double L, La, Lb, Lc, Li;

	bool bdone = false;

	int iter = 0;

	do
	{
		L = La = Lb = Lc = 0;
		for (i=0; i<N; i++)
		{
			const vec3d& r = y[i];

			Li = sqrt(SQR(r.x - rc.x) + SQR(r.y - rc.y) + SQR(r.z - rc.z));
			L += Li;
			La += (rc.x - r.x) / Li;
			Lb += (rc.y - r.y) / Li;
			Lc += (rc.z - r.z) / Li;
		}
		L /= N; La /= N; Lb /= N; Lc /= N;

		rc.x = ra.x + L*La;
		rc.y = ra.y + L*Lb;
		rc.z = ra.z + L*Lc;

		drc.x = fabs(rc.x - rcp.x);
		drc.y = fabs(rc.y - rcp.y);
		drc.z = fabs(rc.z - rcp.z);

		if ((drc.x < TOL) && (drc.y < TOL) && (drc.z < TOL)) bdone = true;
		else rcp = rc;

		if (iter == maxiter) bdone = true; else iter++;
	}
	while (!bdone);	

	// store results in sphere object
	m_rc = rc;
	m_R = L;

	return true;
}

double SphereFit::ObjFunc(const vector<vec3d>& y)
{
	double f = 0;
	int N = (int) y.size();
	for (int i=0; i<N; ++i) 
	{
		double l = sqrt((y[i] - m_rc)*(y[i] - m_rc)) - m_R;
		f += l*l;
	}
	return sqrt(f / N);
}
