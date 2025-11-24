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
#include "FESmoothSurfaceMesh.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSNodeNodeList.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <MeshLib/MeshTools.h>

FESmoothSurfaceMesh::FESmoothSurfaceMesh() : FESurfaceModifier("Smooth")
{
	AddIntParam(1, "iterations");
	AddDoubleParam(0.0, "lambda");
	AddBoolParam(false, "preserve shape");
	AddBoolParam(false, "preserve edges");
    AddBoolParam(false, "selection only");
}

FSSurfaceMesh* FESmoothSurfaceMesh::Apply(FSSurfaceMesh* pm)
{
	// create a copy of the mesh
	FSSurfaceMesh* newMesh = new FSSurfaceMesh(*pm);

	// apply smoothing
	bool bshape = GetBoolValue(2);
	bool bedge = GetBoolValue(3);
    bool bselo = GetBoolValue(4);
	ShapeSmoothMesh(*newMesh, *pm, bshape, bedge, bselo);

	// all done
	return newMesh;
}

void FESmoothSurfaceMesh::ShapeSmoothMesh(FSSurfaceMesh& mesh, const FSSurfaceMesh& backMesh, bool preserveShape, bool preserveEdges, bool selectionOnly)
{
	int niter = GetIntValue(0);
	double w = GetFloatValue(1);
	int N = mesh.Nodes();

	std::vector<int> faceIDs(N, -1);
    
	// smooth node positions
	for (int n = 0; n<niter; ++n)
	{
		// tag all selected nodes
		mesh.TagAllNodes(1);
		if (selectionOnly) {
			for (int i = 0; i < mesh.Faces(); ++i) {
				FSFace& face = mesh.Face(i);
				if (!face.IsSelected()) {
					for (int j = 0; j < face.Nodes(); ++j) {
						FSNode& node = mesh.Node(face.n[j]);
						node.m_ntag = 0;
					}
				}
			}
		}

		// clear tags
		// first = count of how often a node was visited
		// second = ID (edge or face) that the nodes should be back-projected to
		std::vector< std::pair<int, int> > tag(N, std::pair<int, int>(0, -1));

		// storage for new node positions
		std::vector<vec3d> newPos(N, vec3d(0, 0, 0));

		// tag all immovable nodes
		if (preserveShape || preserveEdges || selectionOnly)
		{
			for (int i = 0; i<N; ++i)
				if ((mesh.Node(i).m_gid >= 0) || (mesh.Node(i).m_ntag == 0))
				{
					newPos[i] = mesh.Node(i).r;
					tag[i].first = -1;
					tag[i].second = mesh.Node(i).m_gid;
				}
		}

		if (preserveEdges)
		{
			// lock all edges nodes
			for (int i = 0; i<mesh.Edges(); ++i)
			{
				FSEdge& edge = mesh.Edge(i);
				if (edge.m_gid >= 0)
				{
					int ne = edge.Nodes();
					for (int j=0; j<ne; ++j) 
					{
						tag[edge.n[j]].first = -1;
					}
				}
			}

			for (int i=0; i<N; ++i)
			{
				if (tag[i].first == -1)
				{
					newPos[i] = mesh.Node(i).r;
				}
			}
		}

		// process edge nodes
		if ((preserveEdges == false) && (preserveShape == true))
		{
			for (int i = 0; i<mesh.Edges(); ++i)
			{
				FSEdge& edge = mesh.Edge(i);
				if (edge.m_gid >= 0)
				{
					int ne = edge.Nodes();
					for (int j = 0; j<ne; ++j)
					{
						vec3d& rj = mesh.Node(edge.n[j]).r;
						for (int k = 0; k<ne; ++k)
						{
							int nk = edge.n[k];
							if (tag[nk].first != -1)
							{
								newPos[nk] += rj;
								tag[nk].first++;

								assert((tag[nk].second == -1) || (tag[nk].second == edge.m_gid));
								tag[nk].second = edge.m_gid;
							}
						}
					}
				}
			}
			for (int i = 0; i<N; ++i)
			{
				if (tag[i].first > 0)
				{
					newPos[i] /= (double)tag[i].first;

					// project the node back to the edge
					newPos[i] = projectToEdge(backMesh, newPos[i], tag[i].second);

					tag[i].first = -1;
				}
			}
		}

		// process face nodes
		for (int i = 0; i<mesh.Faces(); ++i)
		{
			FSFace& face = mesh.Face(i);
			int nf = face.Nodes();
			for (int j = 0; j<nf; ++j)
			{
				vec3d& rj = mesh.Node(face.n[j]).r;
				for (int k = 0; k<nf; ++k)
				{
					int nk = face.n[k];
					if (tag[nk].first != -1)
					{
						newPos[nk] += rj;
						tag[nk].first++;

						tag[nk].second = face.m_gid;
					}
				}
			}
		}
		for (int i = 0; i<N; ++i)
		{
			if (tag[i].first > 0)
			{
				newPos[i] /= (double)tag[i].first;

				if (preserveShape)
				{
					// project the node back to the surface
					assert(tag[i].second >= 0);

					if (faceIDs[i] != -1)
					{
						newPos[i] = projectToPatch(backMesh, newPos[i], tag[i].second, faceIDs[i], 2);
					}
					else
						newPos[i] = projectToSurface(backMesh, newPos[i], tag[i].second, &faceIDs[i]);
				}

				tag[i].first = -1;
			}
		}

		// assign new node positions
		for (int i = 0; i<N; ++i)
		{
			FSNode& ni = mesh.Node(i);
			if (tag[i].first == -1)
			{
				vec3d& vi = newPos[i];
				ni.r = ni.r*w + vi*(1.0 - w);
			}
		}
	}
}
