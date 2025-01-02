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
#include "RTMesh.h"
using namespace rt;

Mesh::Mesh()
{
	clear();
}

void Mesh::clear()
{
	const size_t ALLOC_SIZE = 1024 * 1024;
	tri.clear();
	tri.reserve(ALLOC_SIZE);
}

bool intersectTri(rt::Tri& tri, const Ray& ray, Intersect& intersect)
{
	const double tol = 0.0001;

	const Vec3& v1 = tri.r[0];
	const Vec3& v2 = tri.r[1];
	const Vec3& v3 = tri.r[2];

	Vec3 e1 = v2 - v1;
	Vec3 e2 = v3 - v1;

	// calculate the triangle normal
	Vec3 fn = Vec3::cross(e1, e2);
	fn.normalize();

	// find the intersection of the point with the plane
	const Vec3& r0 = ray.origin;
	const Vec3& nn = ray.direction;
	if (fn * nn == 0.0) return false;
	double l = fn * (v1 - r0) / (fn * nn);
	if (l < 0) return false;
	Vec3 q = r0 + nn * l;

	// find  the natural coordinates

	double A[2][2] = { { e1 * e1, e1 * e2 }, { e2 * e1, e2 * e2 } };
	double Di = 1.0 / (A[0][0] * A[1][1] - A[0][1] * A[1][0]);
	double Ai[2][2] = {
		{  A[1][1] * Di, -A[0][1] * Di },
		{ -A[1][0] * Di,  A[0][0] * Di}
	};

	Vec3 E1 = e1 * Ai[0][0] + e2 * Ai[0][1];
	Vec3 E2 = e1 * Ai[1][0] + e2 * Ai[1][1];

	double r = (q - v1) * E1;
	double s = (q - v1) * E2;

	intersect.point = q;
	intersect.r[0] = r;
	intersect.r[1] = s;

	return ((r >= -tol) && (s >= -tol) && (r + s <= 1.0 + tol));
}

template <class T>
T interpolate(T v[3], double r[2])
{
	double h0 = 1 - r[0] - r[1];
	double h1 = r[0];
	double h2 = r[1];
	return v[0] * h0 + v[1] * h1 + v[2] * h2;
}

bool rt::intersect(Mesh& mesh, const Ray& ray, Point& point)
{
	Vec3 c = ray.origin;
	size_t imin = -1;
	double dmin2 = 0;
	Intersect q;
	for (size_t i = 0; i < mesh.triangles(); ++i)
	{
		rt::Tri& tri = mesh.triangle(i);
		Intersect p;
		if (intersectTri(tri, ray, p))
		{
			double D2 = (p.point - c).sqrLength();
			if ((imin == -1) || (D2 < dmin2))
			{
				imin = i;
				dmin2 = D2;
				q = p;
			}
		}
	}

	if (imin != -1)
	{
		rt::Tri& tri = mesh.triangle(imin);
		point.r = interpolate(tri.r, q.r);
		point.n = interpolate(tri.n, q.r); point.n.normalize();
		point.t = interpolate(tri.t, q.r); 
		point.c = interpolate(tri.c, q.r); point.c.clamp();
	}

	return (imin != -1);
}
