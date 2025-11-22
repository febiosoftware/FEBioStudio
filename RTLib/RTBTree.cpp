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
#include "RTBTree.h"
#include <FSCore/FSLogger.h>
#include <stack>
#include <omp.h>
#include <thread>

using namespace gl;

size_t rt::Btree::Block::size() const
{
	size_t n = tris.size();
	if (child[0]) n += child[0]->size();
	if (child[1]) n += child[1]->size();
	return n;
}

void rt::Btree::Block::split(int level)
{
	if (level <= 0) return;
	for (int i = 0; i < 2; ++i)
	{
		if (child[i] == nullptr)
		{
			Box box_i = box.split(i);
			child[i] = new Block;
			child[i]->box = box_i;
			child[i]->split(level - 1);
		}
	}
}

void rt::Btree::Block::add(rt::Tri* tri, int level)
{
	if (level == 0)
	{
		tris.push_back(tri);
	}
	else
	{
		for (int i = 0; i < 2; ++i)
		{
			if (child[i] == nullptr)
			{
				Box box_n = box.split(i);

				if (intersectBox(box_n, *tri))
				{
					child[i] = new Block;
					child[i]->box = box_n;
					child[i]->add(tri, level - 1);
				}
			}
			else if (intersectBox(child[i]->box, *tri))
			{
				child[i]->add(tri, level - 1);
			}
		}
	}
}

std::vector<rt::Btree::Block*> rt::Btree::leaves()
{
	std::vector<rt::Btree::Block*> l;
	std::stack<rt::Btree::Block*> S;
	S.push(root);
	while (!S.empty())
	{
		rt::Btree::Block* b = S.top(); S.pop();
		if ((b->child[0] == nullptr) || (b->child[1] == nullptr))
		{
			l.push_back(b);
		}
		else
		{
			if (b->child[0]) S.push(b->child[0]);
			if (b->child[1]) S.push(b->child[1]);
		}
	}
	return l;
}

size_t rt::Btree::blocks() const
{
	size_t n = 0;
	std::stack<rt::Btree::Block*> S;
	S.push(root);
	while (!S.empty())
	{
		rt::Btree::Block* b = S.top(); S.pop();
		n++;
		if (b->child[0]) S.push(b->child[0]);
		if (b->child[1]) S.push(b->child[1]);
	}
	return n;
}

void rt::Btree::Build(Mesh& mesh, int levels)
{
	delete root;
	root = new Block;

	if (output) FSLogger::Write("Building binary tree ...\n");
	int ntriangles = (int)mesh.triangles();
	if (output) FSLogger::Write("  Nr of triangles : %d\n", ntriangles);
	Box box;
	for (size_t i = 0; i < mesh.triangles(); ++i)
	{
		rt::Tri& tri = mesh.triangle(i);
		box += tri.r[0];
		box += tri.r[1];
		box += tri.r[2];
	}
	root->box = box;

	if (levels < 0) levels = 0;
	if (output) FSLogger::Write("  Splitting levels : %d\n", levels);
	int m = 0;
	std::vector<rt::Btree::Block*> leaves;
#pragma omp parallel shared(leaves)
	{
		// the the number of threads in this parallel region
//		int numThreads = omp_get_num_threads();
        int numThreads = std::thread::hardware_concurrency();

		// only execute in parallel if it's worth it
		if ((levels < 2) || (numThreads < 2))
		{
			// let's do it in serial
#pragma omp master
			for (int i = 0; i < ntriangles; ++i)
			{
				rt::Tri& tri = mesh.triangle(i);
				root->add(&tri, levels);
			}
		}
		else
		{
			// create initial split
#pragma omp single
			{
				m = (int)log2(numThreads);
				if (m > levels - 1) m = levels - 1;
				int actualThreads = (int)pow(2, m);
				if (output) FSLogger::Write("  Using %d threads.\n", actualThreads);
				root->split(m);
				leaves = rt::Btree::leaves();
			}

			// loop over all the triangles and sort them in the blocks
//			int threadId = omp_get_thread_num();
            int threadId = std::thread::hardware_concurrency();
			if ((threadId >= 0) && (threadId < (int)leaves.size()))
			{
				for (int i = 0; i < ntriangles; ++i)
				{
					rt::Tri& tri = mesh.triangle(i);
					if (intersectBox(leaves[threadId]->box, tri))
						leaves[threadId]->add(&tri, levels - m);
				}
			}
		}
	}

	int nrblocks = (int)blocks();
	if (output) FSLogger::Write("  Nr. of blocks : %d\n", nrblocks);
	if (output) FSLogger::Write("  Nr. of triangles in BTree : %d\n", (int)root->size());
}

bool rt::Btree::intersect(const Ray& ray, Point& p)
{
	Vec3 c;
	if (root->box.intersect(ray, c) == false) return false;

	bool found = false;
	double Dmin = 0;

	std::stack<rt::Btree::Block*> S;
	S.push(root);
	while (!S.empty())
	{
		rt::Btree::Block* block = S.top(); S.pop();
		if (block->tris.empty() == false)
		{
			rt::Point tmp;
			if (intersectTriangles(block->tris, ray, tmp))
			{
				double D2 = (tmp.r - ray.origin).sqrLength();
				if ((found == false) || (D2 < Dmin))
				{
					found = true;
					Dmin = D2;
					p = tmp;
				}
			}
		}

		Vec3 c1, c2;
		bool b1 = (block->child[0] ? block->child[0]->box.intersect(ray, c1) : false);
		bool b2 = (block->child[1] ? block->child[1]->box.intersect(ray, c2) : false);

		if (b1)
		{
			if (!found || ((c1 - ray.origin).sqrLength() < Dmin)) S.push(block->child[0]);
		}
		if (b2)
		{
			if (!found || ((c2 - ray.origin).sqrLength() < Dmin)) S.push(block->child[1]);
		}
	}

	return found;
}
