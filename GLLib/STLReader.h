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
#include <vector>
#include <list>
#include <FSCore/math3d.h>
#include "GLMesh.h"
#include <string>

class STLReader
{
	struct FACET
	{
		float	norm[3];
		float	v1[3];
		float	v2[3];
		float	v3[3];
		int		n[3];
		int		nid;
	};

	struct NODE
	{
		vec3d	r;
		int		n;
	};

	class OBOX
	{
	public:
		vec3d		r0, r1;		// points defining box
		std::vector<int>	NL;			// list of nodes inside this box

	public:
		OBOX() {}
		OBOX(const OBOX& b)
		{
			r0 = b.r0; r1 = b.r1;
			NL = b.NL;
		}
	};

public:
	STLReader();

	GLMesh* Load(const std::string& filename);

protected:
	bool read_line(char* szline, const char* sz);

	int find_node(const vec3d& r, const double eps = 1e-14);
	int FindBox(const vec3d& r);

	::BOX BoundingBox();

private:
	bool read_ascii(const char* szfile);
	bool read_binary(const char* szfile);

private:
	bool read_facet(FACET& f);

	GLMesh* BuildMesh();

protected:
	std::list<FACET>	m_Face;
	std::vector<NODE>	m_Node;
	int					m_nline;	// line counter

	int					m_NB;
	std::vector<OBOX>	m_BL;		// box lists

	BOX				m_box;		// bounding box

	FILE* m_fp = nullptr;
};
