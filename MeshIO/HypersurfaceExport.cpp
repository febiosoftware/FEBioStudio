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

#include "HypersurfaceExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>

HypersurfaceExport::HypersurfaceExport(FSProject& prj) : FSFileExport(prj)
{
}

HypersurfaceExport::~HypersurfaceExport(void)
{
}

bool HypersurfaceExport::Write(const char* szfile)
{
	int i, j, k;

	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	// write the header
	fprintf(fp, "# HyperSurface 0.1 ASCII\n\n");
	fprintf(fp, "Parameters {\n");
	fprintf(fp, "\tMaterials {\n");
	fprintf(fp, "\t\tExterior {\n");
	fprintf(fp, "\t\t\tName \"Surface\"\n");
	fprintf(fp, "\t\t\tId 1\n");
	fprintf(fp, "\t\t}\n");
	fprintf(fp, "\t}\n");
	fprintf(fp, "}\n\n");

	FSModel* ps = &m_prj.GetFSModel();
	GModel& model = ps->GetModel();

	// count total nr of faces
	int faces = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsSelected() && po->GetFEMesh())
		{
			FSMesh& m = *po->GetFEMesh();
			int NF = m.Faces();
			for (int i=0; i<NF; ++i)
			{
				if (m.Face(i).Nodes() == 4) faces += 2; else faces += 1;
			}
		}
	}
	if (faces == 0) return false;

	// count nr of surface nodes
	int nodes = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsSelected())
		{
			FSMesh& m = *po->GetFEMesh();
			for (j=0; j<m.Nodes(); ++j) m.Node(j).m_ntag = 0;

			for (j=0; j<m.Faces(); ++j)
			{
				FSFace& f = m.Face(j);
				for (k=0; k<f.Nodes(); ++k) m.Node(f.n[k]).m_ntag = 1;
			}

			for (j=0; j<m.Nodes(); ++j) if (m.Node(j).m_ntag == 1) m.Node(j).m_ntag = ++nodes;
		}
	}

	// write the nodes
	fprintf(fp, "Vertices %d\n", nodes);
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsSelected())
		{
			FSMesh& m = *po->GetFEMesh();
			for (j=0; j<m.Nodes(); ++j)
			{
				FSNode& n = m.Node(j);
				vec3d r = po->GetTransform().LocalToGlobal(n.r);
				if (n.m_ntag) fprintf(fp, "%g %g %g\n", r.x, r.y, r.z);
			}
		}
	}

	// write the surface
	fprintf(fp, "Patches 1\n");
	fprintf(fp, "{\n\n");
	fprintf(fp, "Triangles %d\n", faces);

	int nf[4];
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		if (po->IsSelected())
		{
			FSMesh& m = *po->GetFEMesh();
			for (j=0; j<m.Faces(); ++j)
			{
				FSFace& f = m.Face(j);
				if (f.Nodes() == 3)
				{
					nf[0] = m.Node(f.n[0]).m_ntag;
					nf[1] = m.Node(f.n[1]).m_ntag;
					nf[2] = m.Node(f.n[2]).m_ntag;

					fprintf(fp, "%d %d %d\n", nf[0], nf[1], nf[2]);
				}
				else
				{
					nf[0] = m.Node(f.n[0]).m_ntag;
					nf[1] = m.Node(f.n[1]).m_ntag;
					nf[2] = m.Node(f.n[2]).m_ntag;
					nf[3] = m.Node(f.n[3]).m_ntag;

					fprintf(fp, "%d %d %d\n", nf[0], nf[1], nf[2]);
					fprintf(fp, "%d %d %d\n", nf[2], nf[3], nf[0]);
				}
			}
		}
	}

	fprintf(fp, "}\n");

	fclose(fp);

	return true;
}
