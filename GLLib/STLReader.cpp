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

#include "STLReader.h"

STLReader::STLReader()
{
}

// read a line from the input file
bool STLReader::read_line(char* szline, const char* sz)
{
	m_nline++;

	int l = (int)strlen(sz);
	if (fgets(szline, 255, m_fp) == 0) return false;

	// remove leading white space
	int n = 0;
	while (isspace(szline[n])) ++n;
	if (n != 0)
	{
		char* ch = szline + n;
		n = 0;
		while (*ch) szline[n++] = *ch++;
	}

	if (strncmp(szline, sz, l) != 0) return false;

	return true;
}

// Load an STL file
GLMesh* STLReader::Load(const std::string& filename)
{
	const char* szfile = filename.c_str();
	if (szfile == nullptr) return nullptr;

	// try to read ascii STL
	if (read_ascii(szfile) == false)
	{
		// try to read binary STL
		if (read_binary(szfile) == false)
		{
			return nullptr;
		}
	}

	// build the nodes
	GLMesh* pm = BuildMesh();

	return pm;
}

bool STLReader::read_ascii(const char* szfile)
{
	m_nline = 0;

	if (m_fp) fclose(m_fp);

	// try to open the file
	m_fp = fopen(szfile, "rt");
	if (m_fp == nullptr) return false;

	// read the first line
	char szline[256] = { 0 };
	if (read_line(szline, "solid") == false) return false;

	// clear the list
	m_Face.clear();

	// read all the triangles
	int nc = 0;
	FACET face;
	do
	{
		// read the facet line
		if (read_line(szline, "facet normal") == false)
		{
			// check for the endsolid tag
			if (strncmp(szline, "endsolid", 8) == 0) break;
			else return false;
		}

		// read the outer loop line
		if (read_line(szline, "outer loop") == false) return false;

		// read the vertex data
		float x, y, z;
		if (read_line(szline, "vertex ") == false) return false;
		sscanf(szline, "vertex %g%g%g", &x, &y, &z);
		face.v1[0] = x;	face.v1[1] = y; face.v1[2] = z;

		if (read_line(szline, "vertex ") == false) return false;
		sscanf(szline, "vertex %g%g%g", &x, &y, &z);
		face.v2[0] = x;	face.v2[1] = y; face.v2[2] = z;

		if (read_line(szline, "vertex ") == false) return false;
		sscanf(szline, "vertex %g%g%g", &x, &y, &z);
		face.v3[0] = x;	face.v3[1] = y; face.v3[2] = z;

		// read the endloop tag
		if (read_line(szline, "endloop") == false) return false;

		// read the endfacet tag
		if (read_line(szline, "endfacet") == false) return false;

		// add the facet to the list
		m_Face.push_back(face);
	} while (1);

	// close the file
	fclose(m_fp);
	m_fp = nullptr;

	return true;
}

