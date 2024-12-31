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
#include "glx.h"
#include "GLMesh.h"
#include "GLRenderEngine.h"

void glx::drawLine(GLRenderEngine& re, double x0, double y0, double x1, double y1, double a0, double a1, GLColor c, int n)
{
	double x, y;
	double f, g;
	double h1, h2, h3, a;
	re.begin(GLRenderEngine::LINESTRIP);
	for (int i = 0; i <= n; ++i)
	{
		f = ((double)i / (double)n);
		g = (1.0 - f);
		h1 = (g * (1.0 - 2.0 * f));
		h2 = (f * (1.0 - 2.0 * g));
		h3 = (4.0 * f * g);

		a = (255.0 * (h1 * a0 + h2 * a0 + h3 * a1));
		if (a > 255) a = 255;
		if (a < 0) a = 0;
		c.a = a;
		re.setColor(c);

		x = x0 + i * (x1 - x0) / n;
		y = y0 + i * (y1 - y0) / n;

		re.vertex(vec3d(x, y,0));
	}
	re.end();
}

void glx::drawCircle(GLRenderEngine& re, double R, int N)
{
	double x, y;
	re.begin(GLRenderEngine::LINELOOP);
	{
		for (int i = 0; i < N; ++i)
		{
			x = R * cos(i * 2 * PI / N);
			y = R * sin(i * 2 * PI / N);
			re.vertex(vec3d(x, y, 0));
		}
	}
	re.end();
}

void glx::drawCircle(GLRenderEngine& re, const vec3d& c, double R, int N)
{
	double x, y;
	re.begin(GLRenderEngine::LINELOOP);
	{
		for (int i = 0; i < N; ++i)
		{
			x = c.x + R * cos(i * 2 * PI / N);
			y = c.y + R * sin(i * 2 * PI / N);
			re.vertex(vec3d(x, y, c.z));
		}
	}
	re.end();
}

void glx::drawCircle(GLRenderEngine& re, const vec3d& c, const vec3d& normal, double R, int N)
{
	quatd q0(vec3d(0, 0, 1), normal);

	// special case when n0 == (0,0,-1)
	// In this case, the quat cannot be determined uniquely
	// so we need to specify it explicitly. 
	if (vec3d(0, 0, 1) * normal == -1)
	{
		q0 = quatd(vec3d(PI, 0, 0));
	}

	re.begin(GLRenderEngine::TRIANGLEFAN);
	{
		re.normal(normal);
		re.vertex(c);
		for (int i = 0; i <= N; ++i)
		{
			double x = R * cos(i * 2 * PI / N);
			double y = R * sin(i * 2 * PI / N);
			vec3d p = c + q0 * (vec3d(x, y, 0));
			re.vertex(p);
		}
	}
	re.end();
}

void glx::drawArc(GLRenderEngine& re, const vec3d& c, double R, double w0, double w1, int N)
{
	re.begin(GLRenderEngine::LINESTRIP);
	{
		for (int i = 0; i <= N; ++i)
		{
			double w = w0 + i * (w1 - w0) / N;
			double x = c.x + R * cos(w);
			double y = c.y + R * sin(w);
			re.vertex(vec3d(x, y, c.z));
		}
	}
	re.end();
}

void glx::drawHelix(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R, double p, int N)
{
	vec3d c = b - a;
	double L = c.Length();

	if (L > 2 * R) {
		c.Normalize();
		vec3d e = c ^ vec3d(1, 0, 0);
		if (e.Length() == 0) e = c ^ vec3d(0, 1, 0);
		e.Normalize();
		vec3d d = e ^ c; d.Normalize();
		double fp = (L - 2 * R) / p;
		int n = fp * N;
		double dq = 2 * PI / N;

		vec3d va = a + c * R;
		vec3d vb = b - c * R;
		vec3d x;
		std::vector<vec3d> points;
		points.push_back(a);
		points.push_back(va);
		for (int i = 0; i < n; ++i)
		{
			x = va + (d * cos(i * dq) + e * sin(i * dq)) * R + c * (((L - 2 * R) * i) / n);
			points.push_back(x);
		}
		points.push_back(vb);
		points.push_back(b);

		re.renderPath(points);
	}
	else
		re.renderLine(a, b);
}

