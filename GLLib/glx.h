#pragma once
#include <FSCore/color.h>
#include <MathLib/mat3d.h>
#include <qopengl.h>
#ifdef __APPLE__
    #include <OpenGL/GLU.h>
#else
    #include <GL/glu.h>
#endif

namespace glx {

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

void quad4(vec3d r[4], vec3d n[4]);
void quad4(vec3d r[4], vec3f n[4], float t[4]);
void quad8(vec3d r[8], vec3f n[8], float t[8]);
void quad9(vec3d r[9], vec3f n[9], float t[9]);

void tri3(vec3d r[3], vec3f n[3]);
void tri3(vec3d r[3], vec3d n[3]);
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

inline void vertex3d(const vec3d& r) { glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, double t) { glTexCoord1d(t); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3d& n) { glNormal3d(n.x, n.y, n.z); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3d& n, double t) { glNormal3d(n.x, n.y, n.z); glTexCoord1d(t); glVertex3d(r.x, r.y, r.z); }
inline void vertex3d(const vec3d& r, const vec3d& n, const GLColor& c) { glNormal3d(n.x, n.y, n.z); glColor3ub(c.r, c.g, c.b); glVertex3d(r.x, r.y, r.z); }

void smoothQUAD4(vec3d r[ 4], vec3f n[ 4], float t[ 4], int ndivs);
void smoothQUAD8(vec3d r[ 8], vec3f n[ 8], float t[ 8], int ndivs);
void smoothQUAD9(vec3d r[ 9], vec3f n[ 9], float t[ 9], int ndivs);
void smoothTRI6 (vec3d r[ 6], vec3f n[ 6], float t[ 6], int ndivs);
void smoothTRI7 (vec3d r[ 7], vec3f n[ 7], float t[ 7], int ndivs);
void smoothTRI10(vec3d r[10], vec3f n[10], float t[10], int ndivs);

}
