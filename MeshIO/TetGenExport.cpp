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

#include "TetGenExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>

TetGenExport::TetGenExport(FSProject& prj) : FSFileExport(prj)
{
}

TetGenExport::~TetGenExport()
{
}

bool TetGenExport::Write(const char *szfile)
{
	// get the model
	FSModel* ps = &m_prj.GetFSModel();
	GModel& model = ps->GetModel();

	char sznode[256] = {0};
	strncpy(sznode, szfile, strlen(szfile));
	char* ch = strrchr(sznode, '.');
	if (ch == 0) ch = sznode+strlen(sznode);
	sprintf(ch, ".node");
	FILE* fp = fopen(sznode, "wt");

	// write the nodes
	fprintf(fp, "%d %d %d %d\n", model.FENodes(), 3, 0, 0);
	int n = 1;
	for (int i=0; i<model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		int NN = pm->Nodes();
		for (int j=0; j<NN; ++j, ++n)
		{
			FSNode& nd = pm->Node(j);
			nd.m_ntag = n;
			fprintf(fp, "%d %lg %lg %lg\n", n, nd.r.x, nd.r.y, nd.r.z);
		}
	}
	fclose(fp);

	// write the elements
	fp = fopen(szfile, "wt");
	fprintf(fp, "%d %d %d\n", model.Elements(), 4, 0);
	n = 1;
	for (int i=0; i<model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		int NE = pm->Elements();
		for (int j=0; j<NE; ++j, ++n)
		{
			FSElement& el = pm->Element(j);
			assert(el.Type() == FE_TET4);
			int m[4];
			m[0] = pm->Node(el.m_node[0]).m_ntag;
			m[1] = pm->Node(el.m_node[1]).m_ntag;
			m[2] = pm->Node(el.m_node[2]).m_ntag;
			m[3] = pm->Node(el.m_node[3]).m_ntag;
			fprintf(fp, "%d %d %d %d %d\n", n, m[0], m[1], m[2], m[3]);
		}
	}
	fclose(fp);
	return true;
}
