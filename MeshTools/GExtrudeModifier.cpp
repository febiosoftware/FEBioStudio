/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "GModifier.h"
#include <GeomLib/GObject.h>
#include "FETetGenMesher.h"

//-----------------------------------------------------------------------------
GExtrudeModifier::GExtrudeModifier()
{
	AddDoubleParam(1.0, "distance", "distance");
}

//-----------------------------------------------------------------------------
void GExtrudeModifier::Apply(GObject* po)
{
	int i;
	vec3d t(0,0,1);
	double d = GetFloatValue(DIST);

	// duplicate all the nodes
	int N = po->Nodes();
	for (i=0; i<N; ++i)
	{
		GNode& n = *po->Node(i);
		po->AddNode(n.LocalPosition() + t*d, n.Type(), true);
	}

	for (i=0; i<N; ++i) po->Node(i)->m_ntag = -1;

	// duplicate all the edges
	int E = po->Edges();
	for (i=0; i<E; ++i)
	{
		GEdge& e = *po->Edge(i);
		po->Node(e.m_node[0])->m_ntag = 1;
		po->Node(e.m_node[1])->m_ntag = 1;

		switch (e.m_ntype)
		{
		case EDGE_LINE:
			po->AddLine(e.m_node[0] + N, e.m_node[1] + N);
			break;
		case EDGE_3P_CIRC_ARC:
			po->AddCircularArc(e.m_cnode + N, e.m_node[0] + N, e.m_node[1] + N);
			break;
		default:
			assert(false);
		}
	}

	// add vertical edges
	int m = 2*E;
	for (i=0; i<N; ++i)
	{
		GNode& node = *po->Node(i);
		if (node.m_ntag == 1)
		{
			node.m_ntag = m++;
			po->AddLine(i, i + N);
		}
	}

	// create new top-surfaces
	int F = po->Faces();
	vector<int> node;
	vector<pair<int, int> > edge;
	for (i=0; i<F; ++i)
	{
		GFace& f = *po->Face(i);
		int nn = f.m_node.size();
		node.resize(nn);
		for (int j=0; j<nn; ++j) node[j] = f.m_node[j]+N;

		int ne = f.m_edge.size();
		edge.resize(ne);
		for (int j=0; j<ne; ++j) 
		{
			edge[j].first = f.m_edge[j].nid + E;
			edge[j].second = f.m_edge[j].nwn;
		}

		po->AddFacet(node, edge, FACE_POLYGON);
	}

	// create new side-walls
	node.resize(4);
	edge.resize(4);
	for (i=0; i<E; ++i)
	{
		GEdge& e = *po->Edge(i);
		int n0 = e.m_node[0];
		int n1 = e.m_node[1];
		int m0 = po->Node(n0)->m_ntag; assert(m0 != -1);
		int m1 = po->Node(n1)->m_ntag; assert(m1 != -1);
		node[0] = n0;
		node[1] = n1;
		node[2] = n1 + N;
		node[3] = n0 + N;

		// TODO: I don't think the winding is correct
		//       I should probably copy the winding from the bottom and top edges
		edge[0].first =     i; edge[0].second =  1;
		edge[1].first =    m1; edge[1].second =  1;
		edge[2].first = E + i; edge[2].second = -1;
		edge[3].first =    m0; edge[3].second = -1;
		po->AddFacet(node, edge, FACE_EXTRUDE);
	}

	// create the parts
	// we should already have one part
	assert(po->Parts()==1);

	// but we should have one for each face
	for (i=1; i<F; ++i) po->AddPart();

	// Now, we need to figure out which faces belong to which parts
	// The bottom and top faces belong to the corresponding part
	for (i=0; i<F; ++i)
	{
		GFace& f0 = *po->Face(i);
		GFace& f1 = *po->Face(i+F);

		f0.m_nPID[0] = i;
		f0.m_nPID[1] = -1;

		f1.m_nPID[0] = i;
		f1.m_nPID[1] = -1;
	}

	// The side walls will be trickier since they can be interior faces
	// If so, we need to find the two parts that they belong to.
	for (i=0; i<E; ++i)
	{
		GFace& f = *po->Face(2*F + i);
		f.m_nPID[0] = -1;
		f.m_nPID[1] = -1;
		int m = 0;
		for (int j=0; j<F; ++j)
		{
			GFace& fj = *po->Face(j);
			if (fj.HasEdge(i))
			{
				assert(m<2);
				f.m_nPID[m++] = fj.m_nPID[0];
			}
		}
		assert(f.m_nPID[0] != -1);
	}

	// find all vertices
	po->UpdateNodeTypes();
}

//-----------------------------------------------------------------------------
GMesh* GExtrudeModifier::BuildGMesh(GObject* po)
{
	po->GObject::BuildGMesh();
	return 0;
}

//-----------------------------------------------------------------------------
FEMesh* GExtrudeModifier::BuildFEMesh(GObject* po)
{
	FETetGenMesher tet(po);
	tet.SetFloatValue(FETetGenMesher::ELSIZE, 0.2);
	return tet.BuildMesh();
}