void glx::drawSphere(GLRenderEngine& re, const vec3d& c, float R)
{
	re.pushTransform();
	re.translate(c);
	drawSphere(re, R);
	re.popTransform();
}

void glx::drawSphere(GLRenderEngine& re, float R)
{
	const int M = 10;
	const int N = 16;
	for (int j = 0; j < M; ++j)
	{
		double th0 = -0.5 * PI + PI * j / (double)M;
		double th1 = -0.5 * PI + PI * (j + 1) / (double)M;

		double z1 = sin(th0);
		double z2 = sin(th1);

		double ct0 = cos(th0);
		double ct1 = cos(th1);

		double r1 = R * ct0;
		double r2 = R * ct1;

		if (j == 0)
		{
			re.begin(GLRenderEngine::TRIANGLEFAN);
			{
				vec3d ri1(0, 0, 0);
				vec3d rb = ri1;
				vec3d nb(0, 0, -1);
				re.normal(nb); re.vertex(rb);

				for (int i = 0; i <= N; ++i)
				{
					double w = 2 * PI * i / (double)N;
					double x = cos(w);
					double y = sin(w);

					vec3d ri0(r2 * x, r2 * y, R * z2);
					vec3d ra = ri0;
					vec3d na(ct1 * x, ct1 * y, z2);
					re.normal(na); re.vertex(ra);
				}
			}
			re.end();
		}
		else if (j == M - 1)
		{
			re.begin(GLRenderEngine::TRIANGLEFAN);
			{
				vec3d ri1(0, 0, R);
				vec3d rb = ri1;
				vec3d nb(0, 0, 1);
				re.normal(nb); re.vertex(rb);

				for (int i = 0; i <= N; ++i)
				{
					double w = 2 * PI * i / (double)N;
					double x = cos(w);
					double y = sin(w);

					vec3d ri0(r1 * x, r1 * y, R * z1);
					vec3d ra = ri0;
					vec3d na(ct0 * x, ct0 * y, z1);
					re.normal(na); re.vertex(ra);
				}
			}
			re.end();
		}
		else
		{
			re.begin(GLRenderEngine::QUADSTRIP);
			for (int i = 0; i <= N; ++i)
			{
				double w = 2 * PI * i / (double)N;
				double x = cos(w);
				double y = sin(w);

				vec3d ri0(r1 * x, r1 * y, R * z1);
				vec3d ri1(r2 * x, r2 * y, R * z2);
				vec3d ra = ri0;
				vec3d rb = ri1;

				vec3d na(ct0 * x, ct0 * y, z1);
				vec3d nb(ct1 * x, ct1 * y, z2);

				re.normal(nb); re.vertex(rb);
				re.normal(na); re.vertex(ra);
			}
			re.end();
		}
	}
}

void glx::drawHalfSphere(GLRenderEngine& re, float R)
{
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
			re.begin(GLRenderEngine::QUADSTRIP);
			for (int i = 0; i <= N; ++i)
			{
				double w = 2 * PI * i / (double)N;
				double x = cos(w);
				double y = sin(w);

				vec3d ri0(r1 * x, r1 * y, R * z1);
				vec3d ri1(r2 * x, r2 * y, R * z2);
				vec3d ra = ri0;
				vec3d rb = ri1;

				vec3d na(ct0 * x, ct0 * y, z1);
				vec3d nb(ct1 * x, ct1 * y, z2);

				re.normal(nb); re.vertex(rb);
				re.normal(na); re.vertex(ra);
			}
			re.end();
		}
		else
		{
			re.begin(GLRenderEngine::TRIANGLEFAN);
			{
				vec3d ri1(0, 0, R);
				vec3d rb = ri1;
				vec3d nb(0, 0, R);
				re.normal(nb); re.vertex(rb);

				for (int i = 0; i <= N; ++i)
				{
					double w = 2 * PI * i / (double)N;
					double x = cos(w);
					double y = sin(w);

					vec3d ri0(r1 * x, r1 * y, R * z1);
					vec3d ra = ri0;
					vec3d na(ct0 * x, ct0 * y, z1);
					re.normal(na); re.vertex(ra);
				}
			}
			re.end();
		}
	}
}

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

