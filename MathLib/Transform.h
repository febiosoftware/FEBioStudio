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

#pragma once
#include "math3d.h"

//-------------------------------------------------------------------
// This class describes a transformation. 
// It is used to define the global position, rotation, and scale of an object
class Transform
{
public:
	// constructor
	Transform() { Reset(); }

	// reset the transform
	void Reset()
	{
		m_pos = vec3d(0, 0, 0);
		m_scl = vec3d(1, 1, 1);
		m_rot = m_roti = quatd(0, 0, 0, 1);
	}

	// set position of object
	void SetPosition(const vec3d& r) { m_pos = r; }

	// get position of object
	const vec3d& GetPosition() const { return m_pos; }

	// set orientation of object
	void SetRotation(const quatd& q) { m_rot = q; m_roti = q.Inverse(); }

	//! get orientation
	const quatd& GetRotation() const { return m_rot; }

	// get inverse of rotation
	quatd GetRotationInverse() const { return m_roti; }

	// set the scale of the object
	void SetScale(vec3d s) { m_scl = s; }

	//! get scale of the object
	const vec3d& GetScale() const { return m_scl; }

	// Rotate angle w around an axis defined by the position vectors a, b.
	void Rotate(const vec3d& a, const vec3d& b, double w)
	{
		double wr = PI*w / 180.0;
		vec3d n = (b - a); n.Normalize();
		quatd q(wr, n);
		Rotate(q, a);
	}

	// rotate around the center rc
	void Rotate(quatd q, vec3d rc)
	{
		m_rot = q*m_rot;
		m_roti = m_rot.Inverse();

		m_rot.MakeUnit();
		m_roti.MakeUnit();

		m_pos = rc + q*(m_pos - rc);
	}

	// scale an object
	void Scale(double s, vec3d r, vec3d rc)
	{
		vec3d r0 = GlobalToLocal(rc);

		double a = s - 1;
		m_roti.RotateVector(r);
		r.Normalize();

		r.x = 1 + a*fabs(r.x);
		r.y = 1 + a*fabs(r.y);
		r.z = 1 + a*fabs(r.z);

		m_scl.x *= r.x;
		m_scl.y *= r.y;
		m_scl.z *= r.z;

		m_pos -= LocalToGlobal(r0) - rc;
	}

	// translate the transform
	void Translate(const vec3d& dr) { m_pos += dr; }

	// convert from local to global coordinates
	vec3d LocalToGlobal(const vec3d& r) const
	{
		return m_pos + m_rot*vec3d(r.x*m_scl.x, r.y*m_scl.y, r.z*m_scl.z);
	}

	// convert from global to local coordinates
	vec3d GlobalToLocal(const vec3d& r) const
	{
		vec3d p = m_roti*(r - m_pos);
		return vec3d(p.x / m_scl.x, p.y / m_scl.y, p.z / m_scl.z);
	}

	//! get a normal-like vector from global to local
	vec3d LocalToGlobalNormal(const vec3d& n) const
	{
		// NOTE: scaling is turned off because this is used in the generation of material axes.
		//       If I use scaling the axes may no longer be orthogonal. Maybe I should create another
		//       function for this since this is now inconsistent with the reverse operation.
//		return m_rot*vec3d(n.x / m_scl.x, n.y / m_scl.y, n.z / m_scl.z);
		return m_rot*vec3d(n.x, n.y, n.z);
	}

	//! get a normal-like vector from global to local
	vec3d GlobalToLocalNormal(const vec3d& n) const
	{
		vec3d m = m_roti*n;
		m.x /= m_scl.x; m.y /= m_scl.y; m.z /= m_scl.z;
		m.Normalize();
		return m;
	}

	bool operator == (const Transform& T)
	{
		return ((m_pos == T.m_pos) && (m_scl == T.m_scl) && (m_rot == T.m_rot));
	}

private:
	// transform data
	vec3d	m_pos;	//!< position of object
	vec3d	m_scl;	//!< scale of object
	quatd	m_rot;	//!< orientation of object
	quatd	m_roti;	//!< inverse orientation
};
