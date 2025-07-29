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

#include "stdafx.h"
#include "Interpolator.h"
#include <FSCore/util.h> // for gain function

int Interpolator::m_nsteps = 20;
double Interpolator::m_smooth = 0.8;

//=============================================================================
Interpolator::Interpolator()
{
	m_banim = false;
	m_g0 = m_g1 = m_gt = 0;
	m_at = 0;
	m_da = 0;
}
//-----------------------------------------------------------------------------
void Interpolator::Target(double g)
{
	m_g1 = g;
	m_g0 = m_gt;
	m_at = 0;
	m_da = (m_g1 - m_g0) / m_nsteps;
	m_banim = true;
}

//-----------------------------------------------------------------------------
bool Interpolator::Update()
{
	if (m_banim)
	{
		m_at += m_da;
		if (m_at >= 1.0)
		{
			m_gt = m_g1;
			m_at = 1;
			m_da = 0;
			m_banim = false;
		}
		else
		{
			double g = Interpolator::m_smooth;
			double f = gain(g, m_at);
			m_gt = m_g0*(1.0 - f) + m_g1*f;
		}
	}
	return m_banim;
}

//-----------------------------------------------------------------------------
void Interpolator::HitTarget()
{
	m_gt = m_g1;
	m_at = 1;
	m_da = 0;
	m_banim = false;
}

//=============================================================================
VecInterpolator::VecInterpolator()
{
	m_banim = false;
	m_at = 0;
	m_da = 0;
}

//-----------------------------------------------------------------------------
void VecInterpolator::Target(vec3d v)
{
	m_v1 = v;
	m_v0 = m_vt;
	m_da = 1.0 / (double) Interpolator::m_nsteps;
	m_at = 0;
	m_banim = true;
}

//-----------------------------------------------------------------------------
bool VecInterpolator::Update()
{
	if (m_banim)
	{
		m_at += m_da;
		if (m_at >= 1.0)
		{
			m_at = 1.0;
			m_da = 0;
			m_vt = m_v1;
			m_banim = false;
		}
		else 
		{
			double g = Interpolator::m_smooth;
			double f = gain(g, m_at);
			m_vt = m_v0*(1.0 - f) + m_v1*f;
		}
	}
	return m_banim;
}

//-----------------------------------------------------------------------------
void VecInterpolator::HitTarget()
{
	m_vt = m_v1;
	m_at = 1;
	m_da = 0;
	m_banim = false;
}

//=============================================================================
QuatInterpolator::QuatInterpolator()
{
	m_banim = false;
	m_at = 0;
	m_da = 0;
}
//-----------------------------------------------------------------------------
void QuatInterpolator::Target(quatd q)
{
	m_q1 = q;
	m_q0 = m_qt;
	m_da = 1.0 / (double) Interpolator::m_nsteps;
	m_at = 0;
	m_banim = true;
}

//-----------------------------------------------------------------------------
bool QuatInterpolator::Update()
{
	if (m_banim)
	{
		m_at += m_da;
		if (m_at >= 1.0)
		{
			m_at = 1.0;
			m_da = 0;
			m_qt = m_q1;
			m_banim = false;
		}
		else 
		{
			double g = Interpolator::m_smooth;
			double f = gain(g, m_at);
			m_qt = quatd::slerp(m_q0, m_q1, f);
		}
	}
	return m_banim;
}

//-----------------------------------------------------------------------------
void QuatInterpolator::HitTarget()
{
	m_qt = m_q1;
	m_at = 1;
	m_da = 0;
	m_banim = false;
}
