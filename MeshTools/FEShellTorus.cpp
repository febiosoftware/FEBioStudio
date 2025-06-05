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

// FEShellTorus.cpp: implementation of the FEShellTorus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellTorus.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellTorus::FEShellTorus()
{
	m_pobj = nullptr;

	m_t = 0.01;
	m_nd = 6;
	m_ns = 12;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments");
}

FSMesh* FEShellTorus::BuildMesh(GObject* po)
{
	m_pobj = dynamic_cast<GTorus*>(po);
	if (m_pobj == nullptr) return nullptr;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R0 = param.GetFloatValue(GTorus::RIN);
	double R1 = param.GetFloatValue(GTorus::ROUT);

	// get parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_t = GetFloatValue(T);
	double t = m_t;
	int nd = m_nd;
	int ns = m_ns;

	// check parameters
	if (nd < 1) nd = 1;
	if (ns < 1) ns = 1;

	nd *= 4;
	ns *= 4;

	// count nodes and elements
	int nodes = nd*ns;
	int elems = nd*ns;

	// allocate storage for mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// create nodes
	vec3d r;
	FSNode* pn = pm->NodePtr();
	for (int i=0; i<nd; ++i)
	{
		double w0 = 2.0*i*PI/nd - PI*0.5;
		double cw0 = cos(w0), sw0 = sin(w0);

		for (int j=0; j<ns; ++j, ++pn)
		{
			double w1 = 2.0*PI - 2.0*j*PI/ns;

			r.x = R1*cos(w1) + R0;
			r.y = 0;
			r.z = R1*sin(w1);

			pn->r.x = cw0*r.x;
			pn->r.y = sw0*r.x;
			pn->r.z = r.z;
		}
	}

	// create elements
	int eid = 0;
	for (int i=0; i<nd; ++i)
	{
		for (int j=0; j<ns; ++j)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			int* n = pe->m_node;

			n[0] = NodeIndex(i  , j  );
			n[1] = NodeIndex(i  , j+1);
			n[2] = NodeIndex(i+1, j+1);
			n[3] = NodeIndex(i+1, j  );

			pe->SetType(FE_QUAD4);
			pe->m_gid = 0;
		}
	}
	
	// assign shell thickness
	for (int i=0; i<elems; ++i)
	{
		FEElement_* pe = pm->ElementPtr(i);

		pe->m_h[0] = t;
		pe->m_h[1] = t;
		pe->m_h[2] = t;
		pe->m_h[3] = t;
	}

	BuildNodes(pm);
	BuildFaces(pm);
	BuildEdges(pm);

	pm->BuildMesh();

	return pm;
}

void FEShellTorus::BuildNodes(FSMesh* pm)
{
	pm->Node(NodeIndex(0,     0)).m_gid = 0;
	pm->Node(NodeIndex(0,  m_ns)).m_gid = 1;
	pm->Node(NodeIndex(0,2*m_ns)).m_gid = 2;
	pm->Node(NodeIndex(0,3*m_ns)).m_gid = 3;

	pm->Node(NodeIndex(m_nd,     0)).m_gid = 4;
	pm->Node(NodeIndex(m_nd,  m_ns)).m_gid = 5;
	pm->Node(NodeIndex(m_nd,2*m_ns)).m_gid = 6;
	pm->Node(NodeIndex(m_nd,3*m_ns)).m_gid = 7;

	pm->Node(NodeIndex(2*m_nd,     0)).m_gid = 8;
	pm->Node(NodeIndex(2*m_nd,  m_ns)).m_gid = 9;
	pm->Node(NodeIndex(2*m_nd,2*m_ns)).m_gid = 10;
	pm->Node(NodeIndex(2*m_nd,3*m_ns)).m_gid = 11;

	pm->Node(NodeIndex(3*m_nd,     0)).m_gid = 12;
	pm->Node(NodeIndex(3*m_nd,  m_ns)).m_gid = 13;
	pm->Node(NodeIndex(3*m_nd,2*m_ns)).m_gid = 14;
	pm->Node(NodeIndex(3*m_nd,3*m_ns)).m_gid = 15;
}

void FEShellTorus::BuildFaces(FSMesh* pm)
{
	int i;
	int nfaces = pm->Elements();
	pm->Create(0,0,nfaces);
	FSFace* pf = pm->FacePtr();
	for (i=0; i<nfaces; ++i, ++pf)
	{
		FEElement_* pe = pm->ElementPtr(i);

		int i1 = i/(4*m_ns);
		int i2 = i%(4*m_ns);
		FSFace& f = *pf;
		f.SetType(FE_FACE_QUAD4);
		f.m_gid = 4*(i1/m_nd) + i2/m_ns;
		f.n[0] = pe->m_node[0];
		f.n[1] = pe->m_node[1];
		f.n[2] = pe->m_node[2];
		f.n[3] = pe->m_node[3];
	}	
}

void FEShellTorus::BuildEdges(FSMesh* pm)
{
	int i;
	int nd = 4*m_nd;
	int ns = 4*m_ns;
	pm->Create(0,0,0,4*nd+4*ns);
	FSEdge* pe = pm->EdgePtr();
	for (i=0; i<nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =    4*i/nd; pe->n[0] = NodeIndex(i,     0); pe->n[1] = NodeIndex(i+1,     0); }
	for (i=0; i<nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  4+4*i/nd; pe->n[0] = NodeIndex(i,  ns/4); pe->n[1] = NodeIndex(i+1,  ns/4); }
	for (i=0; i<nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  8+4*i/nd; pe->n[0] = NodeIndex(i,  ns/2); pe->n[1] = NodeIndex(i+1,  ns/2); }
	for (i=0; i<nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 12+4*i/nd; pe->n[0] = NodeIndex(i,3*ns/4); pe->n[1] = NodeIndex(i+1,3*ns/4); }

	for (i=0; i<ns; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 16+4*i/ns; pe->n[0] = NodeIndex(     0, i); pe->n[1] = NodeIndex(     0, i+1); }
	for (i=0; i<ns; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 20+4*i/ns; pe->n[0] = NodeIndex(  nd/4, i); pe->n[1] = NodeIndex(  nd/4, i+1); }
	for (i=0; i<ns; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 24+4*i/ns; pe->n[0] = NodeIndex(  nd/2, i); pe->n[1] = NodeIndex(  nd/2, i+1); }
	for (i=0; i<ns; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 28+4*i/ns; pe->n[0] = NodeIndex(3*nd/4, i); pe->n[1] = NodeIndex(3*nd/4, i+1); }
}
