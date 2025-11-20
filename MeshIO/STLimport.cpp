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

#include "STLimport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GModel.h>

//-----------------------------------------------------------------------------
STLimport::STLimport(FSProject& prj) : FSFileImport(prj)
{
}

//-----------------------------------------------------------------------------
STLimport::~STLimport(void)
{
}

//-----------------------------------------------------------------------------
// read a line from the input file
bool STLimport::read_line(char* szline, const char* sz)
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

//-----------------------------------------------------------------------------
// Load an STL model
bool STLimport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// try to read ascii STL
	if (read_ascii(szfile) == false)
	{
		// try to read binary STL
		if (read_binary(szfile) == false)
		{
			return false;
		}
	}

	// build the nodes
	GObject* po = build_mesh();

//	static int nc = 1;
//	char sz[256];
//	sprintf(sz, "STL-Object%02d", nc++);
//	po->SetName(sz);
	const char* szname = strrchr(szfile, '/');
	if (szname == nullptr)
	{
		szname = strrchr(szfile, '\\');
		if (szname == nullptr) szname = szfile; else szname++;
	}
	else szname++;
	po->SetName(szname);

	// add the object to the model
	m_pfem->GetModel().AddObject(po);

	return true;
}

//-----------------------------------------------------------------------------
bool STLimport::read_ascii(const char* szfile)
{
	m_nline = 0;

	// try to open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s.", szfile);

	// read the first line
	char szline[256] = { 0 };
	if (read_line(szline, "solid") == false) return errf("First line must be solid definition.");

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
			else return errf("Error encountered at line %d", m_nline);
		}

		// read the outer loop line
		if (read_line(szline, "outer loop") == false) return errf("Error encountered at line %d", m_nline);

		// read the vertex data
		float x, y, z;
		if (read_line(szline, "vertex ") == false) return errf("Error encountered at line %d", m_nline);
		sscanf(szline, "vertex %g%g%g", &x, &y, &z);
		face.v1[0] = x;	face.v1[1] = y; face.v1[2] = z;

		if (read_line(szline, "vertex ") == false) return errf("Error encountered at line %d", m_nline);
		sscanf(szline, "vertex %g%g%g", &x, &y, &z);
		face.v2[0] = x;	face.v2[1] = y; face.v2[2] = z;

		if (read_line(szline, "vertex ") == false) return errf("Error encountered at line %d", m_nline);
		sscanf(szline, "vertex %g%g%g", &x, &y, &z);
		face.v3[0] = x;	face.v3[1] = y; face.v3[2] = z;

		// read the endloop tag
		if (read_line(szline, "endloop") == false) return errf("Error encountered at line %d", m_nline);

		// read the endfacet tag
		if (read_line(szline, "endfacet") == false) return errf("Error encountered at line %d", m_nline);

		// add the facet to the list
		m_Face.push_back(face);
	} while (1);

	// close the file
	Close();

	return true;
}