void glx::drawSmoothPath(GLRenderEngine& re, const vec3d& r0, const vec3d& r1, float R, const vec3d& n0, const vec3d& n1, float t0, float t1, int nsegs)
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

		re.begin(GLRenderEngine::QUADSTRIP);
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

			re.texCoord1d(ta); re.normal(nb); re.vertex(rb);
			re.texCoord1d(tb); re.normal(na); re.vertex(ra);
		}
		re.end();
	}
}

void glx::drawSmoothPath(GLRenderEngine& re, const std::vector<vec3d>& path, float R, float t0, float t1, int leftCap, int rightCap)
{
	int NP = (int)path.size();
	if (NP < 2) return;

	vec3d r0 = path[0];
	vec3d r1 = path[1];
	vec3d e1 = r1 - r0; e1.Normalize();
	vec3d r2;

	re.pushTransform();

	for (int i = 0; i < NP - 1; ++i)
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
		glx::drawSmoothPath(re, r0, r1, R, e1, e2, tex0, tex1, 32);

		// render caps
		if (i == 0)
		{
			re.translate(r0);
			re.rotate(-e1);
			re.texCoord1d(tex0);
			if (leftCap == 0)
			{
				glx::drawHalfSphere(re, R);
			}
			else
			{
				glx::drawDisk(re, R, 16);
			}
		}
		if (i == NP - 2)
		{
			re.translate(r1);
			re.rotate(e2);
			re.texCoord1d(tex1);
			if (rightCap == 0)
				glx::drawHalfSphere(re, R);
			else
			{
				glx::drawDisk(re, R, 16);
			}
		}

		// prep for next segment
		r0 = r1;
		r1 = r2;
		e1 = e2;
	}

	re.popTransform();
}

void glx::drawCylinder(GLRenderEngine& re, float R, float H, float t0, float t1, int N)
{
	re.begin(GLRenderEngine::QUADSTRIP);
	for (int i = 0; i <= N; ++i)
	{
		double w = 2 * PI * i / (double)N;
		double x = cos(w);
		double y = sin(w);

		vec3d ra(R * x, R * y, 0);
		vec3d rb(R * x, R * y, H);

		vec3d na(x, y, 0.0);
		vec3d nb(x, y, 0.0);

		re.texCoord1d(t1); re.normal(nb); re.vertex(rb);
		re.texCoord1d(t0); re.normal(na); re.vertex(ra);
	}
	re.end();
}

void glx::drawCylinder(GLRenderEngine& re, float R, float H, int N)
{
	re.begin(GLRenderEngine::QUADSTRIP);
	for (int i = 0; i <= N; ++i)
	{
		double w = 2 * PI * i / (double)N;
		double x = cos(w);
		double y = sin(w);

		vec3d ra(R * x, R * y, 0);
		vec3d rb(R * x, R * y, H);

		vec3d na(x, y, 0.0);
		vec3d nb(x, y, 0.0);

		re.normal(nb); re.vertex(rb);
		re.normal(na); re.vertex(ra);
	}
	re.end();
}

void glx::drawCappedCylinder(GLRenderEngine& re, float R, float H, float t0, float t1, int N, int leftCap, int rightCap)
{
	// render cylinder
	glx::drawCylinder(re, R, H, t0, t1, N);

	// render caps
	re.pushTransform();
	re.rotate(-vec3d(0, 0, 1));
	re.texCoord1d(t0);
	if (leftCap == 0)
	{
		glx::drawHalfSphere(re, R);
	}
	else
	{
		glx::drawDisk(re, R, 16);
	}
	re.popTransform();

	re.pushTransform();
	re.translate(vec3d(0, 0, H));
	re.texCoord1d(t1);
	if (rightCap == 0)
		glx::drawHalfSphere(re, R);
	else
	{
		glx::drawDisk(re, R, 16);
	}
	re.popTransform();
}

