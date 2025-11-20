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
#include "FECylinderInBox.h"
#include <GeomLib/GPrimitive.h>
#include "FEModifier.h"

FECylinderInBox::FECylinderInBox(GObject& o) : FEMultiBlockMesh(o)
{
	m_nx = 10;
	m_ny = 10;
	m_nz = 10;
	m_nr = 10;

	m_gz = 1;
	m_gr = 1;
	m_bz = false;
	m_br = false;

	m_nelem = 0;

	// define the tube parameters
	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
	AddIntParam(m_nz, "nz", "Nz");
	AddIntParam(m_nr, "nr", "Nr");

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddIntParam(m_nelem, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

//-----------------------------------------------------------------------------
bool FECylinderInBox::BuildMultiBlock()
{
	GCylinderInBox* po = dynamic_cast<GCylinderInBox*>(&m_o);
	if (po == 0) return false;

	// get the object parameters
	double W = po->GetFloatValue(GCylinderInBox::WIDTH );
	double H = po->GetFloatValue(GCylinderInBox::HEIGHT);
	double D = po->GetFloatValue(GCylinderInBox::DEPTH );
	double R = po->GetFloatValue(GCylinderInBox::RADIUS);

	double w = W*0.5;
	double h = H*0.5;
	double d = D;
	double r = R / sqrt(2.0);

	// get meshing parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);
	m_nr = GetIntValue(NR);

	m_gz = GetFloatValue(GZ);
	m_gr = GetFloatValue(GR);

	m_bz = GetBoolValue(BZ);
	m_br = GetBoolValue(BR);

	// create the MB nodes
	m_MBNode.clear();
	AddNode(vec3d(-w, -h, 0)).SetID(0);
	AddNode(vec3d( w, -h, 0)).SetID(1);
	AddNode(vec3d( w,  h, 0)).SetID(2);
	AddNode(vec3d(-w,  h, 0)).SetID(3);
	AddNode(vec3d(-w, -h, d)).SetID(4);
	AddNode(vec3d( w, -h, d)).SetID(5);
	AddNode(vec3d( w,  h, d)).SetID(6);
	AddNode(vec3d(-w,  h, d)).SetID(7);
							 
	AddNode(vec3d(-r, -r, 0)).SetID(8);
	AddNode(vec3d( r, -r, 0)).SetID(9);
	AddNode(vec3d( r,  r, 0)).SetID(10);
	AddNode(vec3d(-r,  r, 0)).SetID(11);
	AddNode(vec3d(-r, -r, d)).SetID(12);
	AddNode(vec3d( r, -r, d)).SetID(13);
	AddNode(vec3d( r,  r, d)).SetID(14);
	AddNode(vec3d(-r,  r, d)).SetID(15);

	AddNode(vec3d(0, 0, 0), NODE_SHAPE);
	AddNode(vec3d(0, 0, d), NODE_SHAPE);

	// create the blocks
	m_MBlock.resize(4);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0,1,9,8,4,5,13,12);
	b1.SetSizes(m_nx, m_nr, m_nz);
	b1.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1,2,10,9,5,6,14,13);
	b2.SetSizes(m_ny, m_nr, m_nz);
	b2.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(2,3,11,10,6,7,15,14);
	b3.SetSizes(m_nx, m_nr, m_nz);
	b3.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(3,0,8,11,7,4,12,15);
	b4.SetSizes(m_ny, m_nr, m_nz);
	b4.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	// update the MB data
	BuildMB();

	// assign face ID's
	SetBlockFaceID(b1, 0, -1, 4, -1,  8, 12);
	SetBlockFaceID(b2, 1, -1, 5, -1,  9, 13);
	SetBlockFaceID(b3, 2, -1, 6, -1, 10, 14);
	SetBlockFaceID(b4, 3, -1, 7, -1, 11, 15);

	MBFace& F1 = GetBlockFace( 0, 0); SetFaceEdgeID(F1,  0, 9,   4,  8);
	MBFace& F2 = GetBlockFace( 1, 0); SetFaceEdgeID(F2,  1, 10,  5,  9);
	MBFace& F3 = GetBlockFace( 2, 0); SetFaceEdgeID(F3,  2, 11,  6, 10);
	MBFace& F4 = GetBlockFace( 3, 0); SetFaceEdgeID(F4,  3,  8,  7, 11);
	MBFace& F5 = GetBlockFace( 0, 2); SetFaceEdgeID(F5, 12, 20, 16, 21);
	MBFace& F6 = GetBlockFace( 1, 2); SetFaceEdgeID(F6, 13, 21, 17, 22);
	MBFace& F7 = GetBlockFace( 2, 2); SetFaceEdgeID(F7, 14, 22, 18, 23);
	MBFace& F8 = GetBlockFace( 3, 2); SetFaceEdgeID(F8, 15, 23, 19, 20);

	MBFace& F9  = GetBlockFace( 0, 4); SetFaceEdgeID(F9 , 12, 25,  0, 24);
	MBFace& F10 = GetBlockFace( 1, 4); SetFaceEdgeID(F10, 13, 26,  1, 25);
	MBFace& F11 = GetBlockFace( 2, 4); SetFaceEdgeID(F11, 14, 27,  2, 26);
	MBFace& F12 = GetBlockFace( 3, 4); SetFaceEdgeID(F12, 15, 24,  3, 27);

	MBFace& F13 = GetBlockFace( 0, 5); SetFaceEdgeID(F13,  4, 29, 16, 28);
	MBFace& F14 = GetBlockFace( 1, 5); SetFaceEdgeID(F14,  5, 30, 17, 29);
	MBFace& F15 = GetBlockFace( 2, 5); SetFaceEdgeID(F15,  6, 31, 18, 30);
	MBFace& F16 = GetBlockFace( 3, 5); SetFaceEdgeID(F16,  7, 28, 19, 31);

	// set the edges
	GetFaceEdge(F5, 0).SetWinding(-1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 16;
	GetFaceEdge(F5, 2).SetWinding( 1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 17;
	GetFaceEdge(F6, 0).SetWinding(-1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 16;
	GetFaceEdge(F6, 2).SetWinding( 1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 17;
	GetFaceEdge(F7, 0).SetWinding(-1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 16;
	GetFaceEdge(F7, 2).SetWinding( 1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 17;
	GetFaceEdge(F8, 0).SetWinding(-1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 16;
	GetFaceEdge(F8, 2).SetWinding( 1).SetType(EDGE_3P_CIRC_ARC).m_cnode = 17;

	UpdateMB();

	return true;
}

FSMesh* FECylinderInBox::BuildMesh()
{
	if (!BuildMultiBlock()) return nullptr;

	// set element type
	int nelem = GetIntValue(NELEM);
	switch (nelem)
	{
	case 0: SetElementType(FE_HEX8 ); break;
	case 1: SetElementType(FE_HEX20); break;
	case 2: SetElementType(FE_HEX27); break;
	}

	// create the MB
	FSMesh* pm = FEMultiBlockMesh::BuildMBMesh();

	return pm;
}
