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

#include "BYUExport.h"
#include <FEMLib/FSProject.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>

BYUExport::BYUExport(FSProject& prj) : FSFileExport(prj)
{
}

BYUExport::~BYUExport(void)
{
}

bool BYUExport::Write(const char* szfile)
{
	int i, j, k, n;

	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	FSModel* ps = &m_prj.GetFSModel();
	GModel& model = ps->GetModel();

	// for now we put everything in one part
	int parts = 1;

	// count total nr of faces
	int faces = model.FEFaces();

	// count nr of face edges
	int edges = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		FSMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return false;
		FSMesh& m = *pm;
		for (j=0; j<m.Nodes(); ++j) m.Node(j).m_ntag = 0;
		for (j=0; j<m.Faces(); ++j)
		{
			FSFace& f = m.Face(j);
			n = f.Nodes();
			edges += n;
			for (k=0; k<n; ++k) m.Node(f.n[k]).m_ntag = 1;
		}
	}

	// count nr of nodes
	int nodes = 0;
	for (i=0; i<model.Objects(); ++i)
	{
		FSMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Nodes(); ++j) if (m.Node(j).m_ntag == 1) m.Node(j).m_ntag = ++nodes;
	}

	// --- H E A D E R ---
	fprintf(fp, "%d %d %d %d\n", parts, nodes, faces, edges);

	// --- P A R T ---
	int nfirst = 1;
	int nlast = faces;
	fprintf(fp, "%d %d\n", nfirst, nlast);

	// --- N O D E S ---
	for (i=0; i<model.Objects(); ++i)
	{
		FSMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Nodes(); ++j)
		{
			FSNode& n = m.Node(j);
			if (n.m_ntag)
				fprintf(fp, "%g %g %g\n", n.r.x, n.r.y, n.r.z);
		}
	}

	// --- E D G E S ---
	for (i=0; i<model.Objects(); ++i)
	{
		FSMesh& m = *model.Object(i)->GetFEMesh();
		for (j=0; j<m.Faces(); ++j)
		{
			FSFace& f = m.Face(j);
			n = f.Nodes();
			if (n == 3)
				fprintf(fp, "%d %d %d\n", m.Node(f.n[0]).m_ntag, m.Node(f.n[1]).m_ntag, -m.Node(f.n[2]).m_ntag);
			else if (n == 4)
				fprintf(fp, "%d %d %d %d\n", m.Node(f.n[0]).m_ntag, m.Node(f.n[1]).m_ntag, m.Node(f.n[2]).m_ntag, -m.Node(f.n[3]).m_ntag);
			else 
				assert(false);
		}
	}

	fclose(fp);

	return true;
}
