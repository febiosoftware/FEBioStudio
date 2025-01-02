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
#include <FSCore/color.h>

namespace rt
{
	struct Vec4;

	struct Vec3
	{
		Vec3() { d[0] = d[1] = d[2] = 0.0; }
		Vec3(double x, double y, double z) { d[0] = x; d[1] = y; d[2] = z; }

		explicit Vec3(const vec3f& v)
		{
			d[0] = v.x;
			d[1] = v.y;
			d[2] = v.z;
		}

		explicit Vec3(const vec3d& v)
		{
			d[0] = v.x;
			d[1] = v.y;
			d[2] = v.z;
		}

		explicit Vec3(const Vec4& v);

		void operator = (const Vec4& v);

		double x() const { return d[0]; }
		double y() const { return d[1]; }
		double z() const { return d[2]; }

		void x(double v) { d[0] = v; }
		void y(double v) { d[1] = v; }
		void z(double v) { d[2] = v; }

		double& operator[] (size_t n) { return d[n]; }
		double operator[] (size_t n) const { return d[n]; }

		void normalize()
		{
			double L = sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
			if (L != 0)
			{
				d[0] /= L;
				d[1] /= L;
				d[2] /= L;
			}
		}

		double sqrLength() const {
			return d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
		}

	public:
		Vec3 operator + (const Vec3& a) const
		{
			return Vec3(d[0] + a.d[0], d[1] + a.d[1], d[2] + a.d[2]);
		}

		Vec3 operator - (const Vec3& a) const
		{
			return Vec3(d[0] - a.d[0], d[1] - a.d[1], d[2] - a.d[2]);
		}

		Vec3 operator * (double a) const
		{
			return Vec3(a * d[0], a * d[1], a * d[2]);
		}

		double operator * (const Vec3& a) const
		{
			return d[0] * a.d[0] + d[1] * a.d[1] + d[2] * a.d[2];
		}

		static Vec3 cross(const Vec3& a, const Vec3& b)
		{
			return Vec3(
				a.d[1]*b.d[2] - a.d[2]*b.d[1],
				a.d[2]*b.d[0] - a.d[0]*b.d[2],
				a.d[0]*b.d[1] - a.d[1]*b.d[0]
				);
		}

	public:
		double d[3];
	};

	struct Vec4
	{
		Vec4() { d[0] = d[1] = d[2] = 0.0; d[3] = 1.0; }
		Vec4(double x, double y, double z, double w = 1) { d[0] = x; d[1] = y; d[2] = z; d[3] = w; }

		explicit Vec4(const vec3f& v, double w = 1)
		{
			d[0] = v.x;
			d[1] = v.y;
			d[2] = v.z;
			d[3] = w;
		}

		explicit Vec4(const vec3d& v, double w = 1)
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

	inline Vec3::Vec3(const Vec4& v)
	{
		d[0] = v[0]; d[1] = v[1]; d[2] = v[2];
	}

	inline void Vec3::operator = (const Vec4& v)
	{
		d[0] = v[0]; d[1] = v[1]; d[2] = v[2];
	}

	class Matrix4
	{
	public:
		Matrix4()
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

		void operator += (const Vec4& r)
		{
			d[0][3] += r.x();
			d[1][3] += r.y();
			d[2][3] += r.z();
			d[3][3] += r.w();
		}

		void operator *= (const Matrix4& q)
		{
			for (int i = 0; i < 4; ++i)
			{
				double v[4] = { d[i][0], d[i][1], d[i][2], d[i][3]};
				for (int j = 0; j < 4; ++j)
				{
					d[i][j] = v[0] * q[0][j] + v[1] * q[1][j] + v[2] * q[2][j] + v[3] * q[3][j];
				}
			}
		}

		Vec4 operator * (const Vec4& r)
		{
			return Vec4(
				d[0][0] * r[0] + d[0][1] * r[1] + d[0][2] * r[2] + d[0][3] * r[3],
				d[1][0] * r[0] + d[1][1] * r[1] + d[1][2] * r[2] + d[1][3] * r[3],
				d[2][0] * r[0] + d[2][1] * r[1] + d[2][2] * r[2] + d[2][3] * r[3],
				d[3][0] * r[0] + d[3][1] * r[1] + d[3][2] * r[2] + d[3][3] * r[3]);
		}

