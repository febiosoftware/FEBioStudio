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
#include "BYUExport.h"
#include "FEPostModel.h"
#include <stdio.h>

using namespace Post;

BYUExport::BYUExport(void)
{
}

BYUExport::~BYUExport(void)
{
}

bool BYUExport::Save(FEPostModel &fem, const char *szfile)
{
	int i, j, n;

	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	// get the selected mesh
	FSMeshBase& mesh = *fem.GetFEMesh(0);

	// for now we put everything in one part
	int parts = 1;

	// count nr of faces and face edges
	int faces = 0;
	int edges = 0;
	for (i=0; i<mesh.Nodes(); ++i) mesh.Node(i).m_ntag = 0;
	for (i=0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		n = f.Nodes();
		assert(n==3||n==4);
		edges += (n==3? 3 : 6) ;
		faces += (n==3? 1 : 2);
		for (j=0; j<n; ++j) mesh.Node(f.n[j]).m_ntag = 1;
	}

	// count nr of nodes
	int nodes = 0;
	for (i=0; i<mesh.Nodes(); ++i) if (mesh.Node(i).m_ntag == 1) mesh.Node(i).m_ntag = ++nodes;

	// --- H E A D E R ---
	fprintf(fp, "%d %d %d %d\n", parts, nodes, faces, edges);

	// --- P A R T ---
	int nfirst = 1;
	int nlast = faces;
	fprintf(fp, "%d %d\n", nfirst, nlast);

	// --- N O D E S ---
	for (i=0; i<mesh.Nodes(); ++i)
	{
		FENode& n = mesh.Node(i);
		if (n.m_ntag)
		{
			vec3d r = n.pos();
			fprintf(fp, "%g %g %g\n", r.x, r.y, r.z);
		}
	}

	// --- E D G E S ---
	for (i=0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		n = f.Nodes();
		if (n == 3)
			fprintf(fp, "%d %d %d\n", mesh.Node(f.n[0]).m_ntag, mesh.Node(f.n[1]).m_ntag, -mesh.Node(f.n[2]).m_ntag);
		else if (n == 4)
		{
			fprintf(fp, "%d %d %d\n", mesh.Node(f.n[0]).m_ntag, mesh.Node(f.n[1]).m_ntag, -mesh.Node(f.n[2]).m_ntag);
			fprintf(fp, "%d %d %d\n", mesh.Node(f.n[2]).m_ntag, mesh.Node(f.n[3]).m_ntag, -mesh.Node(f.n[0]).m_ntag);
		}
		else
			assert(false);
	}

	fclose(fp);

	return true;
}
