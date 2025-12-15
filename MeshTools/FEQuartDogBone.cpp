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
#include "FEQuartDogBone.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>

//-----------------------------------------------------------------------------
FEQuartDogBone::FEQuartDogBone(GObject& o) : FEMultiBlockMesh(o)
{
	AddIntParam(10, "nx", "Nx");
	AddIntParam(30, "ny", "Ny");
	AddIntParam( 1, "nz", "Nz");

	AddDoubleParam(1, "R-bias");
	AddDoubleParam(1, "Z-bias");
	AddBoolParam(false, "bz", "Z-mirrored bias");

	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

//-----------------------------------------------------------------------------
bool FEQuartDogBone::BuildMultiBlock()
{
	GQuartDogBone* po = dynamic_cast<GQuartDogBone*>(&m_o);
	if (po == nullptr) return false;

	// see if we should do 4-block or 6-block
	double cw = po->GetFloatValue(GQuartDogBone::CWIDTH);
	double ch = po->GetFloatValue(GQuartDogBone::CHEIGHT);
	double R = po->GetFloatValue(GQuartDogBone::RADIUS);
	double W = po->GetFloatValue(GQuartDogBone::WING);

	if (ch < cw - W - R) return BuildMultiBlockMesh4();
	else return BuildMultiBlockMesh6();
}

FSMesh* FEQuartDogBone::BuildMesh()
{
	GQuartDogBone* po = dynamic_cast<GQuartDogBone*>(&m_o);
	if (po == nullptr) return nullptr;

	if (!BuildMultiBlock()) return nullptr;

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

	return pm;
}

//-----------------------------------------------------------------------------
bool FEQuartDogBone::BuildMultiBlockMesh4()
{
	GQuartDogBone* po = dynamic_cast<GQuartDogBone*>(&m_o);
	if (po == nullptr) return false;

	// get parameters
	double cw = po->GetFloatValue(GQuartDogBone::CWIDTH);
	double ch = po->GetFloatValue(GQuartDogBone::CHEIGHT);
	double R = po->GetFloatValue(GQuartDogBone::RADIUS);
	double h = po->GetFloatValue(GQuartDogBone::DEPTH);
	double L = po->GetFloatValue(GQuartDogBone::LENGTH);
	double W = po->GetFloatValue(GQuartDogBone::WING);

	double a = sqrt(2.0)*0.5;
	double px = cw - W - R * a;
	double py = ch + R * (1.0 - a);

	int NX = GetIntValue(N_X);
	int NY = GetIntValue(N_Y);
	int NZ = GetIntValue(N_Z);

	double gr = GetFloatValue(R_BIAS);
	double gz = GetFloatValue(Z_BIAS);
	bool bz = GetBoolValue(BZ);

	if (NX < 1) NX = 1;
	if (NY < 2) NY = 2;

	int nx = NX;

	double fy = L / (ch + R + L);
	int ny2 = (int)(fy*NY); if (ny2 < 1) ny2 = 1;
	int ny1 = NY - ny2;

	int nx1 = ny1;
	double fx = W / (cw - W);
	int nx2 = (int)(fx*nx1); if (nx2 < 1) nx2 = 1;

	int nz = NZ;

	// create the MB nodes
	m_MBNode.clear();
	AddNode(vec3d(0, 0, 0));
	AddNode(vec3d(cw - W, 0, 0));
	AddNode(vec3d(cw, 0, 0));
	AddNode(vec3d(cw, ch, 0));
	AddNode(vec3d(cw - W, ch, 0));
	AddNode(vec3d(px, py, 0));
	AddNode(vec3d(cw - W - R, ch + R, 0));
	AddNode(vec3d(cw - W - R, ch + R + L, 0));
	AddNode(vec3d(0, ch + R + L, 0));
	AddNode(vec3d(0, ch + R, 0));

	AddNode(vec3d(cw-W, ch+R, 0), NODE_SHAPE);

	AddNode(vec3d(0, 0, h));
	AddNode(vec3d(cw - W, 0, h));
	AddNode(vec3d(cw, 0, h));
	AddNode(vec3d(cw, ch, h));
	AddNode(vec3d(cw - W, ch, h));
	AddNode(vec3d(px, py, h));
	AddNode(vec3d(cw - W - R, ch + R, h));
	AddNode(vec3d(cw - W - R, ch + R + L, h));
	AddNode(vec3d(0, ch + R + L, h));
	AddNode(vec3d(0, ch + R, h));

	AddNode(vec3d(cw - W, ch + R, h), NODE_SHAPE);

	// create the MB blocks
	m_MBlock.resize(4);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0, 1, 4, 5, 11, 12, 15, 16);
	b1.SetSizes(nx1, nx, nz);
	b1.SetZoning(1, gr, gz, false, false, bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(0, 5, 6, 9, 11, 16, 17, 20);
	b2.SetSizes(nx, ny1, nz);
	b2.SetZoning(gr, 1, gz, false, false, bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(1, 2, 3, 4, 12, 13, 14, 15);
	b3.SetSizes(nx2, nx, nz);
	b3.SetZoning(1, gr, gz, false, false, bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(9, 6, 7, 8, 20, 17, 18, 19);
	b4.SetSizes(nx, ny2, nz);
	b4.SetZoning(gr, 1, gz, false, false, bz);

	BuildMB();

	SetBlockFaceID(b1,  1, -1,  4, -1, 0, 8);
	SetBlockFaceID(b2, -1,  4, -1,  7, 0, 8);
	SetBlockFaceID(b3,  1,  2,  3, -1, 0, 8);
	SetBlockFaceID(b4, -1,  5,  6,  7, 0, 8);

	MBFace& F1 = GetBlockFace(0, 0); SetFaceEdgeID(F1, 0, -1, 7, 14);
	MBFace& F2 = GetBlockFace(2, 0); SetFaceEdgeID(F2, 0, 15, 7, -1);
	MBFace& F3 = GetBlockFace(2, 1); SetFaceEdgeID(F3, 1, 16, 8, 15);
	MBFace& F4 = GetBlockFace(2, 2); SetFaceEdgeID(F4, 2, 17, 9, 16);
	MBFace& F5 = GetBlockFace(0, 2); SetFaceEdgeID(F5, 3, -1, 10, 17);
	MBEdge& E1 = GetEdge(F5.m_edge[0]);	E1.m_ntype = EDGE_3P_CIRC_ARC; E1.m_cnode = 10; E1.m_orient = -1;
	MBEdge& E2 = GetEdge(F5.m_edge[2]); E2.m_ntype = EDGE_3P_CIRC_ARC; E2.m_cnode = 21;

	MBFace& F6 = GetBlockFace(1, 1); SetFaceEdgeID(F6, 3, 18, 10, -1);
	MBEdge& E3 = GetEdge(F6.m_edge[0]); E3.m_ntype = EDGE_3P_CIRC_ARC; E3.m_cnode = 10; E3.m_orient = -1;
	MBEdge& E4 = GetEdge(F6.m_edge[2]); E4.m_ntype = EDGE_3P_CIRC_ARC; E4.m_cnode = 21;

	MBFace& F7 = GetBlockFace(3, 1); SetFaceEdgeID(F7, 4, 19, 11, 18);
	MBFace& F8 = GetBlockFace(3, 2); SetFaceEdgeID(F8, 5, 20, 12, 19);
	MBFace& F9 = GetBlockFace(3, 3); SetFaceEdgeID(F9, 6, -1, 13, 20);
	MBFace& F10 = GetBlockFace(1, 3); SetFaceEdgeID(F10, 6, 14, 13, -1);

	m_MBNode[0].SetID(0);
	m_MBNode[2].SetID(1);
	m_MBNode[3].SetID(2);
	m_MBNode[4].SetID(3);
	m_MBNode[6].SetID(4);
	m_MBNode[7].SetID(5);
	m_MBNode[8].SetID(6);

	m_MBNode[11].SetID(8);
	m_MBNode[13].SetID(9);
	m_MBNode[14].SetID(10);
	m_MBNode[15].SetID(11);
	m_MBNode[17].SetID(12);
	m_MBNode[18].SetID(13);
	m_MBNode[19].SetID(14);

	UpdateMB();

	return true;
}

//-----------------------------------------------------------------------------
bool FEQuartDogBone::BuildMultiBlockMesh6()
{
	GQuartDogBone* po = dynamic_cast<GQuartDogBone*>(&m_o);
	if (po == nullptr) return false;

	// get parameters
	double cw = po->GetFloatValue(GQuartDogBone::CWIDTH);
	double ch = po->GetFloatValue(GQuartDogBone::CHEIGHT);
	double R = po->GetFloatValue(GQuartDogBone::RADIUS);
	double h = po->GetFloatValue(GQuartDogBone::DEPTH);
	double L = po->GetFloatValue(GQuartDogBone::LENGTH);
	double W = po->GetFloatValue(GQuartDogBone::WING);

	double a = sqrt(2.0)*0.5;
	double px = cw - W - R * a;
	double py = ch + R * (1.0 - a);

	double x1 = cw - W;
	double x2 = cw;
	double x3 = cw - W - R;
	double y2 = ch;
	double y1 = y2 - x3;
	double y3 = ch + R;
	double y4 = ch + R + L;

	int NX = GetIntValue(N_X);
	int NY = GetIntValue(N_Y);
	int NZ = GetIntValue(N_Z);

	double gr = GetFloatValue(R_BIAS);
	double gz = GetFloatValue(Z_BIAS);
	bool bz = GetBoolValue(BZ);

	if (NX < 1) NX = 1;
	if (NY < 3) NY = 3;

	double f1 = y1 / y4;
	double f2 = (y3 - y1) / y4;
	int ny1 = (int)(f1*NY); if (ny1 < 1) ny1 = 1;
	int ny2 = (int)(f2*NY); if (ny2 < 1) ny2 = 1;
	int ny3 = NY - ny1 - ny2;

	double fx = W / (cw - W);
	int nx1 = ny2;
	int nx2 = (int)(fx*nx1); if (nx2 < 1) nx2 = 1;

	int nz = NZ;

	// create the MB nodes
	m_MBNode.clear();
	AddNode(vec3d( 0,  0, 0));
	AddNode(vec3d(x1,  0, 0));
	AddNode(vec3d(x2,  0, 0));
	AddNode(vec3d( 0, y1, 0));
	AddNode(vec3d(x1, y1, 0));
	AddNode(vec3d(x2, y1, 0));
	AddNode(vec3d(px, py, 0));
	AddNode(vec3d(x1, y2, 0));
	AddNode(vec3d(x2, y2, 0));
	AddNode(vec3d( 0, y3, 0));
	AddNode(vec3d(x3, y3, 0));
	AddNode(vec3d( 0, y4, 0));
	AddNode(vec3d(x3, y4, 0));

	AddNode(vec3d(x1, y3, 0), NODE_SHAPE);

	AddNode(vec3d( 0,  0, h));
	AddNode(vec3d(x1,  0, h));
	AddNode(vec3d(x2,  0, h));
	AddNode(vec3d( 0, y1, h));
	AddNode(vec3d(x1, y1, h));
	AddNode(vec3d(x2, y1, h));
	AddNode(vec3d(px, py, h));
	AddNode(vec3d(x1, y2, h));
	AddNode(vec3d(x2, y2, h));
	AddNode(vec3d( 0, y3, h));
	AddNode(vec3d(x3, y3, h));
	AddNode(vec3d( 0, y4, h));
	AddNode(vec3d(x3, y4, h));

	AddNode(vec3d(x1, y3, h), NODE_SHAPE);

	// create the MB blocks
	m_MBlock.resize(6);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0, 1, 4, 3, 14, 15, 18, 17);
	b1.SetSizes(nx1, ny1, nz);
	b1.SetZoning(1, 1, gz, false, false, bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1, 2, 5, 4, 15, 16, 19, 18);
	b2.SetSizes(nx2, ny1, nz);
	b2.SetZoning(1, 1, gz, false, false, bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(3, 4, 7, 6, 17, 18, 21, 20);
	b3.SetSizes(nx1, NX, nz);
	b3.SetZoning(1, gr, gz, false, false, bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(4, 5, 8, 7, 18, 19, 22, 21);
	b4.SetSizes(nx2, NX, nz);
	b4.SetZoning(1, gr, gz, false, false, bz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(3, 6, 10, 9, 17, 20, 24, 23);
	b5.SetSizes(NX, ny2, nz);
	b5.SetZoning(gr, 1, gz, false, false, bz);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(9, 10, 12, 11, 23, 24, 26, 25);
	b6.SetSizes(NX, ny3, nz);
	b6.SetZoning(gr, 1, gz, false, false, bz);

	BuildMB();

	SetBlockFaceID(b1,  1, -1, -1,  7, 0, 8);
	SetBlockFaceID(b2,  1,  2, -1, -1, 0, 8);
	SetBlockFaceID(b3, -1, -1,  4, -1, 0, 8);
	SetBlockFaceID(b4, -1,  2,  3, -1, 0, 8);
	SetBlockFaceID(b5, -1,  4, -1,  7, 0, 8);
	SetBlockFaceID(b6, -1,  5,  6,  7, 0, 8);

	MBFace& F1  = GetBlockFace(0, 0); SetFaceEdgeID(F1 , 0, -1, 7, 14);
	MBFace& F2  = GetBlockFace(1, 0); SetFaceEdgeID(F2 , 0, 15, 7, -1);
	MBFace& F3  = GetBlockFace(1, 1); SetFaceEdgeID(F3 , 1, -1, 8, 15);
	MBFace& F4  = GetBlockFace(3, 1); SetFaceEdgeID(F4 , 1, 16, 8, -1);
	MBFace& F5  = GetBlockFace(3, 2); SetFaceEdgeID(F5 , 2, 17, 9, 16);
	MBFace& F6  = GetBlockFace(2, 2); SetFaceEdgeID(F6 , 3, -1, 10, 17);
	MBFace& F7  = GetBlockFace(4, 1); SetFaceEdgeID(F7 , 3, 18, 10, -1);
	MBFace& F8  = GetBlockFace(5, 1); SetFaceEdgeID(F8 , 4, 19, 11, 18);
	MBFace& F9  = GetBlockFace(5, 2); SetFaceEdgeID(F9 , 5, 20, 12, 19);
	MBFace& F10 = GetBlockFace(5, 3); SetFaceEdgeID(F10, 6, -1, 13, 20);
	MBFace& F11 = GetBlockFace(4, 3); SetFaceEdgeID(F11, 6, -1, 13, -1);
	MBFace& F12 = GetBlockFace(0, 3); SetFaceEdgeID(F12, 6, 14, 13, -1);

	MBEdge& E1 = GetEdge(F6.m_edge[0]);	E1.m_ntype = EDGE_3P_CIRC_ARC; E1.m_cnode = 13; E1.m_orient = -1;
	MBEdge& E2 = GetEdge(F6.m_edge[2]); E2.m_ntype = EDGE_3P_CIRC_ARC; E2.m_cnode = 27;
	MBEdge& E3 = GetEdge(F7.m_edge[0]); E3.m_ntype = EDGE_3P_CIRC_ARC; E3.m_cnode = 13; E3.m_orient = -1;
	MBEdge& E4 = GetEdge(F7.m_edge[2]); E4.m_ntype = EDGE_3P_CIRC_ARC; E4.m_cnode = 27;

	m_MBNode[ 0].SetID(0);
	m_MBNode[ 2].SetID(1);
	m_MBNode[ 8].SetID(2);
	m_MBNode[ 7].SetID(3);
	m_MBNode[10].SetID(4);
	m_MBNode[12].SetID(5);
	m_MBNode[11].SetID(6);

	m_MBNode[14].SetID(8);
	m_MBNode[16].SetID(9);
	m_MBNode[22].SetID(10);
	m_MBNode[21].SetID(11);
	m_MBNode[24].SetID(12);
	m_MBNode[26].SetID(13);
	m_MBNode[25].SetID(14);

	UpdateMB();

	return true;
}


//-----------------------------------------------------------------------------
FSMesh* FEQuartDogBone::BuildMeshLegacy()
{
	GQuartDogBone* po = dynamic_cast<GQuartDogBone*>(&m_o);
	if (po == nullptr) return nullptr;

	ParamBlock& param = po->GetParamBlock();
	double cw = param.GetFloatValue(GQuartDogBone::CWIDTH );
	double ch = param.GetFloatValue(GQuartDogBone::CHEIGHT);
	double R  = param.GetFloatValue(GQuartDogBone::RADIUS );
	double h  = param.GetFloatValue(GQuartDogBone::DEPTH  );
	double L  = param.GetFloatValue(GQuartDogBone::LENGTH );
	double W  = param.GetFloatValue(GQuartDogBone::WING   );

	double a = sqrt(2.0)*0.5;
	double px = cw - W - R*a;
	double py = ch + R*(1.0-a);

	int NX = GetIntValue(N_X);
	int NY = GetIntValue(N_Y);
	int NZ = GetIntValue(N_Z);

	if (NX < 3) NX = 3;
	if (NY < 3) NY = 3;

	double fx1 = px/cw;
	double fx2 = (cw - W - px)/(cw - px);
	double fy1 = py / (ch+R+L);
	double fy2 = (ch+R-py)/(ch+R+L-py);


	int nx = (int) (NX*fx1); if (nx < 1) nx = 1;
	int lx = (int) (fx2*(NX - nx)); if (lx < 1) lx = 1;
	int mx = NX - nx - lx;

	int ny = (int) (NY*fy1); if (ny < 1) ny = 1;
	int ly = (int) (fy2*(NY - ny)); if (ly < 1) ly = 1;
	int my = NY - ny - ly;

	int nz = NZ;

	// create the MB nodes
	m_MBNode.resize(24);
	m_MBNode[ 0].m_r = vec3d(0     ,0     ,0);
	m_MBNode[ 1].m_r = vec3d(px    ,0     ,0);
	m_MBNode[ 2].m_r = vec3d(cw-W  ,0     ,0);
	m_MBNode[ 3].m_r = vec3d(cw    ,0     ,0);
	m_MBNode[ 4].m_r = vec3d(cw    ,ch    ,0);
	m_MBNode[ 5].m_r = vec3d(cw-W  ,ch    ,0);
	m_MBNode[ 6].m_r = vec3d(px    ,py    ,0);
	m_MBNode[ 7].m_r = vec3d(cw-W-R,ch+R  ,0);
	m_MBNode[ 8].m_r = vec3d(cw-W-R,ch+R+L,0);
	m_MBNode[ 9].m_r = vec3d(0     ,ch+R+L,0);
	m_MBNode[10].m_r = vec3d(0     ,ch+R  ,0);
	m_MBNode[11].m_r = vec3d(0     ,py    ,0);

	m_MBNode[12].m_r = vec3d(0     ,0     ,h);
	m_MBNode[13].m_r = vec3d(px    ,0     ,h);
	m_MBNode[14].m_r = vec3d(cw-W  ,0     ,h);
	m_MBNode[15].m_r = vec3d(cw    ,0     ,h);
	m_MBNode[16].m_r = vec3d(cw    ,ch    ,h);
	m_MBNode[17].m_r = vec3d(cw-W  ,ch    ,h);
	m_MBNode[18].m_r = vec3d(px    ,py    ,h);
	m_MBNode[19].m_r = vec3d(cw-W-R,ch+R  ,h);
	m_MBNode[20].m_r = vec3d(cw-W-R,ch+R+L,h);
	m_MBNode[21].m_r = vec3d(0     ,ch+R+L,h);
	m_MBNode[22].m_r = vec3d(0     ,ch+R  ,h);
	m_MBNode[23].m_r = vec3d(0     ,py    ,h);

	// create the MB blocks
	m_MBlock.resize(5);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0,1,6,11,12,13,18,23);
	b1.SetSizes(nx,ny,nz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(1);
	b2.SetNodes(1,2,5,6,13,14,17,18);
	b2.SetSizes(lx,ny,nz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(2);
	b3.SetNodes(2,3,4,5,14,15,16,17);
	b3.SetSizes(mx,ny,nz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(3);
	b4.SetNodes(11,6,7,10,23,18,19,22);
	b4.SetSizes(nx,ly,nz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(4);
	b5.SetNodes(10,7,8,9,22,19,20,21);
	b5.SetSizes(nx,my,nz);


	BuildMB();

	SetBlockFaceID(b1, 1, -1, -1,  7, 0, 8);
	SetBlockFaceID(b2, 1, -1,  4, -1, 0, 8);
	SetBlockFaceID(b3, 1,  2,  3, -1, 0, 8);
	SetBlockFaceID(b4,-1,  4, -1,  7, 0, 8);
	SetBlockFaceID(b5,-1,  5,  6,  7, 0, 8);

	MBFace& F1  = GetBlockFace( 0, 0); SetFaceEdgeID(F1, 0, -1,  7, 14);
	MBFace& F2  = GetBlockFace( 1, 0); SetFaceEdgeID(F2, 0, -1,  7, -1);
	MBFace& F3  = GetBlockFace( 2, 0); SetFaceEdgeID(F3, 0, 15,  7, -1);
	MBFace& F4  = GetBlockFace( 2, 1); SetFaceEdgeID(F4, 1, 16,  8, 15);
	MBFace& F5  = GetBlockFace( 2, 2); SetFaceEdgeID(F5, 2, 17,  9, 16);
	MBFace& F6  = GetBlockFace( 1, 2); SetFaceEdgeID(F6, 3, -1, 10, 17);
	MBFace& F7  = GetBlockFace( 3, 1); SetFaceEdgeID(F7, 3, 18, 10, -1);
	MBFace& F8  = GetBlockFace( 4, 1); SetFaceEdgeID(F8, 4, 19, 11, 18);
	MBFace& F9  = GetBlockFace( 4, 2); SetFaceEdgeID(F9, 5, 20, 12, 19);
	MBFace& F10 = GetBlockFace( 4, 3); SetFaceEdgeID(F10, 6,-1, 13, 20);
	MBFace& F11 = GetBlockFace( 3, 3); SetFaceEdgeID(F11, 6,-1, 13, -1);
	MBFace& F12 = GetBlockFace( 0, 3); SetFaceEdgeID(F12, 6, 14, 13,-1);

	m_MBNode[0].SetID(0);
	m_MBNode[3].SetID(1);
	m_MBNode[4].SetID(2);
	m_MBNode[5].SetID(3);
	m_MBNode[7].SetID(4);
	m_MBNode[8].SetID(5);
	m_MBNode[9].SetID(6);

	m_MBNode[12].SetID(8);
	m_MBNode[15].SetID(9);
	m_MBNode[16].SetID(10);
	m_MBNode[17].SetID(11);
	m_MBNode[19].SetID(12);
	m_MBNode[20].SetID(13);
	m_MBNode[21].SetID(14);

	// create the MB
	FSMesh* pm = FEMultiBlockMesh::BuildMBMesh();

	// get all the nodes from block 1
	std::vector<int> node;
	pm->FindNodesFromPart(1, node);
	
	// project these nodes onto the cylinder
	double x0 = cw-W, y0 = ch + R;
	for (int i=0; i<(int)node.size(); ++i)
	{
		vec3d& q = pm->Node(node[i]).r;

		double r = (q.x - px)/(cw - W - px);
		double h = py*(1.0 - r) + r*ch;
		double s = q.y / h;
		double y = y0 - sqrt(R*R - (q.x - x0)*(q.x - x0));
		q.y = s*y;
	}

	// get all the nodes from block 3
	pm->FindNodesFromPart(3, node);
	
	// project these nodes onto the cylinder
	for (int i=0; i<(int)node.size(); ++i)
	{
		vec3d& q = pm->Node(node[i]).r;

		double s = (q.y - py)/(ch + R - py);
		double w = px*(1.0 - s) + s*(cw-W-R);
		double r = q.x / w;
		double x = x0 - sqrt(R*R - (q.y - y0)*(q.y - y0));
		q.x = r*x;
	}

	// reset the element group ID's
	for (int i=0; i<pm->Elements(); ++i) pm->Element(i).m_gid = 0;

	// update the mesh
	pm->BuildMesh();

	return pm;
}
