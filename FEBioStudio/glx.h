#pragma once
#include <PostLib/color.h>
#include <MathLib/mat3d.h>
#include <qopengl.h>
#ifdef __APPLE__
    #include <OpenGL/GLU.h>
#else
    #include <GL/glu.h>
#endif

namespace GLX {

void translate(const vec3d& r);
void rotate(const quatd& q);

void drawLine(double x0, double y0, double x1, double y1, double a0, double a1, GLColor c, int n);
void drawCircle(double R, int N);
void drawCircle(const vec3d& c, double R, int N);
void drawPoint(const vec3d& p);
void drawLine(const vec3d& a, const vec3d& b);
void drawLine(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB);
void drawLine_(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB);
void drawArc(const vec3d& c, double R, double w0, double w1, int N);
void drawHelix(const vec3d& a, const vec3d& b, double R, double p, int N);

void drawQuad(vec3d r[4], vec3d n[4], GLColor c);
void drawQuad(vec3d r[4], vec3d& n, GLColor& c);
void drawTriangle(vec3d r[3], vec3d n[3], GLColor c);
void drawTriangle(vec3d r[3], vec3d& n, GLColor& c);

void drawLine(double x0, double y0, double x1, double y1);
void drawLine(double x0, double y0, double z0, double x1, double y1, double z1);
void drawLine(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2);

inline void vertex(const vec3d& n, const vec3d& r)
{
	glNormal3d(n.x, n.y, n.z);
	glVertex3d(r.x, r.y, r.z);
}

inline void vertex(const vec3d& r)
{
	glVertex3d(r.x, r.y, r.z);
}
}
