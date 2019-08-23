#pragma once
#include <MathLib/math3d.h>
#include <MathLib/Interpolator.h>

//=============================================================================
// This class implements a camera that can be used to navigate a 3D world.
// It uses the interpolater class to allow animatable transistions between
// two viewpoints.
class CGLCamera  
{
public:
	// constructor/destructor
	CGLCamera();
	virtual ~CGLCamera();

	// reset the camera
	void Reset();

	// set the GL transformation matrix
	void Transform();

	// update camera position (for animations)
	void Update(bool bhit = false);

	// set line-draw or decal mode
	void LineDrawMode(bool b) { m_bdecal = b; }


public:
	// rotate around target
	void Orbit(quatd& q);

	// rotate aroun camera's axis
	void Pan(quatd& q);

	// move camera in camera plane
	void Truck(vec3d& v);

	// move camera forward or backward
	void Dolly(double f);

	// zoom in or out
	void Zoom(double f);

	// get the camera's orientation
	quatd GetOrientation() { return m_rot.Value(); }

	// set the camera's target
	void SetTarget(vec3d r);

	// set the orientation of the camera
	void SetOrientation(quatd q) { m_rot.Target(q); }

	// sets the distance to the target
	void SetTargetDistance(double z) { vec3d r = m_trg.Target(); r.z = z; m_trg.Target(r); }

	// gets the distance to the target
	double FinalTargetDistance() { return m_trg.Target().z; }

	// gets the distance to the target
	double TargetDistance() { return m_trg.Value().z; }

	// convert world coordinates to camera coordinates
	vec3d WorldToCam(vec3d r) const;

	// convert camera coordinates to world coordinates
	vec3d CamToWorld(vec3d r) const;

	// see if the camera is still animating
	bool IsAnimating();

	// get the position in global coordinates
	vec3d GlobalPosition() const;

public:
	vec3d Position() const { return m_pos.Value(); }
	vec3d FinalPosition() const {return m_pos.Target(); } 

	vec3d Target() const { return m_trg.Value(); }
	vec3d FinalTarget() const {return m_trg.Target(); } 

public:
	VecInterpolator		m_pos;	// position of target in global coordinates
	VecInterpolator		m_trg;	// position of target in local coordinates
	QuatInterpolator	m_rot;	// orientation of camera

	bool	m_bdecal;	// decal or line draw mode
};
