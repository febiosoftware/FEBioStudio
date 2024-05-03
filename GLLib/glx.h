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
#include <qopengl.h>
#ifdef __APPLE__
    #include <OpenGL/GLU.h>
#else
    #include <GL/glu.h>
#endif
#include <vector>
#include <FSCore/box.h>

namespace glx {

void translate(const vec3d& r);
void rotate(const quatd& q);

void drawLine(double x0, double y0, double x1, double y1, double a0, double a1, GLColor c, int n);
void drawCircle(double R, int N);
void drawCircle(const vec3d& c, double R, int N);
void drawCircle(const vec3d& c, const vec3d& normal, double R, int N);
void drawPoint(const vec3d& p);
void drawLine(const vec3d& a, const vec3d& b);
void drawLine(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB);

void line(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB);
void line(const vec3f& a, const vec3f& b, const GLColor& colA, const GLColor& colB);
void line(const vec3d& a, const vec3d& b);

void drawArc(const vec3d& c, double R, double w0, double w1, int N);
void drawHelix(const vec3d& a, const vec3d& b, double R, double p, int N);

void drawSphere(const vec3d& r, float R);
void drawHalfSphere(const vec3d& r0, float R, const vec3d& n0, float tex = 0.f);
void drawSmoothPath(const vec3d& r0, const vec3d& r1, float R, const vec3d& n0, const vec3d& n1, float t0 = 0.f, float t1 = 1.f, int nsegs = 16);
void drawSmoothPath(const std::vector<vec3d>& path, float R, float t0 = 0.f, float t1 = 1.f, int leftCap = 0, int rightCap = 0);
void drawCylinder(const vec3d& r0, const vec3d& r1, float R, float t0 = 0.f, float t1 = 1.f, int N = 16);
void drawCappedCylinder(const vec3d& r0, const vec3d& r1, float R, float t0 = 0.f, float t1 = 1.f, int N = 16, int leftCap = 0, int rightCap = 0);

vec3d interpolate(const vec3d& r0, const vec3d& r1, const vec3d& n0, const vec3d& n1, double t);

void quad4(vec3d r[4], vec3d n[4]);
void quad4(vec3d r[4], vec3d n[4], GLColor c[4]);
void quad4(vec3d r[4], vec3f n[4], float t[4]);
void quad8(vec3d r[8], vec3f n[8], float t[8]);
void quad9(vec3d r[9], vec3f n[9], float t[9]);

void tri3(vec3d r[3], vec3f n[3]);
void tri3(vec3d r[3], vec3d n[3]);
void tri3(vec3d r[3], vec3d n[3], GLColor c[3]);
void tri3(vec3d r[3], vec3f n[3], float t[3]);

void tri6(vec3d r[6], vec3f n[6], float t[6]);
void tri7(vec3d r[7], vec3f n[7], float t[7]);
void tri10(vec3d r[10], vec3f n[10], float t[10]);

// use inside GL_LINES
void lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3);
void lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3, const vec3d& r4);
void lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3, const vec3d& r4, const vec3d& r5, const vec3d& r6);
void lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3, const vec3d& r4, const vec3d& r5, const vec3d& r6, const vec3d& r7, const vec3d& r8);
void lineLoop(const vec3d r[9]);

void drawLine(double x0, double y0, double x1, double y1);
void drawLine(double x0, double y0, double z0, double x1, double y1, double z1);
void drawLine(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2);

void drawBox(double wx, double wy, double wz);

inline void vertex3d(const vec3d& r) { glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, double t) { glTexCoord1d(t); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3d& n) { glNormal3d(n.x, n.y, n.z); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3f& n) { glNormal3d(n.x, n.y, n.z); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3d& n, double t) { glNormal3d(n.x, n.y, n.z); glTexCoord1d(t); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3f& n, double t) { glNormal3d(n.x, n.y, n.z); glTexCoord1d(t); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3d& n, const GLColor& c) { glNormal3d(n.x, n.y, n.z); glColor3ub(c.r, c.g, c.b); glVertex3d(r.x, r.y, r.z); }

inline void glcolor(const GLColor& c) { glColor3ub(c.r, c.g, c.b); }

inline void line(const vec3f& a, const vec3f& b) { glVertex3fv(&a.x); glVertex3fv(&b.x); }

void smoothQUAD4(vec3d r[ 4], vec3f n[ 4], float t[ 4], int ndivs);
void smoothQUAD8(vec3d r[ 8], vec3f n[ 8], float t[ 8], int ndivs);
void smoothQUAD9(vec3d r[ 9], vec3f n[ 9], float t[ 9], int ndivs);
void smoothTRI6 (vec3d r[ 6], vec3f n[ 6], float t[ 6], int ndivs);
void smoothTRI7 (vec3d r[ 7], vec3f n[ 7], float t[ 7], int ndivs);
void smoothTRI10(vec3d r[10], vec3f n[10], float t[10], int ndivs);

void renderRigidBody(double R);
void renderJoint(double R);
void renderRevoluteJoint(double R);
void renderCylindricalJoint(double R);
void renderPlanarJoint(double R);
void renderPrismaticJoint(double R);
void renderRigidLock(double R);
void renderAxis(double R);
void renderAxes(double R, const vec3d& pos, const quatd& q, GLColor c);
void renderSpring(const vec3d& a, const vec3d& b, double R, int N = 25);
void renderDamper(const vec3d& a, const vec3d& b, double R);
void renderContractileForce(const vec3d& a, const vec3d& b, double R);

void renderBox(const BOX& bbox, bool partial = true, double scale = 1.0);
}
