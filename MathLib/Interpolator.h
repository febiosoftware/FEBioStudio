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
#include "math3d.h"

//=============================================================================
//! This class implements an interpolator that interpolates between two values.
class Interpolator 
{
public:
	Interpolator();

	bool Update();

	void Target(double g);

	double Value() const { return m_gt; }

	void HitTarget();

	double Target() const { return m_g1; }

public:
	double	m_g0;	// starting value
	double	m_g1;	// target value
	double	m_gt;	// current value
	double	m_at;	// interpolation value
	double	m_da;	// update value
	bool	m_banim;

public:
	static int		m_nsteps;
	static double	m_smooth;	// 0 = linear, 1 < smooth
};

//=============================================================================
//! special interpolator for vectors
class VecInterpolator
{
public:
	VecInterpolator();

	bool Update();

	void Target(vec3d q);

	vec3d Value() const { return m_vt; }

	void HitTarget();

	vec3d Target() const { return m_v1; }

public:
	vec3d	m_v0;	// starting value
	vec3d	m_v1;	// target value
	vec3d	m_vt;	// current value
	double	m_at;	// interpolation value (between 0 and 1)
	double	m_da;	// update value
	bool	m_banim;
};


//=============================================================================
//! special interpolator for quaternions
class QuatInterpolator
{
public:
	QuatInterpolator();

	bool Update();

	void Target(quatd q);

	quatd Value() const { return m_qt; }

	void HitTarget();

	quatd Target() const { return m_q1; }

public:
	quatd	m_q0;	// starting value
	quatd	m_q1;	// target value
	quatd	m_qt;	// current value
	double	m_at;	// interpolation value (between 0 and 1)
	double	m_da;	// update value
	bool	m_banim;
};
