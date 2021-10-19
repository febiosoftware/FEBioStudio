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

// FEShellTube.cpp: implementation of the FEShellTube class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellTube.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>
#include "FEMultiQuadMesh.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellTube::FEShellTube(GThinTube* po)
{
	m_pobj = po;

	m_t = 0.01;
	m_nd = 16;
	m_nz = 16;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_nz, "nz", "Stacks");
}

FEMesh* FEShellTube::BuildMesh()
{
//	return BuildMeshLegacy();
	return BuildMultiQuadMesh();
}

FEMesh* FEShellTube::BuildMultiQuadMesh()
{
	// get discretization parameters
	int nd = GetIntValue(NDIV);
	int nz = GetIntValue(NSTACK);

	// check parameters
	if (nd < 1) nd = 1;
	if (nz < 1) nz = 1;

	FEMultiQuadMesh MQ;
	MQ.Build(m_pobj);

	// set tesselation
	MQ.SetFaceSizes(0, nd, nz);
	MQ.SetFaceSizes(1, nd, nz);
	MQ.SetFaceSizes(2, nd, nz);
	MQ.SetFaceSizes(3, nd, nz);

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

FEMesh* FEShellTube::BuildMeshLegacy()
{
	int i, j;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R = param.GetFloatValue(GThinTube::RAD);
	double h = param.GetFloatValue(GThinTube::H);

	// get parameters
	m_t = GetFloatValue(T);
	double t = m_t;
	m_nd = GetIntValue(NDIV);
	m_nz = GetIntValue(NSTACK);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_nz < 1) m_nz = 1;

	int nd = 4*m_nd;
	int nz = m_nz;

	// create the mesh
	int nodes = nd*(nz+1);
	int elems = nd*nz;

	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	// create the nodes
	FENode* pn = pm->NodePtr();
	double x, y, z;
	for (j=0; j<=nz; ++j)
	{
		z = j*h/nz;
		for (i=0; i<nd; ++i, ++pn)
		{
			x = R*cos(2*i*PI/nd);
			y = R*sin(2*i*PI/nd);
			
			pn->r = vec3d(x, y, z);
		}
	}

	pm->Node(NodeIndex(     0,   0)).m_gid = 0;
	pm->Node(NodeIndex(  nd/4,   0)).m_gid = 1;
	pm->Node(NodeIndex(  nd/2,   0)).m_gid = 2;
	pm->Node(NodeIndex(3*nd/4,   0)).m_gid = 3;
	pm->Node(NodeIndex(     0,m_nz)).m_gid = 4;
	pm->Node(NodeIndex(  nd/4,m_nz)).m_gid = 5;
	pm->Node(NodeIndex(  nd/2,m_nz)).m_gid = 6;
	pm->Node(NodeIndex(3*nd/4,m_nz)).m_gid = 7;

	// create the elements
	int eid = 0;
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nd; ++i)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			pe->SetType(FE_QUAD4);
			pe->m_gid = 0; //4*i/nd;
			int* n = pe->m_node;
			n[0] = j*nd + i;
			n[1] = j*nd + (i+1)%nd;
			n[2] = n[1] + nd;
			n[3] = n[0] + nd;
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

	BuildFaces(pm);
	BuildEdges(pm);
	
	pm->BuildMesh();

	return pm;
}

void FEShellTube::BuildFaces(FEMesh* pm)
{
	int i, j;
	int nd = 4*m_nd;
	int nfaces = m_nz*nd;
	pm->Create(0,0,nfaces);
	FEFace* pf = pm->FacePtr();
	for (j=0; j<m_nz; ++j)
		for (i=0; i<nd; ++i, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 4*i/nd;
			f.n[0] = NodeIndex(i, j);
			f.n[1] = NodeIndex(i+1, j);
			f.n[2] = NodeIndex(i+1, j+1);
			f.n[3] = NodeIndex(i, j+1);
		}

}

void FEShellTube::BuildEdges(FEMesh* pm)
{
	int i;
	int nd = 4*m_nd;
	int nedges = 2*nd+4*m_nz;
	pm->Create(0,0,0,nedges);
	FEEdge* pe = pm->EdgePtr();

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 0; pe->n[0] = NodeIndex(i,0); pe->n[1] = NodeIndex(i+1,0); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 1; pe->n[0] = NodeIndex(i,0); pe->n[1] = NodeIndex(i+1,0); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 2; pe->n[0] = NodeIndex(i,0); pe->n[1] = NodeIndex(i+1,0); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 3; pe->n[0] = NodeIndex(i,0); pe->n[1] = NodeIndex(i+1,0); }

	for (i=     0; i<  nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 4; pe->n[0] = NodeIndex(i,m_nz); pe->n[1] = NodeIndex(i+1,m_nz); }
	for (i=  nd/4; i<  nd/2; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 5; pe->n[0] = NodeIndex(i,m_nz); pe->n[1] = NodeIndex(i+1,m_nz); }
	for (i=  nd/2; i<3*nd/4; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 6; pe->n[0] = NodeIndex(i,m_nz); pe->n[1] = NodeIndex(i+1,m_nz); }
	for (i=3*nd/4; i<  nd  ; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 7; pe->n[0] = NodeIndex(i,m_nz); pe->n[1] = NodeIndex(i+1,m_nz); }

	for (i=0; i<m_nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  8; pe->n[0] = NodeIndex(     0,i); pe->n[1] = NodeIndex(     0,i+1); }
	for (i=0; i<m_nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  9; pe->n[0] = NodeIndex(  nd/4,i); pe->n[1] = NodeIndex(  nd/4,i+1); }
	for (i=0; i<m_nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 10; pe->n[0] = NodeIndex(  nd/2,i); pe->n[1] = NodeIndex(  nd/2,i+1); }
	for (i=0; i<m_nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 11; pe->n[0] = NodeIndex(3*nd/4,i); pe->n[1] = NodeIndex(3*nd/4,i+1); }
}