// read a line from the input file
bool STLReader::read_facet(STLReader::FACET& f)
{
	unsigned short att;
	if (fread(f.norm, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v1, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v2, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v3, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(&att, sizeof(att), 1, m_fp) != 1) return false;
	return true;
}

// Load an STL model
bool STLReader::read_binary(const char* szfile)
{
	if (m_fp) fclose(m_fp);

	// try to open the file
	m_fp = fopen(szfile, "rb");
	if (m_fp == nullptr) return false;

	// read the header
	char szbuf[80] = { 0 };
	if (fread(szbuf, 80, 1, m_fp) != 1) return false;

	// clear the list
	m_Face.clear();

	// read the number of triangles
	int numtri = 0;
	fread(&numtri, sizeof(int), 1, m_fp);
	if (numtri <= 0) return false;

	// read all the triangles
	FACET face;
	for (int i = 0; i < numtri; ++i)
	{
		// read the facet line
		if (read_facet(face) == false) return false;

		// add the facet to the list
		m_Face.push_back(face);
	}

	// close the file
	fclose(m_fp);
	m_fp = nullptr;

	return true;
}

GLMesh* STLReader::BuildMesh()
{
	int i;

	// number of facets
	int NF = (int)m_Face.size();

	// find the bounding box of the model
	BOX& b = m_box = BoundingBox();
	double h = 0.01 * b.GetMaxExtent();
	b.Inflate(h, h, h);
	vec3d r0(b.x0, b.y0, b.z0);
	vec3d r1(b.x1, b.y1, b.z1);

	// create the box partition
	int NB = m_NB = (int)pow((double)(NF * 3), 0.17) + 1;
	m_BL.resize(NB * NB * NB);
	int n = 0;
	for (i = 0; i < NB; ++i)
		for (int j = 0; j < NB; ++j)
			for (int k = 0; k < NB; ++k)
			{
				OBOX& bn = m_BL[n++];
				bn.r0.x = r0.x + i * (r1.x - r0.x) / NB;
				bn.r0.y = r0.x + j * (r1.x - r0.x) / NB;
				bn.r0.z = r0.x + k * (r1.x - r0.x) / NB;

				bn.r1.x = r0.x + (i + 1) * (r1.x - r0.x) / NB;
				bn.r1.y = r0.x + (j + 1) * (r1.x - r0.x) / NB;
				bn.r1.z = r0.x + (k + 1) * (r1.x - r0.x) / NB;
			}

	// reserve space for nodes
	m_Node.reserve(NF * 3);

	// create the nodes
	std::list<FACET>::iterator pf = m_Face.begin();
	int nid = 0;
	for (i = 0; i < NF; ++i, ++pf)
	{
		FACET& f = *pf;
		vec3d v1 = vec3d(f.v1[0], f.v1[1], f.v1[2]); f.n[0] = find_node(v1);
		vec3d v2 = vec3d(f.v2[0], f.v2[1], f.v2[2]); f.n[1] = find_node(v2);
		vec3d v3 = vec3d(f.v3[0], f.v3[1], f.v3[2]); f.n[2] = find_node(v3);
		f.nid = nid++;
	}
	int NN = (int)m_Node.size();

	// create the mesh
	GLMesh* pm = new GLMesh;
	pm->Create(NN, NF);

	// create nodes
	for (i = 0; i < NN; ++i)
	{
		vec3d& ri = m_Node[i].r;
		GLMesh::NODE& node = pm->Node(i);
		node.r = to_vec3f(ri);
	}

	// create elements
	std::list<FACET>::iterator is = m_Face.begin();
	for (i = 0; i < NF; ++i, ++is)
	{
		FACET& f = *is;
		int n = f.nid;
		if (n >= 0)
		{
			GLMesh::FACE& face = pm->Face(n);
			face.n[0] = f.n[0];
			face.n[1] = f.n[1];
			face.n[2] = f.n[2];
		}
	}

	// update the mesh
	pm->Update();

	return pm;
}

int STLReader::FindBox(const vec3d& r)
{
	BOX& b = m_box;
	int i = (int)(m_NB * (r.x - b.x0) / (b.x1 - b.x0));
	int j = (int)(m_NB * (r.y - b.y0) / (b.y1 - b.y0));
	int k = (int)(m_NB * (r.z - b.z0) / (b.z1 - b.z0));
	return k + j * (m_NB + i * m_NB);
}

int STLReader::find_node(const vec3d& r, const double eps)
{
	// get the box in which this node lies
	int nb = FindBox(r);
	assert((nb >= 0) && (nb < (int)m_BL.size()));
	OBOX& b = m_BL[nb];

	// see if this node is already in this box
	int N = (int)b.NL.size();
	for (int i = 0; i < N; ++i)
	{
		NODE& ni = m_Node[b.NL[i]];
		vec3d& ri = ni.r;
		if ((ri - r) * (ri - r) < eps) return ni.n;
	}

	N = (int)m_Node.size();
	NODE nd;
	nd.r = r;
	nd.n = N;
	m_Node.push_back(nd);
	b.NL.push_back(nd.n);
	return nd.n;
}

BOX STLReader::BoundingBox()
{
	std::list<FACET>::iterator pf = m_Face.begin();
	vec3d r = vec3d(pf->v1[0], pf->v1[1], pf->v1[2]);
	BOX b(r, r);
	for (pf = m_Face.begin(); pf != m_Face.end(); ++pf)
	{
		b += vec3d(pf->v1[0], pf->v1[1], pf->v1[2]);
		b += vec3d(pf->v2[0], pf->v2[1], pf->v2[2]);
		b += vec3d(pf->v3[0], pf->v3[1], pf->v3[2]);
	}
	return b;
}
