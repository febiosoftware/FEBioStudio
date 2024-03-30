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

#include "HyperSurfaceImport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GModel.h>

HyperSurfaceImport::HyperSurfaceImport(FSProject& prj) : FSFileImport(prj)
{
}

HyperSurfaceImport::~HyperSurfaceImport(void)
{
}

bool HyperSurfaceImport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();

	// open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s", szfile);

	int i;
	char szline[256];

	// find the vertices
	char* ch;
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("Error while reading file");
		if (strncmp(szline, "Vertices", 8) == 0) break;
	}
	while (true);

	int nodes = 0;
	sscanf(szline+8, "%d", &nodes);

	FSSurfaceMesh* pm = new FSSurfaceMesh();
	pm->Create(nodes, 0, 0);

	// read the nodes
	for (i=0; i<nodes; ++i)
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) { delete pm; Close(); return false; }

		FSNode& node = pm->Node(i);
		sscanf(szline, "%lg%lg%lg", &node.r.x, &node.r.y, &node.r.z);
	}

	// find the triangles
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) { delete pm; Close(); return false; }
		if (strncmp(szline, "Triangles", 9) == 0) break;
	}
	while (true);

	int elems = 0;
	sscanf(szline+9, "%d", &elems);
	pm->Create(0, 0, elems);

	// read the elements
	int ne[3];
	for (i=0; i<elems; ++i)
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) { delete pm; Close(); return false; }

		FSFace& face = pm->Face(i);
		face.SetType(FE_FACE_TRI3);
		sscanf(szline, "%d%d%d", &ne[0], &ne[1], &ne[2]);

		face.m_gid = 0;
		face.n[0] = ne[0] - 1;
		face.n[1] = ne[1] - 1;
		face.n[2] = ne[2] - 1;
		face.n[3] = ne[2] - 1;
	}

	// close the file
	Close();

	// create a new object
	pm->RebuildMesh();
	GObject* po = new GSurfaceMeshObject(pm);

	// add the object to the model
	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
