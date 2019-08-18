#pragma once
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
