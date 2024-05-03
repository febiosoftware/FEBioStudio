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
#include <qopengl.h>
#include "glx.h"

//-----------------------------------------------------------------------------
void glx::translate(const vec3d& r)
{
	glTranslated(r.x, r.y, r.z);
}

//-----------------------------------------------------------------------------
void glx::rotate(const quatd& q)
{
	double w = q.GetAngle();
	if (w != 0)
	{
		vec3d r = q.GetVector();
		if (r.Length() > 1e-6) glRotated(w * 180 / PI, r.x, r.y, r.z);
		else glRotated(w * 180 / PI, 1, 0, 0);
	}
}

//-----------------------------------------------------------------------------
void glx::drawLine(double x0, double y0, double x1, double y1, double a0, double a1, GLColor c, int n)
{
	double x, y;
	double f, g;
	double h1, h2, h3, a;
	glBegin(GL_LINE_STRIP);
	for (int i=0; i<=n; ++i)
	{
		f = ((double) i / (double) n);
		g = (1.0 - f);
		h1 = (g*(1.0 - 2.0*f));
		h2 = (f*(1.0 - 2.0*g));
		h3 = (4.0*f*g);

		a = (255.0*(h1*a0 + h2*a0 + h3*a1));
		if (a>255) a = 255;
		if (a < 0) a = 0;
		glColor4ub(c.r, c.g, c.b, (uchar) a);

		x = x0 + i*(x1 - x0)/n;
		y = y0 + i*(y1 - y0)/n;

		glVertex2d(x, y);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawCircle(double R, int N)
{
	double x, y;
	glBegin(GL_LINE_LOOP);
	{
		for (int i=0; i<N; ++i)
		{
			x = R*cos(i*2*PI/N);
			y = R*sin(i*2*PI/N);
			glVertex3d(x,y,0);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawCircle(const vec3d& c, double R, int N)
{
	double x, y;
	glBegin(GL_LINE_LOOP);
	{
		for (int i=0; i<N; ++i)
		{
			x = c.x + R*cos(i*2*PI/N);
			y = c.y + R*sin(i*2*PI/N);
			glVertex3d(x,y,c.z);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawCircle(const vec3d& c, const vec3d& normal, double R, int N)
{
	quatd q0(vec3d(0, 0, 1), normal);

	// special case when n0 == (0,0,-1)
	// In this case, the quat cannot be determined uniquely
	// so we need to specify it explicitly. 
	if (vec3d(0, 0, 1) * normal == -1)
	{
		q0 = quatd(vec3d(PI, 0, 0));
	}

	glBegin(GL_TRIANGLE_FAN);
	{
		glNormal3d(normal.x, normal.y, normal.z);
		glVertex3d(c.x, c.y, c.z);
		for (int i = 0; i <= N; ++i)
		{
			double x = R * cos(i * 2 * PI / N);
			double y = R * sin(i * 2 * PI / N);
			vec3d p = c + q0 * (vec3d(x, y, 0));
			glVertex3d(p.x, p.y, p.z);
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawPoint(const vec3d& r)
{
	glBegin(GL_POINTS);
	{
		glVertex3d(r.x, r.y, r.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawLine(const vec3d& a, const vec3d& b)
{
	glBegin(GL_LINES);
	{
		glVertex3d(a.x, a.y, a.z);
		glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawLine(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB)
{
	glBegin(GL_LINES);
	{
		glColor3ub(colA.r, colA.g, colA.b); glVertex3d(a.x, a.y, a.z);
		glColor3ub(colB.r, colB.g, colB.b); glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::line(const vec3d& a, const vec3d& b, const GLColor& colA, const GLColor& colB)
{
	glColor3ub(colA.r, colA.g, colA.b); glVertex3d(a.x, a.y, a.z);
	glColor3ub(colB.r, colB.g, colB.b); glVertex3d(b.x, b.y, b.z);
}

//-----------------------------------------------------------------------------
void glx::line(const vec3f& a, const vec3f& b, const GLColor& colA, const GLColor& colB)
{
	glColor3ub(colA.r, colA.g, colA.b); glVertex3f(a.x, a.y, a.z);
	glColor3ub(colB.r, colB.g, colB.b); glVertex3f(b.x, b.y, b.z);
}

//-----------------------------------------------------------------------------
void glx::line(const vec3d& a, const vec3d& b)
{
	glVertex3d(a.x, a.y, a.z);
	glVertex3d(b.x, b.y, b.z);
}

//-----------------------------------------------------------------------------
void glx::drawArc(const vec3d& c, double R, double w0, double w1, int N)
{
	glBegin(GL_LINE_STRIP);
	{
		for (int i=0; i<=N; ++i)
		{
			double w = w0 + i*(w1 - w0)/N;
			double x = c.x + R*cos(w);
			double y = c.y + R*sin(w);
			glVertex3d(x, y, c.z);
		}		
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawHelix(const vec3d& a, const vec3d& b, double R, double p, int N)
{
    vec3d c = b - a;
    double L = c.Length();
    
    if (L > 2*R) {
        c.Normalize();
        vec3d e = c ^ vec3d(1,0,0);
        if (e.Length() == 0) e = c ^ vec3d(0,1,0);
        e.Normalize();
        vec3d d = e ^ c; d.Normalize();
        double fp = (L - 2*R)/p;
        int n = fp*N;
        double dq = 2*PI/N;
        
        vec3d va = a + c*R;
        vec3d vb = b - c*R;
        vec3d x;
        glBegin(GL_LINE_STRIP);
        {
            glVertex3d(a.x,a.y,a.z);
            glVertex3d(va.x,va.y,va.z);
            for (int i=0; i<n; ++i)
            {
                x = va + (d*cos(i*dq) + e*sin(i*dq))*R + c*(((L - 2*R)*i)/n);
                glVertex3d(x.x,x.y,x.z);
            }
            glVertex3d(vb.x,vb.y,vb.z);
            glVertex3d(b.x,b.y,b.z);
        }
        glEnd();
    }
    else
        glx::drawLine(a, b);
}

//-----------------------------------------------------------------------------
void glx::drawSphere(const vec3d& r, float R)
{
	GLUquadricObj* pobj = gluNewQuadric();
	glPushMatrix();
	{
		glTranslated(r.x, r.y, r.z);
		gluSphere(pobj, R, 32, 32);
	}
	glPopMatrix();
	gluDeleteQuadric(pobj);
}

//-----------------------------------------------------------------------------
void glx::drawHalfSphere(const vec3d& r0, float R, const vec3d& n0, float tex)
{
	quatd q0(vec3d(0, 0, 1), n0);

	// special case when n0 == (0,0,-1)
	// In this case, the quat cannot be determined uniquely
	// so we need to specify it explicitly. 
	if (vec3d(0, 0, 1) * n0 == -1)
	{
		q0 = quatd(vec3d(PI, 0, 0));
	}

	const int M = 5;
	const int N = 16;
	for (int j = 0; j < M; ++j)
	{
		double th0 = 0.5 * PI * j / (double)M;
		double th1 = 0.5 * PI * (j + 1) / (double)M;
		double z1 = sin(th0);
		double z2 = sin(th1);

		double ct0 = cos(th0);
		double ct1 = cos(th1);

		double r1 = R * ct0;
		double r2 = R * ct1;

		if (j < M - 1)
		{
			glBegin(GL_QUAD_STRIP);
			for (int i = 0; i <= N; ++i)
			{
				double w = 2 * PI * i / (double)N;
				double x = cos(w);
				double y = sin(w);

				vec3d ri0(r1 * x, r1 * y, R * z1); q0.RotateVector(ri0);
				vec3d ri1(r2 * x, r2 * y, R * z2); q0.RotateVector(ri1);
				vec3d ra = r0 + ri0;
				vec3d rb = r0 + ri1;

				vec3d na(ct0*x, ct0*y, z1); q0.RotateVector(na);
				vec3d nb(ct1*x, ct1*y, z2); q0.RotateVector(nb);

				glTexCoord1d(tex); glNormal3d(nb.x, nb.y, nb.z); glVertex3d(rb.x, rb.y, rb.z);
				glTexCoord1d(tex); glNormal3d(na.x, na.y, na.z); glVertex3d(ra.x, ra.y, ra.z);
			}
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLE_FAN);
			{
				vec3d ri1(0, 0, R); q0.RotateVector(ri1);
				vec3d rb = r0 + ri1;
				vec3d nb(0, 0, R); q0.RotateVector(nb);
				glTexCoord1d(tex); glNormal3d(nb.x, nb.y, nb.z); glVertex3d(rb.x, rb.y, rb.z);

				for (int i = 0; i <= N; ++i)
				{
					double w = 2 * PI * i / (double)N;
					double x = cos(w);
					double y = sin(w);

					vec3d ri0(r1 * x, r1 * y, R * z1); q0.RotateVector(ri0);
					vec3d ra = r0 + ri0;
					vec3d na(ct0*x, ct0*y, z1); q0.RotateVector(na);
					glTexCoord1d(tex); glNormal3d(na.x, na.y, na.z); glVertex3d(ra.x, ra.y, ra.z);
				}
			}
			glEnd();
		}
	}
}

//-----------------------------------------------------------------------------
vec3d glx::interpolate(const vec3d& r0, const vec3d& r1, const vec3d& n0, const vec3d& n1, double t)
{
	double ax[4], ay[4], az[4];
	ax[0] = r0.x; ax[1] = n0.x; ax[2] = 3.0 * (r1.x - r0.x) - 2.0 * n0.x - n1.x; ax[3] = n1.x + n0.x - 2.0 * (r1.x - r0.x);
	ay[0] = r0.y; ay[1] = n0.y; ay[2] = 3.0 * (r1.y - r0.y) - 2.0 * n0.y - n1.y; ay[3] = n1.y + n0.y - 2.0 * (r1.y - r0.y);
	az[0] = r0.z; az[1] = n0.z; az[2] = 3.0 * (r1.z - r0.z) - 2.0 * n0.z - n1.z; az[3] = n1.z + n0.z - 2.0 * (r1.z - r0.z);

	vec3d r;
	r.x = ((ax[3] * t + ax[2]) * t + ax[1]) * t + ax[0];
	r.y = ((ay[3] * t + ay[2]) * t + ay[1]) * t + ay[0];
	r.z = ((az[3] * t + az[2]) * t + az[1]) * t + az[0];

	return r;
}

//-----------------------------------------------------------------------------
void glx::drawSmoothPath(const vec3d& r0, const vec3d& r1, float R, const vec3d& n0, const vec3d& n1, float t0, float t1, int nsegs)
{
	quatd q0(vec3d(0, 0, 1), n0);
	quatd q1(vec3d(0, 0, 1), n1);

	double L = (r1 - r0).Length();
	vec3d m0 = n0 * L;
	vec3d m1 = n1 * L;

	int M = nsegs;
	if (M < 2) M = 2;

	const int N = 16;
	for (int j = 0; j < M; ++j)
	{
		quatd qa = quatd::slerp(q0, q1, (double)j / M);
		quatd qb = quatd::slerp(q0, q1, (double)(j + 1) / M);

		vec3d rj0 = interpolate(r0, r1, m0, m1, (double)j / M);
		vec3d rj1 = interpolate(r0, r1, m0, m1, (double)(j + 1.0) / M);

		float ta = t0 + j * (t1 - t0) / M;
		float tb = t0 + (j + 1) * (t1 - t0) / M;

		glBegin(GL_QUAD_STRIP);
		for (int i = 0; i <= N; ++i)
		{
			double w = 2 * PI * i / (double)N;
			double x = cos(w);
			double y = sin(w);

			vec3d ri0(R * x, R * y, 0); qa.RotateVector(ri0);
			vec3d ri1(R * x, R * y, 0); qb.RotateVector(ri1);
			vec3d ra = rj0 + ri0;
			vec3d rb = rj1 + ri1;

			vec3d na(x, y, 0.0); qa.RotateVector(na);
			vec3d nb(x, y, 0.0); qb.RotateVector(nb);

			glTexCoord1d(ta); glNormal3d(nb.x, nb.y, nb.z); glVertex3d(rb.x, rb.y, rb.z);
			glTexCoord1d(tb); glNormal3d(na.x, na.y, na.z); glVertex3d(ra.x, ra.y, ra.z);
		}
		glEnd();
	}
}

void glx::drawSmoothPath(const std::vector<vec3d>& path, float R, float t0, float t1, int leftCap, int rightCap)
{
	int NP = (int)path.size();
	if (NP < 2) return;

	vec3d r0 = path[0];
	vec3d r1 = path[1];
	vec3d e1 = r1 - r0; e1.Normalize();
	vec3d r2;

	for (int i = 0; i < NP-1; ++i)
	{
		vec3d e2 = e1;
		r2 = r1;
		if (i < NP - 2)
		{
			r2 = path[i + 2];
			e2 = r2 - r0; e2.Normalize();
		}
		else {
			e2 = r1 - r0; e2.Normalize();
		}

		float tex0 = t0 + (i / (NP - 1.0) * (t1 - t0));
		float tex1 = t0 + ((i + 1) / (NP - 1.0) * (t1 - t0));

		// render cylinder
		glx::drawSmoothPath(r0, r1, R, e1, e2, tex0, tex1, 32);
		
		// render caps
		if (i == 0)
		{
			if (leftCap == 0)
				glx::drawHalfSphere(r0, R, -e1, tex0);
			else
			{
				glTexCoord1d(tex0);
				glx::drawCircle(r0, -e1, R, 16);
			}
		}
		if (i == NP - 2)
		{
			if (rightCap == 0)
				glx::drawHalfSphere(r1, R, e2, tex1);
			else
			{
				glTexCoord1d(tex1);
				glx::drawCircle(r1, e2, R, 16);
			}
		}

		// prep for next segment
		r0 = r1;
		r1 = r2;
		e1 = e2;
	}
}

void glx::drawCylinder(const vec3d& r0, const vec3d& r1, float R, float t0, float t1, int N)
{
	vec3d n = r1 - r0; n.Normalize();
	quatd q(vec3d(0, 0, 1), n);

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= N; ++i)
	{
		double w = 2 * PI * i / (double)N;
		double x = cos(w);
		double y = sin(w);

		vec3d ri0(R * x, R * y, 0); q.RotateVector(ri0);
		vec3d ri1(R * x, R * y, 0); q.RotateVector(ri1);
		vec3d ra = r0 + ri0;
		vec3d rb = r1 + ri1;

		vec3d na(x, y, 0.0); q.RotateVector(na);
		vec3d nb(x, y, 0.0); q.RotateVector(nb);

		glTexCoord1d(t1); glNormal3d(nb.x, nb.y, nb.z); glVertex3d(rb.x, rb.y, rb.z);
		glTexCoord1d(t0); glNormal3d(na.x, na.y, na.z); glVertex3d(ra.x, ra.y, ra.z);
	}
	glEnd();
}

void glx::drawCappedCylinder(const vec3d& r0, const vec3d& r1, float R, float t0, float t1, int N, int leftCap, int rightCap)
{
	vec3d n = r1 - r0; n.Normalize();

	// render cylinder
	glx::drawCylinder(r0, r1, R, t0, t1, N);

	// render caps
	if (leftCap == 0)
		glx::drawHalfSphere(r0, R, -n, t0);
	else
	{
		glTexCoord1d(t0);
		glx::drawCircle(r0, -n, R, 16);
	}

	if (rightCap == 0)
		glx::drawHalfSphere(r1, R, n, t1);
	else
	{
		glTexCoord1d(t1);
		glx::drawCircle(r1, n, R, 16);
	}
}

void glx::quad4(vec3d r[4], vec3d n[4])
{
	vertex3d(r[0], n[0]); vertex3d(r[1], n[1]); vertex3d(r[2], n[2]);
	vertex3d(r[2], n[2]); vertex3d(r[3], n[3]); vertex3d(r[0], n[0]);
}

void glx::quad4(vec3d r[4], vec3d n[4], GLColor c[4])
{
	vertex3d(r[0], n[0], c[0]); vertex3d(r[1], n[1], c[1]);	vertex3d(r[2], n[2], c[2]);
	vertex3d(r[2], n[2], c[2]);	vertex3d(r[3], n[3], c[3]);	vertex3d(r[0], n[0], c[0]);
}

void glx::quad4(vec3d r[4], vec3f n[4], float t[4])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[1], n[1], t[1]); vertex3d(r[2], n[2], t[2]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[3], n[3], t[3]); vertex3d(r[0], n[0], t[0]);
}

void glx::quad8(vec3d r[8], vec3f n[8], float t[8])
{
	vertex3d(r[7], n[7], t[7]); vertex3d(r[0], n[0], t[0]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[4], n[4], t[4]); vertex3d(r[1], n[1], t[1]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[5], n[5], t[5]); vertex3d(r[2], n[2], t[2]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[6], n[6], t[6]); vertex3d(r[3], n[3], t[3]); vertex3d(r[7], n[7], t[7]);
	vertex3d(r[7], n[7], t[7]); vertex3d(r[4], n[4], t[4]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[7], n[7], t[7]); vertex3d(r[5], n[5], t[5]); vertex3d(r[6], n[6], t[6]);
}

void glx::quad9(vec3d r[9], vec3f n[9], float t[9])
{
	const int T[8][3] = {
		{ 0,4,8 },{ 8,7,0 },{ 4,1,5 },{ 5,8,4 },
		{ 7,8,6 },{ 6,3,7 },{ 8,5,2 },{ 2,6,8 } };

	glNormal3f(n[T[0][0]].x, n[T[0][0]].y, n[T[0][0]].z); glTexCoord1f(t[T[0][0]]); glVertex3f(r[T[0][0]].x, r[T[0][0]].y, r[T[0][0]].z);
	glNormal3f(n[T[0][1]].x, n[T[0][1]].y, n[T[0][1]].z); glTexCoord1f(t[T[0][1]]); glVertex3f(r[T[0][1]].x, r[T[0][1]].y, r[T[0][1]].z);
	glNormal3f(n[T[0][2]].x, n[T[0][2]].y, n[T[0][2]].z); glTexCoord1f(t[T[0][2]]); glVertex3f(r[T[0][2]].x, r[T[0][2]].y, r[T[0][2]].z);

	glNormal3f(n[T[1][0]].x, n[T[1][0]].y, n[T[1][0]].z); glTexCoord1f(t[T[1][0]]); glVertex3f(r[T[1][0]].x, r[T[1][0]].y, r[T[1][0]].z);
	glNormal3f(n[T[1][1]].x, n[T[1][1]].y, n[T[1][1]].z); glTexCoord1f(t[T[1][1]]); glVertex3f(r[T[1][1]].x, r[T[1][1]].y, r[T[1][1]].z);
	glNormal3f(n[T[1][2]].x, n[T[1][2]].y, n[T[1][2]].z); glTexCoord1f(t[T[1][2]]); glVertex3f(r[T[1][2]].x, r[T[1][2]].y, r[T[1][2]].z);

	glNormal3f(n[T[2][0]].x, n[T[2][0]].y, n[T[2][0]].z); glTexCoord1f(t[T[2][0]]); glVertex3f(r[T[2][0]].x, r[T[2][0]].y, r[T[2][0]].z);
	glNormal3f(n[T[2][1]].x, n[T[2][1]].y, n[T[2][1]].z); glTexCoord1f(t[T[2][1]]); glVertex3f(r[T[2][1]].x, r[T[2][1]].y, r[T[2][1]].z);
	glNormal3f(n[T[2][2]].x, n[T[2][2]].y, n[T[2][2]].z); glTexCoord1f(t[T[2][2]]); glVertex3f(r[T[2][2]].x, r[T[2][2]].y, r[T[2][2]].z);

	glNormal3f(n[T[3][0]].x, n[T[3][0]].y, n[T[3][0]].z); glTexCoord1f(t[T[3][0]]); glVertex3f(r[T[3][0]].x, r[T[3][0]].y, r[T[3][0]].z);
	glNormal3f(n[T[3][1]].x, n[T[3][1]].y, n[T[3][1]].z); glTexCoord1f(t[T[3][1]]); glVertex3f(r[T[3][1]].x, r[T[3][1]].y, r[T[3][1]].z);
	glNormal3f(n[T[3][2]].x, n[T[3][2]].y, n[T[3][2]].z); glTexCoord1f(t[T[3][2]]); glVertex3f(r[T[3][2]].x, r[T[3][2]].y, r[T[3][2]].z);

	glNormal3f(n[T[4][0]].x, n[T[4][0]].y, n[T[4][0]].z); glTexCoord1f(t[T[4][0]]); glVertex3f(r[T[4][0]].x, r[T[4][0]].y, r[T[4][0]].z);
	glNormal3f(n[T[4][1]].x, n[T[4][1]].y, n[T[4][1]].z); glTexCoord1f(t[T[4][1]]); glVertex3f(r[T[4][1]].x, r[T[4][1]].y, r[T[4][1]].z);
	glNormal3f(n[T[4][2]].x, n[T[4][2]].y, n[T[4][2]].z); glTexCoord1f(t[T[4][2]]); glVertex3f(r[T[4][2]].x, r[T[4][2]].y, r[T[4][2]].z);

	glNormal3f(n[T[5][0]].x, n[T[5][0]].y, n[T[5][0]].z); glTexCoord1f(t[T[5][0]]); glVertex3f(r[T[5][0]].x, r[T[5][0]].y, r[T[5][0]].z);
	glNormal3f(n[T[5][1]].x, n[T[5][1]].y, n[T[5][1]].z); glTexCoord1f(t[T[5][1]]); glVertex3f(r[T[5][1]].x, r[T[5][1]].y, r[T[5][1]].z);
	glNormal3f(n[T[5][2]].x, n[T[5][2]].y, n[T[5][2]].z); glTexCoord1f(t[T[5][2]]); glVertex3f(r[T[5][2]].x, r[T[5][2]].y, r[T[5][2]].z);

	glNormal3f(n[T[6][0]].x, n[T[6][0]].y, n[T[6][0]].z); glTexCoord1f(t[T[6][0]]); glVertex3f(r[T[6][0]].x, r[T[6][0]].y, r[T[6][0]].z);
	glNormal3f(n[T[6][1]].x, n[T[6][1]].y, n[T[6][1]].z); glTexCoord1f(t[T[6][1]]); glVertex3f(r[T[6][1]].x, r[T[6][1]].y, r[T[6][1]].z);
	glNormal3f(n[T[6][2]].x, n[T[6][2]].y, n[T[6][2]].z); glTexCoord1f(t[T[6][2]]); glVertex3f(r[T[6][2]].x, r[T[6][2]].y, r[T[6][2]].z);

	glNormal3f(n[T[7][0]].x, n[T[7][0]].y, n[T[7][0]].z); glTexCoord1f(t[T[7][0]]); glVertex3f(r[T[7][0]].x, r[T[7][0]].y, r[T[7][0]].z);
	glNormal3f(n[T[7][1]].x, n[T[7][1]].y, n[T[7][1]].z); glTexCoord1f(t[T[7][1]]); glVertex3f(r[T[7][1]].x, r[T[7][1]].y, r[T[7][1]].z);
	glNormal3f(n[T[7][2]].x, n[T[7][2]].y, n[T[7][2]].z); glTexCoord1f(t[T[7][2]]); glVertex3f(r[T[7][2]].x, r[T[7][2]].y, r[T[7][2]].z);
}

void glx::tri3(vec3d r[3], vec3f n[3])
{
	vertex3d(r[0], n[0]);
	vertex3d(r[1], n[1]);
	vertex3d(r[2], n[2]);
}

void glx::tri3(vec3d r[3], vec3d n[3])
{
	vertex3d(r[0], n[0]);
	vertex3d(r[1], n[1]);
	vertex3d(r[2], n[2]);
}

void glx::tri3(vec3d r[3], vec3d n[3], GLColor c[3])
{
	vertex3d(r[0], n[0], c[0]);
	vertex3d(r[1], n[1], c[1]);
	vertex3d(r[2], n[2], c[2]);
}

void glx::tri3(vec3d r[3], vec3f n[3], float t[3])
{
	vertex3d(r[0], n[0], t[0]);
	vertex3d(r[1], n[1], t[1]);
	vertex3d(r[2], n[2], t[2]);
}

void glx::tri6(vec3d r[6], vec3f n[6], float t[6])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[3], n[3], t[3]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[4], n[4], t[4]); vertex3d(r[3], n[3], t[3]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[5], n[5], t[5]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[3], n[3], t[3]); vertex3d(r[4], n[4], t[4]); vertex3d(r[5], n[5], t[5]);
}

void glx::tri7(vec3d r[7], vec3f n[7], float t[7])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[3], n[3], t[3]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[6], n[6], t[6]); vertex3d(r[3], n[3], t[3]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[4], n[4], t[4]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[6], n[6], t[6]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[5], n[5], t[5]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[0], n[0], t[0]); vertex3d(r[6], n[6], t[6]); vertex3d(r[5], n[5], t[5]);
}

void glx::tri10(vec3d r[10], vec3f n[10], float t[10])
{
	vertex3d(r[0], n[0], t[0]); vertex3d(r[3], n[3], t[3]); vertex3d(r[7], n[7], t[7]);
	vertex3d(r[1], n[1], t[1]); vertex3d(r[5], n[5], t[5]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[2], n[2], t[2]); vertex3d(r[8], n[8], t[8]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[7], n[7], t[7]); vertex3d(r[3], n[3], t[3]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[3], n[3], t[3]); vertex3d(r[4], n[4], t[4]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[4], n[4], t[4]); vertex3d(r[5], n[5], t[5]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[5], n[5], t[5]); vertex3d(r[6], n[6], t[6]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[6], n[6], t[6]); vertex3d(r[8], n[8], t[8]);
	vertex3d(r[9], n[9], t[9]); vertex3d(r[8], n[8], t[8]); vertex3d(r[7], n[7], t[7]);
}

void glx::lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3)
{
	vertex3d(r1); vertex3d(r2);
	vertex3d(r2); vertex3d(r3);
	vertex3d(r3); vertex3d(r1);
}

void glx::lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3, const vec3d& r4)
{
	vertex3d(r1); vertex3d(r2);
	vertex3d(r2); vertex3d(r3);
	vertex3d(r3); vertex3d(r4);
	vertex3d(r4); vertex3d(r1);
}

void glx::lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3, const vec3d& r4, const vec3d& r5, const vec3d& r6)
{
	vertex3d(r1); vertex3d(r2);
	vertex3d(r2); vertex3d(r3);
	vertex3d(r3); vertex3d(r4);
	vertex3d(r4); vertex3d(r5);
	vertex3d(r5); vertex3d(r6);
	vertex3d(r6); vertex3d(r1);
}

void glx::lineLoop(const vec3d& r1, const vec3d& r2, const vec3d& r3, const vec3d& r4, const vec3d& r5, const vec3d& r6, const vec3d& r7, const vec3d& r8)
{
	vertex3d(r1); vertex3d(r2);
	vertex3d(r2); vertex3d(r3);
	vertex3d(r3); vertex3d(r4);
	vertex3d(r4); vertex3d(r5);
	vertex3d(r5); vertex3d(r6);
	vertex3d(r6); vertex3d(r7);
	vertex3d(r7); vertex3d(r8);
	vertex3d(r8); vertex3d(r1);
}

void glx::lineLoop(const vec3d r[9])
{
	vertex3d(r[0]); vertex3d(r[1]);
	vertex3d(r[1]); vertex3d(r[2]);
	vertex3d(r[2]); vertex3d(r[3]);
	vertex3d(r[3]); vertex3d(r[4]);
	vertex3d(r[4]); vertex3d(r[5]);
	vertex3d(r[5]); vertex3d(r[6]);
	vertex3d(r[6]); vertex3d(r[7]);
	vertex3d(r[7]); vertex3d(r[8]);
	vertex3d(r[8]); vertex3d(r[0]);
}


void glx::drawLine(double x0, double y0, double x1, double y1)
{
	glBegin(GL_LINES);
	{
		glVertex2d(x0, y0);
		glVertex2d(x1, y1);
	}
	glEnd();
}

void glx::drawLine(double x0, double y0, double z0, double x1, double y1, double z1)
{
	glBegin(GL_LINES);
	{
		glVertex3d(x0, y0, z0);
		glVertex3d(x1, y1, z1);
	}
	glEnd();
}

void glx::drawLine(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2)
{
	glBegin(GL_LINE_STRIP);
	{
		glVertex3d(x0, y0, z0);
		glVertex3d(x1, y1, z1);
		glVertex3d(x2, y2, z2);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
void glx::drawBox(double wx, double wy, double wz)
{
	glBegin(GL_QUADS);
	{
		glNormal3d(1, 0, 0);
		glVertex3d(wx, -wy, -wz);
		glVertex3d(wx,  wy, -wz);
		glVertex3d(wx,  wy,  wz);
		glVertex3d(wx, -wy,  wz);

		glNormal3d(-1, 0, 0);
		glVertex3d(-wx,  wy, -wz);
		glVertex3d(-wx, -wy, -wz);
		glVertex3d(-wx, -wy,  wz);
		glVertex3d(-wx,  wy,  wz);

		glNormal3d(0, 1, 0);
		glVertex3d( wx, wy, -wz);
		glVertex3d(-wx, wy, -wz);
		glVertex3d(-wx, wy,  wz);
		glVertex3d( wx, wy,  wz);

		glNormal3d(0, -1, 0);
		glVertex3d(-wx, -wy, -wz);
		glVertex3d( wx, -wy, -wz);
		glVertex3d( wx, -wy,  wz);
		glVertex3d(-wx, -wy,  wz);

		glNormal3d(0, 0, 1);
		glVertex3d(-wx,  wy, wz);
		glVertex3d( wx,  wy, wz);
		glVertex3d( wx, -wy, wz);
		glVertex3d(-wx, -wy, wz);

		glNormal3d(0, 0, -1);
		glVertex3d( wx,  wy, -wz);
		glVertex3d(-wx,  wy, -wz);
		glVertex3d(-wx, -wy, -wz);
		glVertex3d( wx, -wy, -wz);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 4-noded quadrilateral
void glx::smoothQUAD4(vec3d r[4], vec3f n[4], float t[4], int ndivs)
{
	const int T[2][3] = { { 0,1,2 },{ 2,3,0 } };
	float sa[4], ta[4], h[4];
	for (int i = 0; i<ndivs; ++i)
	{
		sa[0] = -1.f + i*2.f / ndivs;
		sa[1] = -1.f + (i + 1)*2.f / ndivs;
		sa[2] = -1.f + (i + 1)*2.f / ndivs;
		sa[3] = -1.f + i*2.f / ndivs;

		for (int j = 0; j<ndivs; ++j)
		{
			ta[0] = -1.f + j*2.f / ndivs;
			ta[1] = -1.f + j*2.f / ndivs;
			ta[2] = -1.f + (j + 1)*2.f / ndivs;
			ta[3] = -1.f + (j + 1)*2.f / ndivs;

			for (int l = 0; l < 2; ++l)
			{
				for (int k = 0; k < 3; ++k)
				{
					float sak = sa[T[l][k]];
					float tak = ta[T[l][k]];
					h[0] = 0.25f*(1 - sak)*(1 - tak);
					h[1] = 0.25f*(1 + sak)*(1 - tak);
					h[2] = 0.25f*(1 + sak)*(1 + tak);
					h[3] = 0.25f*(1 - sak)*(1 + tak);

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3];
					vec3d rk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(rk.x, rk.y, rk.z);
				}
			}
		}
	}
}

void glx::smoothQUAD8(vec3d r[8], vec3f n[8], float t[8], int ndivs)
{
	const int T[2][3] = { { 0,1,2 },{ 2,3,0 } };
	float sa[4], ta[4], h[8];
	for (int i = 0; i<ndivs; ++i)
	{
		sa[0] = -1.f + i*2.f / ndivs;
		sa[1] = -1.f + (i + 1)*2.f / ndivs;
		sa[2] = -1.f + (i + 1)*2.f / ndivs;
		sa[3] = -1.f + i*2.f / ndivs;

		for (int j = 0; j<ndivs; ++j)
		{
			ta[0] = -1.f + j*2.f / ndivs;
			ta[1] = -1.f + j*2.f / ndivs;
			ta[2] = -1.f + (j + 1)*2.f / ndivs;
			ta[3] = -1.f + (j + 1)*2.f / ndivs;

			for (int l = 0; l < 2; ++l)
			{
				for (int k = 0; k < 3; ++k)
				{
					float sak = sa[T[l][k]];
					float tak = ta[T[l][k]];
					h[4] = 0.5f*(1 - sak*sak)*(1 - tak);
					h[5] = 0.5f*(1 - tak*tak)*(1 + sak);
					h[6] = 0.5f*(1 - sak*sak)*(1 + tak);
					h[7] = 0.5f*(1 - tak*tak)*(1 - sak);

					h[0] = 0.25f*(1 - sak)*(1 - tak) - (h[4] + h[7])*0.5f;
					h[1] = 0.25f*(1 + sak)*(1 - tak) - (h[4] + h[5])*0.5f;
					h[2] = 0.25f*(1 + sak)*(1 + tak) - (h[5] + h[6])*0.5f;
					h[3] = 0.25f*(1 - sak)*(1 + tak) - (h[6] + h[7])*0.5f;

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7];
					vec3d rk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3d(rk.x, rk.y, rk.z);
				}
			}
		}
	}
}

void glx::smoothQUAD9(vec3d r[9], vec3f n[9], float t[9], int ndivs)
{
	const int T[2][3] = { { 0,1,2 },{ 2,3,0 } };
	float sa[4], ta[4], h[9], R[3], S[3];
	for (int i = 0; i<ndivs; ++i)
	{
		sa[0] = -1.f + i*2.f / ndivs;
		sa[1] = -1.f + (i + 1)*2.f / ndivs;
		sa[2] = -1.f + (i + 1)*2.f / ndivs;
		sa[3] = -1.f + i*2.f / ndivs;

		for (int j = 0; j<ndivs; ++j)
		{
			ta[0] = -1.f + j*2.f / ndivs;
			ta[1] = -1.f + j*2.f / ndivs;
			ta[2] = -1.f + (j + 1)*2.f / ndivs;
			ta[3] = -1.f + (j + 1)*2.f / ndivs;

			for (int l = 0; l < 2; ++l)
			{
				for (int k = 0; k < 3; ++k)
				{
					float sak = sa[T[l][k]];
					float tak = ta[T[l][k]];

					R[0] = 0.5f*sak*(sak - 1.f);
					R[1] = 0.5f*sak*(sak + 1.f);
					R[2] = 1.0f - sak*sak;

					S[0] = 0.5f*tak*(tak - 1.f);
					S[1] = 0.5f*tak*(tak + 1.f);
					S[2] = 1.0f - tak*tak;

					h[0] = R[0] * S[0];
					h[1] = R[1] * S[0];
					h[2] = R[1] * S[1];
					h[3] = R[0] * S[1];
					h[4] = R[2] * S[0];
					h[5] = R[1] * S[2];
					h[6] = R[2] * S[1];
					h[7] = R[0] * S[2];
					h[8] = R[2] * S[2];

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8];
					vec3d rk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7] + r[8] * h[8];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7] + t[8] * h[8];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(rk.x, rk.y, rk.z);
				}
			}
		}
	}
}

void glx::smoothTRI6(vec3d r[6], vec3f n[6], float t[6], int ndivs)
{
	float sa[2], ta[2], h[8];
	int nj = ndivs;

	int sl[2][3] = { { 0,1,0 },{ 1,1,0 } };
	int tl[2][3] = { { 0,0,1 },{ 0,1,1 } };

	for (int i = 0; i<ndivs; ++i)
	{
		ta[0] = (float)i / ndivs;
		ta[1] = (float)(i + 1) / ndivs;

		for (int j = 0; j<nj; ++j)
		{
			sa[0] = (float)j / ndivs;
			sa[1] = (float)(j + 1) / ndivs;

			for (int k = 0; k<3; ++k)
			{
				float sak = sa[sl[0][k]];
				float tak = ta[tl[0][k]];
				float rak = 1.f - sak - tak;

				h[0] = rak*(2.f*rak - 1.f);
				h[1] = sak*(2.f*sak - 1.f);
				h[2] = tak*(2.f*tak - 1.f);
				h[3] = 4.f*rak*sak;
				h[4] = 4.f*sak*tak;
				h[5] = 4.f*rak*tak;

				vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5];
				vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5];
				float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5];

				glNormal3f(nk.x, nk.y, nk.z);
				glTexCoord1f(tk);
				glVertex3f(xk.x, xk.y, xk.z);
			}

			if (j != nj - 1)
			{
				for (int k = 0; k<3; ++k)
				{
					float sak = sa[sl[1][k]];
					float tak = ta[tl[1][k]];
					float rak = 1.f - sak - tak;

					h[0] = rak*(2.f*rak - 1.f);
					h[1] = sak*(2.f*sak - 1.f);
					h[2] = tak*(2.f*tak - 1.f);
					h[3] = 4.f*rak*sak;
					h[4] = 4.f*sak*tak;
					h[5] = 4.f*rak*tak;

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5];
					vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(xk.x, xk.y, xk.z);
				}
			}
		}
		nj -= 1;
	}
}

void glx::smoothTRI7(vec3d r[7], vec3f n[7], float t[7], int ndivs)
{
	int sl[2][3] = { { 0,1,0 },{ 1,1,0 } };
	int tl[2][3] = { { 0,0,1 },{ 0,1,1 } };

	float sa[2], ta[2], h[7];
	int nj = ndivs;

	for (int i = 0; i<ndivs; ++i)
	{
		ta[0] = (float)i / ndivs;
		ta[1] = (float)(i + 1) / ndivs;

		for (int j = 0; j<nj; ++j)
		{
			sa[0] = (float)j / ndivs;
			sa[1] = (float)(j + 1) / ndivs;

			for (int k = 0; k<3; ++k)
			{
				float sak = sa[sl[0][k]];
				float tak = ta[tl[0][k]];
				float rak = 1.f - sak - tak;

				h[6] = 27.f*rak*sak*tak;
				h[0] = rak*(2.f*rak - 1.f) + h[6] / 9.f;
				h[1] = sak*(2.f*sak - 1.f) + h[6] / 9.f;
				h[2] = tak*(2.f*tak - 1.f) + h[6] / 9.f;
				h[3] = 4.f*rak*sak - 4.f*h[6] / 9.f;
				h[4] = 4.f*sak*tak - 4.f*h[6] / 9.f;
				h[5] = 4.f*tak*rak - 4.f*h[6] / 9.f;

				vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6];
				vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6];
				float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6];

				glNormal3f(nk.x, nk.y, nk.z);
				glTexCoord1f(tk);
				glVertex3f(xk.x, xk.y, xk.z);
			}

			if (j != nj - 1)
			{
				for (int k = 0; k<3; ++k)
				{
					float sak = sa[sl[1][k]];
					float tak = ta[tl[1][k]];
					float rak = 1.f - sak - tak;

					h[6] = 27.f*rak*sak*tak;
					h[0] = rak*(2.f*rak - 1.f) + h[6] / 9.f;
					h[1] = sak*(2.f*sak - 1.f) + h[6] / 9.f;
					h[2] = tak*(2.f*tak - 1.f) + h[6] / 9.f;
					h[3] = 4.f*rak*sak - 4.f*h[6] / 9.f;
					h[4] = 4.f*sak*tak - 4.f*h[6] / 9.f;
					h[5] = 4.f*tak*rak - 4.f*h[6] / 9.f;

					vec3f nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6];
					vec3d xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6];
					float tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(xk.x, xk.y, xk.z);
				}
			}
		}
		nj -= 1;
	}
}

