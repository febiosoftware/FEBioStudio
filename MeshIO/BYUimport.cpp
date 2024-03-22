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

#include "BYUimport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GModel.h>

BYUimport::BYUimport(FSProject& prj) : FSFileImport(prj)
{
}

BYUimport::~BYUimport(void)
{
}

bool BYUimport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();

	if (!Open(szfile, "rt")) return errf("Failed opening file %s.", szfile);

	char szline[256] = {0}, *ch;
	int nread, i;

	// read the first line
	int nparts = 0;
	int nodes = 0;
	int edges = 0;
	int elems = 0;
	ch = fgets(szline, 255, m_fp);
	if (ch == 0) return errf("An unexpected error occured while reading the file data.");
	nread = sscanf(szline, "%d%d%d%d", &nparts, &nodes, &elems, &edges);
	if (nread != 4) return errf("Error or first line. Is this a BYU file?");

	if (nparts <= 0) return errf("Invalid number of parts.");
	if (nodes <= 0) return errf("Invalid number of nodes.");
	if (edges <= 0) return errf("Invalid number of edges.");
	if (elems <= 0) return errf("Invalid number of polygons.");

	// read the parts
	m_Part.resize(nparts);
	for (i=0; i<nparts; ++i)
	{
		PART& p = m_Part[i];
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		nread = sscanf(szline, "%d%d", &p.n0, &p.n1);
	}

	// create a new mesh
	FSSurfaceMesh* pm = new FSSurfaceMesh();
	pm->Create(nodes, 0, elems);

	// read the nodes
	for (i=0; i<nodes; ++i)
	{
		FSNode& n = pm->Node(i);
		vec3d& r = n.r;
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		nread = sscanf(szline, "%lg%lg%lg", &r.x, &r.y, &r.z);
		if (nread != 3) return errf("An error occured while reading the nodal coordinates.");
	}

	// read the elements
	int n[5];
	for (i=0; i<elems; ++i)
	{
		FSFace& f = pm->Face(i);
		f.m_gid = 0;
		ch = fgets(szline, 255, m_fp);
//		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		nread = sscanf(szline, "%d%d%d%d%d", &n[0], &n[1], &n[2], &n[3], &n[4]);
		switch (nread)
		{
		case 3: 
			f.SetType(FE_FACE_TRI3);
			f.n[0] = n[0] - 1;
			f.n[1] = n[1] - 1;
			f.n[2] = -n[2] - 1;
			f.n[3] = -n[2] - 1;
			break;
		case 4:
			f.SetType(FE_FACE_QUAD4);
			f.n[0] = n[0] - 1;
			f.n[1] = n[1] - 1;
			f.n[2] = n[2] - 1;
			f.n[3] = -n[3] - 1;
			break;
		default:
			delete pm;
			return errf("Only triangular and quadrilateral polygons are supported.");
		}
	}

	Close();

	pm->RebuildMesh();
	GSurfaceMeshObject* po = new GSurfaceMeshObject(pm);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
