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
#include "FESphereInBox.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>


FESphereInBox::FESphereInBox()
{
	m_po = nullptr;

	// define the tube parameters
	AddIntParam(5, "nx", "Nx");
	AddIntParam(5, "ny", "Ny");
	AddIntParam(5, "nz", "Nz");
	AddIntParam(1, "nr", "Nr");

	AddDoubleParam(1.0, "gr", "R-bias");
	AddBoolParam(false, "br", "R-mirrored bias");

	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

//-----------------------------------------------------------------------------
bool FESphereInBox::BuildMultiBlock()
{
	assert(m_po);

	// get the object parameters
	ParamBlock& param = m_po->GetParamBlock();
	double W = param.GetFloatValue(GCylinderInBox::WIDTH);
	double H = param.GetFloatValue(GCylinderInBox::HEIGHT);
	double D = param.GetFloatValue(GCylinderInBox::DEPTH);
	double R = param.GetFloatValue(GCylinderInBox::RADIUS);

	double w = W * 0.5;
	double h = H * 0.5;
	double d = D * 0.5;

	double R2 = R / sqrt(2.0);
	double R3 = R / sqrt(3.0);

	// get meshing parameters
	int nx = GetIntValue(NX);
	int ny = GetIntValue(NY);
	int nz = GetIntValue(NZ);
	int nr = GetIntValue(NR);

	double gr = GetFloatValue(GR);
	bool br = GetBoolValue(BR);

	// create the MB nodes
	const int cnode = 52;	// index of center node
	m_MBNode.resize(52);
	m_MBNode[ 0].m_r = vec3d(-w, -h, 0);
	m_MBNode[ 1].m_r = vec3d( 0, -h, 0);
	m_MBNode[ 2].m_r = vec3d( w, -h, 0);
	m_MBNode[ 3].m_r = vec3d(-w,  0, 0);
	m_MBNode[ 4].m_r = vec3d( 0,  0, 0);
	m_MBNode[ 5].m_r = vec3d( w,  0, 0);
	m_MBNode[ 6].m_r = vec3d(-w,  h, 0);
	m_MBNode[ 7].m_r = vec3d( 0,  h, 0);
	m_MBNode[ 8].m_r = vec3d( w,  h, 0);
	m_MBNode[ 9].m_r = vec3d(-w, -h, d);
	m_MBNode[10].m_r = vec3d( 0, -h, d);
	m_MBNode[11].m_r = vec3d( w, -h, d);
	m_MBNode[12].m_r = vec3d(-w,  0, d);
	m_MBNode[13].m_r = vec3d( w,  0, d);
	m_MBNode[14].m_r = vec3d(-w,  h, d);
	m_MBNode[15].m_r = vec3d( 0,  h, d);
	m_MBNode[16].m_r = vec3d( w,  h, d);
	m_MBNode[17].m_r = vec3d(-w, -h, D);
	m_MBNode[18].m_r = vec3d( 0, -h, D);
	m_MBNode[19].m_r = vec3d( w, -h, D);
	m_MBNode[20].m_r = vec3d(-w,  0, D);
	m_MBNode[21].m_r = vec3d( 0,  0, D);
	m_MBNode[22].m_r = vec3d( w,  0, D);
	m_MBNode[23].m_r = vec3d(-w,  h, D);
	m_MBNode[24].m_r = vec3d( 0,  h, D);
	m_MBNode[25].m_r = vec3d( w,  h, D);

	m_MBNode[26].m_r = vec3d(  0,   0,  -R + d);
	m_MBNode[27].m_r = vec3d(-R2,   0, -R2 + d);
	m_MBNode[28].m_r = vec3d(-R3, -R3, -R3 + d);
	m_MBNode[29].m_r = vec3d(  0, -R2, -R2 + d);
	m_MBNode[30].m_r = vec3d( R3, -R3, -R3 + d);
	m_MBNode[31].m_r = vec3d( R2,   0, -R2 + d);
	m_MBNode[32].m_r = vec3d( R3,  R3, -R3 + d);
	m_MBNode[33].m_r = vec3d(  0,  R2, -R2 + d);
	m_MBNode[34].m_r = vec3d(-R3,  R3, -R3 + d);
	m_MBNode[35].m_r = vec3d( -R,   0,       d);
	m_MBNode[36].m_r = vec3d(-R2, -R2,       d);
	m_MBNode[37].m_r = vec3d(  0,  -R,       d);
	m_MBNode[38].m_r = vec3d( R2, -R2,       d);
	m_MBNode[39].m_r = vec3d(  R,   0,       d);
	m_MBNode[40].m_r = vec3d( R2,  R2,       d);
	m_MBNode[41].m_r = vec3d(  0,   R,       d);
	m_MBNode[42].m_r = vec3d(-R2,  R2,       d);
	m_MBNode[43].m_r = vec3d(-R2,   0,  R2 + d);
	m_MBNode[44].m_r = vec3d(-R3, -R3,  R3 + d);
	m_MBNode[45].m_r = vec3d(  0, -R2,  R2 + d);
	m_MBNode[46].m_r = vec3d( R3, -R3,  R3 + d);
	m_MBNode[47].m_r = vec3d( R2,   0,  R2 + d);
	m_MBNode[48].m_r = vec3d( R3,  R3,  R3 + d);
	m_MBNode[49].m_r = vec3d(  0,  R2,  R2 + d);
	m_MBNode[50].m_r = vec3d(-R3,  R3,  R3 + d);
	m_MBNode[51].m_r = vec3d(  0,   0,   R + d);
	AddNode(vec3d(0, 0, d), NODE_SHAPE);

	// create blocks
	// -Z
	m_MBlock.resize(24);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0, 1, 4, 3, 28, 29, 26, 27);
	b1.SetSizes(nx, ny, nr);
	b1.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1, 2, 5, 4, 29, 30, 31, 26);
	b2.SetSizes(nx, ny, nr);
	b2.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(3, 4, 7, 6, 27, 26, 33, 34);
	b3.SetSizes(nx, ny, nr);
	b3.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(4, 5, 8, 7, 26, 31, 32, 33);
	b4.SetSizes(nx, ny, nr);
	b4.SetZoning(1, 1, gr, false, false, br);

	// -X
	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(0, 3, 12, 9, 28, 27, 35, 36);
	b5.SetSizes(nx, ny, nr);
	b5.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(3, 6, 14, 12, 27, 34, 42, 35);
	b6.SetSizes(nx, ny, nr);
	b6.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b7 = m_MBlock[6];
	b7.SetID(0);
	b7.SetNodes(9, 12, 20, 17, 36, 35, 43, 44);
	b7.SetSizes(nx, ny, nr);
	b7.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b8 = m_MBlock[7];
	b8.SetID(0);
	b8.SetNodes(12, 14, 23, 20, 35, 42, 50, 43);
	b8.SetSizes(nx, ny, nr);
	b8.SetZoning(1, 1, gr, false, false, br);

	// -Y
	MBBlock& b9 = m_MBlock[8];
	b9.SetID(0);
	b9.SetNodes(0, 9, 10, 1, 28, 36, 37, 29);
	b9.SetSizes(nx, ny, nr);
	b9.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b10 = m_MBlock[9];
	b10.SetID(0);
	b10.SetNodes(1, 10, 11, 2, 29, 37, 38, 30);
	b10.SetSizes(nx, ny, nr);
	b10.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b11 = m_MBlock[10];
	b11.SetID(0);
	b11.SetNodes(10, 9, 17, 18, 37, 36, 44, 45);
	b11.SetSizes(nx, ny, nr);
	b11.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b12 = m_MBlock[11];
	b12.SetID(0);
	b12.SetNodes(11, 10, 18, 19, 38, 37, 45, 46);
	b12.SetSizes(nx, ny, nr);
	b12.SetZoning(1, 1, gr, false, false, br);

	// +X 
	MBBlock& b13 = m_MBlock[12];
	b13.SetID(0);
	b13.SetNodes(2, 11, 13, 5, 30, 38, 39, 31);
	b13.SetSizes(nx, ny, nr);
	b13.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b14 = m_MBlock[13];
	b14.SetID(0);
	b14.SetNodes(5, 13, 16, 8, 31, 39, 40, 32);
	b14.SetSizes(nx, ny, nr);
	b14.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b15 = m_MBlock[14];
	b15.SetID(0);
	b15.SetNodes(13, 11, 19, 22, 39, 38, 46, 47);
	b15.SetSizes(nx, ny, nr);
	b15.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b16 = m_MBlock[15];
	b16.SetID(0);
	b16.SetNodes(16, 13, 22, 25, 40, 39, 47, 48);
	b16.SetSizes(nx, ny, nr);
	b16.SetZoning(1, 1, gr, false, false, br);

	// +Y
	MBBlock& b17 = m_MBlock[16];
	b17.SetID(0);
	b17.SetNodes(7, 8, 16, 15, 33, 32, 40, 41);
	b17.SetSizes(nx, ny, nr);
	b17.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b18 = m_MBlock[17];
	b18.SetID(0);
	b18.SetNodes(6, 7, 15, 14, 34, 33, 41, 42);
	b18.SetSizes(nx, ny, nr);
	b18.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b19 = m_MBlock[18];
	b19.SetID(0);
	b19.SetNodes(15, 16, 25, 24, 41, 40, 48, 49);
	b19.SetSizes(nx, ny, nr);
	b19.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b20 = m_MBlock[19];
	b20.SetID(0);
	b20.SetNodes(14, 15, 24, 23, 42, 41, 49, 50);
	b20.SetSizes(nx, ny, nr);
	b20.SetZoning(1, 1, gr, false, false, br);

	// +Z
	MBBlock& b21 = m_MBlock[20];
	b21.SetID(0);
	b21.SetNodes(20, 21, 18, 17, 43, 51, 45, 44);
	b21.SetSizes(nx, ny, nr);
	b21.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b22 = m_MBlock[21];
	b22.SetID(0);
	b22.SetNodes(21, 22, 19, 18, 51, 47, 46, 45);
	b22.SetSizes(nx, ny, nr);
	b22.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b23 = m_MBlock[22];
	b23.SetID(0);
	b23.SetNodes(23, 24, 21, 20, 50, 49, 51, 43);
	b23.SetSizes(nx, ny, nr);
	b23.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b24 = m_MBlock[23];
	b24.SetID(0);
	b24.SetNodes(24, 25, 22, 21, 49, 48, 47, 51);
	b24.SetSizes(nx, ny, nr);
	b24.SetZoning(1, 1, gr, false, false, br);

	// update the MB data
	BuildMB();

	// assign face ID's
	SetBlockFaceID(b1 , -1, -1, -1, -1,  4, 10);
	SetBlockFaceID(b2 , -1, -1, -1, -1,  4, 10);
	SetBlockFaceID(b3 , -1, -1, -1, -1,  4, 10);
	SetBlockFaceID(b4 , -1, -1, -1, -1,  4, 10);

	SetBlockFaceID(b5 , -1, -1, -1, -1,  3,  9);
	SetBlockFaceID(b6 , -1, -1, -1, -1,  3,  9);
	SetBlockFaceID(b7 , -1, -1, -1, -1,  3,  9);
	SetBlockFaceID(b8 , -1,  1, -1, -1,  3,  9);

	SetBlockFaceID(b9 , -1, -1, -1, -1,  0,  6);
	SetBlockFaceID(b10, -1, -1, -1, -1,  0,  6);
	SetBlockFaceID(b11, -1, -1, -1, -1,  0,  6);
	SetBlockFaceID(b12, -1, -1, -1, -1,  0,  6);

	SetBlockFaceID(b13, -1, -1, -1, -1,  1,  7);
	SetBlockFaceID(b14, -1, -1, -1, -1,  1,  7);
	SetBlockFaceID(b15, -1, -1, -1, -1,  1,  7);
	SetBlockFaceID(b16, -1, -1, -1, -1,  1,  7);

	SetBlockFaceID(b17, -1, -1, -1, -1,  2,  8);
	SetBlockFaceID(b18, -1, -1, -1, -1,  2,  8);
	SetBlockFaceID(b19, -1, -1, -1, -1,  2,  8);
	SetBlockFaceID(b20, -1, -1, -1, -1,  2,  8);

	SetBlockFaceID(b21, -1, -1, -1, -1,  5,  6);
	SetBlockFaceID(b22, -1, -1, -1, -1,  5,  6);
	SetBlockFaceID(b23, -1, -1, -1, -1,  5,  6);
	SetBlockFaceID(b24, -1, -1, -1, -1,  5,  6);

	// assign edge ID's
	MBFace& F1  = GetBlockFace( 0, 4); SetFaceEdgeID(F1 , -1, -1,  0,  3);
	MBFace& F2  = GetBlockFace( 1, 4); SetFaceEdgeID(F2 , -1,  1,  0, -1);
	MBFace& F3  = GetBlockFace( 2, 4); SetFaceEdgeID(F3 ,  2, -1, -1,  3);
	MBFace& F4  = GetBlockFace( 3, 4); SetFaceEdgeID(F4 ,  2,  1, -1, -1);

	MBFace& F5  = GetBlockFace( 4, 4); SetFaceEdgeID(F5 , -1, -1, 3, 8);
	MBFace& F6  = GetBlockFace( 5, 4); SetFaceEdgeID(F6 , -1, 11, 3, -1);
	MBFace& F7  = GetBlockFace( 6, 4); SetFaceEdgeID(F7 , 7, -1, -1, 8);
	MBFace& F8  = GetBlockFace( 7, 4); SetFaceEdgeID(F8 , 7, 11, -1, -1);
	
	MBFace& F9  = GetBlockFace( 8, 4); SetFaceEdgeID(F9 , -1, -1, 8, 0);
	MBFace& F10 = GetBlockFace( 9, 4); SetFaceEdgeID(F10, 9, -1, -1, 0);
	MBFace& F11 = GetBlockFace(10, 4); SetFaceEdgeID(F11, 4, 8, -1, -1);
	MBFace& F12 = GetBlockFace(11, 4); SetFaceEdgeID(F12, 4, -1, -1, 9);

	MBFace& F13 = GetBlockFace(12, 4); SetFaceEdgeID(F13, -1, -1, 9, 1);
	MBFace& F14 = GetBlockFace(13, 4); SetFaceEdgeID(F14, 10, -1, -1, 1);
	MBFace& F15 = GetBlockFace(14, 4); SetFaceEdgeID(F15, 5, 9, -1, -1);
	MBFace& F16 = GetBlockFace(15, 4); SetFaceEdgeID(F16, 5, -1, -1, 10);

	MBFace& F17 = GetBlockFace(16, 4); SetFaceEdgeID(F17, -1, 10, 2, -1);
	MBFace& F18 = GetBlockFace(17, 4); SetFaceEdgeID(F18, -1, -1, 2, 11);
	MBFace& F19 = GetBlockFace(18, 4); SetFaceEdgeID(F19, 6, 10, -1, -1);
	MBFace& F20 = GetBlockFace(19, 4); SetFaceEdgeID(F20, 6, -1, -1, 11);

	MBFace& F21 = GetBlockFace(20, 4); SetFaceEdgeID(F21, 4, -1, -1,  7);
	MBFace& F22 = GetBlockFace(21, 4); SetFaceEdgeID(F22,  4,  5, -1, -1);
	MBFace& F23 = GetBlockFace(22, 4); SetFaceEdgeID(F23, -1, -1,  6,  7);
	MBFace& F24 = GetBlockFace(23, 4); SetFaceEdgeID(F24, -1,  5,  6, -1);

	MBFace& F25 = GetBlockFace( 0, 5); SetFaceEdgeID(F25, 12, -1, -1, 15);
	MBFace& F26 = GetBlockFace( 1, 5); SetFaceEdgeID(F26, 12, 13, -1, -1);
	MBFace& F27 = GetBlockFace( 2, 5); SetFaceEdgeID(F27, -1, -1, 14, 15);
	MBFace& F28 = GetBlockFace( 3, 5); SetFaceEdgeID(F28, -1, 13, 14, -1);

	MBFace& F29 = GetBlockFace( 4, 5); SetFaceEdgeID(F29, 15, -1, -1, 20);
	MBFace& F30 = GetBlockFace( 5, 5); SetFaceEdgeID(F30, 15, 23, -1, -1);
	MBFace& F31 = GetBlockFace( 6, 5); SetFaceEdgeID(F31, -1, -1, 19, 20);
	MBFace& F32 = GetBlockFace( 7, 5); SetFaceEdgeID(F32, -1, 23, 19, -1);

	MBFace& F33 = GetBlockFace( 8, 5); SetFaceEdgeID(F33, 20, -1, -1, 12);
	MBFace& F34 = GetBlockFace( 9, 5); SetFaceEdgeID(F34, -1, -1, 14, 12);
	MBFace& F35 = GetBlockFace(10, 5); SetFaceEdgeID(F35, -1, 20, 16, -1);
	MBFace& F36 = GetBlockFace(11, 5); SetFaceEdgeID(F36, -1, -1, 16, 14);
	
	MBFace& F37 = GetBlockFace(12, 5); SetFaceEdgeID(F37, 21, -1, -1, 13);
	MBFace& F38 = GetBlockFace(13, 5); SetFaceEdgeID(F38, -1, -1, 22, 13);
	MBFace& F39 = GetBlockFace(14, 5); SetFaceEdgeID(F39, -1, 21, 17, -1);
	MBFace& F40 = GetBlockFace(15, 5); SetFaceEdgeID(F40, -1, -1, 17, 22);

	MBFace& F41 = GetBlockFace(16, 5); SetFaceEdgeID(F41, 14, 22, -1, -1);
	MBFace& F42 = GetBlockFace(17, 5); SetFaceEdgeID(F42, 14, -1, -1, 23);
	MBFace& F43 = GetBlockFace(18, 5); SetFaceEdgeID(F43, -1, 22, 18, -1);
	MBFace& F44 = GetBlockFace(19, 5); SetFaceEdgeID(F44, -1, -1, 18, 23);

	MBFace& F45 = GetBlockFace(20, 5); SetFaceEdgeID(F45, -1, -1, 16, 19);
	MBFace& F46 = GetBlockFace(21, 5); SetFaceEdgeID(F46, -1, 17, 16, -1);
	MBFace& F47 = GetBlockFace(22, 5); SetFaceEdgeID(F47, 18, -1, -1, 19);
	MBFace& F48 = GetBlockFace(23, 5); SetFaceEdgeID(F48, 18, 17, -1, -1);

	// set the curved edges
	GetFaceEdge(F25, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F25, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F25, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F25, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F26, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F26, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F26, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F26, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F27, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F27, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F27, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F27, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F28, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F28, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F28, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F28, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F29, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F29, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F29, 2).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F29, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F30, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F30, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F30, 2).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F30, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F31, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F31, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F31, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F31, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F32, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F32, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F32, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F32, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F33, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F33, 1).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F33, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F33, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F34, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F34, 1).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F34, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F34, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F35, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F35, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F35, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F35, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F36, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F36, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F36, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F36, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F37, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F37, 1).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F37, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F37, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F38, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F38, 1).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F38, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F38, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F39, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F39, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F39, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F39, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F40, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F40, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F40, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F40, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F41, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F41, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F41, 2).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F41, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F42, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F42, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F42, 2).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F42, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F43, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F43, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F43, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F43, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F44, 0).SetEdge(EDGE_3P_CIRC_ARC, -1, cnode);
	GetFaceEdge(F44, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F44, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F44, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F45, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F45, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F45, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F45, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F46, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F46, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F46, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F46, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F47, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F47, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F47, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F47, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	GetFaceEdge(F48, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F48, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F48, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);
	GetFaceEdge(F48, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, cnode);

	// set the node ID's
	m_MBNode[ 0].SetID( 0);
	m_MBNode[ 2].SetID( 1);
	m_MBNode[ 8].SetID( 2);
	m_MBNode[ 6].SetID( 3);
	m_MBNode[17].SetID( 4);
	m_MBNode[19].SetID( 5);
	m_MBNode[25].SetID( 6);
	m_MBNode[23].SetID( 7);
	m_MBNode[28].SetID( 8);
	m_MBNode[30].SetID( 9);
	m_MBNode[32].SetID(10);
	m_MBNode[34].SetID(11);
	m_MBNode[44].SetID(12);
	m_MBNode[46].SetID(13);
	m_MBNode[48].SetID(14);
	m_MBNode[50].SetID(15);

	UpdateMB();

	return true;
}

