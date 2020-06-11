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

// FEShellDisc.cpp: implementation of the FEShellDisc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellDisc.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellDisc::FEShellDisc(GDisc* po)
{
	m_pobj = po;

	m_r = 0.5;
	m_t = 0.01;
	m_nd = m_nr = 6;

	AddDoubleParam(m_r, "r", "Ratio");
	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_nr, "nr", "Segments");
}

FEMesh* FEShellDisc::BuildMesh()
{
	int i, j, k;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R1 = param.GetFloatValue(GDisc::RADIUS);

	// get parameters
	m_nd = GetIntValue(NDIV);
	m_nr = GetIntValue(NSEG);

	m_r = GetFloatValue(RATIO);
	m_t = GetFloatValue(T);
	double R0 = m_r;
	double t = m_t;
	int nd = m_nd;
	int nr = m_nr;

	R0 *= R1;

	// check parameters
	if (nd   < 1) nd   = 1;
	if (nr   < 1) nr   = 1;

	nd *=2;

	int nodes = (nd+1)*(nd+1) + 4*nd*nr;
	int elems = nd*nd+4*nd*nr;

	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);
	m_nd = nd;
	m_nr = nr;

	// create the inner nodes
	double R, f, dr;
	double h = R0*sqrt(2.0)*0.5;
	FENode* pn = pm->NodePtr();
	vec3d rr;
	for (i=0; i<=nd; ++i)
		for (j=0; j<=nd; ++j, ++pn)
		{
			rr.x = -h + 2*h*i/nd;
			rr.y = -h + 2*h*j/nd;
			rr.z = 0;

			R = rr.Length();
			f = 1 - 0.15*R/h;

			pn->r = rr*f;
		}

	// create the inner elements
	int eid = 0;
	for (i=0; i<nd; ++i)
		for (j=0; j<nd; ++j)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			pe->SetType(FE_QUAD4);
			pe->m_gid = 0; //(i<nd/2? 2*j/nd : 2+(2*(nd-j-1)/nd));
			pe->m_node[0] = j*(nd+1) + i;
			pe->m_node[1] = (j+1)*(nd+1) + i;
			pe->m_node[2] = (j+1)*(nd+1) + i+1;
			pe->m_node[3] = j*(nd+1) + i + 1;
		}	

	// create node index loop
	vector<int>	Nd(4*nd);
	vector<vec3d> Nr(4*nd);
	for (i=0; i<nd; ++i) Nd[i     ] = nd - i - 1;
	for (i=0; i<nd; ++i) Nd[i+nd  ] = (i+1)*(nd+1);
	for (i=0; i<nd; ++i) Nd[i+nd*2] = i + nd*(nd+1)+1;
	for (i=0; i<nd; ++i) Nd[i+nd*3] = nd*(nd+1) - i*(nd+1) - 1;

	for (i=0; i<4*nd; ++i) Nr[i] = pm->Node(Nd[i]).r;

	// create the rest of the mesh
	int noff = (nd+1)*(nd+1);
	for (k=0; k<nr; ++k)
	{
		// create the nodes
		for (i=0; i<4*nd; ++i, ++pn)
		{
			rr = Nr[i];
			R = rr.Length();
			dr = (k+1)*(R1 - R)/nr;
			f = (R + dr)/R;
			pn->r = rr*f;	
		}

		// create the elements
		for (i=0; i<4*nd; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			pe->SetType(FE_QUAD4);
			pe->m_gid = 0; //((i+nd/2+1 + 3*nd)/(nd))%4;
			int* n = pe->m_node;
			n[0] = Nd[i];
			n[1] = noff + i;
			n[2] = noff + (i+1)%(4*nd);
			n[3] = Nd[(i+1)%(4*nd)];
		}

		// set the next layer of node indices
		for (i=0; i<4*nd; ++i) Nd[i] = noff+i;
		noff += 4*nd;
	}

	pm->Node(NodeIndex2(m_nr,0)).m_gid = 3;
	pm->Node(NodeIndex2(m_nr,1)).m_gid = 0;
	pm->Node(NodeIndex2(m_nr,2)).m_gid = 1;
	pm->Node(NodeIndex2(m_nr,3)).m_gid = 2;
	pm->Node(NodeIndex(nd/2, nd/2)).m_gid = 4;

	// assign thickness to shells
	for (i=0; i<elems; ++i)
	{
		FEElement_* pe = pm->ElementPtr(i);

		pe->m_h[0] = t;
		pe->m_h[1] = t;
		pe->m_h[2] = t;
		pe->m_h[3] = t;
	}

	BuildFaces(pm);
	BuildEdges(pm);

	// update the mesh
	pm->BuildMesh();

	return pm;
}

