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
#include "RTMath.h"

bool rt::Box::isInside(const rt::Vec3& p, double eps) const
{
	if (p.x() + eps < x0) return false;
	if (p.x() - eps > x1) return false;

	if (p.y() + eps < y0) return false;
	if (p.y() - eps > y1) return false;

	if (p.z() + eps < z0) return false;
	if (p.z() - eps > z1) return false;

	return true;
}

bool rt::Box::intersect(const rt::Ray& ray) const
{
	if (!valid) return false;

	const Vec3& o = ray.origin;
	const Vec3& t = ray.direction;

	constexpr double eps = 1e-8;

	double l = 0;
	if (t[0] != 0)
	{
		l = (x0 - o[0]) / t[0];
		if (l >= 0)
		{
			double y = o[1] + t[1] * l;
			double z = o[2] + t[2] * l;
			if ((y + eps >= y0) && (y - eps <= y1) &&
				(z + eps >= z0) && (z - eps <= z1)) return true;
		}

		l = (x1 - o[0]) / t[0];
		if (l >= 0)
		{
			double y = o[1] + t[1] * l;
			double z = o[2] + t[2] * l;
			if ((y + eps >= y0) && (y - eps <= y1) &&
				(z + eps >= z0) && (z - eps <= z1)) return true;
		}
	}

	if (t[1] != 0)
	{
		l = (y0 - o[1]) / t[1];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double z = o[2] + t[2] * l;
			if ((x + eps >= x0) && (x - eps <= x1) &&
				(z + eps >= z0) && (z - eps <= z1)) return true;
		}

		l = (y1 - o[1]) / t[1];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double z = o[2] + t[2] * l;
			if ((x + eps >= x0) && (x - eps <= x1) &&
				(z + eps >= z0) && (z - eps <= z1)) return true;
		}
	}

	if (t[2] != 0)
	{
		l = (z0 - o[2]) / t[2];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double y = o[1] + t[1] * l;
			if ((x + eps >= x0) && (x - eps <= x1) && 
				(y + eps >= y0) && (y - eps <= y1)) return true;
		}

		l = (z1 - o[2]) / t[2];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double y = o[1] + t[1] * l;
			if ((x + eps >= x0) && (x - eps <= x1) &&
				(y + eps >= y0) && (y - eps <= y1)) return true;
		}
	}

	return false;
}

bool rt::Box::intersect(const Vec3& a, const Vec3& b) const
{
	if (!valid) return false;

	Vec3 c[6] = {
		Vec3(x0, y0, z0),
		Vec3(x1, y0, z0),
		Vec3(x1, y1, z0),
		Vec3(x0, y1, z0),
		Vec3(x0, y0, z0),
		Vec3(x0, y0, z1)
	};

	const Vec3 n[6] = {
		Vec3( 0, -1,  0),
		Vec3( 1,  0,  0),
		Vec3( 0,  1,  0),
		Vec3(-1,  0,  0),
		Vec3( 0,  0, -1),
		Vec3( 0,  0,  1),
	};

	Vec3 t = b - a;
	for (int i = 0; i < 6; ++i)
	{
		double D = t * n[i];
		if (D != 0)
		{
			double l = (n[i] * (c[i] - a)) / D;
			if ((l >= 0) && (l <= 1))
			{
				Vec3 q = a + t * l;
				if (isInside(q, 1e-8))
				{
					return true;
				}
			}
		}
	}
	return false;
}
