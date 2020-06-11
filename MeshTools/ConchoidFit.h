/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <MathLib/math3d.h>
#include <vector>
using namespace std;

class ConchoidFit  
{
public:
	ConchoidFit();
	virtual ~ConchoidFit();

	void Fit(const vector<vec3d>& y, int maxiter = 100);

	double ObjFunc(double l);

	void Apply(vector<vec3d>& y);

protected:
	void Regression(const vector<vec3d>& y, vec3d& rc, quatd& q, double& a, double& b);
	vec3d Transform(vec3d& rc, quatd& q, const vec3d& p)
	{
		vec3d r = p - rc;
		q.RotateVector(r);
		return r;
	}

	vec3d GetOrientation(const vector<vec3d>& y);

	void FindMinimum();

public:
	vec3d	m_rc;	// center of conchoid
	quatd	m_q;	// orientation of conchoid
	quatd	m_qi;	// inverse of orientation

	double	m_a, m_b;	// conchoid parameters

	vector<vec3d> m_y;
};
