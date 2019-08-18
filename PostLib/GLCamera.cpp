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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace Post;

//-----------------------------------------------------------------------------
void glxTranslate(const vec3f& r)
{
	glTranslated(r.x, r.y, r.z);
}

//-----------------------------------------------------------------------------
void glxRotate(const quat4f& q)
{
	double w = 180.0*q.GetAngle()/PI;
	if (w != 0)
	{
		vec3f r = q.GetVector();
		glRotatef((float)w, r.x, r.y, r.z);
	}
}

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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGLCamera::CGLCamera()
{
	Reset();
}

CGLCamera::~CGLCamera()
{

}

//////////////////////////////////////////////////////////////////////
// Reset camera position/orientation

void CGLCamera::Reset()
{
	SetCameraSpeed(0.8f);
	SetCameraBias(0.8f);
	m_rot.Target(quat4f(0, vec3f(1,0,0)));
	m_pos.Target(vec3f(0,0,0));
	m_trg.Target(vec3f(0,0,0));
	UpdatePosition(true);

	m_bdecal = false;
}

//-----------------------------------------------------------------------------
void CGLCamera::SetCameraSpeed(float f)
{
	if (f > 1.f) f = 1.f;
	if (f < 0.f) f = 0.f;
	m_speed = f;
	Interpolator::m_nsteps = 5 + (int)((1.f - f)*60.f);
}

void CGLCamera::SetCameraBias(float f)
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
void CGLCamera::UpdatePosition(bool bhit)
{
	if (bhit == false)
	{
		m_pos.Next();
		m_trg.Next();
		m_rot.Next();
	}
	else
	{
		m_pos.HitTarget();
		m_trg.HitTarget();
		m_rot.HitTarget();
	}
}

//////////////////////////////////////////////////////////////////////
// Position the camera in space

void CGLCamera::Transform()
{
	// reset the modelview Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	vec3f r = Target();

	// zoom-in a little when in decal mode
//	if (m_bdecal) r.z *= .999f;

	// move the camera back
	glxTranslate(-r);

	// orient the camera
	glxRotate(m_rot.Value());

	// translate to world coordinates
	glxTranslate(-GetPosition());
}

void CGLCamera::Pan(quat4f& q)
{

}	

void CGLCamera::Dolly(float f)
{

}

void CGLCamera::Truck(vec3f& r)
{
	vec3f dr(r);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(GetFinalPosition() + dr);
}

void CGLCamera::Orbit(quat4f& q)
{
	quat4f o = q*m_rot.Target();
	o.MakeUnit();
	m_rot.Target(o);
}

void CGLCamera::Zoom(float f)
{
	SetTargetDistance(GetFinalTargetDistance() * f);
}

void CGLCamera::SetTarget(const vec3f& r)
{
	m_pos.Target(r);
}

// set the target in local coordinates
void CGLCamera::SetLocalTarget(const vec3f& r)
{
	m_trg.Target(r);
}

void CGLCamera::SetViewDirection(const vec3f &r)
{
	if (r.Length() != 0.f)
	{
		m_rot.Target(quat4f(vec3f(0,0,1.f), r).Inverse());
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
