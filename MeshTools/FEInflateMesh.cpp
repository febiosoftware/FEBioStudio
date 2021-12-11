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
#include "FEExtrudeFaces.h"
#include <MeshLib/FEMeshBuilder.h>
#include <MeshLib/FENodeNodeList.h>

FEInflateMesh::FEInflateMesh() : FEModifier("Inflate")
{
	AddDoubleParam(1, "distance");
	AddIntParam(1, "segments");
	AddDoubleParam(1, "mesh bias");
	AddBoolParam(false, "symmetric mesh bias");
}

FSMesh* FEInflateMesh::Apply(FSMesh* pm)
{
	// First, create a temp mesh that will be squised
	FSMesh tmp(*pm);

	// shrink the mesh
	ShrinkMesh(tmp);

	// Apply extrusion
	double d = GetFloatValue(0);
	int nseg = GetIntValue(1);
	double rbias = GetFloatValue(2);
	bool symmBias = GetBoolValue(3);
	FEExtrudeFaces extrude;
	extrude.SetExtrusionDistance(d);
	extrude.SetUseNormalLocal(true);
	extrude.SetSegments(nseg);
	extrude.SetMeshBiasFactor(rbias);
	extrude.SetSymmetricBias(symmBias);
	FSMesh* newMesh = extrude.Apply(&tmp);

	// all done, good to go
	return newMesh;
}

// TODO: The algorithm above could invert elements. We should try to prevent this
void FEInflateMesh::ShrinkMesh(FSMesh& mesh)
{
	// figure out which nodes to move
	mesh.TagAllNodes(-1);
	int nsel = 0;
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FEFace& face = mesh.Face(i);
		if (face.IsSelected())
		{
			nsel++;
			int nn = face.Nodes();
			for (int j = 0; j < nn; ++j) mesh.Node(face.n[j]).m_ntag = 1;
		}
	}

	// count the number of tagged nodes
	int NN0 = mesh.Nodes();
	vector<int> taggedNodes;
	for (int i = 0; i < NN0; ++i)
	{
		FENode& node = mesh.Node(i);
		if (node.m_ntag == 1)
		{
			node.m_ntag = taggedNodes.size();
			taggedNodes.push_back(i);
		}
	}

	// setup the extrusion directions
	vector<vec3d> normal(taggedNodes.size());
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FEFace& face = mesh.Face(i);
		if (face.IsSelected())
		{
			int nn = face.Nodes();
			for (int j = 0; j < nn; ++j)
			{
				vec3d fn = to_vec3d(face.m_nn[j]);
				int ntag = mesh.Node(face.n[j]).m_ntag; assert(ntag >= 0);
				normal[ntag] += fn;
			}
		}
	}
	for (int i = 0; i < taggedNodes.size(); ++i) normal[i].Normalize();

	// setup nodal-displacement vector
	vector<vec3d> displacement(NN0, vec3d(0, 0, 0));

	// re-position tagged nodes
	double d = GetFloatValue(0);
	for (int i = 0; i < taggedNodes.size(); ++i)
	{
		FENode& node = mesh.Node(taggedNodes[i]);
		displacement[taggedNodes[i]] = -normal[i] * d;
	}

	// smooth the nodal displacements
	FENodeNodeList NNL(&mesh, true);
	mesh.TagAllNodes(0);
	for (int i = 0; i < taggedNodes.size(); ++i) mesh.Node(taggedNodes[i]).m_ntag = 1;

	const int MAX_ITER = 100;
	double eps = 0.001*d;
	for (int n = 0; n < MAX_ITER; ++n)
	{
		double dmax = 0;
		for (int i = 0; i < mesh.Nodes(); ++i)
		{
			FENode& nodei = mesh.Node(i);
			if (nodei.m_ntag == 0)
			{
				int nn = NNL.Valence(i);
				if (nn != 0)
				{
					vec3d u0 = displacement[i];

					vec3d ua(0, 0, 0);
					for (int j = 0; j < nn; ++j)
					{
						int nj = NNL.Node(i, j);
						vec3d uj = displacement[nj];
						ua += uj;
					}
					ua /= nn;

					double d = (u0 - ua).SqrLength();
					if (d > dmax) dmax = d;

					displacement[i] = ua;
				}
			}
		}
		dmax = sqrt(dmax);

#ifdef _DEBUG
		fprintf(stderr, "%d - dmax = %lg\n", n, dmax);
#endif
		if (dmax < eps) break;
	}

	// keep original positions
	vector<vec3d> r0(mesh.Nodes());
	for (int i = 0; i < mesh.Nodes(); ++i) r0[i] = mesh.Node(i).r;

	// calculate the initial element volume
	vector<double> elemVol(mesh.Elements(), 0.0);
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		elemVol[i] = mesh.ElementVolume(i);
	}

	// apply nodal displacements
	for (int i = 0; i < mesh.Nodes(); ++i)
	{
		FENode& node = mesh.Node(i);
		node.r += displacement[i];
	}

	// tag nodes, so we know which nodes cannot be moved
	mesh.TagAllNodes(0);
	for (int i = 0; i < mesh.Nodes(); ++i)
		if (mesh.Node(i).m_gid >= 0) mesh.Node(i).m_ntag = 1;
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FEEdge& edge = mesh.Edge(i);
		if (edge.m_gid >= 0)
		{
			int nn = edge.Nodes();
			for (int j = 0; j < nn; ++j) mesh.Node(edge.n[j]).m_ntag = 1;
		}
	}
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FEFace& face = mesh.Face(i);
		if (face.m_gid >= 0)
		{
			int nn = face.Nodes();
			for (int j = 0; j < nn; ++j) mesh.Node(face.n[j]).m_ntag = 1;
		}
	}

	// this may have created some bad or even inverted elements, so check for that
	// TODO: Not sure if this actually works.
	double alpha = 0.5;
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FEElement& el = mesh.Element(i);
		double Vi = mesh.ElementVolume(el);
		double J0 = Vi / elemVol[i];
		if (el.IsType(FE_TET4) && ((Vi < 0) || (J0 < 0.001)))
		{
			vec3d ud[4];
			for (int j = 0; j < 4; ++j) ud[j] = displacement[el.m_node[j]];

			vec3d uc = (ud[0] + ud[1] + ud[2] + ud[3])*0.25;
			
			for (int j = 0; j < 4; ++j)
			{
				if (mesh.Node(el.m_node[j]).m_ntag == 0)
					displacement[el.m_node[j]] = ud[j] * alpha + uc * (1.0 - alpha);
			}

			for (int j = 0; j < 4; ++j)
			{
				mesh.Node(el.m_node[j]).r = r0[el.m_node[j]] + displacement[el.m_node[j]];
			}

			Vi = mesh.ElementVolume(el);
			assert(Vi > 0);
			double J1 = Vi / elemVol[i];

			SetError("element %d: old J = %lg, new J = %lg", i+1, J0, J1);
		}
	}
}