void glx::drawDisk(GLRenderEngine& re, float baseRadius, int N)
{
	re.begin(GLRenderEngine::TRIANGLEFAN);
	{
		re.normal(vec3d(0, 0, 1));
		re.vertex(vec3d(0, 0, 0));
		for (int i = 0; i <= N; ++i)
		{
			double x = baseRadius * cos(i * 2 * PI / N);
			double y = baseRadius * sin(i * 2 * PI / N);
			re.vertex(vec3d(x, y, 0));
		}
	}
	re.end();
}

void glx::drawCappedCylinder(GLRenderEngine& re, float radius, float height, int N)
{
	// render cylinder
	glx::drawCylinder(re, radius, height, N);

	// render caps
	glx::drawCircle(re, vec3d(0, 0, 0), vec3d(0, 0, -1), radius, 16);
	glx::drawCircle(re, vec3d(0, 0, height), vec3d(0, 0, 1), radius, 16);
}

void glx::drawCone(GLRenderEngine& re, float baseRadius, float height, int N)
{
	re.begin(GLRenderEngine::TRIANGLES);
	for (int i = 0; i <= N; ++i)
	{
		double wa = 2 * PI * i / (double)N;
		double wb = 2 * PI * (i + 1) / (double)N;
		double xa = baseRadius * cos(wa);
		double ya = baseRadius * sin(wa);
		double xb = baseRadius * cos(wb);
		double yb = baseRadius * sin(wb);

		re.vertex(vec3d(xa, ya, 0));
		re.vertex(vec3d(xb, yb, 0));
		re.vertex(vec3d(0, 0, height));
	}
	re.end();
}

void glx::drawBox(GLRenderEngine& re, double wx, double wy, double wz)
{
	re.begin(GLRenderEngine::QUADS);
	{
		re.normal(vec3d(1, 0, 0));
		re.vertex(vec3d(wx, -wy, -wz));
		re.vertex(vec3d(wx, wy, -wz));
		re.vertex(vec3d(wx, wy, wz));
		re.vertex(vec3d(wx, -wy, wz));

		re.normal(vec3d(-1, 0, 0));
		re.vertex(vec3d(-wx, wy, -wz));
		re.vertex(vec3d(-wx, -wy, -wz));
		re.vertex(vec3d(-wx, -wy, wz));
		re.vertex(vec3d(-wx, wy, wz));

		re.normal(vec3d(0, 1, 0));
		re.vertex(vec3d(wx, wy, -wz));
		re.vertex(vec3d(-wx, wy, -wz));
		re.vertex(vec3d(-wx, wy, wz));
		re.vertex(vec3d(wx, wy, wz));

		re.normal(vec3d(0, -1, 0));
		re.vertex(vec3d(-wx, -wy, -wz));
		re.vertex(vec3d(wx, -wy, -wz));
		re.vertex(vec3d(wx, -wy, wz));
		re.vertex(vec3d(-wx, -wy, wz));

		re.normal(vec3d(0, 0, 1));
		re.vertex(vec3d(-wx, wy, wz));
		re.vertex(vec3d(wx, wy, wz));
		re.vertex(vec3d(wx, -wy, wz));
		re.vertex(vec3d(-wx, -wy, wz));

		re.normal(vec3d(0, 0, -1));
		re.vertex(vec3d(wx, wy, -wz));
		re.vertex(vec3d(-wx, wy, -wz));
		re.vertex(vec3d(-wx, -wy, -wz));
		re.vertex(vec3d(wx, -wy, -wz));
	}
	re.end();
}

