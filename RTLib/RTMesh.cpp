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
#include <FSCore/FSLogger.h>
#include <stack>
using namespace rt;

Mesh::Mesh()
{
	clear();
}

void Mesh::clear()
{
	const size_t ALLOC_SIZE = 1024 * 1024;
	triList.clear();
	triList.reserve(ALLOC_SIZE);
}

void Mesh::addTri(rt::Tri& tri)
{
	triList.push_back(tri);
}

bool intersectTri(rt::Tri& tri, const Ray& ray, Intersect& intersect)
{
	const double tol = 0.0001;

	const Vec3& fn = tri.fn;
	Vec3* v = tri.r;

	// find the intersection of the point with the plane
	const Vec3& o = ray.origin;
	const Vec3& t = ray.direction;
	double D = fn * t;
	if (D == 0.0) return false;
	double l = fn * (v[0] - o) / D;
	if (l < 0) return false;

	// find the natural coordinates
	const Vec3 e1 = v[1] - v[0];
	const Vec3 e2 = v[2] - v[0];
	double A[3] = { e1 * e1, e2 * e2, e1 * e2 };
	D = A[0] * A[1] - A[2] * A[2];
	if (D == 0) return false;
	double Ai[3] = { A[1] / D, A[0] / D, -A[2] / D};

	Vec3 q = o + t * l;
	const Vec3 qo = q - v[0];
	double qe1 = qo * e1;
	double qe2 = qo * e2;
	double r = (qe1 * Ai[0] + qe2 * Ai[2]);
	double s = (qe1 * Ai[2] + qe2 * Ai[1]);

	if ((r >= -tol) && (s >= -tol) && (r + s <= 1.0 + tol))
	{
		intersect.point = q;
		intersect.r[0] = r;
		intersect.r[1] = s;
		return true;
	}
	else return false;
}

bool intersectTri(rt::Tri& tri, const Vec3& a, const Vec3& b)
{
	constexpr double tol = 0.0001;

	const Vec3& fn = tri.fn;
	Vec3* v = tri.r;

	// find the intersection of the point with the plane
	const Vec3 t = b - a;
	double D = fn * t;
	if (D == 0.0) return false;
	double l = fn * (v[0] - a) / D;
	if ((l < 0) || (l > 1)) return false;

	// find the natural coordinates
	Vec3 e1 = v[1] - v[0];
	Vec3 e2 = v[2] - v[0];
	double A[3] = { e1 * e1, e2 * e2, e1 * e2};
	D = A[0] * A[1] - A[2] * A[2];
	if (D == 0) return false;
	double Ai[3] = { A[1] / D, A[0] / D, -A[2] / D };

	Vec3 q = a + t * l;
	Vec3 q0 = q - v[0];
	double q01 = q0 * e1;
	double q02 = q0 * e2;
	double r = q01 * Ai[0] + q02 * Ai[2];
	double s = q01 * Ai[2] + q02 * Ai[1];
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

bool insideBox(Box& box, rt::Tri& tri)
{
	const double eps = 1e-7;
	Vec3* v = tri.r;
	if (box.isInside(v[0], eps) && box.isInside(v[1], eps) && box.isInside(v[2], eps))
	{
		return true;
	}
	return false;
}

bool rt::intersectBox(Box& box, rt::Tri& tri)
{
	// quick check to see if any of the triangle nodes are inside the box.
	const double eps = 1e-7;
	Vec3* v = tri.r;
	if (box.isInside(v[0], eps) || box.isInside(v[1], eps) || box.isInside(v[2], eps))
	{
		return true;
	}
	// It's possible that the triangle is bigger than the box.
	// Let's try the triangle's center
	Vec3 c = (v[0] + v[1] + v[2]) / 3.0;
	if (box.isInside(c)) return true;

	// ok, we'll do something more clever. 
	// first, if all the box' nodes are on the same side of the triangle
	// then the triangle cannot intersect the box.
	const Vec3& N = tri.fn;
	unsigned int ncase = 0;
	for (int i = 0; i < 8; ++i)
	{
		Vec3 c = box.corner(i);
		double l = (c - v[0]) * N;
		if (l >= 0) ncase |= (1 << i);
	}
	if ((ncase == 0) || (ncase == 255)) return false;

	// There might be an intersection. 
	// Let's see if any of the triangle's edges intersect the box
	for (int i = 0; i < 3; ++i)
	{
		Vec3 a = v[i];
		Vec3 b = v[(i + 1) % 3];
		if (box.intersect(a, b)) return true;
	}

	// None of the triangle's edges are intersecting the box. 
	// But let's see if any of the box's edges intersect the triangle. 
	for (int i = 0; i < 12; ++i)
	{
		Vec3 a, b;
		box.edge(i, a, b);
		if (intersectTri(tri, a, b)) return true;
	}

	// Ok, I don't think there is an intersection.
	return false;
}

bool rt::intersectTriangles(std::vector<rt::Tri*>& tris, const rt::Ray& ray, rt::Point& point)
{
	Vec3 c = ray.origin;
	int imin = -1;
	double Dmin = 0;
	Intersect q;
	for (int i = 0; i < (int)tris.size(); ++i)
	{
		rt::Tri& tri = *tris[i];
		Vec3* v = tri.r;
		Intersect p;
		if ((tri.id != ray.tri_id) && intersectTri(tri, ray, p))
		{
			double D2 = (p.point - ray.origin).sqrLength();
			if ((imin == -1) || (D2 < Dmin))
			{
				imin = i;
				Dmin = D2;
				q = p;
			}
		}
	}

	if (imin != -1)
	{
		rt::Tri& tri = *tris[imin];
		point.r = q.point;// interpolate(tri.r, q.r);
		point.n = interpolate(tri.n, q.r); point.n.normalize();
		point.t = interpolate(tri.t, q.r);
		point.c = interpolate(tri.c, q.r); point.c.clamp();
		point.matid = tri.matid;
		point.tri_id = tri.id;
	}

	return (imin != -1);
}
