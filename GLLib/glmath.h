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
#include <FSCore/math3d.h>

struct vec4d
{
	vec4d() { d[0] = d[1] = d[2] = 0.0; d[3] = 1.0; }
	vec4d(double x, double y, double z, double w = 1) { d[0] = x; d[1] = y; d[2] = z; d[3] = w; }

	explicit vec4d(const vec3d& v, double w = 1)
	{
		d[0] = v.x;
		d[1] = v.y;
		d[2] = v.z;
		d[3] = w;
	}

	double x() const { return d[0]; }
	double y() const { return d[1]; }
	double z() const { return d[2]; }
	double w() const { return d[3]; }
	void x(double v) { d[0] = v; }
	void y(double v) { d[1] = v; }
	void z(double v) { d[2] = v; }
	void w(double v) { d[3] = v; }

	double& operator[] (size_t n) { return d[n]; }
	double operator[] (size_t n) const { return d[n]; }

	double d[4];
};

class mat4d
{
public:
	mat4d()
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				d[i][j] = 0.0;
	}

	void makeIdentity()
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				d[i][j] = (i == j ? 1 : 0);
	}

	void operator += (const vec4d& r)
	{
		d[0][3] += r.x();
		d[1][3] += r.y();
		d[2][3] += r.z();
		d[3][3] += r.w();
	}

	void operator *= (const mat4d& q)
	{
		for (int i = 0; i < 4; ++i)
		{
			double v[4] = { d[i][0], d[i][1], d[i][2], d[i][3] };
			for (int j = 0; j < 4; ++j)
			{
				d[i][j] = v[0] * q[0][j] + v[1] * q[1][j] + v[2] * q[2][j] + v[3] * q[3][j];
			}
		}
	}

	vec4d operator * (const vec4d& r)
	{
		return vec4d(
			d[0][0] * r[0] + d[0][1] * r[1] + d[0][2] * r[2] + d[0][3] * r[3],
			d[1][0] * r[0] + d[1][1] * r[1] + d[1][2] * r[2] + d[1][3] * r[3],
			d[2][0] * r[0] + d[2][1] * r[1] + d[2][2] * r[2] + d[2][3] * r[3],
			d[3][0] * r[0] + d[3][1] * r[1] + d[3][2] * r[2] + d[3][3] * r[3]);
	}

	double* operator [] (size_t n) { return d[n]; }
	const double* operator [] (size_t n) const { return d[n]; }

	static mat4d translate(const vec3d& r)
	{
		mat4d m;
		m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = r.x;
		m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = r.y;
		m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = r.z;
		m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
		return m;
	}

	static mat4d rotate(const quatd& q)
	{
		mat3d R = q.RotationMatrix();
		mat4d m;
		m[0][0] = R[0][0]; m[0][1] = R[0][1]; m[0][2] = R[0][2]; m[0][3] = 0;
		m[1][0] = R[1][0]; m[1][1] = R[1][1]; m[1][2] = R[1][2]; m[1][3] = 0;
		m[2][0] = R[2][0]; m[2][1] = R[2][1]; m[2][2] = R[2][2]; m[2][3] = 0;
		m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
		return m;
	}

	static mat4d scale(double x, double y, double z)
	{
		mat4d m;
		m[0][0] = x; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
		m[1][0] = 0; m[1][1] = y; m[1][2] = 0; m[1][3] = 0;
		m[2][0] = 0; m[2][1] = 0; m[2][2] = z; m[2][3] = 0;
		m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
		return m;
	}

public:
	double d[4][4];
};
