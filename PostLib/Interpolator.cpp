#include "stdafx.h"
#include "Interpolator.h"
using namespace Post;

int Interpolator::m_nsteps = 20;
double Interpolator::m_smooth = 0.8;

static double bias(double g, double x)
{
    return pow(x,log(g)/log(0.5));
}

static double gain(double g, double x)
{
  if (x < 0.5) return bias(1-g,2.0*x)*0.5;
     else return 1 - bias(1-g,2-2*x)*0.5;
}

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
bool Interpolator::Next()
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
void VecInterpolator::Target(vec3f v)
{
	m_v1 = v;
	m_v0 = m_vt;
	m_da = 1.0 / (double) Interpolator::m_nsteps;
	m_at = 0;
	m_banim = true;
}

//-----------------------------------------------------------------------------
bool VecInterpolator::Next()
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
			m_vt = m_v0*(1.f - f) + m_v1*f;
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
void QuatInterpolator::Target(quat4f q)
{
	m_q1 = q;
	m_q0 = m_qt;
	m_da = 1.0 / (double) Interpolator::m_nsteps;
	m_at = 0;
	m_banim = true;
}

//-----------------------------------------------------------------------------
bool QuatInterpolator::Next()
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
			m_qt = quat4f::slerp(m_q0, m_q1, f);
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
