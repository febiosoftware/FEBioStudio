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
#ifdef WIN32
#include <Windows.h>
#include <GL/gl.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif
#ifdef LINUX
#include <GL/gl.h>
#endif
#include "glx.h"

//=============================================================================
GLCameraTransform::GLCameraTransform(const GLCameraTransform& key)
{
	pos = key.pos;
	trg = key.trg;
	rot = key.rot;
	SetName(key.GetName());
}

GLCameraTransform& GLCameraTransform::operator = (const GLCameraTransform& key)
{
	pos = key.pos;
	trg = key.trg;
	rot = key.rot;
	SetName(key.GetName());
	return *this;
}

//=============================================================================
CGLCamera::CGLCamera()
{
	Reset();
}

//-----------------------------------------------------------------------------
CGLCamera::~CGLCamera()
{

}

//-----------------------------------------------------------------------------
void CGLCamera::Reset()
{
	SetCameraSpeed(0.8f);
	SetCameraBias(0.8f);
	m_rot.Target(quatd(0, vec3d(1,0,0)));
	m_pos.Target(vec3d(0,0,0));
	m_trg.Target(vec3d(0,0,0));
	Update(true);

	m_bdecal = false;
	m_bortho = false;
}

//-----------------------------------------------------------------------------
void CGLCamera::SetOrthoProjection(bool b) { m_bortho = b; }

bool CGLCamera::IsOrtho() const { return m_bortho; }

//-----------------------------------------------------------------------------
void CGLCamera::SetCameraSpeed(double f)
{
	if (f > 1.0) f = 1.0;
	if (f < 0.0) f = 0.0;
	m_speed = f;
	Interpolator::m_nsteps = 5 + (int)((1.0 - f)*60.0);
}

//-----------------------------------------------------------------------------
void CGLCamera::SetCameraBias(double f)
{
	if (f > 1.f) f = 1.f;
	if (f < 0.f) f = 0.f;
	m_bias = f;
	Interpolator::m_smooth = 0.5f + f*0.45f;
}

//-----------------------------------------------------------------------------
bool CGLCamera::IsAnimating()
{
	bool banim = false;
	banim |= m_pos.m_banim;
	banim |= m_trg.m_banim;
	banim |= m_rot.m_banim;
	return banim;
}

//-----------------------------------------------------------------------------
void CGLCamera::Update(bool bhit)
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

//-----------------------------------------------------------------------------
// set line-draw or decal mode
void CGLCamera::LineDrawMode(bool b)
{ 
	m_bdecal = b;
	if (m_bdecal)
		glPolygonOffset(0, 0);
	else
		glPolygonOffset(1, 1);
}

//-----------------------------------------------------------------------------
// This sets up the GL matrix transformation for rendering
void CGLCamera::PositionInScene()
{
	// reset the modelview matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// target in camera coordinates
	vec3d r = Target();
	
	// zoom-in a little when in decal mode
	if (m_bdecal)
		glPolygonOffset(0, 0);
	else
		glPolygonOffset(1, 1);

	// position the target in camera coordinates
	glx::translate(-r);

	// orient the camera
	glx::rotate(m_rot.Value());

	// translate to world coordinates
	glx::translate(-GetPosition());
}

//-----------------------------------------------------------------------------
void CGLCamera::Pan(const quatd& q)
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

void CGLCamera::PanView(const vec3d& r)
{
	double f = 0.001f * (double)GetFinalTargetDistance();
	Truck(r*f);
}

void CGLCamera::Dolly(double f)
{
	vec3d dr(0, 0, -f);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}

void CGLCamera::Truck(const vec3d& v)
{
	vec3d dr(v);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}

//-----------------------------------------------------------------------------
void CGLCamera::Orbit(quatd& q)
{
	quatd o = q*m_rot.Target();
	o.MakeUnit();
	m_rot.Target(o);
}

//-----------------------------------------------------------------------------
void CGLCamera::Zoom(double f)
{
	SetTargetDistance(GetFinalTargetDistance() * f);
}

//-----------------------------------------------------------------------------
void CGLCamera::SetTarget(const vec3d& r)
{
	m_pos.Target(r);
}

// set the target in local coordinates
void CGLCamera::SetLocalTarget(const vec3d& r)
{
	m_trg.Target(r);
}

void CGLCamera::SetViewDirection(const vec3d &r)
{
	if (r.Length() != 0.f)
	{
		m_rot.Target(quatd(vec3d(0, 0, 1.f), r).Inverse());
	}
}

void CGLCamera::SetTransform(GLCameraTransform& t)
{
	m_pos.Target(t.pos);
	m_trg.Target(t.trg);
	m_rot.Target(t.rot);
}

void CGLCamera::GetTransform(GLCameraTransform& t)
{
	t.pos = m_pos.Value();
	t.trg = m_trg.Value();
	t.rot = m_rot.Value();
}

//-----------------------------------------------------------------------------
vec3d CGLCamera::WorldToCam(vec3d r) const
{
	r -= Target();
	quatd q = m_rot.Value();
	q.RotateVector(r);
	r -= GetPosition();

	return r;
}

//-----------------------------------------------------------------------------
vec3d CGLCamera::CamToWorld(vec3d r) const
{
	r -= GetPosition();
	m_rot.Value().RotateVector(r);
	r -= Target();

	return r;
}

//-----------------------------------------------------------------------------
// get the position in global coordinates
vec3d CGLCamera::GlobalPosition() const
{
	vec3d r = Target();
	m_rot.Value().Inverse().RotateVector(r);
	r += GetPosition();
	return r;
}
