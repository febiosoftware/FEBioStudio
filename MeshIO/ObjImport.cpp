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
#include "stdafx.h"
#include "ObjImport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GModel.h>

ObjImport::ObjImport(FSProject& prj) : FSFileImport(prj)
{
}

ObjImport::~ObjImport(void)
{

}

bool parseVertex(ObjImport::Vertex& v, const char* sz)
{
	if (sz == nullptr) return false;
	int n = sscanf(sz, "v%lg%lg%lg", &v.x, &v.y, &v.z);
	return (n == 3);
}

bool parseFace(ObjImport::Face& f, const char* sz)
{
	sz++; // eat 'f'
	for (int i = 0; i < 3; ++i)
	{
		while (isspace(*sz)) sz++; // eat space
		f.node[i] = atoi(sz);
		if (i < 2)
			while (!isspace(*sz)) sz++;
	}
	return true;
}

bool ObjImport::Load(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();

	if (!Open(szfile, "rt")) return errf("Failed opening file %s.", szfile);

	char szline[256] = { 0 }, * ch;
	while (!feof(m_fp) && !ferror(m_fp))
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == nullptr) break;

		if (ch[0] == 'v')
		{
			if (ch[1] == 'n')
			{
				// vertex normals (ignore for now)
			}
			else
			{
				// vertex coordinates
				Vertex v;
				if (parseVertex(v, ch))
				{
					m_vert.push_back(v);
				}
			}
		}
		else if (ch[0] == 'f')
		{
			// faces
			ObjImport::Face f;
			if (parseFace(f, ch))
			{
				m_face.push_back(f);
			}
		}
	}
	Close();

	// create a new mesh
	int nodes = m_vert.size();
	int faces = m_face.size();
	FSSurfaceMesh* pm = new FSSurfaceMesh();
	pm->Create(nodes, 0, faces);

	for (int i = 0; i < nodes; ++i)
	{
		FSNode& n = pm->Node(i);
		Vertex& v = m_vert[i];
		n.r = vec3d(v.x, v.y, v.z);
	}

	for (int i = 0; i <faces; ++i)
	{
		FSFace& f = pm->Face(i);
		f.m_gid = 0;
		f.SetType(FE_FACE_TRI3);

		int* n = m_face[i].node;
		f.n[0] = n[0] - 1;
		f.n[1] = n[1] - 1;
		f.n[2] = n[2] - 1;
	}
	pm->RebuildMesh();
	GSurfaceMeshObject* po = new GSurfaceMeshObject(pm);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	return true;
}
