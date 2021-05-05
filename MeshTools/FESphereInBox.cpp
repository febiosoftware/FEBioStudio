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

#include "stdafx.h"
#include "FESphereInBox.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
FESphereInBox::FESphereInBox()
{
	m_po = 0;
}

//-----------------------------------------------------------------------------
FESphereInBox::FESphereInBox(GSphereInBox* po)
{
	m_po = po;

	// define the tube parameters
	AddIntParam(5, "nx", "Nx");
	AddIntParam(5, "ny", "Ny");
	AddIntParam(5, "nz", "Nz");
	AddIntParam(1, "nr", "Nr");

	AddDoubleParam(1.0, "gr", "R-bias");
	AddBoolParam(false, "br", "R-mirrored bias");
}

//-----------------------------------------------------------------------------
FEMesh* FESphereInBox::BuildMesh()
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
	UpdateMB();

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
	FEMesh* pm = FEMultiBlockMesh::BuildMesh();

	// update the mesh
	pm->UpdateMesh();

	// the Multi-block mesher will assign a different smoothing ID
	// to each face, but we don't want that here. 
	// For now, we autosmooth the mesh although we should think of a 
	// better way
	pm->AutoSmooth(60);

	return pm;
}