void FEShellDisc::BuildFaces(FEMesh* pm)
{
	int i;
	int nfaces = pm->Elements();
	pm->Create(0,0,nfaces);
	FEFace* pf = pm->FacePtr();
	for (i=0; i<nfaces; ++i, ++pf)
	{
		FEElement_* pe = pm->ElementPtr(i);

		FEFace& f = *pf;
		f.SetType(FE_FACE_QUAD4);
		if (i<m_nd*m_nd)
		{
			int iy = i/m_nd;
			int ix = i%m_nd;
			f.m_gid = (ix<m_nd/2? 2 - (2*iy)/m_nd: 3-3*((2*iy)/m_nd));
		}
		else f.m_gid = ((i+m_nd/2+1 + m_nd)/(m_nd))%4;

		f.n[0] = pe->m_node[0];
		f.n[1] = pe->m_node[1];
		f.n[2] = pe->m_node[2];
		f.n[3] = pe->m_node[3];
	}
}

void FEShellDisc::BuildEdges(FEMesh* pm)
{
	int i;
	int nedges = 4*m_nd + 2*m_nd + 4*m_nr;
	pm->Create(0,0,0,nedges);
	FEEdge* pe = pm->EdgePtr();
	int N = (m_nd+1)*(m_nd+1) + (m_nr-1)*4*m_nd;
	for (i=0; i<4*m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = ((i+m_nd/2+1)/m_nd+1)%4; pe->n[0] = N+i; pe->n[1] = N+(i+1)%(4*m_nd); }

	for (i=0; i<m_nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 7; pe->n[0] = NodeIndex(m_nd/2, i); pe->n[1] = NodeIndex(m_nd/2, i+1); }
	for (i=0; i<m_nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 5; pe->n[0] = NodeIndex(m_nd/2, i+m_nd/2); pe->n[1] = NodeIndex(m_nd/2, i+1+m_nd/2); }
	for (i=0; i<m_nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 4; pe->n[0] = NodeIndex(i+m_nd/2, m_nd/2); pe->n[1] = NodeIndex(i+1+m_nd/2, m_nd/2); }
	for (i=0; i<m_nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 6; pe->n[0] = NodeIndex(i, m_nd/2); pe->n[1] = NodeIndex(i+1, m_nd/2); }

	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 7; pe->n[0] = NodeIndex2(i, 0); pe->n[1] = NodeIndex2(i+1, 0); }
	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 4; pe->n[0] = NodeIndex2(i, 1); pe->n[1] = NodeIndex2(i+1, 1); }
	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 5; pe->n[0] = NodeIndex2(i, 2); pe->n[1] = NodeIndex2(i+1, 2); }
	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 6; pe->n[0] = NodeIndex2(i, 3); pe->n[1] = NodeIndex2(i+1, 3); }
}

int FEShellDisc::NodeIndex2(int i, int j)
{
	int n = 0;
	if (i==0)
	{
		switch (j)
		{
		case 0: n = m_nd/2*(m_nd+1); break;
		case 1: n = m_nd*(m_nd+1)+m_nd/2; break;
		case 2: n = m_nd/2*(m_nd+1)+m_nd; break;
		case 3: n = m_nd/2; break;
		}
		return n;
	}
	else 
	{
		n = (j*m_nd+3*m_nd/2-1)%(4*m_nd);
		return (m_nd+1)*(m_nd+1)+(i-1)*4*m_nd + n;
	}
}
