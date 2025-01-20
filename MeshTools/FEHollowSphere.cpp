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

// FEHollowSphere.cpp: implementation of the FEHollowSphere class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEHollowSphere.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEHollowSphere::FEHollowSphere(GHollowSphere* po)
{
	m_pobj = po;
	m_nd = 6;
	m_ns = 1;
	m_gr = 1;
	m_br = false;

	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments");

	AddDoubleParam(m_gr, "gr", "R-bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

double gain2(double x, double r, double n)
{
	if ((r == 1) || (n == 0)) return x;
	return (pow(r, n*x) - 1.0)/(pow(r, n) - 1.0);
}

bool FEHollowSphere::BuildMultiBlock()
{
	assert(m_pobj);

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R0 = param.GetFloatValue(GHollowSphere::RIN);
	double R1 = param.GetFloatValue(GHollowSphere::ROUT);

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_gr = GetFloatValue(GR);
	m_br = GetBoolValue(GR2);

	// get parameters
	int nd = m_nd;
	int ns = m_ns;

	// check parameters
	if (nd < 1) nd   = 1;
	if (ns < 1) ns   = 1;

	double d0 = R0/sqrt(2.0);
	double D0 = R0/sqrt(3.0);
	double d1 = R1/sqrt(2.0);
	double D1 = R1/sqrt(3.0);

	double fr = m_gr;
	bool br = m_br;

	// create the MB nodes
	m_MBNode.resize(52);
	m_MBNode[ 0].m_r = vec3d(-D0,-D0,-D0);
	m_MBNode[ 1].m_r = vec3d(  0,-d0,-d0);
	m_MBNode[ 2].m_r = vec3d( D0,-D0,-D0);
	m_MBNode[ 3].m_r = vec3d(-d0,  0,-d0);
	m_MBNode[ 4].m_r = vec3d(  0,  0,-R0);
	m_MBNode[ 5].m_r = vec3d( d0,  0,-d0);
	m_MBNode[ 6].m_r = vec3d(-D0, D0,-D0);
	m_MBNode[ 7].m_r = vec3d(  0, d0,-d0);
	m_MBNode[ 8].m_r = vec3d( D0, D0,-D0);
	m_MBNode[ 9].m_r = vec3d(-d0,-d0,  0);
	m_MBNode[10].m_r = vec3d(  0,-R0,  0);
	m_MBNode[11].m_r = vec3d( d0,-d0,  0);
	m_MBNode[12].m_r = vec3d(-R0,  0,  0);
	m_MBNode[13].m_r = vec3d( R0,  0,  0);
	m_MBNode[14].m_r = vec3d(-d0, d0,  0);
	m_MBNode[15].m_r = vec3d(  0, R0,  0);
	m_MBNode[16].m_r = vec3d( d0, d0,  0);
	m_MBNode[17].m_r = vec3d(-D0,-D0, D0);
	m_MBNode[18].m_r = vec3d(  0,-d0, d0);
	m_MBNode[19].m_r = vec3d( D0,-D0, D0);
	m_MBNode[20].m_r = vec3d(-d0,  0, d0);
	m_MBNode[21].m_r = vec3d(  0,  0, R0);
	m_MBNode[22].m_r = vec3d( d0,  0, d0);
	m_MBNode[23].m_r = vec3d(-D0, D0, D0);
	m_MBNode[24].m_r = vec3d(  0, d0, d0);
	m_MBNode[25].m_r = vec3d( D0, D0, D0);

	m_MBNode[26].m_r = vec3d(-D1,-D1,-D1);
	m_MBNode[27].m_r = vec3d(  0,-d1,-d1);
	m_MBNode[28].m_r = vec3d( D1,-D1,-D1);
	m_MBNode[29].m_r = vec3d(-d1,  0,-d1);
	m_MBNode[30].m_r = vec3d(  0,  0,-R1);
	m_MBNode[31].m_r = vec3d( d1,  0,-d1);
	m_MBNode[32].m_r = vec3d(-D1, D1,-D1);
	m_MBNode[33].m_r = vec3d(  0, d1,-d1);
	m_MBNode[34].m_r = vec3d( D1, D1,-D1);

	m_MBNode[35].m_r = vec3d(-d1, -d1, 0);
	m_MBNode[36].m_r = vec3d(  0, -R1, 0);
	m_MBNode[37].m_r = vec3d( d1, -d1, 0);
	m_MBNode[38].m_r = vec3d(-R1,   0, 0);
	m_MBNode[39].m_r = vec3d( R1,   0, 0);
	m_MBNode[40].m_r = vec3d(-d1,  d1, 0);
	m_MBNode[41].m_r = vec3d(  0,  R1, 0);
	m_MBNode[42].m_r = vec3d( d1,  d1, 0);

	m_MBNode[43].m_r = vec3d(-D1,-D1, D1);
	m_MBNode[44].m_r = vec3d(  0,-d1, d1);
	m_MBNode[45].m_r = vec3d( D1,-D1, D1);
	m_MBNode[46].m_r = vec3d(-d1,  0, d1);
	m_MBNode[47].m_r = vec3d(  0,  0, R1);
	m_MBNode[48].m_r = vec3d( d1,  0, d1);
	m_MBNode[49].m_r = vec3d(-D1, D1, D1);
	m_MBNode[50].m_r = vec3d(  0, d1, d1);
	m_MBNode[51].m_r = vec3d( D1, D1, D1);
	AddNode(vec3d(0, 0, 0), NODE_SHAPE);

	// create the MB block
	m_MBlock.resize(24);

	int MB[24][8] = {
		{ 0, 26, 27,  1,  9, 35, 36, 10},	// 0 ---
		{ 1, 27, 28,  2, 10, 36, 37, 11},	// 1
		{ 9, 35, 36, 10, 17, 43, 44, 18},	// 2 
		{10, 36, 37, 11, 18, 44, 45, 19},	// 3
		{ 2, 28, 31,  5, 11, 37, 39, 13},	// 4 ---
		{ 5, 31, 34,  8, 13, 39, 42, 16},	// 5
		{11, 37, 39, 13, 19, 45, 48, 22},	// 6
		{13, 39, 42, 16, 22, 48, 51, 25},	// 7
		{ 8, 34, 33,  7, 16, 42, 41, 15},	// 8 ---
		{ 7, 33, 32,  6, 15, 41, 40, 14},	// 9
		{16, 42, 41, 15, 25, 51, 50, 24},	// 10
		{15, 41, 40, 14, 24, 50, 49, 23},	// 11
		{ 6, 32, 29,  3, 14, 40, 38, 12},	// 12 ---
		{ 3, 29, 26,  0, 12, 38, 35,  9},	// 13
		{14, 40, 38, 12, 23, 49, 46, 20},	// 14
		{12, 38, 35,  9, 20, 46, 43, 17},	// 15
		{ 0, 26, 29,  3,  1, 27, 30,  4},	// 16 ---
		{ 1, 27, 30,  4,  2, 28, 31,  5},	// 17
		{ 3, 29, 32,  6,  4, 30, 33,  7},	// 18
		{ 4, 30, 33,  7,  5, 31, 34,  8},	// 19
		{17, 43, 44, 18, 20, 46, 47, 21},	// 20 ---
		{18, 44, 45, 19, 21, 47, 48, 22},	// 21
		{20, 46, 47, 21, 23, 49, 50, 24},	// 22
		{21, 47, 48, 22, 24, 50, 51, 25}	// 23
	};

	int i, *n;
	for (i=0; i<24; ++i)
	{
		n = MB[i];
		m_MBlock[i].SetNodes(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
		m_MBlock[i].SetSizes(ns,nd,nd);
		m_MBlock[i].SetID(0);
	}

	m_MBlock[ 0].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 1].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 2].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 3].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 4].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 5].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 6].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 7].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 8].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[ 9].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[10].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[11].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[12].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[13].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[14].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[15].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[16].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[17].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[18].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[19].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[20].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[21].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[22].SetZoning(fr, 1, 1, br, false, false);
	m_MBlock[23].SetZoning(fr, 1, 1, br, false, false);

	// update the MB data
	BuildMB();

	// Face ID's
	MBFace& F1 = GetBlockFace( 5, 1); F1.SetID(0);
	MBFace& F2 = GetBlockFace( 8, 1); F2.SetID(0);
	MBFace& F3 = GetBlockFace(19, 1); F3.SetID(0);

	MBFace& F4 = GetBlockFace( 9, 1); F4.SetID(1);
	MBFace& F5 = GetBlockFace(12, 1); F5.SetID(1);
	MBFace& F6 = GetBlockFace(18, 1); F6.SetID(1);

	MBFace& F7 = GetBlockFace( 0, 1); F7.SetID(2);
	MBFace& F8 = GetBlockFace(13, 1); F8.SetID(2);
	MBFace& F9 = GetBlockFace(16, 1); F9.SetID(2);

	MBFace& F10 = GetBlockFace(17, 1); F10.SetID(3);
	MBFace& F11 = GetBlockFace( 1, 1); F11.SetID(3);
	MBFace& F12 = GetBlockFace( 4, 1); F12.SetID(3);

	MBFace& F13 = GetBlockFace( 7, 1); F13.SetID(4);
	MBFace& F14 = GetBlockFace(10, 1); F14.SetID(4);
	MBFace& F15 = GetBlockFace(23, 1); F15.SetID(4);

	MBFace& F16 = GetBlockFace(11, 1); F16.SetID(5);
	MBFace& F17 = GetBlockFace(14, 1); F17.SetID(5);
	MBFace& F18 = GetBlockFace(22, 1); F18.SetID(5);

	MBFace& F19 = GetBlockFace( 2, 1); F19.SetID(6);
	MBFace& F20 = GetBlockFace(15, 1); F20.SetID(6);
	MBFace& F21 = GetBlockFace(20, 1); F21.SetID(6);

	MBFace& F22 = GetBlockFace( 3, 1); F22.SetID(7);
	MBFace& F23 = GetBlockFace( 6, 1); F23.SetID(7);
	MBFace& F24 = GetBlockFace(21, 1); F24.SetID(7);

	MBFace& F25 = GetBlockFace( 5, 3); F25.SetID(8);
	MBFace& F26 = GetBlockFace( 8, 3); F26.SetID(8);
	MBFace& F27 = GetBlockFace(19, 3); F27.SetID(8);

	MBFace& F28 = GetBlockFace( 9, 3); F28.SetID(9);
	MBFace& F29 = GetBlockFace(12, 3); F29.SetID(9);
	MBFace& F30 = GetBlockFace(18, 3); F30.SetID(9);

	MBFace& F31 = GetBlockFace( 0, 3); F31.SetID(10);
	MBFace& F32 = GetBlockFace(13, 3); F32.SetID(10);
	MBFace& F33 = GetBlockFace(16, 3); F33.SetID(10);

	MBFace& F34 = GetBlockFace( 1, 3); F34.SetID(11);
	MBFace& F35 = GetBlockFace( 4, 3); F35.SetID(11);
	MBFace& F36 = GetBlockFace(17, 3); F36.SetID(11);

	MBFace& F37 = GetBlockFace( 7, 3); F37.SetID(12);
	MBFace& F38 = GetBlockFace(10, 3); F38.SetID(12);
	MBFace& F39 = GetBlockFace(23, 3); F39.SetID(12);

	MBFace& F40 = GetBlockFace(11, 3); F40.SetID(13);
	MBFace& F41 = GetBlockFace(14, 3); F41.SetID(13);
	MBFace& F42 = GetBlockFace(22, 3); F42.SetID(13);

	MBFace& F43 = GetBlockFace( 2, 3); F43.SetID(14);
	MBFace& F44 = GetBlockFace(15, 3); F44.SetID(14);
	MBFace& F45 = GetBlockFace(20, 3); F45.SetID(14);

	MBFace& F46 = GetBlockFace( 3, 3); F46.SetID(15);
	MBFace& F47 = GetBlockFace( 6, 3); F47.SetID(15);
	MBFace& F48 = GetBlockFace(21, 3); F48.SetID(15);

	// Edges
	GetFaceEdge(F13,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(0);
	GetFaceEdge(F14,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(0);
	GetFaceEdge(F16,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(1);
	GetFaceEdge(F17,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(1);
	GetFaceEdge(F20,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(2);
	GetFaceEdge(F19,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(2);
	GetFaceEdge(F22,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(3);
	GetFaceEdge(F23,0).SetEdge(EDGE_3P_CIRC_ARC, -1, 52).SetID(3);

	GetFaceEdge(F13,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(4);
	GetFaceEdge(F15,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(4);
	GetFaceEdge(F16,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(5);
	GetFaceEdge(F15,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(5);
	GetFaceEdge(F17,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(6);
	GetFaceEdge(F21,2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(6);
	GetFaceEdge(F22,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(7);
	GetFaceEdge(F21,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(7);

	GetFaceEdge(F12,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(8);
	GetFaceEdge(F3 ,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(8);
	GetFaceEdge(F2 ,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(9);
	GetFaceEdge(F3 ,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(9);
	GetFaceEdge(F5 ,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(10);
	GetFaceEdge(F9 ,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(10);
	GetFaceEdge(F7 ,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(11);
	GetFaceEdge(F9 ,2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(11);


	GetFaceEdge(F37,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(12);
	GetFaceEdge(F38,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(12);
	GetFaceEdge(F40,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(13);
	GetFaceEdge(F41,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(13);
	GetFaceEdge(F44,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(14);
	GetFaceEdge(F43,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(14);
	GetFaceEdge(F46,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(15);
	GetFaceEdge(F47,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(15);

	GetFaceEdge(F37,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(16);
	GetFaceEdge(F39,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(16);
	GetFaceEdge(F40,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(17);
	GetFaceEdge(F39,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(17);
	GetFaceEdge(F41,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(18);
	GetFaceEdge(F45,2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(18);
	GetFaceEdge(F46,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(19);
	GetFaceEdge(F45,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(19);

	GetFaceEdge(F35,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(20);
	GetFaceEdge(F27,1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(20);
	GetFaceEdge(F26,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(21);
	GetFaceEdge(F27,0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(21);
	GetFaceEdge(F29,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(22);
	GetFaceEdge(F33,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(22);
	GetFaceEdge(F31,3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(23);
	GetFaceEdge(F33,2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52).SetID(23);

	GetFaceEdge(F1, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F1, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F2, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F4, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F4, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F5, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F7, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F7, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F8, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F10, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F10, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F11, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F13, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F13, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F14, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F16, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F16, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F17, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F19, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F19, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F20, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F22, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F22, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F23, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F25, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F25, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F26, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F28, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F28, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F29, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F31, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F31, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F32, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F34, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F34, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F35, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F37, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F37, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F38, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F40, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F40, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F41, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F43, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F43, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F44, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	GetFaceEdge(F46, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F46, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);
	GetFaceEdge(F47, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 52);

	// Node ID's
	m_MBNode[39].SetID(0);
	m_MBNode[41].SetID(1);
	m_MBNode[38].SetID(2);
	m_MBNode[36].SetID(3);
	m_MBNode[30].SetID(4);
	m_MBNode[47].SetID(5);
	m_MBNode[13].SetID(6);
	m_MBNode[15].SetID(7);
	m_MBNode[12].SetID(8);
	m_MBNode[10].SetID(9);
	m_MBNode[ 4].SetID(10);
	m_MBNode[21].SetID(11);

	UpdateMB();

	return true;
}

FSMesh* FEHollowSphere::BuildMesh()
{
	BuildMultiBlock();

	// set element type
	int nelem = GetIntValue(ELEM_TYPE);
	switch (nelem)
	{
	case 0: SetElementType(FE_HEX8); break;
	case 1: SetElementType(FE_HEX20); break;
	case 2: SetElementType(FE_HEX27); break;
	}

	// create the MB
	FSMesh* pm = FEMultiBlockMesh::BuildMesh();

	// update the mesh
	pm->UpdateMesh();

	// the Multi-block mesher will assign a different smoothing ID
	// to each face, but we don't want that here. 
	// For now, we autosmooth the mesh although we should think of a 
	// better way
	pm->AutoSmooth(60);

	return pm;
}
