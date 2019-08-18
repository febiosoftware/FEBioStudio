// FESphere.cpp: implementation of the FESphere class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FESphere.h"
#include <GeomLib/GPrimitive.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


FESphere::FESphere(GSphere* po)
{
	m_pobj = po;

	m_r = 0.5;

	m_nseg = m_ndiv = 4;

	m_gd = 1;
	m_gr = 1;

	m_bd = false;
	m_br = false;

	AddDoubleParam(m_r, "r", "Ratio");
	AddIntParam(m_nseg, "ns", "Segments");
	AddIntParam(m_ndiv, "nd", "Divisions");

	AddDoubleParam(m_gd, "gd", "D-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");
	AddBoolParam(m_bd, "bd", "D-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");
}

FEMesh* FESphere::BuildMesh()
{
	assert(m_pobj);

	// get the object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R1 = param.GetFloatValue(GSphere::RADIUS);

	// get parameters
	double R0 = GetFloatValue(RATIO);
	m_ndiv = GetIntValue(NDIV);
	m_nseg = GetIntValue(NSEG);
	m_r = GetFloatValue(RATIO);
	int nd = m_ndiv;
	int ns = m_nseg;

	m_gd = GetFloatValue(GD);
	m_gr = GetFloatValue(GR);
	m_bd = GetBoolValue(GD2);
	m_br = GetBoolValue(GR2);


	R0 *= R1;

	double d0 = R0/sqrt(2.0);
	double d1 = R1/sqrt(2.0);

	// check parameters
	if (nd < 1) nd = 1;
	if (ns < 1) ns = 1;

	// create the MB nodes
	m_MBNode.resize(53);
	m_MBNode[ 0].m_r = vec3d(-d0,-d0,-d0);
	m_MBNode[ 1].m_r = vec3d(  0,-d0,-d0);
	m_MBNode[ 2].m_r = vec3d( d0,-d0,-d0);
	m_MBNode[ 3].m_r = vec3d(-d0,  0,-d0);
	m_MBNode[ 4].m_r = vec3d(  0,  0,-d0);
	m_MBNode[ 5].m_r = vec3d( d0,  0,-d0);
	m_MBNode[ 6].m_r = vec3d(-d0, d0,-d0);
	m_MBNode[ 7].m_r = vec3d(  0, d0,-d0);
	m_MBNode[ 8].m_r = vec3d( d0, d0,-d0);
	m_MBNode[ 9].m_r = vec3d(-d0,-d0,  0);
	m_MBNode[10].m_r = vec3d(  0,-d0,  0);
	m_MBNode[11].m_r = vec3d( d0,-d0,  0);
	m_MBNode[12].m_r = vec3d(-d0,  0,  0);
	m_MBNode[13].m_r = vec3d(  0,  0,  0);
	m_MBNode[14].m_r = vec3d( d0,  0,  0);
	m_MBNode[15].m_r = vec3d(-d0, d0,  0);
	m_MBNode[16].m_r = vec3d(  0, d0,  0);
	m_MBNode[17].m_r = vec3d( d0, d0,  0);
	m_MBNode[18].m_r = vec3d(-d0,-d0, d0);
	m_MBNode[19].m_r = vec3d(  0,-d0, d0);
	m_MBNode[20].m_r = vec3d( d0,-d0, d0);
	m_MBNode[21].m_r = vec3d(-d0,  0, d0);
	m_MBNode[22].m_r = vec3d(  0,  0, d0);
	m_MBNode[23].m_r = vec3d( d0,  0, d0);
	m_MBNode[24].m_r = vec3d(-d0, d0, d0);
	m_MBNode[25].m_r = vec3d(  0, d0, d0);
	m_MBNode[26].m_r = vec3d( d0, d0, d0);

	m_MBNode[27].m_r = vec3d(-d1,-d1,-d1);
	m_MBNode[28].m_r = vec3d(  0,-d1,-d1);
	m_MBNode[29].m_r = vec3d( d1,-d1,-d1);
	m_MBNode[30].m_r = vec3d(-d1,  0,-d1);
	m_MBNode[31].m_r = vec3d(  0,  0,-d1);
	m_MBNode[32].m_r = vec3d( d1,  0,-d1);
	m_MBNode[33].m_r = vec3d(-d1, d1,-d1);
	m_MBNode[34].m_r = vec3d(  0, d1,-d1);
	m_MBNode[35].m_r = vec3d( d1, d1,-d1);

	m_MBNode[36].m_r = vec3d(-d1, -d1, 0);
	m_MBNode[37].m_r = vec3d(  0, -d1, 0);
	m_MBNode[38].m_r = vec3d( d1, -d1, 0);
	m_MBNode[39].m_r = vec3d(-d1,   0, 0);
	m_MBNode[40].m_r = vec3d( d1,   0, 0);
	m_MBNode[41].m_r = vec3d(-d1,  d1, 0);
	m_MBNode[42].m_r = vec3d(  0,  d1, 0);
	m_MBNode[43].m_r = vec3d( d1,  d1, 0);

	m_MBNode[44].m_r = vec3d(-d1,-d1, d1);
	m_MBNode[45].m_r = vec3d(  0,-d1, d1);
	m_MBNode[46].m_r = vec3d( d1,-d1, d1);
	m_MBNode[47].m_r = vec3d(-d1,  0, d1);
	m_MBNode[48].m_r = vec3d(  0,  0, d1);
	m_MBNode[49].m_r = vec3d( d1,  0, d1);
	m_MBNode[50].m_r = vec3d(-d1, d1, d1);
	m_MBNode[51].m_r = vec3d(  0, d1, d1);
	m_MBNode[52].m_r = vec3d( d1, d1, d1);

	// create the MB block
	m_MBlock.resize(32);

	int MB[32][8] = {
		{ 0,  1,  4,  3,  9, 10, 13, 12},	// 0 --- inner blocks
		{ 1,  2,  5,  4, 10, 11, 14, 13},	// 1
		{ 3,  4,  7,  6, 12, 13, 16, 15},	// 2
		{ 4,  5,  8,  7, 13, 14, 17, 16},	// 3
		{ 9, 10, 13, 12, 18, 19, 22, 21},	// 4
		{10, 11, 14, 13, 19, 20, 23, 22},	// 5
		{12, 13, 16, 15, 21, 22, 25, 24},	// 6
		{13, 14, 17, 16, 22, 23, 26, 25},	// 7
		{ 0, 27, 28,  1,  9, 36, 37, 10}, 	// 8 --- side block 0
		{ 1, 28, 29,  2, 10, 37, 38, 11},	// 9
		{ 9, 36, 37, 10, 18, 44, 45, 19},	// 10
		{10, 37, 38, 11, 19, 45, 46, 20},	// 11
		{ 2, 29, 32,  5, 11, 38, 40, 14},	// 12 --- side block 1
		{ 5, 32, 35,  8, 14, 40, 43, 17},	// 13 
		{11, 38, 40, 14, 20, 46, 49, 23},	// 14
		{14, 40, 43, 17, 23, 49, 52, 26},	// 15
		{ 8, 35, 34,  7, 17, 43, 42, 16},	// 16 --- side block 2
		{ 7, 34, 33,  6, 16, 42, 41, 15},	// 17
		{17, 43, 42, 16, 26, 52, 51, 25},	// 18
		{16, 42, 41, 15, 25, 51, 50, 24},	// 19
		{ 6, 33, 30,  3, 15, 41, 39, 12},	// 20 --- side block 3
		{ 3, 30, 27,  0, 12, 39, 36,  9},	// 21
		{15, 41, 39, 12, 24, 50, 47, 21},	// 22
		{12, 39, 36,  9, 21, 47, 44, 18},	// 23
		{ 0, 27, 30,  3,  1, 28, 31,  4},	// 24 --- side block 4
		{ 1, 28, 31,  4,  2, 29, 32,  5},	// 25
		{ 3, 30, 33,  6,  4, 31, 34,  7},	// 26
		{ 4, 31, 34,  7,  5, 32, 35,  8},	// 27
		{18, 44, 45, 19, 21, 47, 48, 22},	// 28 --- side block 5
		{19, 45, 46, 20, 22, 48, 49, 23},	// 29
		{21, 47, 48, 22, 24, 50, 51, 25},	// 30
		{22, 48, 49, 23, 25, 51, 52, 26}	// 31
	}; 

	int i, *n;
	for (i=0; i<8; ++i)
	{
		n = MB[i];
		m_MBlock[i].SetNodes(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
		m_MBlock[i].SetSizes(nd,nd,nd);
		m_MBlock[i].SetID(0);
	}
	for (i=8; i<32; ++i)
	{
		n = MB[i];
		m_MBlock[i].SetNodes(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
		m_MBlock[i].SetSizes(ns,nd,nd);
		m_MBlock[i].SetID(0);
	}

	// update the MB data
	UpdateMB();

	// Face ID's
	GetBlockFace( 8, 1).SetID(2);
	GetBlockFace( 9, 1).SetID(3);
	GetBlockFace(10, 1).SetID(6);
	GetBlockFace(11, 1).SetID(7);

	GetBlockFace(12, 1).SetID(3);
	GetBlockFace(13, 1).SetID(0);
	GetBlockFace(14, 1).SetID(7);
	GetBlockFace(15, 1).SetID(4);

	GetBlockFace(16, 1).SetID(0);
	GetBlockFace(17, 1).SetID(1);
	GetBlockFace(18, 1).SetID(4);
	GetBlockFace(19, 1).SetID(5);

	GetBlockFace(20, 1).SetID(1);
	GetBlockFace(21, 1).SetID(2);
	GetBlockFace(22, 1).SetID(5);
	GetBlockFace(23, 1).SetID(6);

	GetBlockFace(24, 1).SetID(2);
	GetBlockFace(25, 1).SetID(3);
	GetBlockFace(26, 1).SetID(1);
	GetBlockFace(27, 1).SetID(0);

	GetBlockFace(28, 1).SetID(6);
	GetBlockFace(29, 1).SetID(7);
	GetBlockFace(30, 1).SetID(5);
	GetBlockFace(31, 1).SetID(4);

	// Edge ID's
	GetFaceEdge(GetBlockFace(15,1),0).SetID(0);
	GetFaceEdge(GetBlockFace(18,1),0).SetID(0);
	GetFaceEdge(GetBlockFace(19,1),0).SetID(1);
	GetFaceEdge(GetBlockFace(22,1),0).SetID(1);
	GetFaceEdge(GetBlockFace(23,1),0).SetID(2);
	GetFaceEdge(GetBlockFace(10,1),0).SetID(2);
	GetFaceEdge(GetBlockFace(11,1),0).SetID(3);
	GetFaceEdge(GetBlockFace(14,1),0).SetID(3);

	GetFaceEdge(GetBlockFace(15,1),3).SetID(4);
	GetFaceEdge(GetBlockFace(31,1),0).SetID(4);
	GetFaceEdge(GetBlockFace(19,1),3).SetID(5);
	GetFaceEdge(GetBlockFace(31,1),3).SetID(5);
	GetFaceEdge(GetBlockFace(22,1),1).SetID(6);
	GetFaceEdge(GetBlockFace(28,1),2).SetID(6);
	GetFaceEdge(GetBlockFace(11,1),3).SetID(7);
	GetFaceEdge(GetBlockFace(28,1),1).SetID(7);

	GetFaceEdge(GetBlockFace(12,1),1).SetID(8);
	GetFaceEdge(GetBlockFace(27,1),3).SetID(8);
	GetFaceEdge(GetBlockFace(16,1),1).SetID(9);
	GetFaceEdge(GetBlockFace(27,1),0).SetID(9);
	GetFaceEdge(GetBlockFace(20,1),1).SetID(10);
	GetFaceEdge(GetBlockFace(24,1),1).SetID(10);
	GetFaceEdge(GetBlockFace( 8,1),1).SetID(11);
	GetFaceEdge(GetBlockFace(24,1),2).SetID(11);


	// Node ID's
	m_MBNode[40].SetID(0);
	m_MBNode[42].SetID(1);
	m_MBNode[39].SetID(2);
	m_MBNode[37].SetID(3);
	m_MBNode[31].SetID(4);
	m_MBNode[48].SetID(5);

	// create the MB
	FEMesh* pm = FEMultiBlockMesh::BuildMesh();

	// project the nodes onto a sphere
	double d, w, x, y, z;
	vec3d r0, r1;
	for (i=0; i<pm->Nodes(); ++i)
	{
		vec3d& r = pm->Node(i).r;
		r0 = r;
		x = fabs(r0.x);
		y = fabs(r0.y);
		z = fabs(r0.z);
		d = fmax(fmax(x,y),z);
		r0 /= d/d0;
		w = (d-d0)/(d1 - d0);
		if (w > 0)
		{
			r1 = r0;
			r1.Normalize();
			r1 *= R1;
			r.x = r1.x*w + r0.x*(1-w);
			r.y = r1.y*w + r0.y*(1-w);
			r.z = r1.z*w + r0.z*(1-w);
		}
	}

	// update the mesh
	pm->Update();

	// the Multi-block mesher will assign a different smoothing ID
	// to each face, but we don't want that here. Instead we assign
	// to each face the same smoothing ID
	for (i=0; i<pm->Faces(); ++i) pm->Face(i).m_sid = 0;

	// finally, we update the normals and we are good to go
	pm->UpdateNormals();

	return pm;
}

/*
void FESphere::Create()
{
	int i, j, k, n, m;

	// get parameters
	double R0 = m_Param.GetFloatValue(RIN);
	double R1 = m_Param.GetFloatValue(ROUT);

	int ndiv = m_Param.GetIntValue(NDIV);
	int nseg = m_Param.GetIntValue(NSEG);

	R0 *= R1;

	// check parameters
	if (ndiv < 1) ndiv = 1;
	if (nseg < 1) nseg = 1;

	int N = ndiv;
	int M = nseg;

	int bnodes = (N+1)*(N+1)*(N+1);		// nodes in the box
	int snodes = (N+1)*(6*(N+1)-12)+8;	// nodes on the surface of the box

	int belems = N*N*N;	  // box elements
	int selems = 6*(N*N); // one layer of sphere elements


	int nodes = bnodes + M*snodes;
	int elems = belems + M*selems;

	// allocate storage
	bool bnew = false;
	if ((m_ndiv != ndiv) || (m_nseg != nseg))
	{
		FEMesh::Create(nodes, elems);
		m_ndiv = ndiv;
		m_nseg = nseg;
		bnew = true;
		for (i=0; i<elems; ++i) Element(i).m_pmat = 0;
	}

	double h = R0 / (double)sqrt(2.0);

	// --- create inner box ---

	// a. create box nodes
	n = 0;
	double x, y, z;
	double g = 0.45f;

	double* fg = new double[N+1];
	for (i=0; i<=N; ++i) fg[i] = gain(g, (double) i / (double) N);

	for (k=0; k<=N; ++k)
	{
		z = -h + 2.f*h*fg[k];

		for (j=0; j<=N; ++j)
		{
			y = -h + 2.f*h*fg[j];

			for (i=0; i<=N; ++i, ++n)
			{
				FENode& node = Node(n);

				x = -h + 2.f*h*fg[i];

				node.r = vec3d(x,y,z);
			}
		}
	}

	delete [] fg;

	// b. create elements
	if (bnew)
	{
		n = 0;
		for (k=0; k<N; ++k)
			for (j=0; j<N; ++j)
				for (i=0; i<N; ++i, ++n)
				{
					FEElement& el = Element(n);
					el.m_ntype = FE_HEX8;
					el.m_gid = 0;
					el.m_node[0] = k*(N+1)*(N+1) + j*(N+1) + i;
					el.m_node[1] = k*(N+1)*(N+1) + j*(N+1) + i+1;
					el.m_node[2] = k*(N+1)*(N+1) + (j+1)*(N+1) + i+1;
					el.m_node[3] = k*(N+1)*(N+1) + (j+1)*(N+1) + i;
					el.m_node[4] = (k+1)*(N+1)*(N+1) + j*(N+1) + i;
					el.m_node[5] = (k+1)*(N+1)*(N+1) + j*(N+1) + i+1;
					el.m_node[6] = (k+1)*(N+1)*(N+1) + (j+1)*(N+1) + i+1;
					el.m_node[7] = (k+1)*(N+1)*(N+1) + (j+1)*(N+1) + i;
				}
	}

	// --- create the outer sphere ---

	// a. create nodes
	int* pnode = new int[snodes];
	int* ptag = new int[nodes];
	for (i=0; i<Nodes(); ++i) ptag[i] = -1;

	n = m = 0;
	for (k=0; k<=N; ++k)
		for (j=0; j<=N; ++j)
			for (i=0; i<=N; ++i, ++n)
				if (i==0 || i==N || j==0 || j==N || k==0 || k==N)
				{
					if (ptag[n] == -1)
					{
						pnode[m] = n;
						ptag[n] = bnodes + m;
						m++;
					}

				}

	vec3d norm, r1;
	n = bnodes;
	double f;
	for (i=0; i<M; ++i)
	{
		for (j=0; j<snodes; ++j, ++n)
		{
			FENode& node = Node(n);

			vec3d r0 = Node(pnode[j]).r;
			norm = r0;
			norm.Normalize();

			r1 = norm*R1;

			f = (double)(i+1) / M;

			node.r = r0*(1.f - f) + r1*f;
		}
	}

	// b. create elements
	if (bnew)
	{
		int (*pface)[5] = new int[selems][5];

		m = n = 0;
		for (k=0; k<N; ++k)
			for (j=0; j<N; ++j)
				for (i=0; i<N; ++i, ++n)
					if ((i==0) || (i==N-1) || (j==0) || (j==N-1) || (k==0) || (k==N-1))
					{
						FEElement& el = Element(n);

						if (i==0)
						{
							int* pf = FTHEX8[3];
							pface[m][0] = el.m_node[pf[0]];
							pface[m][1] = el.m_node[pf[1]];
							pface[m][2] = el.m_node[pf[2]];
							pface[m][3] = el.m_node[pf[3]];
							pface[m][4] = 1;
							m++;
						}
						if (i==N-1)
						{
							int* pf = FTHEX8[1];
							pface[m][0] = el.m_node[pf[0]];
							pface[m][1] = el.m_node[pf[1]];
							pface[m][2] = el.m_node[pf[2]];
							pface[m][3] = el.m_node[pf[3]];
							pface[m][4] = 2;
							m++;
						}
						if (j==0)
						{
							int* pf = FTHEX8[0];
							pface[m][0] = el.m_node[pf[0]];
							pface[m][1] = el.m_node[pf[1]];
							pface[m][2] = el.m_node[pf[2]];
							pface[m][3] = el.m_node[pf[3]];
							pface[m][4] = 3;
							m++;
						}
						if (j==N-1)
						{
							int* pf = FTHEX8[2];
							pface[m][0] = el.m_node[pf[0]];
							pface[m][1] = el.m_node[pf[1]];
							pface[m][2] = el.m_node[pf[2]];
							pface[m][3] = el.m_node[pf[3]];
							pface[m][4] = 4;
							m++;
						}
						if (k==0)
						{
							int* pf = FTHEX8[4];
							pface[m][0] = el.m_node[pf[0]];
							pface[m][1] = el.m_node[pf[1]];
							pface[m][2] = el.m_node[pf[2]];
							pface[m][3] = el.m_node[pf[3]];
							pface[m][4] = 5;
							m++;
						}
						if (k==N-1)
						{
							int* pf = FTHEX8[5];
							pface[m][0] = el.m_node[pf[0]];
							pface[m][1] = el.m_node[pf[1]];
							pface[m][2] = el.m_node[pf[2]];
							pface[m][3] = el.m_node[pf[3]];
							pface[m][4] = 6;
							m++;
						}
					}

		n = belems;
		int n1, n2, n3, n4, n5, n6, n7, n8;
		for (m=0; m<M; ++m)
		{
			for (j=0; j<selems; ++j, ++n)
			{
				FEElement& el = Element(n);
				el.m_ntype = FE_HEX8;
				el.m_gid = pface[j][4];

				n1 = pface[j][0];
				n2 = pface[j][1];
				n3 = pface[j][2];
				n4 = pface[j][3];

				n5 = ptag[n1];
				n6 = ptag[n2];
				n7 = ptag[n3];
				n8 = ptag[n4];

				el.m_node[0] = n1;
				el.m_node[1] = n2;
				el.m_node[2] = n3;
				el.m_node[3] = n4;
				el.m_node[4] = n5;
				el.m_node[5] = n6;
				el.m_node[6] = n7;
				el.m_node[7] = n8;
			}

			for (j=0; j<selems; ++j)
			{
				n1 = pface[j][0];
				n2 = pface[j][1];
				n3 = pface[j][2];
				n4 = pface[j][3];

				n5 = ptag[n1];
				n6 = ptag[n2];
				n7 = ptag[n3];
				n8 = ptag[n4];

				pface[j][0] = n5;
				pface[j][1] = n6;
				pface[j][2] = n7;
				pface[j][3] = n8;

				ptag[n5] = n5 + snodes;
				ptag[n6] = n6 + snodes;
				ptag[n7] = n7 + snodes;
				ptag[n8] = n8 + snodes;
			}
		}
		delete [] pface;
	}

	delete [] pnode;
	delete [] ptag;

	Update();
}
*/
