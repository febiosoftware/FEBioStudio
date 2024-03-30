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

#include "MeshExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>
#include <memory>
//using namespace std;

MeshExport::MeshExport(FSProject& prj) : FSFileExport(prj)
{
}

MeshExport::~MeshExport(void)
{
}

bool MeshExport::Write(const char* szfile)
{
	FSModel& fem = m_prj.GetFSModel();
	GModel& model = fem.GetModel();

	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	// write the header
	fprintf(fp, "MeshVersionFormatted 2\n\n");
	fprintf(fp, "Dimension 3\n\n\n");

	// export nodes
	fprintf(fp, "Vertices\n");
	fprintf(fp, "%d\n", model.FENodes());
	int i, j, N = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FSMesh* pm = po->GetFEMesh();

		for (j=0; j<pm->Nodes(); ++j, ++N)
		{
			FSNode& node = pm->Node(j);
			node.m_nid = N;
			vec3d r = po->GetTransform().LocalToGlobal(node.r);
			fprintf(fp, "%lg %lg %lg 0\n", r.x, r.y, r.z);
		}
	}
	fprintf(fp, "\n");

	// we can only export hex, tets and triangles, so let's count them first
	int nhex = 0;
	int ntet = 0;
	int ntri = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		for (j=0; j<pm->Elements(); ++j)
		{
			FSElement& e = pm->Element(j);
			if (e.IsType(FE_HEX8)) nhex++;
			if (e.IsType(FE_TET4)) ntet++;
			if (e.IsType(FE_TRI3)) ntri++;
		}
	}

	// export hex elements
	if (nhex > 0)
	{
		fprintf(fp, "Hexahedra\n");
		fprintf(fp, "%d\n", nhex);
		for (i=0; i<model.Objects(); ++i)
		{
			FSMesh* pm = model.Object(i)->GetFEMesh();
			for (j=0; j<pm->Elements(); ++j)
			{
				FSElement& e = pm->Element(j);
				if (e.IsType(FE_HEX8))
				{
					int nn[8];
					for (int k=0; k<8; ++k) nn[k] = pm->Node(e.m_node[k]).m_nid + 1;
					fprintf(fp, "%d %d %d %d %d %d %d %d\n", nn[0], nn[1], nn[2], nn[3], nn[4], nn[5], nn[6], nn[7]);
				}
			}
		}
		fprintf(fp, "\n");
	}

	// export tet elements
	if (ntet > 0)
	{
		fprintf(fp, "Tetrahedra\n");
		fprintf(fp, "%d\n", ntet);
		for (i=0; i<model.Objects(); ++i)
		{
			FSMesh* pm = model.Object(i)->GetFEMesh();
			for (j=0; j<pm->Elements(); ++j)
			{
				FSElement& e = pm->Element(j);
				if (e.IsType(FE_TET4))
				{
					int nn[4];
					for (int k=0; k<4; ++k) nn[k] = pm->Node(e.m_node[k]).m_nid + 1;
					fprintf(fp, "%d %d %d %d %d\n", nn[0], nn[1], nn[2], nn[3], e.m_gid + 1);
				}
			}
		}
		fprintf(fp, "\n");
	}

	// export tri elements
	if (ntri > 0)
	{
		fprintf(fp, "Triangles\n");
		fprintf(fp, "%d\n", ntri);
		int ntag = 1;
		for (i=0; i<model.Objects(); ++i)
		{
			FSMesh* pm = model.Object(i)->GetFEMesh();
			for (j=0; j<pm->Elements(); ++j)
			{
				FSElement& e = pm->Element(j);
				if (e.IsType(FE_TRI3))
				{
					int nn[3];
					for (int k=0; k<3; ++k) nn[k] = pm->Node(e.m_node[k]).m_nid + 1;
					fprintf(fp, "%d\t%d\t%d\t%d\n", nn[0], nn[1], nn[2], ntag);
				}
			}
		}
		fprintf(fp, "\n");
	}

	fprintf(fp, "End\n");

	fclose(fp);

	return true;
}
