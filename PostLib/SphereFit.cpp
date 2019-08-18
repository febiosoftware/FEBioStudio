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

//-----------------------------------------------------------------------------
SphereFit::SphereFit()
{
	m_R = 0;
}

//-----------------------------------------------------------------------------
SphereFit::~SphereFit()
{

}

//-----------------------------------------------------------------------------
void SphereFit::Apply(vector<vec3f>& y)
{
	int N = (int) y.size();
	for (int i=0; i<N; ++i)
	{
		vec3f r = y[i] - m_rc;
		r.Normalize();
		y[i] = m_rc + r*m_R;
	}
}

//-----------------------------------------------------------------------------
void SphereFit::Fit(const vector<vec3f>& y, int maxiter)
{
	const double TOL = 0.001;
	int i;

	int N = (int) y.size();

	// Get the average coordindates
	vec3f ra = y[0];	
	for (i=1; i<N; i++) ra += y[i];
	ra /= N;

	vec3f rc = ra, rcp = rc, drc;
	double L, La, Lb, Lc, Li;

	bool bdone = false;

	int iter = 0;

	do
	{
		L = La = Lb = Lc = 0;
		for (i=0; i<N; i++)
		{
			const vec3f& r = y[i];

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
}

//-----------------------------------------------------------------------------
double SphereFit::ObjFunc(const vector<vec3f>& y)
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