void glx::renderRigidBody(GLRenderEngine& re, double R)
{
	re.begin(GLRenderEngine::LINELOOP);
	{
		re.vertex(vec3d(R, 0, 0));
		re.vertex(vec3d(0, R, 0));
		re.vertex(vec3d(-R, 0, 0));
		re.vertex(vec3d(0, -R, 0));
	}
	re.end();

	re.begin(GLRenderEngine::LINELOOP);
	{
		re.vertex(vec3d(0, R, 0));
		re.vertex(vec3d(0, 0, R));
		re.vertex(vec3d(0, -R, 0));
		re.vertex(vec3d(0, 0, -R));
	}
	re.end();

	re.begin(GLRenderEngine::LINELOOP);
	{
		re.vertex(vec3d(R, 0, 0));
		re.vertex(vec3d(0, 0, R));
		re.vertex(vec3d(-R, 0, 0));
		re.vertex(vec3d(0, 0, -R));
	}
	re.end();

	re.begin(GLRenderEngine::LINES);
	{
		re.setColor(GLColor(255, 0, 0)); re.vertex(vec3d(0, 0, 0)); re.vertex(vec3d(R, 0, 0));
		re.setColor(GLColor(0, 255, 0)); re.vertex(vec3d(0, 0, 0)); re.vertex(vec3d(0, R, 0));
		re.setColor(GLColor(0, 0, 255)); re.vertex(vec3d(0, 0, 0)); re.vertex(vec3d(0, 0, R));
	}
	re.end();
}

void glx::renderAxis(GLRenderEngine& re, double R)
{
	re.begin(GLRenderEngine::LINES);
	{
		re.setColor(GLColor::Red()); re.vertex(vec3d(0, 0, 0)); re.vertex(vec3d(R, 0, 0));
		re.setColor(GLColor::Green()); re.vertex(vec3d(0, 0, 0)); re.vertex(vec3d(0, R, 0));
		re.setColor(GLColor::Blue()); re.vertex(vec3d(0, 0, 0)); re.vertex(vec3d(0, 0, R));
	}
	re.end();
}

void glx::renderAxes(GLRenderEngine& re, double R, const vec3d& pos, const quatd& q)
{
	vec3d e1 = q * vec3d(1, 0, 0);
	vec3d e2 = q * vec3d(0, 1, 0);
	vec3d e3 = q * vec3d(0, 0, 1);

	vec3d A, B, C;
	A = pos + e1 * R;
	B = pos + e2 * R;
	C = pos + e3 * R;

	re.begin(GLRenderEngine::POINTS);
	{
		re.vertex(pos);
	}
	re.end();

	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(pos); re.vertex(A);
		re.vertex(pos); re.vertex(B);
		re.vertex(pos); re.vertex(C);
	}
	re.end();
}

void glx::renderJoint(GLRenderEngine& re, double R)
{
	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(vec3d(-R, 0, 0)); re.vertex(vec3d(+R, 0, 0));
		re.vertex(vec3d(0, -R, 0)); re.vertex(vec3d(0, +R, 0));
		re.vertex(vec3d(0, 0, -R)); re.vertex(vec3d(0, 0, +R));
	}
	re.end();

	glx::drawCircle(re, R, 25);

	re.rotate(90, 1, 0, 0);
	glx::drawCircle(re, R, 25);
	re.rotate(-90, 1, 0, 0);

	re.rotate(90, 0, 1, 0);
	glx::drawCircle(re, R, 25);
	re.rotate(-90, 0, 1, 0);
}

void glx::renderRevoluteJoint(GLRenderEngine& re, double R)
{
	// line along rotation axis
	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(vec3d(0, 0, -R)); re.vertex(vec3d(0, 0, +R));
	}
	re.end();

	// little circle around origin, in transverse plane
	glx::drawCircle(re, R / 5, 25);

	// half-circle with arrow, in transverse plane
	glx::drawArc(re, vec3d(0, 0, 0), 2 * R / 3, 0, PI, 12);
	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(vec3d(-2 * R / 3, 0, 0)); re.vertex(vec3d(-13 * R / 15, +R / 5, 0));
		re.vertex(vec3d(-2 * R / 3, 0, 0)); re.vertex(vec3d(-7 * R / 15, +R / 5, 0));
	}
	re.end();
}

