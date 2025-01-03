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
#include "RTMath.h"

namespace rt {

	struct Point {
		Vec3 r;
		Vec3 n;
		Vec3 t;
		Color c;
	};

	struct Tri
	{
		Vec3 r[3];
		Vec3 n[3];
		Vec3 t[3];
		Color c[3];

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

			Vec3 e1 = r[1] - r[0];
			Vec3 e2 = r[2] - r[0];
			fn = Vec3::cross(e1, e2);
			fn.normalize();

			return true;
		}

		Vec3 fn;
	};

	class Mesh
	{
	public:
		Mesh();

		void clear();

		void addTri(Tri& t);

		size_t triangles() const { return triList.size(); }

		Tri& triangle(size_t n) { return triList[n]; }
		const Tri& triangle(size_t n) const { return triList[n]; }

	private:
		std::vector<Tri> triList;
	};

	bool intersect(Mesh& mesh, const Ray& ray, Point& q);

	class Btree
	{
	public:
		struct Block
		{
			Block() {}
			~Block() { delete child[0]; delete child[1]; };

			void split(int levels);
			bool add(rt::Tri& tri);

			void prune();

			int level = 0;
			Box box;
			Block* child[2] = { nullptr };
			std::vector<rt::Tri*> tris;

			bool intersect(const Ray& ray, Point& p);

			size_t size() const;
		};

		Btree() {}
		~Btree() { delete root; }

		Block* root = nullptr;

		void Build(rt::Mesh& mesh, int levels);

		bool intersect(const rt::Ray& ray, rt::Point& p);

	private:
		void prune();
	};
}
