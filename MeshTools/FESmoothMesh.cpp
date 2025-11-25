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
#include "FEModifier.h"
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSNodeNodeList.h>
#include <MeshLib/MeshTools.h>
using namespace std;

//=============================================================================
// FESmoothMesh
//-----------------------------------------------------------------------------

FESmoothMesh::FESmoothMesh() : FEModifier("Smooth")
{
	AddIntParam(1, "iterations", "iterations");
	AddDoubleParam(0.0, "lambda", "lambda");
	AddBoolParam(false, "preserve shape", "preserve shape");
	AddBoolParam(false, "project", "project");
	AddBoolParam(false, "volume only");
	AddBoolParam(false, "selected elements only");
}

FSMesh* FESmoothMesh::Apply(FSMesh* pm)
{
	// create a copy of the mesh
	FSMesh* pnm = new FSMesh(*pm);

	// apply smoothing
	bool shape = GetBoolValue(2);
	if (shape) ShapeSmoothMesh(*pnm, *pm);
	else SmoothMesh(*pnm);

	// all done
	return pnm;
}

void FESmoothMesh::SmoothMesh(FSMesh& mesh)
{
	int niter = GetIntValue(0);
	double w = GetFloatValue(1);
	bool volOnly = GetBoolValue(4);
	bool selOnly = GetBoolValue(5);

	// set up the node-element table
	FSNodeNodeList NNL(&mesh);

	mesh.TagAllNodes(1);

	if (volOnly)
	{
		int NF = mesh.Faces();
		for (int i=0; i<NF; ++i)
		{
			FSFace& face = mesh.Face(i);
			int nf = face.Nodes();
			for (int j=0; j<nf; ++j) mesh.Node(face.n[j]).m_ntag = 0;
		}
	}

	if (selOnly)
	{
		int NE = mesh.Elements();
		for (int i = 0; i < NE; ++i)
		{
			FSElement& el = mesh.Element(i);
			if (!el.IsSelected())
			{
				int ne = el.Nodes();
				for (int j = 0; j < ne; ++j) mesh.Node(el.m_node[j]).m_ntag = 0;
			}
		}
	}

	// smooth node positions
	int N = mesh.Nodes();
	for (int n = 0; n < niter; ++n)
	{
		vector<vec3d> r(N, vec3d(0, 0, 0));
		for (int i = 0; i < N; ++i)
		{
			FSNode& ni = mesh.Node(i);
			int nn = NNL.Valence(i);
			if ((ni.m_ntag == 1) && (nn > 0))
			{
				vec3d v(0,0,0);
				for (int j = 0; j < nn; ++j)
				{
					FSNode& nj = mesh.Node(NNL.Node(i, j));
					v += nj.r;
				}
				v /= (double)nn;

				r[i] = ni.r*w + v*(1.0 - w);
			}
		}
		for (int i = 0; i < N; ++i)
		{
			FSNode& ni = mesh.Node(i);
			int nn = NNL.Valence(i);
			if ((ni.m_ntag == 1) && (nn > 0))
			{
				ni.r = r[i];
			}
		}
	}
	mesh.UpdateMesh();
}
/*
void FESmoothMesh::ShapeSmoothMesh(FSMesh& mesh, const FSMesh& backMesh)
{
	int niter = GetIntValue(0);
	double w = GetFloatValue(1);
	bool project = GetBoolValue(2);

	// smooth node positions
	int N = mesh.Nodes();
	for (int n = 0; n<niter; ++n)
	{
		// clear tags
		mesh.TagAllNodes(0);
		vector<pair<int, int> > tag(N, pair<int,int>(0,-1));

		// storage for new node positions
		vector<vec3d> newPos(N, vec3d(0,0,0));

		// tag all immovable nodes
		for (int i = 0; i<N; ++i)
			if (mesh.Node(i).m_gid >= 0) 
			{
				newPos[i] = mesh.Node(i).r;
				tag[i].first = 3;
				tag[i].second = mesh.Node(i).m_gid;
			}

		// process edge nodes
		for (int i=0; i<mesh.Edges(); ++i)
		{
			FSEdge& edge = mesh.Edge(i);
			if (edge.m_gid >= 0)
			{
				int ne = edge.Nodes();
				for (int j=0; j<ne; ++j)
				{
					int nj = edge.n[j];
					if (tag[nj].first == 0)
					{
						tag[nj].first = 2;
						assert((tag[nj].second == -1) || (tag[nj].second == edge.m_gid));
						tag[nj].second = edge.m_gid;
					}
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
				int nj = face.n[j];
				if (tag[nj].first == 0)
				{
					tag[nj].first = 1;
					assert((tag[nj].second == -1) || (tag[nj].second == face.m_gid));
					tag[nj].second = face.m_gid;
				}
			}
		}

		// process element nodes
		for (int i = 0; i<mesh.Elements(); ++i)
		{
			FSElement& elem = mesh.Element(i);
			int ne = elem.Nodes();
			for (int j = 0; j<ne; ++j)
			{
				int nj = elem.m_node[j];
				if (tag[nj].first == 0)
				{
					tag[nj].first++;
					assert((tag[nj].second == -1) || (tag[nj].second == elem.m_gid));
					tag[nj].second = elem.m_gid;
				}
			}
		}

		// calculate new node positions
		for (int i=0; i<mesh.Elements(); ++i)
		{
			FSElement& el = mesh.Element(i);
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j)
			{
				vec3d rj = mesh.Node(el.m_node[j]).r;
				for (int k=0; k<ne; ++k)
				{
					int nk = el.m_node[k];
					if (tag[nk].first != 3)
					{
						newPos[nk] += rj;
						mesh.Node(nk).m_ntag++;
					}
				}
			}
		}

		for (int i=0; i<N; ++i)
		{
			FSNode& node = mesh.Node(i);
			if (node.m_ntag > 0)
			{
				newPos[i] /= (double) node.m_ntag;
			}
		}

		if (project)
		{
			for (int i = 0; i<N; ++i)
			{
				if (tag[i].first == 1) newPos[i] = projectToSurface(backMesh, newPos[i], tag[i].second);
				if (tag[i].first == 2) newPos[i] = projectToEdge   (backMesh, newPos[i], tag[i].second);
			}
		}

		// assign new node positions
		for (int i = 0; i<N; ++i)
		{
			FSNode& ni = mesh.Node(i);
			vec3d& vi = newPos[i];
			ni.r = ni.r*w + vi*(1.0 - w);
		}
	}
}

*/