void glx::renderCylindricalJoint(GLRenderEngine& re, double R)
{
	// line with arrow along rotation axis
	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(vec3d(0, 0, -R)); re.vertex(vec3d(0, 0, +R));
		re.vertex(vec3d(0, 0, +R)); re.vertex(vec3d(+R / 5, 0, +4 * R / 5));
		re.vertex(vec3d(0, 0, +R)); re.vertex(vec3d(-R / 5, 0, +4 * R / 5));
	}
	re.end();

	// little circle around origin, in transverse plane
	glx::drawCircle(re, R / 5, 25);

	// half-circle with arrow, in transverse plane
	glx::drawArc(re, vec3d(0, 0, 0), 2 * R / 3, 0, PI, 12);
	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(vec3d(-2 * R / 3, 0, 0)); re.vertex(vec3d(-13 * R / 15, +R / 5, 0));
		re.vertex(vec3d(-2 * R / 3, 0, 0)); re.vertex(vec3d(-7 * R / 15, +R / 5, 0));
	}
	re.end();
}

void glx::renderPlanarJoint(GLRenderEngine& re, double R)
{
	re.begin(GLRenderEngine::LINES);
	{
		// line along rotation axis
		re.vertex(vec3d(0, 0, -R)); re.vertex(vec3d(0, 0, +R));

		// line with arrow along translation axis 1
		re.vertex(vec3d(-R, 0, 0)); re.vertex(vec3d(+R, 0, 0));
		re.vertex(vec3d(+R, 0, 0)); re.vertex(vec3d(+4 * R / 5, +R / 5, 0));
		re.vertex(vec3d(+R, 0, 0)); re.vertex(vec3d(+4 * R / 5, -R / 5, 0));

		// line with arrow along translation axis 2
		re.vertex(vec3d(0, -R, 0)); re.vertex(vec3d(0, +R, 0));
		re.vertex(vec3d(0, +R, 0)); re.vertex(vec3d(+R / 5, +4 * R / 5, 0));
		re.vertex(vec3d(0, +R, 0)); re.vertex(vec3d(-R / 5, +4 * R / 5, 0));

		// square in the center
		re.vertex(vec3d(-R / 3, +R / 3, 0)); re.vertex(vec3d(+R / 3, +R / 3, 0));
		re.vertex(vec3d(-R / 3, -R / 3, 0)); re.vertex(vec3d(+R / 3, -R / 3, 0));
		re.vertex(vec3d(-R / 3, +R / 3, 0)); re.vertex(vec3d(-R / 3, -R / 3, 0));
		re.vertex(vec3d(+R / 3, +R / 3, 0)); re.vertex(vec3d(+R / 3, -R / 3, 0));
	}
	re.end();

	// little circle around origin, in transverse plane
	glx::drawCircle(re, R / 5, 25);

	// half-circle with arrow, in transverse plane
	glx::drawArc(re, vec3d(0, 0, 0), 2 * R / 3, 0, PI, 12);
	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(vec3d(-2 * R / 3, 0, 0)); re.vertex(vec3d(-13 * R / 15, +R / 5, 0));
		re.vertex(vec3d(-2 * R / 3, 0, 0)); re.vertex(vec3d(-7 * R / 15, +R / 5, 0));
	}
	re.end();
}