//-----------------------------------------------------------------------------
// read a line from the input file
bool STLimport::read_facet(STLimport::FACET& f)
{
	unsigned short att;
	if (fread(f.norm, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v1, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v2, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(f.v3, sizeof(float), 3, m_fp) != 3) return false;
	if (fread(&att, sizeof(att), 1, m_fp) != 1) return false;
	return true;
}

//-----------------------------------------------------------------------------
// Load an STL model
bool STLimport::read_binary(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	m_pfem = &fem;

	// try to open the file
	if (Open(szfile, "rb") == false) return errf("Failed opening file %s.", szfile);

	// read the header
	char szbuf[80] = { 0 };
	if (fread(szbuf, 80, 1, m_fp) != 1) return errf("Failed reading header.");

	// clear the list
	m_Face.clear();

	// read the number of triangles
	int numtri = 0;
	fread(&numtri, sizeof(int), 1, m_fp);
	if (numtri <= 0) return errf("Invalid number of triangles.");

	// read all the triangles
	FACET face;
	for (int i = 0; i < numtri; ++i)
	{
		// read the facet line
		if (read_facet(face) == false) return errf("Error encountered reading triangle data.");

		// add the facet to the list
		m_Face.push_back(face);
	}

	// close the file
	Close();

	return true;
}

//-----------------------------------------------------------------------------
// Build the FE model
GObject* STLimport::build_mesh()
{
	// number of facets
	int NF = (int)m_Face.size();

	// find the bounding box of the model
	BOX& b = m_box = BoundingBox();
	double h = 0.01*b.GetMaxExtent();
	b.Inflate(h,h,h);
	vec3d r0(b.x0, b.y0, b.z0);
	vec3d r1(b.x1, b.y1, b.z1);

	// create the box partition
	int NB = m_NB = (int) pow((double) (NF*3), 0.25) + 1;
	if (NB < 1) NB = 1;
	if (NB > 50) NB = 50;
	m_BL.resize(NB*NB*NB);
	int n = 0;
	for (int i=0; i<NB; ++i)
		for (int j=0; j<NB; ++j)
			for (int k=0; k<NB; ++k)
			{
				OBOX& bn = m_BL[n++];
				bn.r0.x = r0.x + i*(r1.x - r0.x)/NB;
				bn.r0.y = r0.x + j*(r1.x - r0.x)/NB;
				bn.r0.z = r0.x + k*(r1.x - r0.x)/NB;

				bn.r1.x = r0.x + (i+1)*(r1.x - r0.x)/NB;
				bn.r1.y = r0.x + (j+1)*(r1.x - r0.x)/NB;
				bn.r1.z = r0.x + (k+1)*(r1.x - r0.x)/NB;
			}

	// reserve space for nodes
	m_Node.reserve(NF*3);
	
	// create the nodes
	std::list<FACET>::iterator pf = m_Face.begin();
	int nid = 0;
	for (int i=0; i<NF; ++i, ++pf)
	{
		FACET& f = *pf;
        vec3d v1 = vec3d(f.v1[0], f.v1[1], f.v1[2]); f.n[0] = find_node(v1);
        vec3d v2 = vec3d(f.v2[0], f.v2[1], f.v2[2]); f.n[1] = find_node(v2);
        vec3d v3 = vec3d(f.v3[0], f.v3[1], f.v3[2]); f.n[2] = find_node(v3);
		f.nid = nid++;
	}
	int NN = (int)m_Node.size();

	// create the mesh
	FSSurfaceMesh* pm = new FSSurfaceMesh;
	pm->Create(NN, 0, NF);

	// create nodes
	for (int i=0; i<NN; ++i)
	{
		vec3d& ri = m_Node[i].r;
		FSNode& node = pm->Node(i);
		node.pos(ri);
	}

	// create elements
	std::list<FACET>::iterator is = m_Face.begin();
	for (int i=0; i<NF; ++i, ++is)
	{
		FACET& f = *is;
		int n = f.nid;
		if (n >= 0)
		{
			FSFace& face = pm->Face(n);
			face.SetType(FE_FACE_TRI3);
			face.m_gid = 0;
			face.n[0] = f.n[0];
			face.n[1] = f.n[1];
			face.n[2] = f.n[2];
		}
	}

	// update the mesh
	pm->RebuildMesh();
	GSurfaceMeshObject* po = new GSurfaceMeshObject(pm);

	return po;
}

//-----------------------------------------------------------------------------
int STLimport::FindBox(const vec3d& r)
{
	BOX& b = m_box;
	int i = (int)(m_NB*(r.x - b.x0)/(b.x1 - b.x0));
	int j = (int)(m_NB*(r.y - b.y0)/(b.y1 - b.y0));
	int k = (int)(m_NB*(r.z - b.z0)/(b.z1 - b.z0));
	return k + j*(m_NB + i*m_NB);
}

//-----------------------------------------------------------------------------
int STLimport::find_node(const vec3d& r, const double eps)
{
	// get the box in which this node lies
	int nb = FindBox(r);
	assert((nb >= 0)&&(nb<(int)m_BL.size()));
	OBOX& b = m_BL[nb];

	// see if this node is already in this box
	int N = (int)b.NL.size();
	for (int i=0; i<N; ++i)
	{
		NODE& ni = m_Node[b.NL[i]];
		vec3d& ri = ni.r;
		if ((ri - r)*(ri - r) < eps) return ni.n;
	}

	N = (int)m_Node.size();
	NODE nd;
	nd.r = r;
	nd.n = N;
	m_Node.push_back(nd);
	b.NL.push_back(nd.n);
	return nd.n;
}

//-----------------------------------------------------------------------------
BOX STLimport::BoundingBox()
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