void FESmoothMesh::ShapeSmoothMesh(FSMesh& mesh, const FSMesh& backMesh)
{
	int niter = GetIntValue(0);
	double w = GetFloatValue(1);
	bool project = GetBoolValue(3);
	bool volOnly = GetBoolValue(4);

	// smooth node positions
	int N = mesh.Nodes();
	for (int n = 0; n<niter; ++n)
	{
		// clear tags
		vector<pair<int, int> > tag(N, pair<int,int>(0,-1));

		// storage for new node positions
		vector<vec3d> newPos(N, vec3d(0,0,0));

		// tag all immovable nodes
		for (int i = 0; i<N; ++i)
			if (mesh.Node(i).m_gid >= 0) 
			{
				newPos[i] = mesh.Node(i).r;
				tag[i].first = -1;
				tag[i].second = mesh.Node(i).m_gid;
			}

		// process edge nodes
		for (int i=0; i<mesh.Edges(); ++i)
		{
			FSEdge& edge = mesh.Edge(i);
			if (edge.m_gid >= 0)
			{
				int ne = edge.Nodes();
				for (int j=0; j<ne; ++j)
				{
					vec3d& rj = mesh.Node(edge.n[j]).r;
					for (int k=0; k<ne; ++k)
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
		for (int i=0; i<N; ++i)
		{
			if (tag[i].first > 0)
			{
				newPos[i] /= (double) tag[i].first;

				// project the node back to the edge
				if (project)
				{
					newPos[i] = projectToEdge(backMesh, newPos[i], tag[i].second);
				}

				tag[i].first = -1;
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
				for (int k=0; k<nf; ++k)
				{
					int nk = face.n[k];
					if (tag[nk].first != -1)
					{
						newPos[nk] += rj;
						tag[nk].first++;

						assert((tag[nk].second == -1) || (tag[nk].second == face.m_gid));
						tag[nk].second = face.m_gid;
					}
				}
			}
		}
		for (int i = 0; i<N; ++i)
		{
			if (tag[i].first > 0)
			{
				newPos[i] /= (double) tag[i].first;

				// project the node back to the surface
				if (project)
				{
					assert(tag[i].second >= 0);
					newPos[i] = projectToSurface(backMesh, newPos[i], tag[i].second);
				}

				tag[i].first = -1;
			}
		}

		// process element nodes
		for (int i = 0; i<mesh.Elements(); ++i)
		{
			FSElement& elem = mesh.Element(i);
			int ne = elem.Nodes();
			for (int j = 0; j<ne; ++j)
			{
				vec3d& rj = mesh.Node(elem.m_node[j]).r;
				for (int k = 0; k<ne; ++k)
				{
					int nk = elem.m_node[k];
					if (tag[nk].first != -1)
					{
						newPos[nk] += rj;
						tag[nk].first++;

						assert((tag[nk].second == -1) || (tag[nk].second == elem.m_gid));
						tag[nk].second = elem.m_gid;
					}
				}
			}
		}
		for (int i = 0; i<N; ++i)
		{
			if (tag[i].first > 0)
			{
				newPos[i] /= (double) tag[i].first;
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