void glx::renderPrismaticJoint(GLRenderEngine& re, double R)
{
	re.begin(GLRenderEngine::LINES);
	{
		// line with arrow along translation (x-)axis
		re.vertex(vec3d(-R, 0, 0)); re.vertex(vec3d(+R, 0, 0));
		re.vertex(vec3d(+R, 0, 0)); re.vertex(vec3d(+4 * R / 5, +R / 5, 0));
		re.vertex(vec3d(+R, 0, 0)); re.vertex(vec3d(+4 * R / 5, -R / 5, 0));

		// parallel lines above and below
		re.vertex(vec3d(-3 * R / 4, +R / 2, 0)); re.vertex(vec3d(+3 * R / 4, +R / 2, 0));
		re.vertex(vec3d(-3 * R / 4, -R / 2, 0)); re.vertex(vec3d(+3 * R / 4, -R / 2, 0));

		// rectangle in the center
		re.vertex(vec3d(-R / 2, +R / 3, 0)); re.vertex(vec3d(+R / 2, +R / 3, 0));
		re.vertex(vec3d(-R / 2, -R / 3, 0)); re.vertex(vec3d(+R / 2, -R / 3, 0));
		re.vertex(vec3d(-R / 2, +R / 3, 0)); re.vertex(vec3d(-R / 2, -R / 3, 0));
		re.vertex(vec3d(+R / 2, +R / 3, 0)); re.vertex(vec3d(+R / 2, -R / 3, 0));
	}
	re.end();

	// little circle around origin, in x-y plane
	glx::drawCircle(re, R / 5, 25);
}

void glx::renderRigidLock(GLRenderEngine& re, double R)
{
	re.begin(GLRenderEngine::LINES);
	{
		// line along rotation axis
		re.vertex(vec3d(0, 0, -R)); re.vertex(vec3d(0, 0, +R));

		// line with arrow along first axis
		re.vertex(vec3d(-R, 0, 0)); re.vertex(vec3d(+R, 0, 0));
		re.vertex(vec3d(+R, 0, 0)); re.vertex(vec3d(+4 * R / 5, +R / 5, 0));
		re.vertex(vec3d(+R, 0, 0)); re.vertex(vec3d(+4 * R / 5, -R / 5, 0));

		// line with arrow along second axis
		re.vertex(vec3d(0, -R, 0)); re.vertex(vec3d(0, +R, 0));
		re.vertex(vec3d(0, +R, 0)); re.vertex(vec3d(+R / 5, +4 * R / 5, 0));
		re.vertex(vec3d(0, +R, 0)); re.vertex(vec3d(-R / 5, +4 * R / 5, 0));
	}
	re.end();

	// little circle around origin, in transverse plane
	glx::drawCircle(re, R / 5, 25);
}

void glx::renderSpring(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R, int N)
{
	double p = R / 2;
	double L = (b - a).norm();
	if (L != 0) p = L / N;
	glx::drawHelix(re, a, b, R / 2, p, 25);
}

void glx::renderDamper(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R)
{
	re.renderLine(a, b);
}

void glx::renderContractileForce(GLRenderEngine& re, const vec3d& a, const vec3d& b, double R)
{
	re.renderLine(a, b);
}

void glx::renderRigidWall(GLRenderEngine& re, double R)
{
	re.setColor(GLColor(128, 96, 0, 96));
	re.renderRect(-R, -R, R, R);

	re.setColor(GLColor(164, 128, 0));
	re.begin(GLRenderEngine::LINELOOP);
	{
		re.vertex(vec3d(-R, -R, 0));
		re.vertex(vec3d(R, -R, 0));
		re.vertex(vec3d(R, R, 0));
		re.vertex(vec3d(-R, R, 0));
	}
	re.end();

	vec3d r[6];
	r[0] = vec3d(0, 0, 0); r[1] = vec3d(0, 0, R / 2);
	r[2] = vec3d(0, 0, R / 2); r[3] = vec3d(-R * 0.1, 0, R * 0.4);
	r[4] = vec3d(0, 0, R / 2); r[5] = vec3d(R * 0.1, 0, R * 0.4);
	re.begin(GLRenderEngine::LINES);
	{
		re.vertex(r[0]); re.vertex(r[1]);
		re.vertex(r[2]); re.vertex(r[3]);
		re.vertex(r[4]); re.vertex(r[5]);
	}
	re.end();
}

