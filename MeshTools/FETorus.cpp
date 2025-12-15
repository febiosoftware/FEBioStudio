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

// FETorus.cpp: implementation of the FETorus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FETorus.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>
#include <vector>
//using namespace std;

FETorus::FETorus(GObject& o) : FEMultiBlockMesh(o)
{
	m_nd = 2;
	m_ns = 8;

	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments" );

	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

FSMesh* FETorus::BuildMesh()
{
//	return BuildMeshLegacy();
	return BuildMultiBlockMesh();
}

bool FETorus::BuildMultiBlock()
{
	GTorus* po = dynamic_cast<GTorus*>(&m_o);
	if (po == 0) return false;

	// get the object parameters
	double Ro = po->GetFloatValue(GTorus::RIN);
	double Ri = po->GetFloatValue(GTorus::ROUT);

	double a = Ri / sqrt(2.0);
	double b = a * 0.5;

	// get mesh parameters
	int nd = GetIntValue(NDIV);
	int ns = GetIntValue(NSEG);

	// check parameters
	if (nd < 1) nd = 1;
	if (ns < 1) ns = 1;

	// create nodes
	m_MBNode.resize(68);
	m_MBNode[ 0].m_r = vec3d(Ro - b, 0, -b);
	m_MBNode[ 1].m_r = vec3d(Ro    , 0, -b);
	m_MBNode[ 2].m_r = vec3d(Ro + b, 0, -b);
	m_MBNode[ 3].m_r = vec3d(Ro - b, 0,  0);
	m_MBNode[ 4].m_r = vec3d(Ro    , 0,  0);
	m_MBNode[ 5].m_r = vec3d(Ro + b, 0,  0);
	m_MBNode[ 6].m_r = vec3d(Ro - b, 0,  b);
	m_MBNode[ 7].m_r = vec3d(Ro    , 0,  b);
	m_MBNode[ 8].m_r = vec3d(Ro + b, 0,  b);
	m_MBNode[ 9].m_r = vec3d(Ro + Ri, 0, 0);
	m_MBNode[10].m_r = vec3d(Ro + a, 0,  a);
	m_MBNode[11].m_r = vec3d(Ro    , 0, Ri);
	m_MBNode[12].m_r = vec3d(Ro - a, 0,  a);
	m_MBNode[13].m_r = vec3d(Ro -Ri, 0,  0);
	m_MBNode[14].m_r = vec3d(Ro - a, 0, -a);
	m_MBNode[15].m_r = vec3d(Ro    , 0,-Ri);
	m_MBNode[16].m_r = vec3d(Ro +a , 0, -a);

	m_MBNode[17].m_r = vec3d(0, Ro - b,  -b);
	m_MBNode[18].m_r = vec3d(0, Ro    ,  -b);
	m_MBNode[19].m_r = vec3d(0, Ro + b,  -b);
	m_MBNode[20].m_r = vec3d(0, Ro - b,   0);
	m_MBNode[21].m_r = vec3d(0, Ro    ,   0);
	m_MBNode[22].m_r = vec3d(0, Ro + b,   0);
	m_MBNode[23].m_r = vec3d(0, Ro - b,   b);
	m_MBNode[24].m_r = vec3d(0, Ro    ,   b);
	m_MBNode[25].m_r = vec3d(0, Ro + b,   b);
	m_MBNode[26].m_r = vec3d(0, Ro + Ri,  0);
	m_MBNode[27].m_r = vec3d(0, Ro + a,   a);
	m_MBNode[28].m_r = vec3d(0, Ro    ,  Ri);
	m_MBNode[29].m_r = vec3d(0, Ro - a,   a);
	m_MBNode[30].m_r = vec3d(0, Ro -Ri,   0);
	m_MBNode[31].m_r = vec3d(0, Ro - a,  -a);
	m_MBNode[32].m_r = vec3d(0, Ro    , -Ri);
	m_MBNode[33].m_r = vec3d(0, Ro +a ,  -a);

	m_MBNode[34].m_r = vec3d(-Ro + b, 0, -b);
	m_MBNode[35].m_r = vec3d(-Ro    , 0, -b);
	m_MBNode[36].m_r = vec3d(-Ro - b, 0, -b);
	m_MBNode[37].m_r = vec3d(-Ro + b, 0,  0);
	m_MBNode[38].m_r = vec3d(-Ro    , 0,  0);
	m_MBNode[39].m_r = vec3d(-Ro - b, 0,  0);
	m_MBNode[40].m_r = vec3d(-Ro + b, 0,  b);
	m_MBNode[41].m_r = vec3d(-Ro    , 0,  b);
	m_MBNode[42].m_r = vec3d(-Ro - b, 0,  b);
	m_MBNode[43].m_r = vec3d(-Ro - Ri, 0, 0);
	m_MBNode[44].m_r = vec3d(-Ro - a, 0,  a);
	m_MBNode[45].m_r = vec3d(-Ro    , 0, Ri);
	m_MBNode[46].m_r = vec3d(-Ro + a, 0,  a);
	m_MBNode[47].m_r = vec3d(-Ro +Ri, 0,  0);
	m_MBNode[48].m_r = vec3d(-Ro + a, 0, -a);
	m_MBNode[49].m_r = vec3d(-Ro    , 0,-Ri);
	m_MBNode[50].m_r = vec3d(-Ro -a , 0, -a);

	m_MBNode[51].m_r = vec3d(0, -Ro + b, -b);
	m_MBNode[52].m_r = vec3d(0, -Ro    , -b);
	m_MBNode[53].m_r = vec3d(0, -Ro - b, -b);
	m_MBNode[54].m_r = vec3d(0, -Ro + b, 0);
	m_MBNode[55].m_r = vec3d(0, -Ro    , 0);
	m_MBNode[56].m_r = vec3d(0, -Ro - b, 0);
	m_MBNode[57].m_r = vec3d(0, -Ro + b, b);
	m_MBNode[58].m_r = vec3d(0, -Ro    , b);
	m_MBNode[59].m_r = vec3d(0, -Ro - b, b);
	m_MBNode[60].m_r = vec3d(0, -Ro - Ri, 0);
	m_MBNode[61].m_r = vec3d(0, -Ro - a, a);
	m_MBNode[62].m_r = vec3d(0, -Ro    , Ri);
	m_MBNode[63].m_r = vec3d(0, -Ro + a, a);
	m_MBNode[64].m_r = vec3d(0, -Ro + Ri, 0);
	m_MBNode[65].m_r = vec3d(0, -Ro + a, -a);
	m_MBNode[66].m_r = vec3d(0, -Ro    , -Ri);
	m_MBNode[67].m_r = vec3d(0, -Ro - a, -a);

	// build the blocks
	m_MBlock.resize(48);
	MBBlock* B = &m_MBlock[0];
	B[0 ].SetID(0); B[0 ].SetNodes(0,  1, 18, 17, 3,  4, 21, 20); B[0 ].SetSizes(nd,ns,nd);
	B[1 ].SetID(0); B[1 ].SetNodes(1,  2, 19, 18, 4,  5, 22, 21); B[1 ].SetSizes(nd,ns,nd);
	B[2 ].SetID(0); B[2 ].SetNodes(3,  4, 21, 20, 6,  7, 24, 23); B[2 ].SetSizes(nd,ns,nd);
	B[3 ].SetID(0); B[3 ].SetNodes(4,  5, 22, 21, 7,  8, 25, 24); B[3 ].SetSizes(nd,ns,nd);
	B[4 ].SetID(0); B[4 ].SetNodes(5,  9, 26, 22, 8, 10, 27, 25); B[4 ].SetSizes(nd,ns,nd);
	B[5 ].SetID(0); B[5 ].SetNodes(8, 10, 27, 25, 7, 11, 28, 24); B[5 ].SetSizes(nd,ns,nd);
	B[6 ].SetID(0); B[6 ].SetNodes(7, 11, 28, 24, 6, 12, 29, 23); B[6 ].SetSizes(nd,ns,nd);
	B[7 ].SetID(0); B[7 ].SetNodes(6, 12, 29, 23, 3, 13, 30, 20); B[7 ].SetSizes(nd,ns,nd);
	B[8 ].SetID(0); B[8 ].SetNodes(3, 13, 30, 20, 0, 14, 31, 17); B[8 ].SetSizes(nd,ns,nd);
	B[9 ].SetID(0); B[9 ].SetNodes(0, 14, 31, 17, 1, 15, 32, 18); B[9 ].SetSizes(nd,ns,nd);
	B[10].SetID(0); B[10].SetNodes(1, 15, 32, 18, 2, 16, 33, 19); B[10].SetSizes(nd,ns,nd);
	B[11].SetID(0); B[11].SetNodes(2, 16, 33, 19, 5,  9, 26, 22); B[11].SetSizes(nd,ns,nd);

	for (int i = 1; i <=3; ++i)
	{
		for (int j = 0; j < 12; ++j)
		{
			MBBlock& ba = m_MBlock[j];
			MBBlock& bj = m_MBlock[i * 12 + j];
			bj.SetID(0);
			bj.SetSizes(nd, ns, nd);
			for (int k = 0; k < 8; ++k) bj.m_node[k] = (ba.m_node[k] + i * 17)%68;
		}
	}

	BuildMB();

	// set uniform smoothing ID
	for (int i = 0; i < m_MBFace.size(); ++i) m_MBFace[i].m_sid = 0;

	for (int i=0; i<4; ++i)
	{
		SetBlockFaceID(B[ 0 + 12*i], -1, -1, -1, -1, -1, -1);
		SetBlockFaceID(B[ 1 + 12*i], -1, -1, -1, -1, -1, -1);
		SetBlockFaceID(B[ 2 + 12*i], -1, -1, -1, -1, -1, -1);
		SetBlockFaceID(B[ 3 + 12*i], -1, -1, -1, -1, -1, -1);
		SetBlockFaceID(B[ 4 + 12*i], -1,  (3 + (i+1)*4)%16, -1, -1, -1, -1);
		SetBlockFaceID(B[ 5 + 12*i], -1,  (3 + (i+1)*4)%16, -1, -1, -1, -1);
		SetBlockFaceID(B[ 6 + 12*i], -1,  (2 + (i+1)*4)%16, -1, -1, -1, -1);
		SetBlockFaceID(B[ 7 + 12*i], -1,  (2 + (i+1)*4)%16, -1, -1, -1, -1);
		SetBlockFaceID(B[ 8 + 12*i], -1,  (1 + (i+1)*4)%16, -1, -1, -1, -1);
		SetBlockFaceID(B[ 9 + 12*i], -1,  (1 + (i+1)*4)%16, -1, -1, -1, -1);
		SetBlockFaceID(B[10 + 12*i], -1,  (0 + (i+1)*4)%16, -1, -1, -1, -1);
		SetBlockFaceID(B[11 + 12*i], -1,  (0 + (i+1)*4)%16, -1, -1, -1, -1);
	}

	// set edges
	for (int i = 0; i < 4; ++i)
	{
		GetBlockEdge( 0 + i * 12, 3).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 0 + i * 12, 1).SetEdge(EDGE_ZARC,  1);
		GetBlockEdge( 1 + i * 12, 1).SetEdge(EDGE_ZARC,  1);
		GetBlockEdge( 2 + i * 12, 3).SetEdge(EDGE_ZARC,  1);
		GetBlockEdge( 2 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 3 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 2 + i * 12, 7).SetEdge(EDGE_ZARC,  1);
		GetBlockEdge( 2 + i * 12, 5).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 3 + i * 12, 5).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 4 + i * 12, 1).SetEdge(EDGE_ZARC,  1);
		GetBlockEdge( 5 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 6 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 7 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 8 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge( 9 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge(10 + i * 12, 1).SetEdge(EDGE_ZARC, -1);
		GetBlockEdge(11 + i * 12, 1).SetEdge(EDGE_ZARC, -1);

		for (int j=0; j<8; ++j)
			GetBlockEdge(4 + j + i*12, 9).SetEdge(EDGE_3P_CIRC_ARC, 1, 4 + i*17);
	}

	// set edge IDs
	GetBlockEdge(40, 1).SetID(0);
	GetBlockEdge( 4, 1).SetID(1);
	GetBlockEdge(16, 1).SetID(2);
	GetBlockEdge(28, 1).SetID(3);

	GetBlockEdge(46, 1).SetID(4);
	GetBlockEdge(10, 1).SetID(5);
	GetBlockEdge(22, 1).SetID(6);
	GetBlockEdge(34, 1).SetID(7);
	
	GetBlockEdge(44, 1).SetID(8);
	GetBlockEdge( 8, 1).SetID(9);
	GetBlockEdge(20, 1).SetID(10);
	GetBlockEdge(32, 1).SetID(11);

	GetBlockEdge(42, 1).SetID(12);
	GetBlockEdge( 6, 1).SetID(13);
	GetBlockEdge(18, 1).SetID(14);
	GetBlockEdge(30, 1).SetID(15);

	GetBlockEdge(46, 9).SetID(16); GetBlockEdge(47, 9).SetID(16);
	GetBlockEdge(44, 9).SetID(17); GetBlockEdge(45, 9).SetID(17);
	GetBlockEdge(42, 9).SetID(18); GetBlockEdge(43, 9).SetID(18);
	GetBlockEdge(40, 9).SetID(19); GetBlockEdge(41, 9).SetID(19);

	GetBlockEdge(10, 9).SetID(20); GetBlockEdge(11, 9).SetID(20);
	GetBlockEdge(8 , 9).SetID(21); GetBlockEdge( 9, 9).SetID(21);
	GetBlockEdge(6 , 9).SetID(22); GetBlockEdge( 7, 9).SetID(22);
	GetBlockEdge(4 , 9).SetID(23); GetBlockEdge( 5, 9).SetID(23);

	GetBlockEdge(22, 9).SetID(24); GetBlockEdge(23, 9).SetID(24);
	GetBlockEdge(20, 9).SetID(25); GetBlockEdge(21, 9).SetID(25);
	GetBlockEdge(18, 9).SetID(26); GetBlockEdge(19, 9).SetID(26);
	GetBlockEdge(16, 9).SetID(27); GetBlockEdge(17, 9).SetID(27);

	GetBlockEdge(34, 9).SetID(28); GetBlockEdge(35, 9).SetID(28);
	GetBlockEdge(32, 9).SetID(29); GetBlockEdge(33, 9).SetID(29);
	GetBlockEdge(30, 9).SetID(30); GetBlockEdge(31, 9).SetID(30);
	GetBlockEdge(28, 9).SetID(31); GetBlockEdge(29, 9).SetID(31);

	// set node IDs.
	m_MBNode[60].SetID(0);
	m_MBNode[66].SetID(1);
	m_MBNode[64].SetID(2);
	m_MBNode[62].SetID(3);

	m_MBNode[ 9].SetID(4);
	m_MBNode[15].SetID(5);
	m_MBNode[13].SetID(6);
	m_MBNode[11].SetID(7);

	m_MBNode[26].SetID(8);
	m_MBNode[32].SetID(9);
	m_MBNode[30].SetID(10);
	m_MBNode[28].SetID(11);

	m_MBNode[43].SetID(12);
	m_MBNode[49].SetID(13);
	m_MBNode[47].SetID(14);
	m_MBNode[45].SetID(15);

	UpdateMB();

	return true;
}