void glx::smoothTRI10(vec3d r[10], vec3f n[10], float t[10], int ndivs)
{
	float sa[2], ta[2], tk, h[10];
	vec3f nk;
	vec3d xk;
	int i, j, k;
	int nj = ndivs;

	int sl[2][3] = { { 0, 1, 0 },{ 1, 1, 0 } };
	int tl[2][3] = { { 0, 0, 1 },{ 0, 1, 1 } };

	for (i = 0; i<ndivs; ++i)
	{
		ta[0] = (float)i / ndivs;
		ta[1] = (float)(i + 1) / ndivs;

		for (j = 0; j<nj; ++j)
		{
			sa[0] = (float)j / ndivs;
			sa[1] = (float)(j + 1) / ndivs;

			for (k = 0; k<3; ++k)
			{
				float sak = sa[sl[0][k]];
				float tak = ta[tl[0][k]];
				float rak = 1.f - sak - tak;

				h[0] = 0.5f*(3.f*rak - 1.f)*(3.f*rak - 2.f)*rak;
				h[1] = 0.5f*(3.f*sak - 1.f)*(3.f*sak - 2.f)*sak;
				h[2] = 0.5f*(3.f*tak - 1.f)*(3.f*tak - 2.f)*tak;
				h[3] = 9.f / 2.f*(3.f*rak - 1.f)*rak*sak;
				h[4] = 9.f / 2.f*(3.f*sak - 1.f)*rak*sak;
				h[5] = 9.f / 2.f*(3.f*sak - 1.f)*sak*tak;
				h[6] = 9.f / 2.f*(3.f*tak - 1.f)*sak*tak;
				h[7] = 9.f / 2.f*(3.f*rak - 1.f)*rak*tak;
				h[8] = 9.f / 2.f*(3.f*tak - 1.f)*rak*tak;
				h[9] = 27.f*rak*sak*tak;

				nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8] + n[9] * h[9];
				xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7] + r[8] * h[8] + r[9] * h[9];
				tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7] + t[8] * h[8] + t[9] * h[9];

				glNormal3f(nk.x, nk.y, nk.z);
				glTexCoord1f(tk);
				glVertex3f(xk.x, xk.y, xk.z);
			}

			if (j != nj - 1)
			{
				for (k = 0; k<3; ++k)
				{
					float sak = sa[sl[1][k]];
					float tak = ta[tl[1][k]];
					float rak = 1.f - sak - tak;

					h[0] = 0.5f*(3.f*rak - 1.f)*(3.f*rak - 2.f)*rak;
					h[1] = 0.5f*(3.f*sak - 1.f)*(3.f*sak - 2.f)*sak;
					h[2] = 0.5f*(3.f*tak - 1.f)*(3.f*tak - 2.f)*tak;
					h[3] = 9.f / 2.f*(3.f*rak - 1.f)*rak*sak;
					h[4] = 9.f / 2.f*(3.f*sak - 1.f)*rak*sak;
					h[5] = 9.f / 2.f*(3.f*sak - 1.f)*sak*tak;
					h[6] = 9.f / 2.f*(3.f*tak - 1.f)*sak*tak;
					h[7] = 9.f / 2.f*(3.f*rak - 1.f)*rak*tak;
					h[8] = 9.f / 2.f*(3.f*tak - 1.f)*rak*tak;
					h[9] = 27.f*rak*sak*tak;

					nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8] + n[9] * h[9];
					xk = r[0] * h[0] + r[1] * h[1] + r[2] * h[2] + r[3] * h[3] + r[4] * h[4] + r[5] * h[5] + r[6] * h[6] + r[7] * h[7] + r[8] * h[8] + r[9] * h[9];
					tk = t[0] * h[0] + t[1] * h[1] + t[2] * h[2] + t[3] * h[3] + t[4] * h[4] + t[5] * h[5] + t[6] * h[6] + t[7] * h[7] + t[8] * h[8] + t[9] * h[9];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(xk.x, xk.y, xk.z);
				}
			}
		}
		nj -= 1;
	}
}

