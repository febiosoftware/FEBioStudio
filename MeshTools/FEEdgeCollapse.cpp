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
#include "FEEdgeCollapse.h"
#include <MeshLib/FSSurfaceMesh.h>

FEEdgeCollapse::FEEdgeCollapse() : FESurfaceModifier("Edge Collapse")
{
	m_tol = 0.01;
	AddDoubleParam(m_tol, "tolerance", "tolerance");
}

FSSurfaceMesh* FEEdgeCollapse::Apply(FSSurfaceMesh* pm)
{
	// make sure this is a tri mesh
	if (pm->IsType(FE_FACE_TRI3) == false) return 0;

	// create a copy of this mesh
	FSSurfaceMesh* mesh = new FSSurfaceMesh(*pm);

	// first, let's tag all nodes based on whether they are:
	// 0: free
	// 1: edge
	// 2: corner
	// (later, nodes that will be deleted will be tag as < 0)
	mesh->TagAllNodes(0);
	int NN = mesh->Nodes();
	int NE = mesh->Edges();
	for (int i=0; i<NE; ++i)
	{
		FSEdge& e = mesh->Edge(i);
		mesh->Node(e.n[0]).m_ntag = 1;
		mesh->Node(e.n[1]).m_ntag = 1;
	}
	for (int i=0; i<NN; ++i)
	{
		FSNode& node = mesh->Node(i);
		if (node.m_gid != -1) node.m_ntag = 2;
	}

	// find the largest edge length
	double L2max = 0.0;
	int NF = mesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = mesh->Face(i);
		for (int j = 0; j<3; ++j)
		{
			int j0 = face.n[j];
			int j1 = face.n[(j + 1) % 3];
			FSNode& n0 = mesh->Node(j0);
			FSNode& n1 = mesh->Node(j1);
			vec3d r0 = n0.pos();
			vec3d r1 = n1.pos();

			double L2 = (r1 - r0).SqrLength();
			if (L2 > L2max) L2max = L2;
		}
	}

	// mark all faces
	// (later, faces that need to be deleted will be tagged -1)
	mesh->TagAllFaces(0);

	// we'll be calculating the square of lenghts, so we need the square of tol
	m_tol = GetFloatValue(0);
	double tol2 = m_tol*m_tol * L2max;

	// loop over all elements
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = mesh->Face(i);

		for (int j=0; j<3; ++j)
		{
			int j0 = face.n[j];
			int j1 = face.n[(j+1)%3];
			FSNode& n0 = mesh->Node(j0);
			FSNode& n1 = mesh->Node(j1);

			// make sure neither node was processed
			if ((n0.m_ntag >= 0) && (n1.m_ntag >= 0))
			{
				vec3d r0 = n0.pos();
				vec3d r1 = n1.pos();

				double d = (r1 - r0).SqrLength();
				if (d <= tol2)
				{
					if ((n0.m_ntag == n1.m_ntag) && (n0.m_ntag != 2))
					{
						// we can delete either node, so let's pick one and move it to the center
						n0.r = (r0 + r1)*0.5;
						n1.m_ntag = -j0-1;
					}
					else if (n0.m_ntag < n1.m_ntag)
					{
						// node 0 can move
						n0.m_ntag = -j1-1;
					}
					else if (n1.m_ntag < n0.m_ntag)
					{
						// node 1 can move
						n1.m_ntag = -j0-1;
					}

					// tag this face for deletion
					face.m_ntag = -1;

					// also tag the neighbor for deletion
					if (face.m_nbr[j] >= 0) mesh->Face(face.m_nbr[j]).m_ntag = -1;

					break;
				}
			}
		}
	}

	// reindex the nodes
	std::vector<int> index(NN, -1);
	int n = 0;
	for (int i = 0; i<NN; ++i)
	{
		int tag = mesh->Node(i).m_ntag;
		if (tag >= 0) index[i] = n++;
	}
	bool bok = false;
	while (bok == false)
	{
		bok = true;
		for (int i = 0; i<NN; ++i)
		{
			int tag = mesh->Node(i).m_ntag;
			if (tag < 0)
			{
				int m = -tag - 1;
				if (index[m] != -1)
				{
					index[i] = index[m];
				}
				else bok = false;
			}
		}
	}

	// reindex edges
	// (this also markes edges for deletion)
	for (int i=0; i<NE; ++i)
	{
		FSEdge& edge = mesh->Edge(i);
		edge.m_ntag = 0;
		edge.n[0] = index[edge.n[0]];
		edge.n[1] = index[edge.n[1]];

		// if the edge has collapsed, mark it for deletion
		if (edge.n[0] == edge.n[1]) edge.m_ntag = -1;
	}

	// reindex elements
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = mesh->Face(i);
		if (face.m_ntag >= 0)
		{
			face.n[0] = index[face.n[0]];
			face.n[1] = index[face.n[1]];
			face.n[2] = index[face.n[2]];
		}
	}

	// For some reason, the preceding algorithm generates invalid elements.
	// I'm not sure why, but I'll need to fix this. For now, we'll mark any invalid triangle for deletion
	for (int i = 0; i<NF; ++i)
	{
		FSFace& face = mesh->Face(i);
		int* n = face.n;
		if ((n[0] == n[1]) || (n[1] == n[2]) || (n[0] == n[2])) face.m_ntag = -1;
	}

	// reindex faces
	for (int i = 0; i<NF; ++i)
	{
		FSFace& f = mesh->Face(i);
		if (f.m_ntag >= 0)
		{
			f.n[0] = index[f.n[0]];
			f.n[1] = index[f.n[1]];
			f.n[2] = index[f.n[2]];
		}
	}

	// shrink nodes
	n = 0;
	for (int i=0; i<NN; ++i)
	{
		if (mesh->Node(i).m_ntag >= 0)
		{
			if (n != i)
			{
				mesh->Node(n) = mesh->Node(i);
			}
			n++;
		}
	}
	mesh->ResizeNodes(n);

	// shrink faces
	n = 0;
	for (int i = 0; i<NF; ++i)
	{
		if (mesh->Face(i).m_ntag >= 0)
		{
			if (i != n) mesh->Face(n) = mesh->Face(i);
			n++;
		}
	}
	mesh->ResizeFaces(n);

	// shrink edges
	n = 0;
	for (int i = 0; i<NE; ++i)
	{
		if (mesh->Edge(i).m_ntag >= 0)
		{
			if (i != n) mesh->Edge(n) = mesh->Edge(i);
			n++;
		}
	}
	mesh->ResizeEdges(n);

	// we need to rebuild the face neighbors
	mesh->UpdateFaces();

	return mesh;
}
