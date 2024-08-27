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
#include <MeshLib/MeshMetrics.h>
using namespace std;

//-----------------------------------------------------------------------------
const int lut[14][3][4] = {
	{{ 0, 1, 2, 0},{-1,-1,-1,-1},{-1,-1,-1,-1}},
	{{ 0, 3, 5, 1},{ 1, 5, 3, 0},{ 1, 2, 5, 0}},
	{{ 0, 3, 4, 0},{ 1, 4, 3, 1},{ 0, 4, 2, 0}},
	{{ 0, 1, 5, 1},{ 1, 4, 5, 1},{ 2, 5, 4, 0}},
	{{ 0, 1, 5, 0},{ 1, 4, 5, 0},{ 2, 5, 4, 1}},
	{{ 0, 3, 4, 1},{ 1, 4, 3, 0},{ 0, 4, 2, 1}},
	{{ 0, 3, 5, 0},{ 1, 5, 3, 1},{ 1, 2, 5, 1}},
	{{ 0, 1, 2, 1},{-1,-1,-1,-1},{-1,-1,-1,-1}},
	{{ 0, 1, 4, 0},{ 0, 4, 2, 1},{-1,-1,-1,-1}},
	{{ 0, 1, 4, 1},{ 0, 4, 2, 0},{-1,-1,-1,-1}},
	{{ 0, 1, 5, 0},{ 1, 2, 5, 1},{-1,-1,-1,-1}},
	{{ 0, 1, 5, 1},{ 1, 2, 5, 0},{-1,-1,-1,-1}},
	{{ 0, 3, 2, 0},{ 1, 2, 3, 1},{-1,-1,-1,-1}},
	{{ 0, 3, 2, 1},{ 1, 2, 3, 0},{-1,-1,-1,-1}},
};

//-----------------------------------------------------------------------------
FEPlaneCut::FEPlaneCut() : FEModifier("Plane cut")
{
	m_a[0] = 1.0;
	m_a[1] = 0.0;
	m_a[2] = 0.0;
	m_a[3] = 0.0;
}

//-----------------------------------------------------------------------------
void FEPlaneCut::SetPlaneCoefficients(double a[4])
{
	m_a[0] = a[0];
	m_a[1] = a[1];
	m_a[2] = a[2];
	m_a[3] = a[3];
}

