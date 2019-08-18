#pragma once
#include "math3d.h"

namespace Post {

//=============================================================================
//! This class implements an interpolator that interpolates between two values.
class Interpolator 
{
public:
	Interpolator();

	bool Next();

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

	bool Next();

	void Target(vec3f q);

	vec3f Value() const { return m_vt; }

	void HitTarget();

	vec3f Target() const { return m_v1; }

public:
	vec3f	m_v0;	// starting value
	vec3f	m_v1;	// target value
	vec3f	m_vt;	// current value
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

	bool Next();

	void Target(quat4f q);

	quat4f Value() const { return m_qt; }

	void HitTarget();

	quat4f Target() const { return m_q1; }

public:
	quat4f	m_q0;	// starting value
	quat4f	m_q1;	// target value
	quat4f	m_qt;	// current value
	double	m_at;	// interpolation value (between 0 and 1)
	double	m_da;	// update value
	bool	m_banim;
};
}
