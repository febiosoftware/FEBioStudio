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
#include "GLCamera.h"

GLCamera::GLCamera()
{
	Reset();
}

GLCamera::~GLCamera()
{

}

void GLCamera::Reset()
{
	SetCameraSpeed(0.8f);
	SetCameraBias(0.8f);
	m_rot.Target(quatd(0, vec3d(1,0,0)));
	m_pos.Target(vec3d(0,0,0));
	m_trg.Target(vec3d(0,0,0));
	Update(true);
	m_bortho = false;
	m_fnear = 1.f;
	m_ffar = 50000.f;
	m_fov = 45.f;
	m_isMoving = false;
}

void GLCamera::SetOrthoProjection(bool b) { m_bortho = b; }

bool GLCamera::IsOrtho() const { return m_bortho; }

void GLCamera::MakeActive()
{
	Interpolator::m_smooth = 0.5f + m_bias * 0.45f;
	Interpolator::m_nsteps = 5 + (int)((1.0 - m_speed) * 60.0);
}

void GLCamera::SetCameraSpeed(double f)
{
	if (f > 1.0) f = 1.0;
	if (f < 0.0) f = 0.0;
	m_speed = f;
}

void GLCamera::SetCameraBias(double f)
{
	if (f > 1.f) f = 1.f;
	if (f < 0.f) f = 0.f;
	m_bias = f;
}

bool GLCamera::IsAnimating()
{
	bool banim = false;
	banim |= m_pos.m_banim;
	banim |= m_trg.m_banim;
	banim |= m_rot.m_banim;
	return banim;
}

void GLCamera::Update(bool bhit)
{
	if (bhit == false)
	{
		m_pos.Update();
		m_trg.Update();
		m_rot.Update();
	}
	else
	{
		m_pos.HitTarget();
		m_trg.HitTarget();
		m_rot.HitTarget();
	}
}

void GLCamera::Pan(const quatd& q)
{
	vec3d p = GetPosition();
	quatd Q = m_rot.Target();

	Q.RotateVector(p);
	p += Target();

	q.RotateVector(p);

	p -= Target();

	Q = q * Q;
	Q.Inverse().RotateVector(p);

	SetTarget(p);
	SetOrientation(Q);
}

void GLCamera::PanView(const vec3d& r)
{
	double f = 0.001f * (double)GetFinalTargetDistance();
	Truck(r*f);
}

void GLCamera::Dolly(double f)
{
	vec3d dr(0, 0, -f);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}

void GLCamera::Truck(const vec3d& v)
{
	vec3d dr(v);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}

void GLCamera::Orbit(quatd& q)
{
	quatd o = q*m_rot.Target();
	o.MakeUnit();
	m_rot.Target(o);
}

void GLCamera::Zoom(double f)
{
	SetTargetDistance(GetFinalTargetDistance() * f);
}

void GLCamera::SetTarget(const vec3d& r)
{
	m_pos.Target(r);
}

// set the target in local coordinates
void GLCamera::SetLocalTarget(const vec3d& r)
{
	m_trg.Target(r);
}

void GLCamera::SetViewDirection(const vec3d &r)
{
	if (r.Length() != 0.f)
	{
		m_rot.Target(quatd(vec3d(0, 0, 1.f), r).Inverse());
	}
}

void GLCamera::SetTransform(GLCameraTransform& t)
{
	m_pos.Target(t.pos);
	m_trg.Target(t.trg);
	m_rot.Target(t.rot);
}

void GLCamera::GetTransform(GLCameraTransform& t)
{
	t.pos = m_pos.Value();
	t.trg = m_trg.Value();
	t.rot = m_rot.Value();
}

vec3d GLCamera::WorldToCam(vec3d r) const
{
	r -= Target();
	quatd q = m_rot.Value();
	q.RotateVector(r);
	r -= GetPosition();

	return r;
}

vec3d GLCamera::CamToWorld(vec3d r) const
{
	r -= GetPosition();
	m_rot.Value().RotateVector(r);
	r -= Target();

	return r;
}

// get the position in global coordinates
vec3d GLCamera::GlobalPosition() const
{
	vec3d r = Target();
	m_rot.Value().Inverse().RotateVector(r);
	r += GetPosition();
	return r;
}