void glx::renderBox(GLRenderEngine& re, const BOX& bbox, GLColor col, bool partial, double scale)
{
	BOX box = bbox;
	box.Scale(scale);

	re.pushState();
	re.setMaterial(GLMaterial::CONSTANT, col);

	GLMesh mesh;

	if (partial)
	{
		double dx = box.Width() * 0.3;
		double dy = box.Height() * 0.3;
		double dz = box.Depth() * 0.3;

		float x, y, z;
		x = box.x0; y = box.y0, z = box.z0;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x + dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y + dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z + dz));

		x = box.x1; y = box.y0, z = box.z0;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x - dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y + dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z + dz));

		x = box.x1; y = box.y1, z = box.z0;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x - dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y - dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z + dz));

		x = box.x0; y = box.y1, z = box.z0;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x + dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y - dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z + dz));

		x = box.x0; y = box.y0, z = box.z1;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x + dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y + dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z - dz));

		x = box.x1; y = box.y0, z = box.z1;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x - dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y + dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z - dz));

		x = box.x1; y = box.y1, z = box.z1;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x - dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y - dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z - dz));

		x = box.x0; y = box.y1, z = box.z1;
		mesh.AddEdge(vec3f(x, y, z), vec3f(x + dx, y, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y - dy, z));
		mesh.AddEdge(vec3f(x, y, z), vec3f(x, y, z - dz));
	}
	else
	{
		mesh.AddEdge(vec3f(box.x0, box.y0, box.z0), vec3f(box.x1, box.y0, box.z0));
		mesh.AddEdge(vec3f(box.x1, box.y0, box.z0), vec3f(box.x1, box.y1, box.z0));
		mesh.AddEdge(vec3f(box.x1, box.y1, box.z0), vec3f(box.x0, box.y1, box.z0));
		mesh.AddEdge(vec3f(box.x0, box.y1, box.z0), vec3f(box.x0, box.y0, box.z0));

		mesh.AddEdge(vec3f(box.x0, box.y0, box.z1), vec3f(box.x1, box.y0, box.z1));
		mesh.AddEdge(vec3f(box.x1, box.y0, box.z1), vec3f(box.x1, box.y1, box.z1));
		mesh.AddEdge(vec3f(box.x1, box.y1, box.z1), vec3f(box.x0, box.y1, box.z1));
		mesh.AddEdge(vec3f(box.x0, box.y1, box.z1), vec3f(box.x0, box.y0, box.z1));

		mesh.AddEdge(vec3f(box.x0, box.y0, box.z0), vec3f(box.x0, box.y0, box.z1));
		mesh.AddEdge(vec3f(box.x1, box.y0, box.z0), vec3f(box.x1, box.y0, box.z1));
		mesh.AddEdge(vec3f(box.x0, box.y1, box.z0), vec3f(box.x0, box.y1, box.z1));
		mesh.AddEdge(vec3f(box.x1, box.y1, box.z0), vec3f(box.x1, box.y1, box.z1));
	}

	re.renderGMeshEdges(mesh, false);

	re.popState();
}


void glx::renderGlyph(GLRenderEngine& re, glx::GlyphType glyph, float scale, GLColor c)
{
	re.pushState();
	re.setMaterial(GLMaterial::CONSTANT, c);

	switch (glyph)
	{
	case GlyphType::RIGID_BODY: glx::renderRigidBody(re, scale); break;
	case GlyphType::RIGID_WALL: glx::renderRigidWall(re, scale); break;
	case GlyphType::RIGID_JOINT: glx::renderJoint(re, scale); break;
	case GlyphType::REVOLUTE_JOINT: glx::renderRevoluteJoint(re, scale); break;
	case GlyphType::PRISMATIC_JOINT: glx::renderPrismaticJoint(re, scale); break;
	case GlyphType::CYLINDRICAL_JOINT: glx::renderCylindricalJoint(re, scale); break;
	case GlyphType::PLANAR_JOINT: glx::renderPlanarJoint(re, scale); break;
	case GlyphType::RIGID_LOCK: glx::renderRigidLock(re, scale); break;
	}

	re.popState();
}
