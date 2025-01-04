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
	Vec3 q = o + t * l;

	// find the natural coordinates
	Vec3 e1 = v[1] - v[0];
	Vec3 e2 = v[2] - v[0];
	double A[2][2] = { { e1 * e1, e1 * e2 }, { e2 * e1, e2 * e2 } };
	D = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	if (D == 0) return false;
	double Ai[2][2] = {
		{  A[1][1] / D, -A[0][1] / D},
		{ -A[1][0] / D,  A[0][0] / D}
	};

	Vec3 E1 = e1 * Ai[0][0] + e2 * Ai[0][1];
	Vec3 E2 = e1 * Ai[1][0] + e2 * Ai[1][1];

	double r = (q - v[0]) * E1;
	double s = (q - v[0]) * E2;

	intersect.point = q;
	intersect.r[0] = r;
	intersect.r[1] = s;

	return ((r >= -tol) && (s >= -tol) && (r + s <= 1.0 + tol));
}

bool intersectTri(rt::Tri& tri, const Vec3& a, const Vec3& b)
{
	const double tol = 0.0001;

	const Vec3& fn = tri.fn;
	Vec3* v = tri.r;

	// find the intersection of the point with the plane
	const Vec3& o = a;
	const Vec3& t = b - a;
	double D = fn * t;
	if (D == 0.0) return false;
	double l = fn * (v[0] - o) / D;
	if ((l < 0) || (l > 1)) return false;
	Vec3 q = o + t * l;

	// find the natural coordinates
	Vec3 e1 = v[1] - v[0];
	Vec3 e2 = v[2] - v[0];
	double A[2][2] = { { e1 * e1, e1 * e2 }, { e2 * e1, e2 * e2 } };
	D = A[0][0] * A[1][1] - A[0][1] * A[1][0];
	if (D == 0) return false;
	double Ai[2][2] = {
		{  A[1][1] / D, -A[0][1] / D},
		{ -A[1][0] / D,  A[0][0] / D}
	};

	Vec3 E1 = e1 * Ai[0][0] + e2 * Ai[0][1];
	Vec3 E2 = e1 * Ai[1][0] + e2 * Ai[1][1];

	double r = (q - v[0]) * E1;
	double s = (q - v[0]) * E2;
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

void rt::Btree::Block::split(int levels)
{
	level = levels;
	if (levels == 0)
	{
		tris.reserve(1024);
		return;
	}

	double dx = 0.5 * box.width();
	double dy = 0.5 * box.height();
	double dz = 0.5 * box.depth();

	for (int i = 0; i < 2; ++i)
	{
		Box box_n;
		if ((dx >= dy) && (dx >= dz))
		{
			box_n.x0 = box.x0 + i * dx; box_n.x1 = box.x0 + (i + 1) * dx;
			box_n.y0 = box.y0; box_n.y1 = box.y1;
			box_n.z0 = box.z0; box_n.z1 = box.z1;
		}
		else if ((dy >= dx) && (dy >= dz))
		{
			box_n.x0 = box.x0; box_n.x1 = box.x1;
			box_n.y0 = box.y0 + i * dy; box_n.y1 = box.y0 + (i + 1) * dy;
			box_n.z0 = box.z0; box_n.z1 = box.z1;
		}
		else if ((dz >= dx) && (dz >= dy))
		{
			box_n.x0 = box.x0; box_n.x1 = box.x1;
			box_n.y0 = box.y0; box_n.y1 = box.y1;
			box_n.z0 = box.z0 + i * dz; box_n.z1 = box.z0 + (i + 1) * dz;
		}

		box_n.valid = true;
		double R = box_n.maxExtent();
		box_n.inflate(R*1e-9);

		child[i] = new Block;
		child[i]->box = box_n;
		child[i]->split(levels - 1);
	}
}

bool intersectBox(Box& box, rt::Tri& tri)
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

bool rt::Btree::Block::add(rt::Tri& tri)
{
	if (!box.valid) return false;
	if (intersectBox(box, tri))
	{
		if (level == 0)
		{
			tris.push_back(&tri);
			return true;
		}
		else
		{
			bool b1 = false, b2 = false;
			if (child[0]) b1 = child[0]->add(tri);
			if (child[1]) b2 = child[1]->add(tri);
			return (b1 || b2);
		}
	}
	return false;
}

bool intersectTriangles(std::vector<rt::Tri*>& tris, const rt::Ray& ray, rt::Point& point)
{
	Vec3 c = ray.origin;
	int imin = -1;
	double zmax = 0;
	Intersect q;
	for (int i = 0; i < (int)tris.size(); ++i)
	{
		rt::Tri& tri = *tris[i];
		Vec3* v = tri.r;
		if ((imin == -1) || (v[0].z() > zmax) || (v[1].z() > zmax) || (v[2].z() > zmax))
		{
			Intersect p;
			if (intersectTri(tri, ray, p))
			{
				if ((imin == -1) || (p.point.z() > zmax))
				{
					imin = i;
					zmax = p.point.z();
					q = p;
				}
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
	}

	return (imin != -1);
}

bool rt::Btree::Block::intersect(const rt::Ray& ray, rt::Point& point)
{
	if (!box.valid) return false;

	if (box.intersect(ray))
	{
		if (level == 0)
		{
			return intersectTriangles(tris, ray, point);
		}
		else
		{
			Point q1, q2;
			bool b1 = (child[0] ? child[0]->intersect(ray, q1) : false);
			bool b2 = (child[1] ? child[1]->intersect(ray, q2) : false);

			if (b1 && b2)
			{
				if (q1.r.z() > q2.r.z()) point = q1; else point = q2;
			}
			else if (b1) point = q1;
			else if (b2) point = q2;

			return (b1 || b2);
		}
	}
	else return false;
}

size_t rt::Btree::Block::size() const
{
	if (level == 0) return tris.size();
	else
	{
		size_t n = 0;
		if (child[0]) n += child[0]->size();
		if (child[1]) n += child[1]->size();
		return n;
	}
}

void rt::Btree::Block::prune()
{
	for (int i = 0; i < 2; ++i)
	{
		if (child[i])
		{
			if (child[i]->size() == 0)
			{
				delete child[i];
				child[i] = nullptr;
			}
			else child[i]->prune();
		}
	}
}

void rt::Btree::Build(Mesh& mesh, int levels)
{
	delete root;
	root = new Block;

	rt::Box box;
	for (size_t i = 0; i < mesh.triangles(); ++i)
	{
		rt::Tri& tri = mesh.triangle(i);
		box += tri.r[0];
		box += tri.r[1];
		box += tri.r[2];
	}
	double R = box.maxExtent();
	box.inflate(R*1e-9);

	root->box = box;

	if (levels < 0) levels = 0;
	root->split(levels);

	for (int i = 0; i < mesh.triangles(); ++i)
	{
		rt::Tri& tri = mesh.triangle(i);
		root->add(tri);
	}

	root->prune();
}

bool rt::Btree::intersect(const Ray& ray, Point& p)
{
	return root->intersect(ray, p);
}

void rt::Btree::prune()
{
	root->prune();
}