		double* operator [] (size_t n) { return d[n]; }
		const double* operator [] (size_t n) const { return d[n]; }

		static Matrix4 translate(const Vec3& r)
		{
			Matrix4 m;
			m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = r[0];
			m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = r[1];
			m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = r[2];
			m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
			return m;
		}

		static Matrix4 rotate(const quatd& q)
		{
			mat3d R = q.RotationMatrix();
			Matrix4 m;
			m[0][0] = R[0][0]; m[0][1] = R[0][1]; m[0][2] = R[0][2]; m[0][3] = 0;
			m[1][0] = R[1][0]; m[1][1] = R[1][1]; m[1][2] = R[1][2]; m[1][3] = 0;
			m[2][0] = R[2][0]; m[2][1] = R[2][1]; m[2][2] = R[2][2]; m[2][3] = 0;
			m[3][0] =       0; m[3][1] =       0; m[3][2] =       0; m[3][3] = 1;
			return m;
		}

	public:
		double d[4][4];
	};

	struct Color
	{
		Color() { v[0] = v[1] = v[2] = 0.0; v[3] = 1.0; }
		Color(double r, double g, double b, double a = 1.0)
		{
			v[0] = r; v[1] = g; v[2] = b; v[3] = a;
		}

		Color(const GLColor& c)
		{
			c.toDouble(v);
		}

		double& r() { return v[0]; }
		double& g() { return v[1]; }
		double& b() { return v[2]; }
		double& a() { return v[3]; }

		double r() const { return v[0]; }
		double g() const { return v[1]; }
		double b() const { return v[2]; }
		double a() const { return v[3]; }

		void r(double c) { v[0] = c; }
		void g(double c) { v[1] = c; }
		void b(double c) { v[2] = c; }
		void a(double c) { v[3] = c; }

	public:
		Color operator + (const Color& c)
		{
			return Color(v[0] + c.v[0], v[1] + c.v[1], v[2] + c.v[2], v[3] + c.v[3]);
		}

		void operator += (const Color& c)
		{
			v[0] += c.v[0];
			v[1] += c.v[1];
			v[2] += c.v[2];
			v[3] += c.v[3];
		}

		void operator /= (double a)
		{
			v[0] /= a;
			v[1] /= a;
			v[2] /= a;
			v[3] /= a;
		}

		Color operator * (double a)
		{
			return Color(a*v[0], a*v[1], a*v[2], a*v[3]);
		}

		Color operator / (double a)
		{
			return Color(v[0]/a, v[1]/a, v[2]/a, v[3]/a);
		}

		void clamp()
		{
			v[0] = (v[0] < 0 ? 0 : (v[0] > 1 ? 1 : v[0]));
			v[1] = (v[1] < 0 ? 0 : (v[1] > 1 ? 1 : v[1]));
			v[2] = (v[2] < 0 ? 0 : (v[2] > 1 ? 1 : v[2]));
			v[3] = (v[3] < 0 ? 0 : (v[3] > 1 ? 1 : v[3]));
		}

	public:
		double v[4];
	};

	struct Ray
	{
		Ray() {}
		Ray(Vec3 o, Vec3 d) : origin(o), direction(d) {}

		Vec3 origin;
		Vec3 direction;
	};

	struct Intersect
	{
		Vec3 point;
		double r[2] = { 0, 0 };

	};

	struct Box
	{
		double x0 = 0, y0 = 0, z0 = 0;
		double x1 = 0, y1 = 0, z1 = 0;
		bool valid = false;

		void operator += (const Vec3& r)
		{
			if (!valid)
			{
				x0 = x1 = r.x();
				y0 = y1 = r.y();
				z0 = z1 = r.z();
				valid = true;
			}
			else
			{
				if      (r.x() < x0) x0 = r.x();
				else if (r.x() > x1) x1 = r.x();

				if      (r.y() < y0) y0 = r.y();
				else if (r.y() > y1) y1 = r.y();

				if      (r.z() < z0) z0 = r.z();
				else if (r.z() > z1) z1 = r.z();
			}
		}
		
		void inflate(double a)
		{
			x0 -= a; x1 += a;
			y0 -= a; y1 += a;
			z0 -= a; z1 += a;
		}

		bool isInside(const Vec3& p, double eps = 0) const;

		bool intersect(const Ray& ray) const;
	};
}
