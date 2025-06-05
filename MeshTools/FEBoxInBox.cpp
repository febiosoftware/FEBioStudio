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
#include "FEBoxInBox.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEElement.h>

FEBoxInBox::FEBoxInBox()
{
	m_pobj = nullptr;

	AddIntParam(10, "nx", "Nx");
	AddIntParam(10, "ny", "Ny");
	AddIntParam(10, "nz", "Nz");
	AddIntParam(10, "ns", "Segments");

	AddDoubleParam(1, "gx", "X-bias");
	AddDoubleParam(1, "gy", "Y-bias");
	AddDoubleParam(1, "gz", "Z-bias");
	AddDoubleParam(1, "gr", "R-bias");

	AddBoolParam(false, "bx", "X-mirrored bias");
	AddBoolParam(false, "by", "Y-mirrored bias");
	AddBoolParam(false, "bz", "Z-mirrored bias");
	AddBoolParam(false, "br", "R-mirrored bias");

	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("HEX8\0HEX20\0HEX27\0");
}

//-----------------------------------------------------------------------------
// Build the FEMesh
bool FEBoxInBox::BuildMultiBlock()
{
	assert(m_pobj);
	if (m_pobj == nullptr) return false;

	// get object parameters
	double W0 = m_pobj->OuterWidth();
	double H0 = m_pobj->OuterHeight();
	double D0 = m_pobj->OuterDepth();
	double W1 = m_pobj->InnerWidth();
	double H1 = m_pobj->InnerHeight();
	double D1 = m_pobj->InnerDepth();

	int nx = GetIntValue(NX);
	int ny = GetIntValue(NY);
	int nz = GetIntValue(NZ);
	int ns = GetIntValue(NSEG);

	bool bx = GetBoolValue(GX2);
	bool by = GetBoolValue(GY2);
	bool bz = GetBoolValue(GZ2);
	bool br = GetBoolValue(GR2);

	double gx = GetFloatValue(GX);
	double gy = GetFloatValue(GY);
	double gz = GetFloatValue(GZ);
	double gr = GetFloatValue(GR);

	// check parameters
	if (nx < 1) nx = 1;
	if (ny < 1) ny = 1;
	if (nz < 1) nz = 1;
	if (ns < 1) ns = 1;

	if (nx == 1) bx = false;
	if (ny == 1) by = false;
	if (nz == 1) bz = false;
	if (ns == 1) br = false;

	// create the MB nodes
	double w0 = W0 * 0.5;
	double h0 = H0 * 0.5;
	double w1 = W1 * 0.5;
	double h1 = H1 * 0.5;

	double z0 = 0.5 * (D0 - D1);
	double z1 = 0.5 * (D0 + D1);

	m_MBNode.resize(16);
	m_MBNode[ 0].m_r = vec3d(-w0, -h0, 0);
	m_MBNode[ 1].m_r = vec3d( w0, -h0, 0);
	m_MBNode[ 2].m_r = vec3d( w0,  h0, 0);
	m_MBNode[ 3].m_r = vec3d(-w0,  h0, 0);
	m_MBNode[ 4].m_r = vec3d(-w0, -h0, D0);
	m_MBNode[ 5].m_r = vec3d( w0, -h0, D0);
	m_MBNode[ 6].m_r = vec3d( w0,  h0, D0);
	m_MBNode[ 7].m_r = vec3d(-w0,  h0, D0);
	m_MBNode[ 8].m_r = vec3d(-w1, -h1, z0);
	m_MBNode[ 9].m_r = vec3d( w1, -h1, z0);
	m_MBNode[10].m_r = vec3d( w1,  h1, z0);
	m_MBNode[11].m_r = vec3d(-w1,  h1, z0);
	m_MBNode[12].m_r = vec3d(-w1, -h1, z1);
	m_MBNode[13].m_r = vec3d( w1, -h1, z1);
	m_MBNode[14].m_r = vec3d( w1,  h1, z1);
	m_MBNode[15].m_r = vec3d(-w1,  h1, z1);

	// create the MB block
	m_MBlock.resize(6);

	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0, 1, 9, 8, 4, 5, 13, 12);
	b1.SetSizes(nx, ns, nz);
	b1.SetZoning(gx, gr, gz, bx, br, bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1, 2, 10, 9, 5, 6, 14, 13);
	b2.SetSizes(ny, ns, nz);
	b2.SetZoning(gy, gr, gz, by, br, bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(2, 3, 11, 10, 6, 7, 15, 14);
	b3.SetSizes(nx, ns, nz);
	b3.SetZoning((bx ? gx : 1/gx), gr, gz, bx, br, bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(3, 0, 8, 11, 7, 4, 12, 15);
	b4.SetSizes(ny, ns, nz);
	b4.SetZoning((by?gy:1/gy), gr, gz, by, br, bz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(0, 1, 2, 3, 8, 9, 10, 11);
	b5.SetSizes(nx, ny, ns);
	b5.SetZoning(gx, gy, gr, bx, by, br);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(12, 13, 14, 15, 4, 5, 6, 7);
	b6.SetSizes(nx, ny, ns);
	b6.SetZoning(gx, gy, gr, bx, by, br);

	// update the MB data
	BuildMB();

	// next, we assign the face ID's
	MBFace& F1  = GetBlockFace(0, 0); F1.SetID(0);
	MBFace& F2  = GetBlockFace(1, 0); F2.SetID(1);
	MBFace& F3  = GetBlockFace(2, 0); F3.SetID(2);
	MBFace& F4  = GetBlockFace(3, 0); F4.SetID(3);
	MBFace& F5  = GetBlockFace(4, 4); F5.SetID(4);
	MBFace& F6  = GetBlockFace(5, 5); F6.SetID(5);
	MBFace& F7  = GetBlockFace(0, 2); F7.SetID(6);
	MBFace& F8  = GetBlockFace(1, 2); F8.SetID(7);
	MBFace& F9  = GetBlockFace(2, 2); F9.SetID(8);
	MBFace& F10 = GetBlockFace(3, 2); F10.SetID(9);
	MBFace& F11 = GetBlockFace(4, 5); F11.SetID(10);
	MBFace& F12 = GetBlockFace(5, 4); F12.SetID(11);

	// next, assign the edge ID's
	GetFaceEdge(F1, 0).SetID(0);
	GetFaceEdge(F2, 0).SetID(1);
	GetFaceEdge(F3, 0).SetID(2);
	GetFaceEdge(F4, 0).SetID(3);
	GetFaceEdge(F1, 2).SetID(4);
	GetFaceEdge(F2, 2).SetID(5);
	GetFaceEdge(F3, 2).SetID(6);
	GetFaceEdge(F4, 2).SetID(7);
	GetFaceEdge(F1, 3).SetID(8);
	GetFaceEdge(F2, 3).SetID(9);
	GetFaceEdge(F3, 3).SetID(10);
	GetFaceEdge(F4, 3).SetID(11);
	GetFaceEdge(F7 , 0).SetID(12);
	GetFaceEdge(F8 , 0).SetID(13);
	GetFaceEdge(F9 , 0).SetID(14);
	GetFaceEdge(F10, 0).SetID(15);
	GetFaceEdge(F7 , 2).SetID(16);
	GetFaceEdge(F8 , 2).SetID(17);
	GetFaceEdge(F9 , 2).SetID(18);
	GetFaceEdge(F10, 2).SetID(19);
	GetFaceEdge(F7 , 1).SetID(20);
	GetFaceEdge(F8 , 1).SetID(21);
	GetFaceEdge(F9 , 1).SetID(22);
	GetFaceEdge(F10, 1).SetID(23);

	// assign the node ID's
	GetMBNode( 0).SetID( 0);
	GetMBNode( 1).SetID( 1);
	GetMBNode( 2).SetID( 2);
	GetMBNode( 3).SetID( 3);
	GetMBNode( 4).SetID( 4);
	GetMBNode( 5).SetID( 5);
	GetMBNode( 6).SetID( 6);
	GetMBNode( 7).SetID( 7);
	GetMBNode( 8).SetID( 8);
	GetMBNode( 9).SetID( 9);
	GetMBNode(10).SetID(10);
	GetMBNode(11).SetID(11);
	GetMBNode(12).SetID(12);
	GetMBNode(13).SetID(13);
	GetMBNode(14).SetID(14);
	GetMBNode(15).SetID(15);

	UpdateMB();

	return true;
}

FSMesh* FEBoxInBox::BuildMesh(GObject* po)
{
	m_pobj = dynamic_cast<GBoxInBox*>(po);
	if (m_pobj == nullptr) return nullptr;

	BuildMultiBlock();

	// set element type
	int nelem = GetIntValue(ELEM_TYPE);
	switch (nelem)
	{
	case 0: SetElementType(FE_HEX8 ); break;
	case 1: SetElementType(FE_HEX20); break;
	case 2: SetElementType(FE_HEX27); break;
	}

	// create the MB
	return FEMultiBlockMesh::BuildMBMesh();
}
