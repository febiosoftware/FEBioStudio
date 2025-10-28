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
#include <FSCore/color.h>
#include <FSCore/math3d.h>
#include <vector>
#include <FSCore/box.h>

class GLRenderEngine;

namespace glx {

	enum GlyphType {
		RIGID_BODY,
		RIGID_WALL,
		RIGID_JOINT,
		REVOLUTE_JOINT,
		PRISMATIC_JOINT,
		CYLINDRICAL_JOINT,
		PLANAR_JOINT,
		RIGID_LOCK,
		HELICAL_AXIS,
	};


void drawLine(GLRenderEngine& re, double x0, double y0, double x1, double y1, double a0, double a1, GLColor c, int n);
void drawPath2D(GLRenderEngine& re, const std::vector<std::pair<int, int>>& points);

void drawCircle(GLRenderEngine& re, double R, int N);
void drawCircle(GLRenderEngine& re, const vec3d& c, double R, int N);
void drawCircle(GLRenderEngine& re, const vec3d& c, const vec3d& normal, double R, int N);

void drawArc(GLRenderEngine& re, const vec3d& c, double R, double w0, double w1, int N);
void drawHelix(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R, double p, int N);

void drawSphere(GLRenderEngine& re, const vec3d& center, float radius);
void drawSphere(GLRenderEngine& re, float radius);
void drawHalfSphere(GLRenderEngine& re, float R);
void drawSmoothPath(GLRenderEngine& re, const vec3d& r0, const vec3d& r1, float R, const vec3d& n0, const vec3d& n1, float t0 = 0.f, float t1 = 1.f, int nsegs = 16);
void drawSmoothPath(GLRenderEngine& re, const std::vector<vec3d>& path, float R, float t0 = 0.f, float t1 = 1.f, int leftCap = 0, int rightCap = 0);
void drawCappedCylinder(GLRenderEngine& re, float radius, float height, float t0, float t1, int N = 16, int leftCap = 0, int rightCap = 0);
void drawCappedCylinder(GLRenderEngine& re, float radius, float height, int N = 16);

void drawDisk(GLRenderEngine& re, float baseRadius, int N = 16);
void drawCone(GLRenderEngine& re, float baseRadius, float height, int N = 16);
void drawCylinder(GLRenderEngine& re, float radius, float height, float t0, float t1, int N = 16);
void drawCylinder(GLRenderEngine& re, float radius, float height, int N = 16);

vec3d interpolate(const vec3d& r0, const vec3d& r1, const vec3d& n0, const vec3d& n1, double t);

void drawBox(GLRenderEngine& re, double wx, double wy, double wz);

void renderGlyph(GLRenderEngine& re, GlyphType glyph, float scale, GLColor c);
void renderRigidBody(GLRenderEngine& re, double R);
void renderJoint(GLRenderEngine& re, double R);
void renderRevoluteJoint(GLRenderEngine& re, double R);
void renderCylindricalJoint(GLRenderEngine& re, double R);
void renderPlanarJoint(GLRenderEngine& re, double R);
void renderPrismaticJoint(GLRenderEngine& re, double R);
void renderRigidLock(GLRenderEngine& re, double R);
void renderAxis(GLRenderEngine& re, double R);
void renderAxes(GLRenderEngine& re, double R, const vec3d& pos, const quatd& q);
void renderSpring(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R, int N = 25);
void renderDamper(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R);
void renderContractileForce(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R);
void renderRigidWall(GLRenderEngine& re, double R);
void renderHelicalAxis(GLRenderEngine& re, double R);

void renderBox(GLRenderEngine& re, const BOX& bbox, GLColor col = GLColor::White(), bool partial = true, double scale = 1.0);
}
