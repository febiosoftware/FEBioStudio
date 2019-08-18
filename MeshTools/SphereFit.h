// SphereFit.h: interface for the SphereFit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPHEREFIT_H__27881A3C_99E2_460C_8EEB_9B6B2D59DC0C__INCLUDED_)
#define AFX_SPHEREFIT_H__27881A3C_99E2_460C_8EEB_9B6B2D59DC0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <MathLib/math3d.h>
#include <vector>
using namespace std;

class SphereFit  
{
public:
	SphereFit();
	virtual ~SphereFit();

	bool Fit(const vector<vec3d>& y, int maxiter = 100);

	double ObjFunc(const vector<vec3d>& y);

	void Apply(vector<vec3d>& y);

public:
	vec3d	m_rc;	// center of sphere
	double	m_R;	// radius of sphere
};

#endif // !defined(AFX_SPHEREFIT_H__27881A3C_99E2_460C_8EEB_9B6B2D59DC0C__INCLUDED_)