FSMesh* FETorus::BuildMultiBlockMesh()
{
	if (!BuildMultiBlock()) return nullptr;

	// set element type
	int nelem = GetIntValue(ELEM_TYPE);
	switch (nelem)
	{
	case 0: SetElementType(FE_HEX8); break;
	case 1: SetElementType(FE_HEX20); break;
	case 2: SetElementType(FE_HEX27); break;
	}

	return FEMultiBlockMesh::BuildMBMesh();
}

FSMesh* FETorus::BuildMeshLegacy()
{
	GTorus* po = dynamic_cast<GTorus*>(&m_o);
	if (po == 0) return nullptr;

	int i, j, k;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double R0 = param.GetFloatValue(GTorus::RIN);
	double R1 = param.GetFloatValue(GTorus::ROUT);

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;

	int ns = 4*m_ns;
	int nd = 2*m_nd;

	// count nodes and elements
	int nodes = ns*((nd+1)*(nd+1) + 4*nd*nd);
	int elems = ns*(nd*nd+4*nd*nd);

	// allocate storage for mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems);

	// --- create the first layer of nodes ---
	// create the inner nodes
	FSNode* pn = pm->NodePtr();
	vec3d r;
	double h = R1/sqrt(2.0)*0.5;
	double R, f, dr;
	for (i=0; i<=nd; ++i)
		for (j=0; j<=nd; ++j, ++pn)
		{
			r.x = 0;
			r.y = -h + 2*h*i/nd;
			r.z = -h + 2*h*j/nd;

			R = r.Length();
			f = 1 - 0.15*R/h;

			pn->r = vec3d(0,-R0,0) + r*f;
		}

	// create node index loop
	std::vector<int>	Nd; Nd.resize(4*nd);
	std::vector<vec3d> Nr; Nr.resize(4*nd);
	for (i=0; i<nd; ++i) Nd[i     ] = nd - i - 1;
	for (i=0; i<nd; ++i) Nd[i+nd  ] = (i+1)*(nd+1);
	for (i=0; i<nd; ++i) Nd[i+nd*2] = i + nd*(nd+1)+1;
	for (i=0; i<nd; ++i) Nd[i+nd*3] = nd*(nd+1) - i*(nd+1) - 1;

	for (i=0; i<4*nd; ++i) Nr[i] = pm->Node(Nd[i]).r;

	// create the loop nodes
	int noff = (nd+1)*(nd+1);
	for (k=0; k<nd; ++k)
	{
		// create the nodes
		for (i=0; i<4*nd; ++i, ++pn)
		{
			r = Nr[i] - vec3d(0,-R0,0);
			R = r.Length();
			dr = (k+1)*(R1 - R)/nd;
			f = (R + dr)/R;
			pn->r = vec3d(0,-R0,0) + r*f;	
		}
	}

	// --- create the mesh
	int eid = 0;
	int nn = (nd+1)*(nd+1) + 4*nd*nd;
	double cw = cos(-2.*PI/ns);
	double sw = sin(-2.*PI/ns);
	for (k=0; k<ns; ++k)
	{
		// create the next nodes
		if (k<ns-1)
		{
			for (i=0; i<nn; ++i, ++pn)
			{
				r = pn[-nn].r;

				pn->r.x =  cw*r.x + sw*r.y;
				pn->r.y = -sw*r.x + cw*r.y;
				pn->r.z = r.z;
			}
		}

		// reset node loop
		noff = k*nn + (nd+1)*(nd+1);
		for (i=0; i<nd; ++i) Nd[i     ] = k*nn + nd - i - 1;
		for (i=0; i<nd; ++i) Nd[i+nd  ] = k*nn + (i+1)*(nd+1);
		for (i=0; i<nd; ++i) Nd[i+nd*2] = k*nn + i + nd*(nd+1)+1;
		for (i=0; i<nd; ++i) Nd[i+nd*3] = k*nn + nd*(nd+1) - i*(nd+1) - 1;

		// create the inner elements
		for (i=0; i<nd; ++i)
			for (j=0; j<nd; ++j)
			{
				FSElement_* pe = pm->ElementPtr(eid++);

				int* n = pe->m_node;

				n[0] = k*nn + j*(nd+1) + i;
				n[1] = k*nn + (j+1)*(nd+1) + i;
				n[2] = k*nn + (j+1)*(nd+1) + i+1;
				n[3] = k*nn + j*(nd+1) + i + 1;
				n[4] = (k<ns-1? n[0] + nn : n[0] - k*nn);
				n[5] = (k<ns-1? n[1] + nn : n[1] - k*nn);
				n[6] = (k<ns-1? n[2] + nn : n[2] - k*nn);
				n[7] = (k<ns-1? n[3] + nn : n[3] - k*nn);

				pe->SetType(FE_HEX8);
				pe->m_gid = 0;
			}

		// create the outer elements
		for (j=0; j<nd; ++j)
		{
			int* n;
			for (i=0; i<4*nd; ++i)
			{
				FSElement_* pe = pm->ElementPtr(eid++);

				n = pe->m_node;

				n[0] = Nd[i];
				n[1] = noff + i;
				n[2] = noff + (i+1)%(4*nd);
				n[3] = Nd[(i+1)%(4*nd)];
				n[4] = (k<ns-1? n[0] + nn : n[0] - k*nn);
				n[5] = (k<ns-1? n[1] + nn : n[1] - k*nn);
				n[6] = (k<ns-1? n[2] + nn : n[2] - k*nn);
				n[7] = (k<ns-1? n[3] + nn : n[3] - k*nn);

				pe->SetType(FE_HEX8);
				pe->m_gid = 0;
			}

			// set the next layer of node indices
			for (i=0; i<4*nd; ++i) Nd[i] = noff+i;
			noff += 4*nd;
		}
	}

	pm->Node(NodeIndex(   0,0)).m_gid = 0;
	pm->Node(NodeIndex(  nd,0)).m_gid = 1;
	pm->Node(NodeIndex(2*nd,0)).m_gid = 2;
	pm->Node(NodeIndex(3*nd,0)).m_gid = 3;

	pm->Node(NodeIndex(   0, m_ns)).m_gid = 4;
	pm->Node(NodeIndex(  nd, m_ns)).m_gid = 5;
	pm->Node(NodeIndex(2*nd, m_ns)).m_gid = 6;
	pm->Node(NodeIndex(3*nd, m_ns)).m_gid = 7;

	pm->Node(NodeIndex(   0, 2*m_ns)).m_gid = 8;
	pm->Node(NodeIndex(  nd, 2*m_ns)).m_gid = 9;
	pm->Node(NodeIndex(2*nd, 2*m_ns)).m_gid = 10;
	pm->Node(NodeIndex(3*nd, 2*m_ns)).m_gid = 11;

	pm->Node(NodeIndex(   0, 3*m_ns)).m_gid = 12;
	pm->Node(NodeIndex(  nd, 3*m_ns)).m_gid = 13;
	pm->Node(NodeIndex(2*nd, 3*m_ns)).m_gid = 14;
	pm->Node(NodeIndex(3*nd, 3*m_ns)).m_gid = 15;

	BuildFaces(pm);
	BuildEdges(pm);

	pm->BuildMesh();

	return pm;
}

