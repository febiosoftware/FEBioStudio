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
#include <GLLib/GLMath.h>

namespace rt {

	struct Point {
		gl::Vec3 r;
		gl::Vec3 n;
		gl::Vec3 t;
		gl::Color c;
		int matid = -1;
	};

	struct Line
	{
		gl::Vec3 r[2];
		gl::Color c[2];
		int id = -1;
		int matid = -1;

		Point point(unsigned int i) const
		{
			return Point{ r[i], gl::Vec3(0,0,0), gl::Vec3(0,0,0), c[i] };
		}

		Line() {}

		Line(const Point& a, const Point& b)
		{
			r[0] = a.r; r[1] = b.r;
			c[0] = a.c; c[1] = b.c;
		}

		bool process()
		{
			if (r[0] == r[1]) return false;
			return true;
		}
	};

	struct Tri
	{
		gl::Vec3 r[3];
		gl::Vec3 n[3];
		gl::Vec3 t[3];
		gl::Color c[3];
		int id = -1;
		int matid = -1;

		Point point(unsigned int i) const
		{
			return Point{ r[i], n[i], t[i], c[i] };
		}

		Tri() {}

		Tri(const Point& a, const Point& b, const Point& d)
		{
			r[0] = a.r; r[1] = b.r; r[2] = d.r;
			n[0] = a.n; n[1] = b.n; n[2] = d.n;
			t[0] = a.t; t[1] = b.t; t[2] = d.t;
			c[0] = a.c; c[1] = b.c; c[2] = d.c;
		}

		bool process()
		{
			if ((r[0] == r[1]) || (r[0] == r[2]) || (r[1] == r[2])) return false;

			gl::Vec3 e1 = r[1] - r[0];
			gl::Vec3 e2 = r[2] - r[0];
			fn = gl::Vec3::cross(e1, e2);
			fn.normalize();

			return true;
		}

		gl::Vec3 fn;
	};

	struct Intersect
	{
		gl::Vec3 point;
		double r[2] = { 0, 0 };

	};

	struct Fragment
	{
		gl::Color color;
		double depth = 0.0;
	};

	class Mesh
	{
	public:
		Mesh();

		void clear();

		void addTri(Tri& t);

		void addLine(Line& l);

		size_t triangles() const { return triList.size(); }

		Tri& triangle(size_t n) { return triList[n]; }
		const Tri& triangle(size_t n) const { return triList[n]; }

		size_t lines() const { return lineList.size(); }

		Line& line(size_t n) { return lineList[n]; }
		const Line& line(size_t n) const { return lineList[n]; }

	private:
		std::vector<Tri> triList;
		std::vector<Line> lineList;
	};

	bool intersect(Mesh& mesh, const gl::Ray& ray, rt::Point& q);
	bool intersectTriangles(std::vector<rt::Tri*>& tris, const gl::Ray& ray, rt::Point& point);
	bool intersectBox(gl::Box& box, rt::Tri& tri);
}
