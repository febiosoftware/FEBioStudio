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

// FEShellSphere.cpp: implementation of the FEShellSphere class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellSphere.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/MeshMetrics.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellSphere::FEShellSphere()
{
	m_pobj = nullptr;

	m_t = 0.01;
	m_nd = 6;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nd, "nd", "Divisions");
}

FEShellSphere::~FEShellSphere()
{

}

FSMesh* FEShellSphere::BuildMesh(GObject* po)
{
	m_pobj = dynamic_cast<GSphere*>(po);
	if (m_pobj == nullptr) return nullptr;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R = param.GetFloatValue(GSphere::RADIUS);

	// get parameters
	m_t = GetFloatValue(T);
	double t = m_t;
	m_nd = GetIntValue(NDIV);
	int nd = m_nd;

	// check parameters
	if (nd < 1) nd = 1;

	nd *= 2;

	// calculate item numbers
	int elems = nd*nd*6;
	int nodes = 2*(nd+1)*(nd+1) + 4*nd*(nd - 1);

	// allocate storage for the mesh
	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);
	m_nd = nd;

	int i, j, k, n;
	const int N = nd;
	vec3d r;

	int ntag = (N+1)*(N+1)*(N+1);
	int* tag = new int[ntag]; 
	for (i=0; i<ntag; ++i) tag[i] = -1;

	// create the nodes
	n = 0;
	for (i=0; i<=N; ++i)
	{
		for (j=0; j<=N; ++j)
			for (k=0; k<=N; ++k)
				if (i==0 || i==N || j==0 || j==N || k==0 || k==N)
				{
					r.x = -1.0 + i*2.0/N;
					r.y = -1.0 + j*2.0/N;
					r.z = -1.0 + k*2.0/N;
					r.Normalize();
					pm->Node(n).r = r*R;
					tag[i*(N+1)*(N+1) + j*(N+1) + k] = n;
					++n;
				}
	}

	pm->Node( NodeIndex(  N, N/2, N/2) ).m_gid = 0;
	pm->Node( NodeIndex(N/2,   N, N/2) ).m_gid = 1;
	pm->Node( NodeIndex(  0, N/2, N/2) ).m_gid = 2;
	pm->Node( NodeIndex(N/2,   0, N/2) ).m_gid = 3;
	pm->Node( NodeIndex(N/2, N/2,   0) ).m_gid = 4;
	pm->Node( NodeIndex(N/2, N/2,   N) ).m_gid = 5;

	// create the elements
	n = 0;
	int ne[8];
	int* pf;
	for (i=0; i<N; ++i)
		for (j=0; j<N; ++j)
			for (k=0; k<N; ++k)
				if ((i==0) || (i==N-1) || (j==0) || (j==N-1) || (k==0) || (k==N-1))
				{
					ne[0] =     i*(N+1)*(N+1) +     j*(N+1) + k;
					ne[1] = (i+1)*(N+1)*(N+1) +     j*(N+1) + k;
					ne[2] = (i+1)*(N+1)*(N+1) + (j+1)*(N+1) + k;
					ne[3] =     i*(N+1)*(N+1) + (j+1)*(N+1) + k;
					ne[4] =     i*(N+1)*(N+1) +     j*(N+1) + k+1;
					ne[5] = (i+1)*(N+1)*(N+1) +     j*(N+1) + k+1;
					ne[6] = (i+1)*(N+1)*(N+1) + (j+1)*(N+1) + k+1;
					ne[7] =     i*(N+1)*(N+1) + (j+1)*(N+1) + k+1;

					if (i==  0) 
					{
						FSElement& el = pm->Element(n);
						el.SetType(FE_QUAD4);
						el.m_gid = GetElementID(j, k, 2, 1, 6, 5);
						pf = FTHEX8[3];
						el.m_node[0] = tag[ ne[ pf[0] ]]; assert(el.m_node[0] >= 0);
						el.m_node[1] = tag[ ne[ pf[1] ]]; assert(el.m_node[1] >= 0);
						el.m_node[2] = tag[ ne[ pf[2] ]]; assert(el.m_node[2] >= 0);
						el.m_node[3] = tag[ ne[ pf[3] ]]; assert(el.m_node[3] >= 0);
						++n;										
					}
					if (i==N-1)
					{
						FSElement& el = pm->Element(n);
						el.SetType(FE_QUAD4);
						el.m_gid = GetElementID(j, k, 3, 0, 7, 4);
						pf = FTHEX8[1];
						el.m_node[0] = tag[ ne[ pf[0] ]]; assert(el.m_node[0] >= 0);
						el.m_node[1] = tag[ ne[ pf[1] ]]; assert(el.m_node[1] >= 0);
						el.m_node[2] = tag[ ne[ pf[2] ]]; assert(el.m_node[2] >= 0);
						el.m_node[3] = tag[ ne[ pf[3] ]]; assert(el.m_node[3] >= 0);
						++n;										
					}
					if (j==  0) 
					{
						FSElement& el = pm->Element(n);
						el.SetType(FE_QUAD4);
						el.m_gid = GetElementID(i, k, 2, 3, 6, 7);
						pf = FTHEX8[0];
						el.m_node[0] = tag[ ne[ pf[0] ]]; assert(el.m_node[0] >= 0);
						el.m_node[1] = tag[ ne[ pf[1] ]]; assert(el.m_node[1] >= 0);
						el.m_node[2] = tag[ ne[ pf[2] ]]; assert(el.m_node[2] >= 0);
						el.m_node[3] = tag[ ne[ pf[3] ]]; assert(el.m_node[3] >= 0);
						++n;										
					}
					if (j==N-1)
					{
						FSElement& el = pm->Element(n);
						el.SetType(FE_QUAD4);
						el.m_gid = GetElementID(i, k, 1, 0, 5, 4);
						pf = FTHEX8[2];
						el.m_node[0] = tag[ ne[ pf[0] ]]; assert(el.m_node[0] >= 0);
						el.m_node[1] = tag[ ne[ pf[1] ]]; assert(el.m_node[1] >= 0);
						el.m_node[2] = tag[ ne[ pf[2] ]]; assert(el.m_node[2] >= 0);
						el.m_node[3] = tag[ ne[ pf[3] ]]; assert(el.m_node[3] >= 0);
						++n;										
					}
					if (k==  0) 
					{
						FSElement& el = pm->Element(n);
						el.SetType(FE_QUAD4);
						el.m_gid = GetElementID(i, j, 2, 3, 1, 0);
						pf = FTHEX8[4];
						el.m_node[0] = tag[ ne[ pf[0] ]]; assert(el.m_node[0] >= 0);
						el.m_node[1] = tag[ ne[ pf[1] ]]; assert(el.m_node[1] >= 0);
						el.m_node[2] = tag[ ne[ pf[2] ]]; assert(el.m_node[2] >= 0);
						el.m_node[3] = tag[ ne[ pf[3] ]]; assert(el.m_node[3] >= 0);
						++n;										
					}
					if (k==N-1)
					{
						FSElement& el = pm->Element(n);
						el.SetType(FE_QUAD4);
						el.m_gid = GetElementID(i, j, 6, 7, 5, 4);
						pf = FTHEX8[5];
						el.m_node[0] = tag[ ne[ pf[0] ]]; assert(el.m_node[0] >= 0);
						el.m_node[1] = tag[ ne[ pf[1] ]]; assert(el.m_node[1] >= 0);
						el.m_node[2] = tag[ ne[ pf[2] ]]; assert(el.m_node[2] >= 0);
						el.m_node[3] = tag[ ne[ pf[3] ]]; assert(el.m_node[3] >= 0);
						++n;										
					}
				}

	assert(n == elems);
	

	// assign shell thickness
	for (i=0; i<elems; ++i)
	{
		FSElement_* pe = pm->ElementPtr(i);

		pe->m_h[0] = t;
		pe->m_h[1] = t;
		pe->m_h[2] = t;
		pe->m_h[3] = t;
	}

	// cleanup
	delete [] tag;

	BuildFaces(pm);
	BuildEdges(pm);

	pm->BuildMesh();

	return pm;
}

