#pragma once
#include <MathLib/math3d.h>
#include <MathLib/Interpolator.h>
#include "GLObject.h"

namespace Post {

//-----------------------------------------------------------------------------
class GLCameraTransform : public CGLObject
{
public:
	GLCameraTransform(){}
	GLCameraTransform(const GLCameraTransform& key);
	GLCameraTransform& operator = (const GLCameraTransform& key);

public:
	vec3d	pos;	// position
	vec3d	trg;	// target
	quatd	rot;	// rotation
};

//-----------------------------------------------------------------------------
//! Class that represents a camera.
//! It uses the interpolater class to allow animatable transistions between
//! two viewpoints.
class CGLCamera  
{
public:
	//! contructor
	CGLCamera();

	//! destructor
	virtual ~CGLCamera();

	//! reset the camera to its initial position/orientation
	void Reset();

	//! position the camera in space
	void Transform();

	// update camera position (for animations)
	void UpdatePosition(bool bhit = false);

	// set line-draw or decal mode
	void LineDrawMode(bool b) { m_bdecal = b; }

public:
	void SetCameraSpeed(double f);
	double GetCameraSpeed() { return m_speed; }

	void SetCameraBias(double f);
	double GetCameraBias() { return m_bias; }

	// --- camera movements ---
public:
	// pan the camera (rotate around camera center)
	void Pan(const quatd& q);

	// dolly the camera (move camera + target forward/backward)
	void Dolly(double f);

	// truck the camera (move camera in plane)
	void Truck(const vec3d& r);

	// orbit camera (rotate around target)
	void Orbit(const quatd& q);

	// zoom camera (move camera toward/away from target)
	void Zoom(double f);

public:
	// sets the distance to the target
	void SetTargetDistance(double z) { vec3d r = m_trg.Target(); r.z = z; m_trg.Target(r); }

	// gets the distance to the target
	double GetTargetDistance() { return m_trg.Value().z; }

	// gets the distance to the target
	double GetFinalTargetDistance() { return m_trg.Target().z; }

	// set the target + distance
	void SetTarget(const vec3d& r);

	// set the target in local coordinates
	void SetLocalTarget(const vec3d& r);

	// set the orientation of the camera
	void SetOrientation(const quatd& q) { m_rot.Target(q); }

	// get the camera orientation
	quatd GetOrientation() { return m_rot.Value(); }

	// get the target position
	vec3d GetPosition() { return m_pos.Value(); }

	vec3d GetFinalPosition() const {return m_pos.Target(); } 

	vec3d Target() const { return m_trg.Value(); }
	vec3d FinalTarget() const {return m_trg.Target(); } 

	// set the view direction
	void SetViewDirection(const vec3d& r);

	// set the camera transformation
	void SetTransform(GLCameraTransform& t);

	void GetTransform(GLCameraTransform& t);

	// see if the camera is still animating
	bool IsAnimating();

public:
	VecInterpolator		m_pos;	// position of target in global coordinates
	VecInterpolator		m_trg;	// position of target in local coordinates
	QuatInterpolator	m_rot;	// orientation of camera
	bool				m_bdecal;	//!< decal mode flag

private:
	double	m_speed;
	double	m_bias;
};
}