void FETorus::BuildFaces(FSMesh* pm)
{
	int nd = 2*m_nd;
	int ns = 4*m_ns;

	int i, j, n = 0;
	pm->Create(0,0,4*nd*ns);
	for (j=0; j<ns; ++j)
		for (i=0; i<4*nd; ++i, ++n)
		{
			FSFace& f = pm->Face(n);
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 4*(4*j/ns) + i/nd;
			f.n[0] = NodeIndex(i  ,j  );
			f.n[1] = NodeIndex(i+1,j  );
			f.n[2] = NodeIndex(i+1,j+1);
			f.n[3] = NodeIndex(i  ,j+1);
		}
}

void FETorus::BuildEdges(FSMesh* pm)
{
	int nd = 2*m_nd;
	int ns = 4*m_ns;

	int i, j, n=0;
	pm->Create(0,0,0,4*ns + 16*nd);
	for (j=0; j<4; ++j)
		for (i=0; i<ns; ++i, ++n)
		{
			FSEdge& e = pm->Edge(n);
			e.SetType(FE_EDGE2);
			e.m_gid = 4*j + 4*i/ns;
			e.n[0] = NodeIndex(j*nd, i);
			e.n[1] = NodeIndex(j*nd, i+1);
		}

	for (j=0; j<4; ++j)
		for (i=0; i<4*nd; ++i, ++n)
		{
			FSEdge& e = pm->Edge(n);
			e.SetType(FE_EDGE2);
			e.m_gid = 16 + j * 4 + i / nd;
			e.n[0] = NodeIndex(i  , j*ns/4);
			e.n[1] = NodeIndex(i+1, j*ns/4);
		}
}
