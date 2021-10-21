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

#include "FESTLExport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>

FESTLExport::FESTLExport(FEProject& prj) : FEFileExport(prj)
{
}

FESTLExport::~FESTLExport(void)
{
}

bool FESTLExport::Write(const char* szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	int i, j, n;

	FEModel* ps = &m_prj.GetFEModel();
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
			FEMeshBase* pm = po->GetEditableMesh();
			if (pm == 0) return errf("Not all objects are meshed.");
			const char* szname = po->GetName().c_str();
			if (strlen(szname) == 0) szname = "object";

			fprintf(fp, "solid %s\n", szname);

			for (i=0; i<pm->Faces(); ++i)
			{
				FEFace& face = pm->Face(i);
				fprintf(fp, "facet normal %g %g %g\n", face.m_fn.x, face.m_fn.y, face.m_fn.z);
				fprintf(fp, "outer loop\n");
				for (j=0; j<face.Nodes(); ++j)
				{
					vec3d& p = pm->Node(face.n[j]).r;
					vec3d r = pm->LocalToGlobal(p);
					fprintf(fp, "vertex %g %g %g\n", r.x, r.y, r.z);
				}
				fprintf(fp, "endloop\n");
				fprintf(fp, "endfacet\n");
			}
			fprintf(fp, "endsolid\n");
		}
	}

	fclose(fp);

	return true;
}
