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

// FECone.cpp: implementation of the FECone class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FECone.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

FECone::FECone(GObject& o) : FEMultiBlockMesh(o)
{
	m_Rb = 0.5;
	m_nd = m_ns = 4;
	m_nz = 6;

	m_gz = 1;
	m_gr = 1;

	m_bz = true;
	m_br = false;

	AddDoubleParam(m_Rb, "r", "Ratio");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments" );
	AddIntParam(m_nz, "nz", "Stacks"   );

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("HEX8\0HEX20\0HEX27\0");
}

extern double gain2(double x, double r, double n);

bool FECone::BuildMultiBlock()
{
	GCone* po = dynamic_cast<GCone*>(&m_o);
	if (po == nullptr) return false;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double R0 = param.GetFloatValue(GCone::R0);
	double R1 = param.GetFloatValue(GCone::R1);
	double h = param.GetFloatValue(GCone::H);

	// get mesh parameters
	m_Rb = GetFloatValue(RB);
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);

	m_gz = GetFloatValue(GZ);
	m_gr = GetFloatValue(GR);

	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nz == 1) m_bz = false;
	if (m_ns == 1) m_br = false;

	double fz = m_gz;
	double fr = m_gr;

	if (m_Rb < 0) m_Rb = 0;
	if (m_Rb > 1) m_Rb = 1;

	double d00 = m_Rb * R0 / sqrt(2.0);	// bottom inside
	double d01 = R0 / sqrt(2.0);		// bottom outside
	double d10 = m_Rb * R1 / sqrt(2.0);	// top inside
	double d11 = R1 / sqrt(2.0);		// top outside

	// create the MB nodes
	m_MBNode.resize(34);
	m_MBNode[0].m_r = vec3d(-d00, -d00, 0);
	m_MBNode[1].m_r = vec3d(0, -d00, 0);
	m_MBNode[2].m_r = vec3d(d00, -d00, 0);
	m_MBNode[3].m_r = vec3d(-d00, 0, 0);
	m_MBNode[4].m_r = vec3d(0, 0, 0);
	m_MBNode[5].m_r = vec3d(d00, 0, 0);
	m_MBNode[6].m_r = vec3d(-d00, d00, 0);
	m_MBNode[7].m_r = vec3d(0, d00, 0);
	m_MBNode[8].m_r = vec3d(d00, d00, 0);

	m_MBNode[9].m_r = vec3d(-d10, -d10, h);
	m_MBNode[10].m_r = vec3d(0, -d10, h);
	m_MBNode[11].m_r = vec3d(d10, -d10, h);
	m_MBNode[12].m_r = vec3d(-d10, 0, h);
	m_MBNode[13].m_r = vec3d(0, 0, h);
	m_MBNode[14].m_r = vec3d(d10, 0, h);
	m_MBNode[15].m_r = vec3d(-d10, d10, h);
	m_MBNode[16].m_r = vec3d(0, d10, h);
	m_MBNode[17].m_r = vec3d(d10, d10, h);

	m_MBNode[18].m_r = vec3d(-d01, -d01, 0);
	m_MBNode[19].m_r = vec3d(0, -R0, 0);
	m_MBNode[20].m_r = vec3d(d01, -d01, 0);
	m_MBNode[21].m_r = vec3d(R0, 0, 0);
	m_MBNode[22].m_r = vec3d(d01, d01, 0);
	m_MBNode[23].m_r = vec3d(0, R0, 0);
	m_MBNode[24].m_r = vec3d(-d01, d01, 0);
	m_MBNode[25].m_r = vec3d(-R0, 0, 0);

	m_MBNode[26].m_r = vec3d(-d11, -d11, h);
	m_MBNode[27].m_r = vec3d(0, -R1, h);
	m_MBNode[28].m_r = vec3d(d11, -d11, h);
	m_MBNode[29].m_r = vec3d(R1, 0, h);
	m_MBNode[30].m_r = vec3d(d11, d11, h);
	m_MBNode[31].m_r = vec3d(0, R1, h);
	m_MBNode[32].m_r = vec3d(-d11, d11, h);
	m_MBNode[33].m_r = vec3d(-R1, 0, h);

	// create the MB blocks
	m_MBlock.resize(12);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0, 1, 4, 3, 9, 10, 13, 12);
	b1.SetSizes(m_nd, m_nd, m_nz);
	b1.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1, 2, 5, 4, 10, 11, 14, 13);
	b2.SetSizes(m_nd, m_nd, m_nz);
	b2.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(3, 4, 7, 6, 12, 13, 16, 15);
	b3.SetSizes(m_nd, m_nd, m_nz);
	b3.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(4, 5, 8, 7, 13, 14, 17, 16);
	b4.SetSizes(m_nd, m_nd, m_nz);
	b4.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(0, 18, 19, 1, 9, 26, 27, 10);
	b5.SetSizes(m_ns, m_nd, m_nz);
	b5.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(1, 19, 20, 2, 10, 27, 28, 11);
	b6.SetSizes(m_ns, m_nd, m_nz);
	b6.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b7 = m_MBlock[6];
	b7.SetID(0);
	b7.SetNodes(2, 20, 21, 5, 11, 28, 29, 14);
	b7.SetSizes(m_ns, m_nd, m_nz);
	b7.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b8 = m_MBlock[7];
	b8.SetID(0);
	b8.SetNodes(5, 21, 22, 8, 14, 29, 30, 17);
	b8.SetSizes(m_ns, m_nd, m_nz);
	b8.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b9 = m_MBlock[8];
	b9.SetID(0);
	b9.SetNodes(8, 22, 23, 7, 17, 30, 31, 16);
	b9.SetSizes(m_ns, m_nd, m_nz);
	b9.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b10 = m_MBlock[9];
	b10.SetID(0);
	b10.SetNodes(7, 23, 24, 6, 16, 31, 32, 15);
	b10.SetSizes(m_ns, m_nd, m_nz);
	b10.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b11 = m_MBlock[10];
	b11.SetID(0);
	b11.SetNodes(6, 24, 25, 3, 15, 32, 33, 12);
	b11.SetSizes(m_ns, m_nd, m_nz);
	b11.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b12 = m_MBlock[11];
	b12.SetID(0);
	b12.SetNodes(3, 25, 18, 0, 12, 33, 26, 9);
	b12.SetSizes(m_ns, m_nd, m_nz);
	b12.SetZoning(fr, 1, fz, false, false, m_bz);

	// update the MB data
	BuildMB();

	// assign face ID's
	SetBlockFaceID(b1, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b2, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b3, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b4, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b5, -1, 2, -1, -1, 4, 5);
	SetBlockFaceID(b6, -1, 3, -1, -1, 4, 5);
	SetBlockFaceID(b7, -1, 3, -1, -1, 4, 5);
	SetBlockFaceID(b8, -1, 0, -1, -1, 4, 5);
	SetBlockFaceID(b9, -1, 0, -1, -1, 4, 5);
	SetBlockFaceID(b10, -1, 1, -1, -1, 4, 5);
	SetBlockFaceID(b11, -1, 1, -1, -1, 4, 5);
	SetBlockFaceID(b12, -1, 2, -1, -1, 4, 5);

	MBFace& F1 = GetBlockFace(7, 1); SetFaceEdgeID(F1, 0, -1, 4, 8);
	MBFace& F2 = GetBlockFace(8, 1); SetFaceEdgeID(F2, 0, 9, 4, -1);
	MBFace& F3 = GetBlockFace(9, 1); SetFaceEdgeID(F3, 1, -1, 5, 9);
	MBFace& F4 = GetBlockFace(10, 1); SetFaceEdgeID(F4, 1, 10, 5, -1);
	MBFace& F5 = GetBlockFace(11, 1); SetFaceEdgeID(F5, 2, -1, 6, 10);
	MBFace& F6 = GetBlockFace(4, 1); SetFaceEdgeID(F6, 2, 11, 6, -1);
	MBFace& F7 = GetBlockFace(5, 1); SetFaceEdgeID(F7, 3, -1, 7, 11);
	MBFace& F8 = GetBlockFace(6, 1); SetFaceEdgeID(F8, 3, 8, 7, -1);

	GetFaceEdge(F1, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F1, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F1, 2).m_orient = -1;
	GetFaceEdge(F2, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F2, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F2, 2).m_orient = -1;
	GetFaceEdge(F3, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F3, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F3, 2).m_orient = -1;
	GetFaceEdge(F4, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F4, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F4, 2).m_orient = -1;
	GetFaceEdge(F5, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F5, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F5, 2).m_orient = -1;
	GetFaceEdge(F6, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F6, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F6, 2).m_orient = -1;
	GetFaceEdge(F7, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F7, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F7, 2).m_orient = -1;
	GetFaceEdge(F8, 0).m_ntype = EDGE_ZARC;
	GetFaceEdge(F8, 2).m_ntype = EDGE_ZARC; GetFaceEdge(F8, 2).m_orient = -1;

	m_MBNode[21].SetID(0);
	m_MBNode[23].SetID(1);
	m_MBNode[25].SetID(2);
	m_MBNode[19].SetID(3);
	m_MBNode[29].SetID(4);
	m_MBNode[31].SetID(5);
	m_MBNode[33].SetID(6);
	m_MBNode[27].SetID(7);

	UpdateMB();

	return true;
}

FSMesh* FECone::BuildMesh()
{
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
		// update the mesh
		pm->UpdateMesh();

		// the Multi-block mesher will assign a different smoothing ID
		// to each face, but we don't want that here. 
		// For now, we autosmooth the mesh although we should think of a 
		// better way
		pm->AutoSmooth(60);
	}

	return pm;
}
