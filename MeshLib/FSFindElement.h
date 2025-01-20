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
#include <FSCore/box.h>
#include <vector>

class FSCoreMesh;

class FSFindElement
{
public:
	class OCTREE_BOX
	{
	public:
		BOX					m_box;
		std::vector<OCTREE_BOX*>	m_child;
		int					m_elem;
		int					m_level;

	public:
		OCTREE_BOX();
		~OCTREE_BOX();

		void split(int levels);

		OCTREE_BOX* Find(const vec3f& r);

		bool IsInside(const vec3f& r) const { return m_box.IsInside(to_vec3d(r)); }

		void Add(BOX& b, int nelem);
	};

public:
	FSFindElement(FSCoreMesh& mesh);

	void Init(int nframe = 0);
	void Init(std::vector<bool>& flags, int nframe = 0);

	bool FindElement(const vec3f& x, int& nelem, double r[3]);

	BOX BoundingBox() const { return m_bound.m_box; }

private:
	void InitReferenceFrame(std::vector<bool>& flags);
	void InitCurrentFrame(std::vector<bool>& flags);

	bool FindInReferenceFrame(const vec3f& x, int& nelem, double r[3]);
	bool FindInCurrentFrame(const vec3f& x, int& nelem, double r[3]);

private:
	OCTREE_BOX* FindBox(const vec3f& r);

private:
	OCTREE_BOX	m_bound;
	FSCoreMesh&	m_mesh;
	int			m_nframe;	// = 0 reference, 1 = current
};

inline bool FSFindElement::FindElement(const vec3f& x, int& nelem, double r[3])
{
	return (m_nframe == 0 ? FindInReferenceFrame(x, nelem, r) : FindInCurrentFrame(x, nelem, r));
}

class FSMesh;

bool FindElement2D(const vec2d& r, int& elem, double q[2], FSMesh* mesh);
