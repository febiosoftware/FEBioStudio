// GLCamera.cpp: implementation of the CGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GLCamera.h"
#include "glx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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
	m_rot.Target(quatd(0, vec3d(1,0,0)));
	m_pos.Target(vec3d(0,0,0));
	m_trg.Target(vec3d(0,0,0));
	Update(true);

	m_bdecal = false;
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
// This sets up the GL matrix transformation for rendering
void CGLCamera::Transform()
{
	// reset the modelview matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// position the light
	GLfloat l1[] = {0.5f, 0.5f, 1.f, 0.f};
	glLightfv(GL_LIGHT0, GL_POSITION, l1);

//	GLfloat l2[] = {-0.5f, -0.3f, 0.f, 0.f};
//	glLightfv(GL_LIGHT1, GL_POSITION, l2);

	// target in camera coordinates
	vec3d r = Target();
	
	// zoom-in a little when in decal mode
	if (m_bdecal) r.z *= .999;

	// position the target in camera coordinates
	GLX::translate(-r);

	// orient the camera
	GLX::rotate(m_rot.Value());

	// translate to world coordinates
	GLX::translate(-Position());
}

//-----------------------------------------------------------------------------
void CGLCamera::SetTarget(vec3d r)
{
	m_pos.Target(r);
}

//-----------------------------------------------------------------------------
vec3d CGLCamera::WorldToCam(vec3d r) const
{
	r += Target();

	quatd q = m_rot.Value().Inverse();

	q.RotateVector(r);

	r += Position();

	return r;
}

//-----------------------------------------------------------------------------
vec3d CGLCamera::CamToWorld(vec3d r) const
{
	r -= Position();
	m_rot.Value().RotateVector(r);
	r -= Target();

	return r;
}

//-----------------------------------------------------------------------------
// get the position in global coordinates
vec3d CGLCamera::GlobalPosition() const
{
	return WorldToCam(vec3d(0, 0, 0));
}

//-----------------------------------------------------------------------------
void CGLCamera::Orbit(quatd& q)
{
	quatd o = q*m_rot.Target();
	o.MakeUnit();
	m_rot.Target(o);
}

//-----------------------------------------------------------------------------
void CGLCamera::Pan(quatd& q)
{
	Orbit(q);
	vec3d r0(0, 0, -FinalTargetDistance()), r1 = r0;
	q.RotateVector(r1);
	vec3d dr = r0 - r1;
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}	

//-----------------------------------------------------------------------------
void CGLCamera::Truck(vec3d& v)
{
	vec3d dr(v);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}

//-----------------------------------------------------------------------------
void CGLCamera::Zoom(double f)
{
	SetTargetDistance(FinalTargetDistance() * f);
}

//-----------------------------------------------------------------------------
void CGLCamera::Dolly(double f)
{
	vec3d dr(0, 0, -FinalTargetDistance()*f);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}