void glx::renderRigidBody(double R)
{
	glBegin(GL_LINE_LOOP);
	{
		glVertex3d(R, 0, 0);
		glVertex3d(0, R, 0);
		glVertex3d(-R, 0, 0);
		glVertex3d(0, -R, 0);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	{
		glVertex3d(0, R, 0);
		glVertex3d(0, 0, R);
		glVertex3d(0, -R, 0);
		glVertex3d(0, 0, -R);
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	{
		glVertex3d(R, 0, 0);
		glVertex3d(0, 0, R);
		glVertex3d(-R, 0, 0);
		glVertex3d(0, 0, -R);
	}
	glEnd();

	glBegin(GL_LINES);
	{
		glColor3ub(255, 0, 0); glVertex3d(0, 0, 0); glVertex3d(R, 0, 0);
		glColor3ub(0, 255, 0); glVertex3d(0, 0, 0); glVertex3d(0, R, 0);
		glColor3ub(0, 0, 255); glVertex3d(0, 0, 0); glVertex3d(0, 0, R);
	}
	glEnd();
}

void glx::renderAxis(double R)
{
	glBegin(GL_LINES);
	{
		glColor3ub(255, 0, 0); glVertex3d(0, 0, 0); glVertex3d(R, 0, 0);
		glColor3ub(0, 255, 0); glVertex3d(0, 0, 0); glVertex3d(0, R, 0);
		glColor3ub(0, 0, 255); glVertex3d(0, 0, 0); glVertex3d(0, 0, R);
	}
	glEnd();
}

void glx::renderAxes(double R, const vec3d& pos, const quatd& q, GLColor c)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	vec3d e1 = q * vec3d(1, 0, 0);
	vec3d e2 = q * vec3d(0, 1, 0);
	vec3d e3 = q * vec3d(0, 0, 1);

	vec3d A, B, C;
	A = pos + e1 * R;
	B = pos + e2 * R;
	C = pos + e3 * R;

	glBegin(GL_POINTS);
	{
		glVertex3f(pos.x, pos.y, pos.z);
	}
	glEnd();

	glx::glcolor(c);
	glBegin(GL_LINES);
	{
		glVertex3f(pos.x, pos.y, pos.z); glVertex3f(A.x, A.y, A.z);
		glVertex3f(pos.x, pos.y, pos.z); glVertex3f(B.x, B.y, B.z);
		glVertex3f(pos.x, pos.y, pos.z); glVertex3f(C.x, C.y, C.z);
	}
	glEnd();

	glPopAttrib();
}

void glx::renderJoint(double R)
{
	glBegin(GL_LINES);
	{
		glVertex3d(-R, 0, 0); glVertex3d(+R, 0, 0);
		glVertex3d(0, -R, 0); glVertex3d(0, +R, 0);
		glVertex3d(0, 0, -R); glVertex3d(0, 0, +R);
	}
	glEnd();

	glx::drawCircle(R, 25);

	glRotatef(90.f, 1.f, 0.f, 0.f);
	glx::drawCircle(R, 25);
	glRotatef(-90.f, 1.f, 0.f, 0.f);

	glRotatef(90.f, 0.f, 1.f, 0.f);
	glx::drawCircle(R, 25);
	glRotatef(-90.f, 0.f, 1.f, 0.f);
}

void glx::renderRevoluteJoint(double R)
{
	// line along rotation axis
	glBegin(GL_LINES);
	{
		glVertex3d(0, 0, -R); glVertex3d(0, 0, +R);
	}
	glEnd();

	// little circle around origin, in transverse plane
	glx::drawCircle(R / 5, 25);

	// half-circle with arrow, in transverse plane
	glx::drawArc(vec3d(0, 0, 0), 2 * R / 3, 0, PI, 12);
	glBegin(GL_LINES);
	{
		glVertex3d(-2 * R / 3, 0, 0); glVertex3d(-13 * R / 15, +R / 5, 0);
		glVertex3d(-2 * R / 3, 0, 0); glVertex3d(-7 * R / 15, +R / 5, 0);
	}
	glEnd();
}

void glx::renderCylindricalJoint(double R)
{
	// line with arrow along rotation axis
	glBegin(GL_LINES);
	{
		glVertex3d(0, 0, -R); glVertex3d(0, 0, +R);
		glVertex3d(0, 0, +R); glVertex3d(+R / 5, 0, +4 * R / 5);
		glVertex3d(0, 0, +R); glVertex3d(-R / 5, 0, +4 * R / 5);
	}
	glEnd();

	// little circle around origin, in transverse plane
	glx::drawCircle(R / 5, 25);

	// half-circle with arrow, in transverse plane
	glx::drawArc(vec3d(0, 0, 0), 2 * R / 3, 0, PI, 12);
	glBegin(GL_LINES);
	{
		glVertex3d(-2 * R / 3, 0, 0); glVertex3d(-13 * R / 15, +R / 5, 0);
		glVertex3d(-2 * R / 3, 0, 0); glVertex3d(-7 * R / 15, +R / 5, 0);
	}
	glEnd();
}

void glx::renderPlanarJoint(double R)
{
	glBegin(GL_LINES);
	{
		// line along rotation axis
		glVertex3d(0, 0, -R); glVertex3d(0, 0, +R);

		// line with arrow along translation axis 1
		glVertex3d(-R, 0, 0); glVertex3d(+R, 0, 0);
		glVertex3d(+R, 0, 0); glVertex3d(+4 * R / 5, +R / 5, 0);
		glVertex3d(+R, 0, 0); glVertex3d(+4 * R / 5, -R / 5, 0);

		// line with arrow along translation axis 2
		glVertex3d(0, -R, 0); glVertex3d(0, +R, 0);
		glVertex3d(0, +R, 0); glVertex3d(+R / 5, +4 * R / 5, 0);
		glVertex3d(0, +R, 0); glVertex3d(-R / 5, +4 * R / 5, 0);

		// square in the center
		glVertex3d(-R / 3, +R / 3, 0); glVertex3d(+R / 3, +R / 3, 0);
		glVertex3d(-R / 3, -R / 3, 0); glVertex3d(+R / 3, -R / 3, 0);
		glVertex3d(-R / 3, +R / 3, 0); glVertex3d(-R / 3, -R / 3, 0);
		glVertex3d(+R / 3, +R / 3, 0); glVertex3d(+R / 3, -R / 3, 0);
	}
	glEnd();

	// little circle around origin, in transverse plane
	glx::drawCircle(R / 5, 25);

	// half-circle with arrow, in transverse plane
	glx::drawArc(vec3d(0, 0, 0), 2 * R / 3, 0, PI, 12);
	glBegin(GL_LINES);
	{
		glVertex3d(-2 * R / 3, 0, 0); glVertex3d(-13 * R / 15, +R / 5, 0);
		glVertex3d(-2 * R / 3, 0, 0); glVertex3d(-7 * R / 15, +R / 5, 0);
	}
	glEnd();
}

void glx::renderPrismaticJoint(double R)
{
	glBegin(GL_LINES);
	{
		// line with arrow along translation (x-)axis
		glVertex3d(-R, 0, 0); glVertex3d(+R, 0, 0);
		glVertex3d(+R, 0, 0); glVertex3d(+4 * R / 5, +R / 5, 0);
		glVertex3d(+R, 0, 0); glVertex3d(+4 * R / 5, -R / 5, 0);

		// parallel lines above and below
		glVertex3d(-3 * R / 4, +R / 2, 0); glVertex3d(+3 * R / 4, +R / 2, 0);
		glVertex3d(-3 * R / 4, -R / 2, 0); glVertex3d(+3 * R / 4, -R / 2, 0);

		// rectangle in the center
		glVertex3d(-R / 2, +R / 3, 0); glVertex3d(+R / 2, +R / 3, 0);
		glVertex3d(-R / 2, -R / 3, 0); glVertex3d(+R / 2, -R / 3, 0);
		glVertex3d(-R / 2, +R / 3, 0); glVertex3d(-R / 2, -R / 3, 0);
		glVertex3d(+R / 2, +R / 3, 0); glVertex3d(+R / 2, -R / 3, 0);
	}
	glEnd();

	// little circle around origin, in x-y plane
	glx::drawCircle(R / 5, 25);
}

void glx::renderRigidLock(double R)
{
	glBegin(GL_LINES);
	{
		// line along rotation axis
		glVertex3d(0, 0, -R); glVertex3d(0, 0, +R);

		// line with arrow along first axis
		glVertex3d(-R, 0, 0); glVertex3d(+R, 0, 0);
		glVertex3d(+R, 0, 0); glVertex3d(+4 * R / 5, +R / 5, 0);
		glVertex3d(+R, 0, 0); glVertex3d(+4 * R / 5, -R / 5, 0);

		// line with arrow along second axis
		glVertex3d(0, -R, 0); glVertex3d(0, +R, 0);
		glVertex3d(0, +R, 0); glVertex3d(+R / 5, +4 * R / 5, 0);
		glVertex3d(0, +R, 0); glVertex3d(-R / 5, +4 * R / 5, 0);
	}
	glEnd();

	// little circle around origin, in transverse plane
	glx::drawCircle(R / 5, 25);
}

void glx::renderSpring(const vec3d& a, const vec3d& b, double R, int N)
{
	double p = R / 2;
	double L = (b - a).norm();
	if (L != 0) p = L / N;
	glx::drawHelix(a, b, R / 2, p, 25);
}

void glx::renderDamper(const vec3d& a, const vec3d& b, double R)
{
	glx::drawLine(a, b);
}

void glx::renderContractileForce(const vec3d& a, const vec3d& b, double R)
{
	glx::drawLine(a, b);
}

inline void render_triad(double x, double y, double z, double dx, double dy, double dz)
{
	glVertex3d(x, y, z); glVertex3d(x + dx, y, z);
	glVertex3d(x, y, z); glVertex3d(x, y + dy, z);
	glVertex3d(x, y, z); glVertex3d(x, y, z + dz);
}

void glx::renderBox(const BOX& bbox, bool partial, double scale)
{
	// push attributes
	glPushAttrib(GL_ENABLE_BIT);

	// set attributes
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_LIGHTING);

	BOX box = bbox;
	box.Scale(scale);

	if (partial)
	{
		double dx = box.Width() * 0.3;
		double dy = box.Height() * 0.3;
		double dz = box.Depth() * 0.3;
		glBegin(GL_LINES);
		{
			render_triad(box.x0, box.y0, box.z0, dx, dy, dz);
			render_triad(box.x1, box.y0, box.z0, -dx, dy, dz);
			render_triad(box.x1, box.y1, box.z0, -dx, -dy, dz);
			render_triad(box.x0, box.y1, box.z0, dx, -dy, dz);

			render_triad(box.x0, box.y0, box.z1, dx, dy, -dz);
			render_triad(box.x1, box.y0, box.z1, -dx, dy, -dz);
			render_triad(box.x1, box.y1, box.z1, -dx, -dy, -dz);
			render_triad(box.x0, box.y1, box.z1, dx, -dy, -dz);
		}
		glEnd();
	}
	else
	{
		glBegin(GL_LINES);
		{
			glVertex3d(box.x0, box.y0, box.z0); glVertex3d(box.x1, box.y0, box.z0);
			glVertex3d(box.x1, box.y0, box.z0); glVertex3d(box.x1, box.y1, box.z0);
			glVertex3d(box.x1, box.y1, box.z0); glVertex3d(box.x0, box.y1, box.z0);
			glVertex3d(box.x0, box.y1, box.z0); glVertex3d(box.x0, box.y0, box.z0);

			glVertex3d(box.x0, box.y0, box.z1); glVertex3d(box.x1, box.y0, box.z1);
			glVertex3d(box.x1, box.y0, box.z1); glVertex3d(box.x1, box.y1, box.z1);
			glVertex3d(box.x1, box.y1, box.z1); glVertex3d(box.x0, box.y1, box.z1);
			glVertex3d(box.x0, box.y1, box.z1); glVertex3d(box.x0, box.y0, box.z1);

			glVertex3d(box.x0, box.y0, box.z0); glVertex3d(box.x0, box.y0, box.z1);
			glVertex3d(box.x1, box.y0, box.z0); glVertex3d(box.x1, box.y0, box.z1);
			glVertex3d(box.x0, box.y1, box.z0); glVertex3d(box.x0, box.y1, box.z1);
			glVertex3d(box.x1, box.y1, box.z0); glVertex3d(box.x1, box.y1, box.z1);
		}
		glEnd();
	}

	// restore attributes
	glPopAttrib();
}