FSMesh* FESphereInBox::BuildMesh(GObject* po)
{
	m_po = dynamic_cast<GSphereInBox*>(po);
	if (m_po == nullptr) return nullptr;

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
	FSMesh* pm = FEMultiBlockMesh::BuildMBMesh();

	// update the mesh
	pm->UpdateMesh();

	// the Multi-block mesher will assign a different smoothing ID
	// to each face, but we don't want that here. 
	// For now, we autosmooth the mesh although we should think of a 
	// better way
	pm->AutoSmooth(60);

	return pm;
}

//-----------------------------------------------------------------------------
/*
FSMesh* FESphereInBox::BuildMesh()
{
	assert(m_po);

	// get the object parameters
	ParamBlock& param = m_po->GetParamBlock();
	double W = param.GetFloatValue(GCylinderInBox::WIDTH );
	double H = param.GetFloatValue(GCylinderInBox::HEIGHT);
	double D = param.GetFloatValue(GCylinderInBox::DEPTH );
	double R = param.GetFloatValue(GCylinderInBox::RADIUS);

	double w = W*0.5;
	double h = H*0.5;
	double d = D*0.5;

	double R3 = R / sqrt(3.0);

	// get meshing parameters
	int nx = GetIntValue(NX);
	int ny = GetIntValue(NY);
	int nz = GetIntValue(NZ);
	int nr = GetIntValue(NR);

	double gr = GetFloatValue(GR);
	bool br = GetBoolValue(BR);

	// create the MB nodes
	m_MBNode.resize(16);
	m_MBNode[ 0].m_r = vec3d(-w, -h, 0);
	m_MBNode[ 1].m_r = vec3d( w, -h, 0);
	m_MBNode[ 2].m_r = vec3d( w,  h, 0);
	m_MBNode[ 3].m_r = vec3d(-w,  h, 0);
	m_MBNode[ 4].m_r = vec3d(-w, -h, D);
	m_MBNode[ 5].m_r = vec3d( w, -h, D);
	m_MBNode[ 6].m_r = vec3d( w,  h, D);
	m_MBNode[ 7].m_r = vec3d(-w,  h, D);
	
	m_MBNode[ 8].m_r = vec3d(-R3, -R3, -R3 + d);
	m_MBNode[ 9].m_r = vec3d( R3, -R3, -R3 + d);
	m_MBNode[10].m_r = vec3d( R3,  R3, -R3 + d);
	m_MBNode[11].m_r = vec3d(-R3,  R3, -R3 + d);
	m_MBNode[12].m_r = vec3d(-R3, -R3,  R3 + d);
	m_MBNode[13].m_r = vec3d( R3, -R3,  R3 + d);
	m_MBNode[14].m_r = vec3d( R3,  R3,  R3 + d);
	m_MBNode[15].m_r = vec3d(-R3,  R3,  R3 + d);
	AddNode(vec3d(0, 0, d), NODE_SHAPE);

	// create the blocks
	m_MBlock.resize(6);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0,1,9,8,4,5,13,12);
	b1.SetSizes(nx, nr, nz);
	b1.SetZoning(1, gr, 1, false, br, false);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1,2,10,9,5,6,14,13);
	b2.SetSizes(ny, nr, nz);
	b2.SetZoning(1, gr, 1, false, br, false);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(2,3,11,10,6,7,15,14);
	b3.SetSizes(nx, nr, nz);
	b3.SetZoning(1, gr, 1, false, br, false);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(3,0,8,11,7,4,12,15);
	b4.SetSizes(ny, nr, nz);
	b4.SetZoning(1, gr, 1, false, br, false);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(0,1,2,3,8,9,10,11);
	b5.SetSizes(nx, ny, nr);
	b5.SetZoning(1, 1, gr, false, false, br);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(7, 6, 5, 4, 15, 14, 13, 12);
	b6.SetSizes(nx, ny, nr);
	b6.SetZoning(1, 1, gr, false, false, br);

	// update the MB data
	BuildMB();

	// assign face ID's
	SetBlockFaceID(b1, 0, -1,  6, -1, -1, -1);
	SetBlockFaceID(b2, 1, -1,  7, -1, -1, -1);
	SetBlockFaceID(b3, 2, -1,  8, -1, -1, -1);
	SetBlockFaceID(b4, 3, -1,  9, -1, -1, -1);
	SetBlockFaceID(b5,-1, -1, -1, -1,  4, 10);
	SetBlockFaceID(b6,-1, -1, -1, -1,  5, 11);

	// assign edge ID's
	MBFace& F1 = GetBlockFace( 0, 0); SetFaceEdgeID(F1,  0, 9,   4,  8);
	MBFace& F2 = GetBlockFace( 1, 0); SetFaceEdgeID(F2,  1, 10,  5,  9);
	MBFace& F3 = GetBlockFace( 2, 0); SetFaceEdgeID(F3,  2, 11,  6, 10);
	MBFace& F4 = GetBlockFace( 3, 0); SetFaceEdgeID(F4,  3,  8,  7, 11);
	MBFace& F5 = GetBlockFace( 4, 4); SetFaceEdgeID(F5,  2,  1,  0,  3);
	MBFace& F6 = GetBlockFace( 5, 4); SetFaceEdgeID(F6,  4,  5,  6,  7);

	MBFace& F7  = GetBlockFace( 0, 2); SetFaceEdgeID(F7 , 12, 20, 16, 21);
	MBFace& F8  = GetBlockFace( 1, 2); SetFaceEdgeID(F8 , 13, 21, 17, 22);
	MBFace& F9  = GetBlockFace( 2, 2); SetFaceEdgeID(F9 , 14, 22, 18, 23);
	MBFace& F10 = GetBlockFace( 3, 2); SetFaceEdgeID(F10, 15, 23, 19, 20);
	MBFace& F11 = GetBlockFace( 4, 5); SetFaceEdgeID(F11, 12, 13, 14, 15);
	MBFace& F12 = GetBlockFace( 5, 5); SetFaceEdgeID(F12, 18, 17, 16, 19);

	GetFaceEdge(F7, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F7, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F7, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F7, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);

	GetFaceEdge(F8, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F8, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);

	GetFaceEdge(F9, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F9, 1).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F9, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F9, 3).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);

	GetFaceEdge(F10, 0).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);
	GetFaceEdge(F10, 2).SetEdge(EDGE_3P_CIRC_ARC, 1, 16);

	// set the node ID's
	m_MBNode[ 0].SetID(0);
	m_MBNode[ 1].SetID(1);
	m_MBNode[ 2].SetID(2);
	m_MBNode[ 3].SetID(3);
	m_MBNode[ 4].SetID(4);
	m_MBNode[ 5].SetID(5);
	m_MBNode[ 6].SetID(6);
	m_MBNode[ 7].SetID(7);
	m_MBNode[ 8].SetID(8);
	m_MBNode[ 9].SetID(9);
	m_MBNode[10].SetID(10);
	m_MBNode[11].SetID(11);
	m_MBNode[12].SetID(12);
	m_MBNode[13].SetID(13);
	m_MBNode[14].SetID(14);
	m_MBNode[15].SetID(15);

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
*/
