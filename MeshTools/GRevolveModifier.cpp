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
GRevolveModifier::GRevolveModifier()
{
	AddDoubleParam(360.0, "angle", "Angle");
	AddIntParam(1, "divisions", "Divisions");
}

//-----------------------------------------------------------------------------
//! For now we assume that the axis of revolution is the y-axis
void GRevolveModifier::Apply(GObject* po)
{
	int i, j;
	const double tol = 1.e-6;

	// figure out the divisions
	int D = GetIntValue(DIVS); if (D < 1) D = 1;
	int M = D;
	double wd = GetFloatValue(ANGLE);
	if (wd == 360) M -= 1;

	// find out how many nodes are on either
	// side of the axis
	int N = po->Nodes();
	int nl = 0; // nodes left of axis
	int nr = 0; // nodes right of axis
	for (i=0; i<N; ++i)
	{
		GNode& n = *po->Node(i);
		n.m_ntag = -1;
		double x = n.LocalPosition().x;
		if (x < -tol) nl++;
		else if (x > tol) nr++;
		else
		{
			// snap the node onto the axis
			n.LocalPosition().x = 0;
			n.m_ntag = -2;
		}
	}

	// first make sure that all nodes are on
	// one side of the y-axis
	if ((nl == 0) && (nr == 0))
	{
		assert(false);
		return;
	}
	if ((nl != 0) && (nr != 0))
	{
		assert(false);
		return;
	}

	// create all the new nodes
	// but do not duplicate nodes on the y-axis
	int NN = (M+1)*N;
	vector<int> nn(NN);
	for (i=0; i<N; ++i) nn[i] = i;
	for (i=0; i<M; ++i)
	{
		double w = wd*(i+1.0)/ (double) D;
		double cw = cos(PI*w/180.0);
		double sw = sin(PI*w/180.0);

		for (j=0; j<N; ++j)
		{
			// get the source node's location
			GNode& n = *po->Node(j);
			vec3d& r = n.LocalPosition();

			// rotate the node
			vec3d rp;
			rp.x = cw*r.x - sw*r.z;
			rp.y = r.y;
			rp.z = sw*r.x + cw*r.z;

			// add the new node
			nn[N*(i+1)+j] = po->AddNode(rp, n.Type());
		}
	}

	// create all the new edges
	// first the in-plane edges
	// do not copy edges on the y-axis
	int E = po->Edges();
	int NE = E*(M+1);
	vector<int> ne(NE);
	for (i=0; i<E; ++i) ne[i] = i;
	for (i=0; i<M; ++i)
	{
		for (j=0; j<E; ++j)
		{
			GEdge& e = *po->Edge(i*E + j);
			int n0 = nn[e.m_node[0] + N];
			int n1 = nn[e.m_node[1] + N];

			switch (e.m_ntype)
			{
			case EDGE_LINE       : ne[(i+1)*E + j] = po->AddLine(n0, n1); break;
			case EDGE_3P_CIRC_ARC: ne[(i+1)*E + j] = po->AddCircularArc(nn[e.m_cnode + N], n0, n1); break;
			case EDGE_3P_ARC     : ne[(i+1)*E + j] = po->AddArcSection (nn[e.m_cnode + N], n0, n1); break;
			default:
				assert(false);
			}
		}
	}

	// mark vertices for extrusion
	// only mark the vertices that are not on the axis
	// (they were tagged earlier with -2)
	for (i=0; i<po->Edges(); ++i)
	{
		GEdge& e = *po->Edge(i);
		int n0 = e.m_node[0];
		int n1 = e.m_node[1];
		if (po->Node(n0)->m_ntag == -1) po->Node(n0)->m_ntag = 1;
		if (po->Node(n1)->m_ntag == -1) po->Node(n1)->m_ntag = 1;
	}

	// then the revolved edges
	int m = po->Edges();
	for (i=0; i<D; ++i)
	{
		for (j=0; j<N; ++j)
		{
			int i0 = nn[i*N + j];
			int i1 = nn[((i+1)*N + j)%NN];
			GNode& node = *po->Node(i0);
			if (node.m_ntag == 1)
			{
				node.m_ntag = m++;
				po->AddYArc(i0, i1);
			}
		}
	}

	// create new section-surfaces
	// if the original wire is closed
	int F = po->Faces();
	vector<int> edge;
	for (i=0; i<M; ++i)
	{
		for (j=0; j<F; ++j)
		{
			GFace& f = *po->Face(j);

			int ef = (int)f.m_edge.size();
			edge.resize(ef);
			for (int k=0; k<ef; ++k) 
			{
				edge[k] = ne[f.m_edge[k].nid + (i+1)*E];
			}
			po->AddFacet(edge, FACE_POLYGON);
		}
	}

	// every in-plane edge now creates a face
	for (i=0; i<D; ++i)
	{
		for (j=0; j<E; ++j)
		{
			GEdge& e0 = *po->Edge(ne[i*E + j]);
			GEdge& e1 = *po->Edge(ne[((i+1)*E + j)%NE]);
			int m0 = po->Node(e0.m_node[0])->m_ntag;
			int m1 = po->Node(e0.m_node[1])->m_ntag;
			e0.m_ntag = -1;

			if ((m0 > 0)&&(m1 > 0))
			{
				edge.resize(4);
				edge[0] = ne[         i*E+j];
				edge[1] =                 m1;
				edge[2] = ne[((i+1)*E+j)%NE];
				edge[3] =                 m0;
				po->AddFacet(edge, FACE_REVOLVE);
				e0.m_ntag = 1;
			}
			else if (m0 > 0)
			{
				// the second edge has to be the revolved arc
				edge.resize(3);
				edge[0] = ne[((i+1)*E+j)%NE];
				edge[1] =                 m0;
				edge[2] = ne[       i*E + j];
				po->AddFacet(edge, FACE_REVOLVE_WEDGE);
				e0.m_ntag = 1;
			}
			else if (m1 > 0)
			{
				// the second edge has to be the revolved arc
				edge.resize(3);
				edge[0] = ne[       i*E + j];
				edge[1] =                 m1;
				edge[2] = ne[((i+1)*E+j)%NE];
				po->AddFacet(edge, FACE_REVOLVE_WEDGE);
				e0.m_ntag = 1;
			}
		}
	}

	// create the parts
	// we should already have one part
	assert(po->Parts()==1);

	// but we should have one for each face
	for (i=1; i<F*D; ++i) po->AddPart();
	int NP = po->Parts();

	// Now, we need to figure out which faces belong to which parts
	// The bottom and top faces belong to the corresponding part
	for (i=0; i<=M; ++i)
	{
		for (j=0; j<F; ++j)
		{
			GFace& f = *po->Face(i*F + j);

			int pid = i*F + j;
			if (pid >= NP) pid -= F;
			f.m_nPID[0] = pid;
			f.m_nPID[1] = ((i==0)||(i==M)? -1 : (i-1)*F + j);
		}
	}
	
	// The side walls will be trickier since they can be interior faces
	// If so, we need to find the two parts that they belong to.
	int nf = F*(M+1);
	for (i=0; i<D; ++i)
	{
		for (j=0; j<E; ++j)
		{
			GEdge* pe = po->Edge(i*E + j);
			if (pe->m_ntag == 1)
			{
				GFace& f = *po->Face(nf++);
				f.m_nPID[0] = -1;
				f.m_nPID[1] = -1;
				int m = 0;
				for (int k=0; k<F; ++k)
				{
					GFace& fk = *po->Face(i*F + k);
					if (fk.HasEdge(i*E + j))
					{
						assert(m<2);
						f.m_nPID[m++] = fk.m_nPID[0];
					}
				}
				assert(f.m_nPID[0] != -1);
			}
		}
	}

	// find all vertices
	po->UpdateNodeTypes();
}

//-----------------------------------------------------------------------------
GMesh* GRevolveModifier::BuildGMesh(GObject* po)
{
	po->GObject::BuildGMesh();
	return 0;
}

//-----------------------------------------------------------------------------
FEMesh* GRevolveModifier::BuildFEMesh(GObject* po)
{
	FETetGenMesher tet(po);
	tet.SetFloatValue(FETetGenMesher::ELSIZE, 0.2);
	return tet.BuildMesh();
}
