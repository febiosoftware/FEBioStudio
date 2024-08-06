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

#include "STLExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>

void stl_write_face(FILE* fp, const vec3d& fn, const vec3d& r0, const vec3d& r1, const vec3d& r2)
{
	fprintf(fp, "facet normal %g %g %g\n", fn.x, fn.y, fn.z);
	fprintf(fp, "outer loop\n");

	fprintf(fp, "vertex %g %g %g\n", r0.x, r0.y, r0.z);
	fprintf(fp, "vertex %g %g %g\n", r1.x, r1.y, r1.z);
	fprintf(fp, "vertex %g %g %g\n", r2.x, r2.y, r2.z);

	fprintf(fp, "endloop\n");
	fprintf(fp, "endfacet\n");
}

void stl_write_solid(FILE* fp, FSMeshBase* pm, const char* solidName)
{
	fprintf(fp, "solid %s\n", solidName);

	vec3d r[FSFace::MAX_NODES];
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);

		vec3d fn = to_vec3d(face.m_fn);

		for (int j = 0; j < face.Nodes(); ++j)
		{
			vec3d& p = pm->Node(face.n[j]).r;
			r[j] = pm->LocalToGlobal(p);
		}

		vec3d q[3];
		switch (face.Type())
		{
		case FE_FACE_TRI3:
			stl_write_face(fp, fn, r[0], r[1], r[2]);
			break;
		case FE_FACE_QUAD4:
			stl_write_face(fp, fn, r[0], r[1], r[2]);
			stl_write_face(fp, fn, r[2], r[3], r[0]);
			break;
		default:
			assert(false);
		}
	}
	fprintf(fp, "endsolid\n");
}

bool stl_write_surface(FILE* fp, FSSurface* surf)
{
	std::string name = surf->GetName();
	fprintf(fp, "solid %s\n", name.c_str());

	FSMesh* pm = surf->GetMesh();
	if (pm == nullptr) return false;

	vec3d r[FSFace::MAX_NODES];
	int NF = surf->size();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = *surf->GetFace(i);

		vec3d fn = to_vec3d(face.m_fn);

		for (int j = 0; j < face.Nodes(); ++j)
		{
			vec3d& p = pm->Node(face.n[j]).r;
			r[j] = pm->LocalToGlobal(p);
		}

		vec3d q[3];
		switch (face.Type())
		{
		case FE_FACE_TRI3:
			stl_write_face(fp, fn, r[0], r[1], r[2]);
			break;
		case FE_FACE_QUAD4:
			stl_write_face(fp, fn, r[0], r[1], r[2]);
			stl_write_face(fp, fn, r[2], r[3], r[0]);
			break;
		default:
			assert(false);
		}
	}
	fprintf(fp, "endsolid\n");

	return true;
}

STLExport::STLExport(FSProject& prj) : FSFileExport(prj)
{
}

STLExport::~STLExport(void)
{
}

bool STLExport::Write(const char* szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	int n;

	FSModel* ps = &m_prj.GetFSModel();
	GModel& model = ps->GetModel();

	// only the selected object is exported, unless no object is selected, 
	// then we export them all. So, we first need to know how many are selected.
	int nsel = 0;
	for (n=0; n<model.Objects(); ++n)
	{
		GObject* po = model.Object(n);
		if (po->IsSelected()) nsel++;
	}

	// export the selected objects (or all, if none are selected)
	for (n=0; n<model.Objects(); ++n)
	{
		GObject* po = model.Object(n);
		if ((nsel==0) || po->IsSelected())
		{
			FSMeshBase* pm = po->GetEditableMesh();
			if (pm == 0) return errf("Not all objects are meshed.");
			const char* szname = po->GetName().c_str();
			if (strlen(szname) == 0) szname = "object";

			stl_write_solid(fp, pm, szname);
		}
	}
	fclose(fp);

	return true;
}

bool STLExport::Write(const char* szfile, GObject* po)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	FSMeshBase* pm = po->GetEditableMesh();
	if (pm == 0) return errf("Not all objects are meshed.");
	const char* szname = po->GetName().c_str();
	if (strlen(szname) == 0) szname = "object";

	// write the solid
	stl_write_solid(fp, pm, szname);
	
	fclose(fp);

	return true;
}

bool STLExport::Write(const char* szfile, FSMesh* pm)
{
	if (pm == 0) return false;

	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	// write the solid
	stl_write_solid(fp, pm, "marching_cubes_export");

	fclose(fp);

	return true;
}

bool STLExport::Write(const char* szfile, FSSurface* surf)
{
	if (surf == nullptr) return false;
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;
	bool b = stl_write_surface(fp, surf);
	fclose(fp);
	return b;
}