void FEShellSphere::BuildFaces(FSMesh* pm)
{
	int i;
	int nfaces = pm->Elements();
	pm->Create(0,0,nfaces);
	FSFace* pf = pm->FacePtr();
	for (i=0; i<nfaces; ++i, ++pf)
	{
		FSElement_* pe = pm->ElementPtr(i);

		FSFace& f = *pf;
		f.SetType(FE_FACE_QUAD4);
		f.m_gid = pe->m_gid;
		pe->m_gid = 0;
		f.n[0] = pe->m_node[0];
		f.n[1] = pe->m_node[1];
		f.n[2] = pe->m_node[2];
		f.n[3] = pe->m_node[3];
	}
}

void FEShellSphere::BuildEdges(FSMesh* pm)
{
	int i;
	int N = m_nd;
	int N2 = N/2;
	int nedges = 12*m_nd;
	pm->Create(0,0,0,nedges);
	FSEdge* pe = pm->EdgePtr();
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?10:6); pe->n[0] = NodeIndex(0, N2,  i); pe->n[1] = NodeIndex(0,  N2, i+1); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?2:1); pe->n[0] = NodeIndex(0,  i, N2); pe->n[1] = NodeIndex(0, i+1,  N2); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?2:3); pe->n[0] = NodeIndex(i,  0, N2); pe->n[1] = NodeIndex(i+1, 0,  N2); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?11:7); pe->n[0] = NodeIndex(N2,  0, i); pe->n[1] = NodeIndex(N2, 0,  i+1); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?8:4); pe->n[0] = NodeIndex(N, N2, i); pe->n[1] = NodeIndex(N, N2,  i+1); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?3:0); pe->n[0] = NodeIndex(N, i, N2); pe->n[1] = NodeIndex(N, i+1,  N2); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?1:0); pe->n[0] = NodeIndex(i, N, N2); pe->n[1] = NodeIndex(i+1, N,  N2); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?9:5); pe->n[0] = NodeIndex(N2, N, i); pe->n[1] = NodeIndex(N2, N,  i+1); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?10:8); pe->n[0] = NodeIndex(i, N2, 0); pe->n[1] = NodeIndex(i+1, N2,  0); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?11:9); pe->n[0] = NodeIndex(N2, i, 0); pe->n[1] = NodeIndex(N2, i+1,  0); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?6:4); pe->n[0] = NodeIndex(i, N2, N); pe->n[1] = NodeIndex(i+1, N2,  N); }
	for (i=0; i<m_nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = (i<N/2?7:5); pe->n[0] = NodeIndex(N2, i, N); pe->n[1] = NodeIndex(N2, i+1,  N); }
}

int FEShellSphere::NodeIndex(int i, int j, int k)
{
	int N = m_nd;
	if      (i==0) return j*(N+1)+k;
	else if (i==N) return (N+1)*(N+1) + (N-1)*4*N + j*(N+1)+k;
	else if (j==0) return (N+1)*(N+1) + (i-1)*4*N+k;
	else if (j==N) return (N+1)*(N+1) + (i-1)*4*N + 3*N-1 + k;
	else if (k==0) return (N+1)*(N+1) + (i-1)*4*N + (N+1) + (j-1)*2;
	else if (k==N) return (N+1)*(N+1) + (i-1)*4*N + (N+1) + (j-1)*2+1;

	return -1;
}
