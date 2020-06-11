/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "FEPLYImport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <MeshTools/GModel.h>

FEPLYImport::FEPLYImport(FEProject& prj) : FEFileImport(prj)
{
}

FEPLYImport::~FEPLYImport()
{
}

bool FEPLYImport::Load(const char* szfile)
{
	FEModel& fem = m_prj.GetFEModel();

	if (!Open(szfile, "rt")) return errf("Failed opening file %s.", szfile);

	char szline[256] = {0}, *ch;

	// read the first line
	ch = fgets(szline, 255, m_fp);
	if (ch == 0) return errf("An unexpected error occured while reading the file data.");
	int n;
	if (n = strncmp(szline, "ply", 3) != 0) 
	{
		return errf("This is not a valid ply file.");
	}

	// find vertices and faces
	int verts = 0;
	int faces = 0;
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		if (strstr(ch, "element vertex") != 0) verts = atoi(ch+14);
		if (strstr(ch, "element face"  ) != 0) faces = atoi(ch+12);
	}
	while (strstr(ch, "end_header") == 0);

	// make sure we have vertices and faces
	if (verts == 0) return errf("No vertex data found.");
	if (faces == 0) return errf("No face data found.");

	FESurfaceMesh* pm = new FESurfaceMesh;
	pm->Create(verts, 0, faces);

	for (int i=0; i<verts; ++i)
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		FENode& n = pm->Node(i);
		vec3d& r = n.r;
		sscanf(szline, "%lg%lg%lg", &r.x, &r.y, &r.z);
	}

	for (int i=0; i<faces; ++i)
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		FEFace& el = pm->Face(i);
		el.SetType(FE_FACE_TRI3);
		sscanf(szline, "%*d%d%d%d", &el.n[0], &el.n[1], &el.n[2]);
	}

	Close();

	pm->BuildMesh();
	GSurfaceMeshObject* po = new GSurfaceMeshObject(pm);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
