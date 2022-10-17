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
#include <FECore/vec3d.h>
#include <FECore/quatd.h>
#include "Interpolator.h"
#include <PostLib/GLObject.h>
#include <string>

//-----------------------------------------------------------------------------
// TODO: We need to inherit this from CGLObject because it is used in the modeltree
// in PostView. Fix this.
class GLCameraTransform : public Post::CGLObject
{
public:
	GLCameraTransform() {}
	GLCameraTransform(const GLCameraTransform& key);
	GLCameraTransform& operator = (const GLCameraTransform& key);

public:
	vec3d		pos;	// position
	vec3d		trg;	// target
	quatd		rot;	// rotation
};

//=============================================================================
// This class implements a camera that can be used to navigate a 3D world.
// It uses the interpolater class to allow animatable transistions between
// two viewpoints.
class CGLCamera  
{
public:
	// constructor/destructor
	CGLCamera();

	//! destructor
	virtual ~CGLCamera();

	// reset the camera
	void Reset();

	// set the GL transformation matrix
	void Transform();

	// update camera position (for animations)
	void Update(bool bhit = false);

	// set line-draw or decal mode
	void LineDrawMode(bool b);

public:
	void SetCameraSpeed(double f);
	double GetCameraSpeed() { return m_speed; }

	void SetCameraBias(double f);
	double GetCameraBias() { return m_bias; }

public:
	// rotate aroun camera's axis
	void Pan(const quatd& q);

	// move camera forward or backward
	void Dolly(double f);

	// move camera in camera plane
	void Truck(vec3d& v);

	// rotate around target
	void Orbit(quatd& q);

	// zoom in or out
	void Zoom(double f);

public:
	// sets the distance to the target
	void SetTargetDistance(double z) { vec3d r = m_trg.Target(); r.z = z; m_trg.Target(r); }

	// gets the distance to the target
	double GetTargetDistance() { return m_trg.Value().z; }

	// gets the distance to the target
	double GetFinalTargetDistance() { return m_trg.Target().z; }

	// set the camera's target
	void SetTarget(const vec3d& r);

	// set the target in local coordinates
	void SetLocalTarget(const vec3d& r);

	// set the orientation of the camera
	void SetOrientation(quatd q) { m_rot.Target(q); }

	// get the camera's orientation
	quatd GetOrientation() { return m_rot.Value(); }

	// get the target position
	vec3d GetPosition() const { return m_pos.Value(); }

	vec3d FinalPosition() const { return m_pos.Target(); }

	vec3d Target() const { return m_trg.Value(); }
	vec3d FinalTarget() const { return m_trg.Target(); }

	// set the view direction
	void SetViewDirection(const vec3d& r);

	// set the camera transformation
	void SetTransform(GLCameraTransform& t);

	void GetTransform(GLCameraTransform& t);

	// see if the camera is still animating
	bool IsAnimating();

	// convert world coordinates to camera coordinates
	vec3d WorldToCam(vec3d r) const;

	// convert camera coordinates to world coordinates
	vec3d CamToWorld(vec3d r) const;

	// get the position in global coordinates
	vec3d GlobalPosition() const;

public:
	VecInterpolator		m_pos;	// position of target in global coordinates
	VecInterpolator		m_trg;	// position of target in local coordinates
	QuatInterpolator	m_rot;	// orientation of camera
	bool	m_bdecal;			// decal or line draw mode

private:
	double	m_speed;
	double	m_bias;

	double	m_depthScale;	// depth scale for line drawing
};
