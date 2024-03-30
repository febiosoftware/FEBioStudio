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

#include "ViewpointExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>

ViewpointExport::ViewpointExport(FSProject& prj) : FSFileExport(prj)
{
}

ViewpointExport::~ViewpointExport(void)
{
}

bool ViewpointExport::Write(const char* szfile)
{
	int m, n, k;
	// the file name should just be the base name of the coordinate
	// and element file. So we make seperate filenames for each
	// of the two files.
	char szfelm[512] = {0}, szfcor[512] = {0};
	sprintf(szfelm, "%s.elm", szfile);	// element file name
	sprintf(szfcor, "%s.cor", szfile);	// coordinate file name

	// write coordinates file
	FILE* fp = fopen(szfcor, "wt");
	if (fp == 0) return false;

	FSModel* ps = &m_prj.GetFSModel();
	GModel& model = ps->GetModel();

	int nn = 1;
	for (m=0; m<model.Objects(); ++m)
	{
		FSMesh* pm = model.Object(m)->GetFEMesh();
		for (n=0; n<pm->Nodes(); ++n, ++nn)
		{
			FSNode& node = pm->Node(n);
			node.m_ntag = nn;
			vec3d& r = node.r;

			fprintf(fp, "%d, %lg, %lg, %lg\n", nn, r.x, r.y, r.z);
		}
	}

	fclose(fp);

	// write element connectivity file
	fp = fopen(szfelm, "wt");
	if (fp == 0) return false;

	int ne = 1;
	char szdefault[] = "part";
	for (m=0; m<model.Objects(); ++m)
	{
		GObject* po = model.Object(m);
		FSMesh* pm = po->GetFEMesh();
		const char* sz = po->GetName().c_str();
		if (strlen(sz) == 0) sz = szdefault;
		for (n=0; n<pm->Elements(); ++n, ++ne)
		{
			FSElement& el = pm->Element(n);
			int* l = el.m_node;
			fprintf(fp, "%s", sz);
			for (k=0; k<el.Nodes(); ++k) fprintf(fp, " %d", pm->Node(l[k]).m_ntag);
			fprintf(fp, "\n");
		}
	}

	fclose(fp);

	return true;
}