//-----------------------------------------------------------------------------
FSMesh* FEPlaneCut::Apply(FSMesh* pm)
{
	// make sure this is a triangle mesh
	if (pm->IsType(FE_TRI3) == false)
	{
		SetError("This is not a triangle mesh.");
		return nullptr;
	}

	// get the characteristic mesh size (i.e. smallest edge length)
	double h = 0.01*FEMeshMetrics::ShortestEdge(*pm);

	// nodal values
	int NN = pm->Nodes();
	vector<double> val(NN);
	for (int i=0; i<NN; ++i)
	{
		vec3d& r = pm->Node(i).r;
		val[i] = m_a[0]*r.x + m_a[1]*r.y + m_a[2]*r.z - m_a[3];
		if (fabs(val[i])<h) val[i] = 0.0;
	}

	// loop over all elements and determine case number
	const int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FSElement& ei = pm->Element(i);
		ei.m_ntag = 0;
		double v[3];
		v[0] = val[ei.m_node[0]]; 
		v[1] = val[ei.m_node[1]]; 
		v[2] = val[ei.m_node[2]]; 

		// check special cases first
		if ((v[0]==0.0)||(v[1]==0.0)||(v[2]==0.0))
		{
			ei.m_ntag = -1;

			// all three nodes are zero
			// NOTE: This should never happen unless all nodes are colinear
			if ((v[0]==0.0)&&(v[1]==0.0)&&(v[2]==0.0)) ei.m_ntag = 0;

			// two nodes are zero
			if ((v[0]==0.0)&&(v[1]==0.0)&&(v[2]<0.0)) ei.m_ntag = 0;
			if ((v[1]==0.0)&&(v[2]==0.0)&&(v[0]<0.0)) ei.m_ntag = 0;
			if ((v[2]==0.0)&&(v[0]==0.0)&&(v[1]<0.0)) ei.m_ntag = 0;

			if ((v[0]==0.0)&&(v[1]==0.0)&&(v[2]>0.0)) ei.m_ntag = 7;
			if ((v[1]==0.0)&&(v[2]==0.0)&&(v[0]>0.0)) ei.m_ntag = 7;
			if ((v[2]==0.0)&&(v[0]==0.0)&&(v[1]>0.0)) ei.m_ntag = 7;

			// one node is zero (other two on same side)
			if ((v[0]==0.0)&&(v[1]<0.0)&&(v[2]<0.0)) ei.m_ntag = 0;
			if ((v[1]==0.0)&&(v[2]<0.0)&&(v[0]<0.0)) ei.m_ntag = 0;
			if ((v[2]==0.0)&&(v[0]<0.0)&&(v[1]<0.0)) ei.m_ntag = 0;

			if ((v[0]==0.0)&&(v[1]>0.0)&&(v[2]>0.0)) ei.m_ntag = 7;
			if ((v[1]==0.0)&&(v[2]>0.0)&&(v[0]>0.0)) ei.m_ntag = 7;
			if ((v[2]==0.0)&&(v[0]>0.0)&&(v[1]>0.0)) ei.m_ntag = 7;

			// one node is zero (element is bisected)
			if ((v[0] == 0.0)&&(v[1]<0.0)&&(v[2]>0.0)) ei.m_ntag = 8;
			if ((v[0] == 0.0)&&(v[1]>0.0)&&(v[2]<0.0)) ei.m_ntag = 9;

			if ((v[1] == 0.0)&&(v[2]>0.0)&&(v[0]<0.0)) ei.m_ntag = 10;
			if ((v[1] == 0.0)&&(v[2]<0.0)&&(v[0]>0.0)) ei.m_ntag = 11;

			if ((v[2] == 0.0)&&(v[0]<0.0)&&(v[1]>0.0)) ei.m_ntag = 12;
			if ((v[2] == 0.0)&&(v[0]>0.0)&&(v[1]<0.0)) ei.m_ntag = 13;

			assert(ei.m_ntag != -1);
		}
		else
		{
			// handle general case
			if (v[0] > 0.0) ei.m_ntag |= 1;
			if (v[1] > 0.0) ei.m_ntag |= 2;
			if (v[2] > 0.0) ei.m_ntag |= 4;
		}
	}

	// counts new number of elements
	int NE2 = 0;
	for (int i=0; i<NE; ++i)
	{
		FSElement& ei = pm->Element(i);
		if ((ei.m_ntag > 0) && (ei.m_ntag < 7)) NE2 += 3;
		else if (ei.m_ntag > 7) NE2 += 2;
		else NE2++;
	}

	// determine the edge lists
	vector<EDGE> EL;			// list of all edges
	EL.reserve(NE2 - NE);
	vector<EDGELIST> EEL;		// list of element edges
	EEL.resize(NE);
	for (int i=0; i<NE; ++i)
	{
		FSElement& ei = pm->Element(i);
		for (int j=0; j<3; ++j)
		{
			int jp1 = (j+1)%3;
			double w0 = val[ei.m_node[j  ]];
			double w1 = val[ei.m_node[jp1]];
			if (w0*w1 < 0.0)
			{
				// add the edge
				EDGE e;
				e.n0 = ei.m_node[j  ];
				e.n1 = ei.m_node[jp1];
				e.w = w0/(w0 - w1);

				// make sure this edge does not exist yet
				int ne = (int) EL.size();
				int nedge = -1;
				for (int k=0; k<ne; ++k)
				{
					EDGE& ek = EL[k];
					if (((ek.n0 == e.n0)&&(ek.n1 == e.n1)) || ((ek.n0 == e.n1)&&(ek.n1 == e.n0))) { nedge = k; break; }
				}

				// if not found, add it to the list
				if (nedge == -1)
				{
					EL.push_back(e);
					nedge = ne;
				}

				// mark the element's edge
				EEL[i].n[j] = nedge;
			}
			else EEL[i].n[j] = -1;
		}
	}

	// create a new mesh
	FSMesh* pnew = new FSMesh;
	int N1 = (int) EL.size();		// number of new nodes (i.e. number of cut edges)
	pnew->Create(NN + N1, NE2);

	// copy nodes
	for (int i=0; i<NN; ++i) pnew->Node(i) = pm->Node(i);

	// add new nodes
	for (int i=0; i<N1; ++i)
	{
		FSNode& ni = pnew->Node(i + NN);
		vec3d& r0 = pm->Node(EL[i].n0).r;
		vec3d& r1 = pm->Node(EL[i].n1).r;
		ni.r = r0 + (r1 - r0)*EL[i].w;
	}

	// copy elements
	int ne = 0;
	for (int i=0; i<NE; ++i)
	{
		FSElement& es = pm->Element(i);

		// get the node numbers
		int n[6];
		n[0] = es.m_node[0];
		n[1] = es.m_node[1];
		n[2] = es.m_node[2];
		if (EEL[i].n[0] >= 0) n[3] = NN + EEL[i].n[0]; else n[3] = -1;
		if (EEL[i].n[1] >= 0) n[4] = NN + EEL[i].n[1]; else n[4] = -1;
		if (EEL[i].n[2] >= 0) n[5] = NN + EEL[i].n[2]; else n[5] = -1;

		for (int j=0; j<3; ++j)
		{
			const int *lj = lut[es.m_ntag][j];
			if (lj[0]==-1) break;
			FSElement& ed = pnew->Element(ne++);
			ed = es;
			ed.m_node[0] = n[lj[0]]; assert(ed.m_node[0] != -1);
			ed.m_node[1] = n[lj[1]]; assert(ed.m_node[1] != -1);
			ed.m_node[2] = n[lj[2]]; assert(ed.m_node[2] != -1);
			ed.m_gid = lj[3]; assert(lj[3] != -1);
		}
	}
	assert(ne == pnew->Elements());

	// next, we reconstruct all faces, edges and nodes
	pnew->RebuildMesh();

	return pnew;
}
