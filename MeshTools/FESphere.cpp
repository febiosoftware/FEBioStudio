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

// FESphere.cpp: implementation of the FESphere class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FESphere.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>

FESphere::FESphere(GObject& o) : FEMultiBlockMesh(o)
{
	m_r = 0.5;

	m_nseg = m_ndiv = 4;

	m_gd = 1;
	m_gr = 1;

	m_bd = false;
	m_br = false;

	AddDoubleParam(m_r, "r", "Ratio");
	AddIntParam(m_nseg, "ns", "Segments");
	AddIntParam(m_ndiv, "nd", "Divisions");

//	AddDoubleParam(m_gd, "gd", "D-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");
//	AddBoolParam(m_bd, "bd", "D-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("HEX8\0HEX20\0HEX27\0");
}

bool FESphere::BuildMultiBlock()
{
	GSphere* po = dynamic_cast<GSphere*>(&m_o);
	if (po == nullptr) return false;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double R1 = param.GetFloatValue(GSphere::RADIUS);

	// get parameters
	double R0 = GetFloatValue(RATIO);
	m_ndiv = GetIntValue(NDIV);
	m_nseg = GetIntValue(NSEG);
	m_r = GetFloatValue(RATIO);
	int nd = m_ndiv;
	int ns = m_nseg;

	m_gd = 1.0;// GetFloatValue(GD);
	m_gr = GetFloatValue(GR);
	m_bd = false;// GetBoolValue(GD2);
	m_br = GetBoolValue(GR2);


	R0 *= R1;

	double d0 = R0/sqrt(2.0);
	double d1 = R1/sqrt(2.0);
	double d2 = R1/sqrt(3.0);

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

	m_MBNode[27].m_r = vec3d(-d2,-d2,-d2);
	m_MBNode[28].m_r = vec3d(  0,-d1,-d1);
	m_MBNode[29].m_r = vec3d( d2,-d2,-d2);
	m_MBNode[30].m_r = vec3d(-d1,  0,-d1);
	m_MBNode[31].m_r = vec3d(  0,  0,-R1);
	m_MBNode[32].m_r = vec3d( d1,  0,-d1);
	m_MBNode[33].m_r = vec3d(-d2, d2,-d2);
	m_MBNode[34].m_r = vec3d(  0, d1,-d1);
	m_MBNode[35].m_r = vec3d( d2, d2,-d2);

	m_MBNode[36].m_r = vec3d(-d1, -d1, 0);
	m_MBNode[37].m_r = vec3d(  0, -R1, 0);
	m_MBNode[38].m_r = vec3d( d1, -d1, 0);
	m_MBNode[39].m_r = vec3d(-R1,   0, 0);
	m_MBNode[40].m_r = vec3d( R1,   0, 0);
	m_MBNode[41].m_r = vec3d(-d1,  d1, 0);
	m_MBNode[42].m_r = vec3d(  0,  R1, 0);
	m_MBNode[43].m_r = vec3d( d1,  d1, 0);

	m_MBNode[44].m_r = vec3d(-d2,-d2, d2);
	m_MBNode[45].m_r = vec3d(  0,-d1, d1);
	m_MBNode[46].m_r = vec3d( d2,-d2, d2);
	m_MBNode[47].m_r = vec3d(-d1,  0, d1);
	m_MBNode[48].m_r = vec3d(  0,  0, R1);
	m_MBNode[49].m_r = vec3d( d1,  0, d1);
	m_MBNode[50].m_r = vec3d(-d2, d2, d2);
	m_MBNode[51].m_r = vec3d(  0, d1, d1);
	m_MBNode[52].m_r = vec3d( d2, d2, d2);

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
		m_MBlock[i].SetZoning(m_gr, 1, 1, m_br, false, false);
		m_MBlock[i].SetID(0);
	}

	// update the MB data
	BuildMB();

	// Face ID's
	MBFace& F1 = GetBlockFace(13, 1); F1.SetID(0);
	MBFace& F2 = GetBlockFace(16, 1); F2.SetID(0);
	MBFace& F3 = GetBlockFace(27, 1); F3.SetID(0);

	MBFace& F4 = GetBlockFace(17, 1); F4.SetID(1);
	MBFace& F5 = GetBlockFace(20, 1); F5.SetID(1);
	MBFace& F6 = GetBlockFace(26, 1); F6.SetID(1);

	MBFace& F7 = GetBlockFace( 8, 1); F7.SetID(2);
	MBFace& F8 = GetBlockFace(21, 1); F8.SetID(2);
	MBFace& F9 = GetBlockFace(24, 1); F9.SetID(2);

	MBFace& F10 = GetBlockFace( 9, 1); F10.SetID(3);
	MBFace& F11 = GetBlockFace(12, 1); F11.SetID(3);
	MBFace& F12 = GetBlockFace(25, 1); F12.SetID(3);

	MBFace& F13 = GetBlockFace(15, 1); F13.SetID(4);
	MBFace& F14 = GetBlockFace(18, 1); F14.SetID(4);
	MBFace& F15 = GetBlockFace(31, 1); F15.SetID(4);

	MBFace& F16 = GetBlockFace(19, 1); F16.SetID(5);
	MBFace& F17 = GetBlockFace(22, 1); F17.SetID(5);
	MBFace& F18 = GetBlockFace(30, 1); F18.SetID(5);

	MBFace& F19 = GetBlockFace(10, 1); F19.SetID(6);
	MBFace& F20 = GetBlockFace(23, 1); F20.SetID(6);
	MBFace& F21 = GetBlockFace(28, 1); F21.SetID(6);

	MBFace& F22 = GetBlockFace(11, 1); F22.SetID(7);
	MBFace& F23 = GetBlockFace(14, 1); F23.SetID(7);
	MBFace& F24 = GetBlockFace(29, 1); F24.SetID(7);

	// Edge ID's
	GetFaceEdge(F13, 0).SetID(0);
	GetFaceEdge(F14, 0).SetID(0);
	GetFaceEdge(F16, 0).SetID(1);
	GetFaceEdge(F17, 0).SetID(1);
	GetFaceEdge(F20, 0).SetID(2);
	GetFaceEdge(F19, 0).SetID(2);
	GetFaceEdge(F22, 0).SetID(3);
	GetFaceEdge(F23, 0).SetID(3);

	GetFaceEdge(F13, 3).SetID(4);
	GetFaceEdge(F15, 0).SetID(4);
	GetFaceEdge(F16, 3).SetID(5);
	GetFaceEdge(F15, 3).SetID(5);
	GetFaceEdge(F17, 1).SetID(6);
	GetFaceEdge(F21, 2).SetID(6);
	GetFaceEdge(F22, 3).SetID(7);
	GetFaceEdge(F21, 1).SetID(7);

	GetFaceEdge(F11, 1).SetID(8);
	GetFaceEdge(F3 , 3).SetID(8);
	GetFaceEdge(F2 , 1).SetID(9);
	GetFaceEdge(F3 , 0).SetID(9);
	GetFaceEdge(F5 , 1).SetID(10);
	GetFaceEdge(F9 , 1).SetID(10);
	GetFaceEdge(F7 , 1).SetID(11);
	GetFaceEdge(F9 , 2).SetID(11);

	// set the edges
	GetFaceEdge(F13, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F14, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F16, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F17, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F20, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F19, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F22, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F23, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, 13);
	GetFaceEdge(F13, 3).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F15, 0).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F16, 3).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F15, 3).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F17, 1).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F21, 2).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F22, 3).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F21, 1).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F11, 1).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F3 , 3).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F2 , 1).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F3 , 0).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F5 , 1).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F9 , 1).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F7 , 1).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);
	GetFaceEdge(F9 , 2).SetEdge(EDGE_3P_CIRC_ARC,  1, 13);

	GetFaceEdge(F13, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F13, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F14, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	GetFaceEdge(F16, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F16, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F17, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	GetFaceEdge(F20, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F20, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F19, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	GetFaceEdge(F22, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F22, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F23, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	GetFaceEdge(F3, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F3, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F2, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	GetFaceEdge(F4, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F5, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F5, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	GetFaceEdge(F7, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F7, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F8, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	GetFaceEdge(F10, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F10, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);
	GetFaceEdge(F11, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 13);

	// Node ID's
	m_MBNode[40].SetID(0);
	m_MBNode[42].SetID(1);
	m_MBNode[39].SetID(2);
	m_MBNode[37].SetID(3);
	m_MBNode[31].SetID(4);
	m_MBNode[48].SetID(5);

	UpdateMB();

	return true;
}

FSMesh* FESphere::BuildMesh()
{
	GSphere* po = dynamic_cast<GSphere*>(&m_o);
	if (po == nullptr) return nullptr;

	if (!BuildMultiBlock()) return nullptr;

	// set element type
	int nelem = GetIntValue(ELEM_TYPE);
	switch (nelem)
	{
	case 0: SetElementType(FE_HEX8 ); break;
	case 1: SetElementType(FE_HEX20); break;
	case 2: SetElementType(FE_HEX27); break;
	}

	// create the MB
	FSMesh* pm = FEMultiBlockMesh::BuildMBMesh();
	if (pm)
	{
		// finally, we update the normals and we are good to go
		pm->UpdateNormals();
	}

	return pm;
}
