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

// FEShellRing.cpp: implementation of the FEShellRing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellRing.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>
#include "FEMultiQuadMesh.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellRing::FEShellRing(GRing* po)
{
	m_pobj = po;

	m_t = 0.01;
	m_ns = 16;
	m_nr = 8;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_ns, "ns", "Slices");
	AddIntParam(m_nr, "nr", "Division");
}

FEMesh* FEShellRing::BuildMesh()
{
//	return BuildMeshLegacy();
	return BuildMultiQuadMesh();
}

FEMesh* FEShellRing::BuildMultiQuadMesh()
{
	// build the mesh data structures
	FEMultiQuadMesh MQ;
	MQ.Build(m_pobj);

	// set discretization
	int nd = GetIntValue(NDIV); if (nd < 1) nd = 1;
	int ns = GetIntValue(NSLICE); if (ns < 1) ns = 1;
	MQ.SetFaceSizes(0, ns, nd);
	MQ.SetFaceSizes(1, ns, nd);
	MQ.SetFaceSizes(2, ns, nd);
	MQ.SetFaceSizes(3, ns, nd);

	// Build the mesh
	FEMesh* pm = MQ.BuildMesh();
	if (pm == nullptr) return nullptr;

	// assign shell thickness
	double t = GetFloatValue(T);
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		el.m_h[0] = el.m_h[1] = el.m_h[2] = el.m_h[3] = t;
	}

	return pm;
}

FEMesh* FEShellRing::BuildMeshLegacy()
{
	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R0 = param.GetFloatValue(GRing::RIN);
	double R1 = param.GetFloatValue(GRing::ROUT);

	// get parameters
	m_t = GetFloatValue(T);
	double t = m_t;
	m_ns = GetIntValue(NSLICE);
	m_nr = GetIntValue(NDIV);
	int ns = 4*m_ns;
	int nr = m_nr;

	// check parameters
	if (ns < 4) ns = 4;
	if (nr < 1) nr = 1;

	int nodes = ns*(nr + 1);
	int elems = ns*nr;

	// allocate storage
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);
	m_nr = nr;
	m_ns = ns;

	int i, j;

	// create nodes
	FENode* pn = pm->NodePtr();
	double x, y, R, w;
	for (i=0; i<ns; ++i)
	{
		w = (2.0*PI*i) / (double) ns;
		for (j=0; j<=nr; ++j, ++pn)
		{
			R = R0 + j*(R1 - R0)/nr;
			x = R*cos(w);
			y = R*sin(w);

			pn->r = vec3d(x, y, 0);
		}
	}

	// create elements
	int eid = 0;
	int i1;
	for (i=0; i<ns; ++i)
	{
		i1 = (i+1)%ns;
		for (j=0; j<nr; ++j)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			pe->SetType(FE_QUAD4);
			pe->m_gid = 0;//4*i/ns;
			pe->m_node[0] = i*(nr+1)+j;
			pe->m_node[1] = i*(nr+1)+j+1;
			pe->m_node[2] = i1*(nr+1)+j+1;
			pe->m_node[3] = i1*(nr+1)+j;
		}
	}
	
	// assign shell thickness
	for (i=0; i<elems; ++i)
	{
		FEElement_* pe = pm->ElementPtr(i);

		pe->m_h[0] = t;
		pe->m_h[1] = t;
		pe->m_h[2] = t;
		pe->m_h[3] = t;
	}

	pm->Node(NodeIndex(nr,      0)).m_gid = 0;
	pm->Node(NodeIndex(nr,   ns/4)).m_gid = 1;
	pm->Node(NodeIndex(nr,   ns/2)).m_gid = 2;
	pm->Node(NodeIndex(nr, 3*ns/4)).m_gid = 3;
	pm->Node(NodeIndex( 0,      0)).m_gid = 4;
	pm->Node(NodeIndex( 0,   ns/4)).m_gid = 5;
	pm->Node(NodeIndex( 0,   ns/2)).m_gid = 6;
	pm->Node(NodeIndex( 0, 3*ns/4)).m_gid = 7;

	BuildFaces(pm);
	BuildEdges(pm);

	// update the mesh
	pm->BuildMesh();

	return pm;
}

void FEShellRing::BuildFaces(FEMesh* pm)
{
	int i, j;
	int nfaces = m_ns*m_nr;
	pm->Create(0,0,nfaces);
	FEFace* pf = pm->FacePtr();
	for (j=0; j<m_ns; ++j)
		for (i=0; i<m_nr; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 4 * j / m_ns;
			f.n[0] = NodeIndex(i  , j  );
			f.n[1] = NodeIndex(i+1, j  );
			f.n[2] = NodeIndex(i+1, j+1);
			f.n[3] = NodeIndex(i  , j+1);
		}

}

void FEShellRing::BuildEdges(FEMesh* pm)
{
	int i;
	int nedges = 2*m_ns + 4*m_nr;
	pm->Create(0,0,0,nedges);
	FEEdge* pe = pm->EdgePtr();
	for (i=       0; i<  m_ns/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 0; pe->n[0] = NodeIndex(m_nr, i); pe->n[1] = NodeIndex(m_nr, i+1); }
	for (i=  m_ns/4; i<  m_ns/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 1; pe->n[0] = NodeIndex(m_nr, i); pe->n[1] = NodeIndex(m_nr, i+1); }
	for (i=  m_ns/2; i<3*m_ns/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 2; pe->n[0] = NodeIndex(m_nr, i); pe->n[1] = NodeIndex(m_nr, i+1); }
	for (i=3*m_ns/4; i<  m_ns  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 3; pe->n[0] = NodeIndex(m_nr, i); pe->n[1] = NodeIndex(m_nr, i+1); }

	for (i=       0; i<  m_ns/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 4; pe->n[0] = NodeIndex(0, i); pe->n[1] = NodeIndex(0, i+1); }
	for (i=  m_ns/4; i<  m_ns/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 5; pe->n[0] = NodeIndex(0, i); pe->n[1] = NodeIndex(0, i+1); }
	for (i=  m_ns/2; i<3*m_ns/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 6; pe->n[0] = NodeIndex(0, i); pe->n[1] = NodeIndex(0, i+1); }
	for (i=3*m_ns/4; i<  m_ns  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 7; pe->n[0] = NodeIndex(0, i); pe->n[1] = NodeIndex(0, i+1); }

	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  8; pe->n[0] = NodeIndex(i,       0); pe->n[1] = NodeIndex(i+1,        0); }
	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  9; pe->n[0] = NodeIndex(i,  m_ns/4); pe->n[1] = NodeIndex(i+1,   m_ns/4); }
	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 10; pe->n[0] = NodeIndex(i,  m_ns/2); pe->n[1] = NodeIndex(i+1,   m_ns/2); }
	for (i=0; i<m_nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 11; pe->n[0] = NodeIndex(i,3*m_ns/4); pe->n[1] = NodeIndex(i+1, 3*m_ns/4); }
}
