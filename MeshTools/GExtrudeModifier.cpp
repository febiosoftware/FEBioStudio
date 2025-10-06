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
#include "GModifier.h"
#include <GeomLib/GObject.h>
#include "FETetGenMesher.h"

//-----------------------------------------------------------------------------
GExtrudeModifier::GExtrudeModifier()
{
	SetName("Extrude");
	AddDoubleParam(1.0, "distance");
	AddIntParam(1, "segments");
}

//-----------------------------------------------------------------------------
void GExtrudeModifier::Apply(GObject* po)
{
	vec3d t(0,0,1);
	double D = GetFloatValue(DIST);
	int NDIV = GetIntValue(NDIVS);
	if (NDIV < 1) NDIV = 1;

	// duplicate all the nodes
	int N = po->Nodes();
	for (int j = 1; j <= NDIV; ++j)
	{
		double d = (j * D) / NDIV;
		for (int i = 0; i < N; ++i)
		{
			GNode& n = *po->Node(i);
			po->AddNode(n.LocalPosition() + t * d, n.Type(), true);
		}
	}

	for (int i=0; i<N; ++i) po->Node(i)->m_ntag = -1;

	// duplicate all the edges
	int E = po->Edges();
	for (int j = 1; j <= NDIV; ++j)
	{
		for (int i = 0; i < E; ++i)
		{
			GEdge& e = *po->Edge(i);
			po->Node(e.m_node[0])->m_ntag = 1;
			po->Node(e.m_node[1])->m_ntag = 1;

			switch (e.m_ntype)
			{
			case EDGE_LINE:
				po->AddLine(e.m_node[0] + j*N, e.m_node[1] + j * N);
				break;
			case EDGE_3P_CIRC_ARC:
				po->AddCircularArc(e.m_cnode[0] + j * N, e.m_node[0] + j * N, e.m_node[1] + j * N);
				break;
			case EDGE_BEZIER:
				{
					GEdge* newEdge = new GEdge(po);
					newEdge->m_ntype = EDGE_BEZIER;
					newEdge->m_node[0] = e.m_node[0] + j * N;
					newEdge->m_node[1] = e.m_node[1] + j * N;
					newEdge->m_cnode = e.m_cnode;
					for (int k = 0; k < e.m_cnode.size(); ++k) newEdge->m_cnode[k] = e.m_cnode[k] + j*N;
					po->AddEdge(newEdge);
				}
				break;
			default:
				assert(false);
			}
		}
	}

	int C = 0;
	for (int i = 0; i < N; ++i)
	{
		GNode& node = *po->Node(i);
		if (node.m_ntag == 1) C++;
	}

	// add vertical edges
	for (int j = 1; j <= NDIV; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			GNode& node = *po->Node(i);
			if (node.m_ntag == 1)
			{
				po->AddLine(i+ (j-1)*N, i + j*N);
			}
		}
	}

	// create new top-surfaces
	int F = po->Faces();
	std::vector<int> node;
	std::vector< std::pair<int, int> > edge;
	for (int n = 1; n <= NDIV; ++n)
	{
		for (int i = 0; i < F; ++i)
		{
			GFace& f = *po->Face(i);
			int nn = f.m_node.size();
			node.resize(nn);
			for (int j = 0; j < nn; ++j) node[j] = f.m_node[j] + n*N;

			int ne = f.m_edge.size();
			edge.resize(ne);
			for (int j = 0; j < ne; ++j)
			{
				edge[j].first = f.m_edge[j].nid + n*E;
				edge[j].second = f.m_edge[j].nwn;
			}

			po->AddFacet(node, edge, FACE_POLYGON);
		}
	}

	// create new side-walls
	node.resize(4);
	edge.resize(4);
	for (int j = 1; j <=NDIV; ++j)
	{
		for (int i = 0; i < E; ++i)
		{
			GEdge& e = *po->Edge(i);
			int n0 = e.m_node[0];
			int n1 = e.m_node[1];
			node[0] = n0 + (j - 1)*N;
			node[1] = n1 + (j - 1)*N;
			node[2] = n1 + j*N;
			node[3] = n0 + j*N;

			// TODO: I don't think the winding is correct
			//       I should probably copy the winding from the bottom and top edges
			int m0 = E * (NDIV + 1) + (j - 1)*C + i;
			int m1 = E * (NDIV + 1) + (j - 1)*C + (i + 1) % C;
			
			edge[0].first = i + (j-1)*E; edge[0].second = 1;
			edge[1].first = m1; edge[1].second = 1;
			edge[2].first = i + j*E; edge[2].second = -1;
			edge[3].first = m0; edge[3].second = -1;
			po->AddFacet(node, edge, FACE_EXTRUDE);
		}
	}

	// create the parts
	// we should already have one part
	assert(po->Parts()==1);

	// but we should have one for each face
	for (int i=1; i<F*NDIV; ++i) po->AddSolidPart();
	const int NP = po->Parts();

	// Now, we need to figure out which faces belong to which parts
	// The bottom and top faces belong to the corresponding part
	for (int j = 0; j <= NDIV; ++j)
	{
		for (int i = 0; i < F; ++i)
		{
			GFace& f = *po->Face(j*F + i);

			int pid = j * F + i;
			if (pid >= NP) pid -= F;
			f.m_nPID[0] = pid;
			f.m_nPID[1] = ((j == 0) || (j == NDIV) ? -1 : (j - 1)*F + i);
		}
	}

	if (F == 0)
	{
		for (int i = 0; i < po->Faces(); ++i)
		{
			GFace& f = *po->Face(i);
			f.m_nPID[0] = 0;
			f.m_nPID[1] = -1;
		}
	}
	else
	{
		// The side walls will be trickier since they can be interior faces
		// If so, we need to find the two parts that they belong to.
		int nf = F * (NDIV + 1);
		for (int n = 0; n < NDIV; ++n)
		{
			for (int j = 0; j < E; ++j)
			{
				GEdge* pe = po->Edge(n * E + j);
				GFace& f = *po->Face(nf++);
				f.m_nPID[0] = -1;
				f.m_nPID[1] = -1;
				int m = 0;
				for (int k = 0; k < F; ++k)
				{
					GFace& fk = *po->Face(n * F + k);
					if (fk.HasEdge(n * E + j))
					{
						assert(m < 2);
						f.m_nPID[m++] = fk.m_nPID[0];
					}
				}
				assert(f.m_nPID[0] != -1);
			}
		}
	}

	// flip bottom faces
	for (int i = 0; i < F; ++i)
	{
		GFace* face = po->Face(i);
		face->Invert();
	}

	// find all vertices
	po->UpdateNodeTypes();
}

//-----------------------------------------------------------------------------
GLMesh* GExtrudeModifier::BuildGMesh(GObject* po)
{
	po->GObject::BuildGMesh();
	return 0;
}

//-----------------------------------------------------------------------------
FSMesh* GExtrudeModifier::BuildFEMesh(GObject* po)
{
	FETetGenMesher tet(*po);
	tet.SetFloatValue(FETetGenMesher::ELSIZE, 0.2);
	return tet.BuildMesh();
}
