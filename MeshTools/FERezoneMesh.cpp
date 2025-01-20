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
#include "FEModifier.h"
#include <MeshLib/MeshTools.h>
#include <MeshLib/FSFaceEdgeList.h>

FERezoneMesh::FERezoneMesh() : FEModifier("Rezone")
{
	AddDoubleParam(1, "strength");
	AddDoubleParam(0.5, "extent");
}

FSMesh* FERezoneMesh::Apply(FSMesh* pm)
{
	// see if any faces are selected. 
	if (pm->CountSelectedFaces() == 0) return nullptr;

	// Identify node types:
	// - 0 = internal node, can move in any direction
	// - 1 = surface node, has to remain on surface
	// - 2 = edge node, has to remain on edge
	// - 3 = vertex or node on selected facet, cannot move
	int N = pm->Nodes();
	std::vector< std::pair<int, int> > tag(N, std::pair<int, int>(0, -1));
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		for (int j = 0; j < face.Nodes(); ++j)
		{
			tag[face.n[j]].first = 1;
			tag[face.n[j]].second = face.m_gid;
		}
	}
	for (int i = 0; i < pm->Edges(); ++i)
	{
		FSEdge& edge = pm->Edge(i);
		if (edge.m_gid >= 0)
		{
			for (int j = 0; j < edge.Nodes(); ++j)
			{
				tag[edge.n[j]].first = 2;
				tag[edge.n[j]].second = edge.m_gid;
			}
		}
	}
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& node = pm->Node(i);
		if (node.m_gid >= 0)
		{
			tag[i].first = 3;
			tag[i].second = -1;
		}
	}
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.IsSelected())
		{
			for (int j = 0; j < face.Nodes(); ++j)
			{
				tag[face.n[j]].first = 3;
				tag[face.n[j]].second = -1;
			}
		}
	}

	// get the selected surface
	FSMesh* surf = pm->ExtractFaces(true);

	// for each node, find closest point to a selected facet
	std::vector<vec3d> trg(N);
	for (int i = 0; i < N; ++i)
	{
		FSNode& node = pm->Node(i);
		if (tag[i].first != 3)
		{
			vec3d q = projectToSurface(*surf, node.r);
			trg[i] = q;
		}
	}

	BOX box = pm->GetBoundingBox();
	double R = box.GetMaxExtent();

	double w = GetFloatValue(0);
	double t = GetFloatValue(1) / R;

	// create a copy of the mesh
	FSMesh* newMesh = new FSMesh(*pm);

	// move the nodes
	for (int i = 0; i < N; ++i)
	{
		FSNode& node = newMesh->Node(i);

		int ntype = tag[i].first;
		if (ntype != 3)
		{
			vec3d r0 = node.r;

			double l = (trg[i] - r0).Length();

			double f = 1.0 - exp(-pow(l / t, w));
			vec3d e = trg[i] - r0; e.Normalize();

			vec3d r = r0 + e*(l - l*f);

			if (ntype == 1)
			{
				int faceId = tag[i].second;
				vec3d q = projectToSurface(*pm, r, faceId);
				r = q;
			}
			else if (ntype == 2)
			{
				int edgeId = tag[i].second;
				vec3d q = projectToEdge(*pm, r, edgeId);
				r = q;
			}

			node.r = r;
		}
	}

	newMesh->UpdateMesh();

	return newMesh;
}
