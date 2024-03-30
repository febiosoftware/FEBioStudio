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

#include "PLYImport.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GModel.h>

PLYImport::PLYImport(FSProject& prj) : FSFileImport(prj)
{
	m_mesh = nullptr;
}

PLYImport::~PLYImport()
{
}

bool PLYImport::Load(const char* szfile)
{
	// make sure the mesh pointer is reset
	m_mesh = nullptr;

	FSModel& fem = m_prj.GetFSModel();

	if (read_file(szfile) == false)
	{
		if (m_mesh) delete m_mesh;
		return false;
	}

	assert(m_mesh);
	m_mesh->BuildMesh();
	GSurfaceMeshObject* po = new GSurfaceMeshObject(m_mesh);

	char szname[256];
	FileTitle(szname);
	po->SetName(szname);
	fem.GetModel().AddObject(po);

	m_mesh = nullptr;
	return true;
}

bool PLYImport::read_file(const char* szfile)
{
	if (!Open(szfile, "rb")) return errf("Failed opening file %s.", szfile);

	char szline[256] = {0}, *ch;

	// read the first line
	ch = fgets(szline, 255, m_fp);
	if (ch == 0) return errf("An unexpected error occured while reading the file data.");
	if (strncmp(szline, "ply", 3) != 0) 
	{
		return errf("This is not a valid ply file.");
	}

	// read the next line, this should tell us if this is an ascii or binary file
	bool binary = false;
	ch = fgets(szline, 255, m_fp);
	if (ch == 0) return errf("An unexpected error occured while reading the file data.");
	if (strstr(szline, "format ascii") != 0)
	{
		binary = false;
	}
	else if (strstr(szline, "format binary_little_endian") != 0)
	{
		binary = true;
	}
	else
	{
		return errf("This is not a PLY ascii file.");
	}

	// find vertices and faces
	int verts = 0;
	int vertData = 0;
	int faces = 0;
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return errf("An unexpected error occured while reading the file data.");
		if (strstr(ch, "element vertex") != 0) verts = atoi(ch+14);
		if (strstr(ch, "element face"  ) != 0) faces = atoi(ch+12);
		if ((strstr(ch, "property") != 0) && (verts > 0) && (faces == 0))
		{
			// read the vertex properties
			// (we just need to know the total data size for each vertex)
			if      (strstr(ch + 8, "char"  ) != 0) vertData += 1;
			else if (strstr(ch + 8, "uchar" ) != 0) vertData += 1;
			else if (strstr(ch + 8, "short" ) != 0) vertData += 2;
			else if (strstr(ch + 8, "ushort") != 0) vertData += 2;
			else if (strstr(ch + 8, "int"   ) != 0) vertData += 4;
			else if (strstr(ch + 8, "uint"  ) != 0) vertData += 4;
			else if (strstr(ch + 8, "float" ) != 0) vertData += 4;
			else if (strstr(ch + 8, "double") != 0) vertData += 8;
			else { assert(false); return false; }
		}
	}
	while (strstr(ch, "end_header") == 0);

	// make sure we have vertices and faces
	if (verts == 0) return errf("No vertex data found.");
	if (faces == 0) return errf("No face data found.");

	// allocate a mesh
	m_mesh = new FSSurfaceMesh;
	FSSurfaceMesh& mesh = *m_mesh;
	mesh.Create(verts, 0, faces);

	if (binary == false)
	{
		for (int i = 0; i < verts; ++i)
		{
			ch = fgets(szline, 255, m_fp);
			if (ch == 0) return errf("An unexpected error occured while reading the file data.");
			FSNode& n = mesh.Node(i);
			vec3d& r = n.r;
			sscanf(szline, "%lg%lg%lg", &r.x, &r.y, &r.z);
		}

		for (int i = 0; i < faces; ++i)
		{
			ch = fgets(szline, 255, m_fp);
			if (ch == 0) return errf("An unexpected error occured while reading the file data.");
			FSFace& el = mesh.Face(i);
			int n[5];
			int nread = sscanf(szline, "%d%d%d%d%d", &n[0], &n[1], &n[2], &n[3], &n[4]);

			if (n[0] == 3)
			{
				assert(nread > 3);
				el.SetType(FE_FACE_TRI3);
				el.n[0] = n[1];
				el.n[1] = n[2];
				el.n[2] = n[3];
			}
			else if (n[0] == 4)
			{
				assert(nread > 4);
				el.SetType(FE_FACE_QUAD4);
				el.n[0] = n[1];
				el.n[1] = n[2];
				el.n[2] = n[3];
				el.n[3] = n[4];
			}
			else
			{
				assert(false);
				return false;
			}
		}
	}
	else
	{
		char* buf = new char[vertData];
		for (int i = 0; i < verts; ++i)
		{
			size_t nread = fread(buf, sizeof(char), vertData, m_fp);
			if (nread != vertData) return errf("An unexpected error occured while reading vertex data.");
			float* r = (float*) buf;

			FSNode& n = mesh.Node(i);
			n.r = vec3d(r[0], r[1], r[2]);
		}
		delete[] buf;

		for (int i = 0; i < faces; ++i)
		{
			// read the number of vertices 
			unsigned char vertices;
			fread(&vertices, sizeof(unsigned char), 1, m_fp);

			int n[4];
			int nread = fread(n, sizeof(int), vertices, m_fp);
			if (nread != vertices) return errf("An unexpected error occured while reading element data.");

			FSFace& el = mesh.Face(i);

			if (vertices == 3)
			{
				el.SetType(FE_FACE_TRI3);
				el.n[0] = n[0];
				el.n[1] = n[1];
				el.n[2] = n[2];
			}
			else if (vertices == 4)
			{
				el.SetType(FE_FACE_QUAD4);
				el.n[0] = n[0];
				el.n[1] = n[1];
				el.n[2] = n[2];
				el.n[3] = n[3];
			}
			else
			{
				assert(false);
				return false;
			}
		}
	}

	Close();


	return true;
}
