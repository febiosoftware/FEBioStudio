// SphereFit.h: interface for the SphereFit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPHEREFIT_H__27881A3C_99E2_460C_8EEB_9B6B2D59DC0C__INCLUDED_)
#define AFX_SPHEREFIT_H__27881A3C_99E2_460C_8EEB_9B6B2D59DC0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "math3d.h"
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
// Tool that fits a sphere to a set of points
class SphereFit  
{
public:
	SphereFit();
	virtual ~SphereFit();

	//! calculate the best fit sphere
	void Fit(const vector<vec3f>& y, int maxiter = 100);

	//! evaluate the objective function
	double ObjFunc(const vector<vec3f>& y);

	//! project a set of points onto the sphere
	void Apply(vector<vec3f>& y);

public:
	vec3f	m_rc;	//!< center of sphere
	double	m_R;	//!< radius of sphere
};

#endif // !defined(AFX_SPHEREFIT_H__27881A3C_99E2_460C_8EEB_9B6B2D59DC0C__INCLUDED_)
