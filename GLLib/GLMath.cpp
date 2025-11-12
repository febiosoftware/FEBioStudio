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
#include "GLMath.h"

bool gl::Box::isInside(const gl::Vec3& p) const
{
	if (p.x()< x0) return false;
	if (p.x()> x1) return false;

	if (p.y()< y0) return false;
	if (p.y()> y1) return false;

	if (p.z()< z0) return false;
	if (p.z()> z1) return false;

	return true;
}

bool gl::Box::intersect(const gl::Ray& ray, gl::Vec3& q) const
{
	if (!valid) return false;

	const Vec3& o = ray.origin;
	const Vec3& t = ray.direction;

	bool intersected = false;

	double l = 0, lmin = 0;
	if (t[0] != 0)
	{
		l = (x0 - o[0]) / t[0];
		if (l >= 0)
		{
			double y = o[1] + t[1] * l;
			double z = o[2] + t[2] * l;
			if ((y >= y0) && (y <= y1) &&
				(z >= z0) && (z <= z1))
			{
				q = Vec3(x0, y, z);
				lmin = l;
				intersected = true;
			}
		}

		l = (x1 - o[0]) / t[0];
		if (l >= 0)
		{
			double y = o[1] + t[1] * l;
			double z = o[2] + t[2] * l;
			if ((y >= y0) && (y <= y1) &&
				(z >= z0) && (z <= z1))
			{
				if ((intersected == false) || (l < lmin))
				{
					q = Vec3(x1, y, z);
					lmin = l;
					intersected = true;
				}
			}
		}
	}

	if (t[1] != 0)
	{
		l = (y0 - o[1]) / t[1];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double z = o[2] + t[2] * l;
			if ((x >= x0) && (x <= x1) &&
				(z >= z0) && (z <= z1))
			{
				if (!intersected || (l < lmin))
				{
					q = Vec3(x, y0, z);
					lmin = l;
					intersected = true;
				}
			}
		}

		l = (y1 - o[1]) / t[1];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double z = o[2] + t[2] * l;
			if ((x >= x0) && (x <= x1) &&
				(z >= z0) && (z <= z1))
			{
				if (!intersected || (l < lmin))
				{
					q = Vec3(x, y1, z);
					lmin = l;
					intersected = true;
				}
			}
		}
	}

	if (t[2] != 0)
	{
		l = (z0 - o[2]) / t[2];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double y = o[1] + t[1] * l;
			if ((x >= x0) && (x <= x1) &&
				(y >= y0) && (y <= y1))
			{
				if (!intersected || (l < lmin))
				{
					q = Vec3(x, y, z0);
					lmin = l;
					intersected = true;
				}
			}
		}

		l = (z1 - o[2]) / t[2];
		if (l >= 0)
		{
			double x = o[0] + t[0] * l;
			double y = o[1] + t[1] * l;
			if ((x >= x0) && (x <= x1) &&
				(y >= y0) && (y <= y1))
			{
				if (!intersected || (l < lmin))
				{
					q = Vec3(x, y, z1);
					lmin = l;
					intersected = true;
				}
			}
		}
	}

	return intersected;
}

bool gl::Box::intersect(const gl::Vec3& a, const gl::Vec3& b) const
{
	if (!valid) return false;

	const Vec3 o = a;
	const Vec3 t = b - a;

	double l = 0;
	if (t[0] != 0)
	{
		l = (x0 - o[0]) / t[0];
		if ((l >= 0) && (l <= 1))
		{
			double y = o[1] + t[1] * l;
			double z = o[2] + t[2] * l;
			if ((y >= y0) && (y <= y1) &&
				(z >= z0) && (z <= z1)) return true;
		}

		l = (x1 - o[0]) / t[0];
		if ((l >= 0) && (l <= 1))
		{
			double y = o[1] + t[1] * l;
			double z = o[2] + t[2] * l;
			if ((y >= y0) && (y <= y1) &&
				(z >= z0) && (z <= z1)) return true;
		}
	}

	if (t[1] != 0)
	{
		l = (y0 - o[1]) / t[1];
		if ((l >= 0) && (l <= 1))
		{
			double x = o[0] + t[0] * l;
			double z = o[2] + t[2] * l;
			if ((x >= x0) && (x <= x1) &&
				(z >= z0) && (z <= z1)) return true;
		}

		l = (y1 - o[1]) / t[1];
		if ((l >= 0) && (l <= 1))
		{
			double x = o[0] + t[0] * l;
			double z = o[2] + t[2] * l;
			if ((x >= x0) && (x <= x1) &&
				(z >= z0) && (z <= z1)) return true;
		}
	}

	if (t[2] != 0)
	{
		l = (z0 - o[2]) / t[2];
		if ((l >= 0) && (l <= 1))
		{
			double x = o[0] + t[0] * l;
			double y = o[1] + t[1] * l;
			if ((x >= x0) && (x <= x1) &&
				(y >= y0) && (y <= y1)) return true;
		}

		l = (z1 - o[2]) / t[2];
		if ((l >= 0) && (l <= 1))
		{
			double x = o[0] + t[0] * l;
			double y = o[1] + t[1] * l;
			if ((x >= x0) && (x <= x1) &&
				(y >= y0) && (y <= y1)) return true;
		}
	}

	return false;
}
