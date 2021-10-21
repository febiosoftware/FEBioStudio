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

// FEBox.cpp: implementation of the FEBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEBox.h"
#include "FEModifier.h"
#include <GeomLib/GPrimitive.h>

//-----------------------------------------------------------------------------
// Class constructor
FEBox::FEBox(GBox* po)
{
	m_pobj = po;

	m_ctype = SIMPLE;
	m_nelem = 0;

	m_r = 0.5;
	m_nx = m_ny = m_nz = m_ns = 10;
	m_gx = m_gy = m_gz = m_gr = 1;
	m_bx = m_by = m_bz = m_br = false;

	AddIntParam(m_ctype, "ctype", "Mesh Type")->SetEnumNames("Regular\0Butterfly 3D\0Butterfly 2D\0");
	AddDoubleParam(m_r, "r", "Ratio");	

	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
	AddIntParam(m_nz, "nz", "Nz");
	AddIntParam(m_ns, "ns", "Segments");

	AddDoubleParam(m_gx, "gx", "X-bias");
	AddDoubleParam(m_gy, "gy", "Y-bias");
	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bx, "bx", "X-mirrored bias");
	AddBoolParam(m_by, "by", "Y-mirrored bias");
	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddIntParam(m_nelem, "elem", "Element")->SetEnumNames("Hex8\0Hex20\0Hex27\0Tet4\0Tet10\0Tet15\0Tet20\0");
}

//-----------------------------------------------------------------------------
// Build the FEMesh
FEMesh* FEBox::BuildMesh()
{
	assert(m_pobj);

	FEMesh* pm = 0;

	m_ctype = GetIntValue(CTYPE);

	switch (m_ctype)
	{
	case SIMPLE: pm = CreateRegular(); break;
	case BUTTERFLY3D: pm = CreateButterfly3D(); break;
	case BUTTERFLY2D: pm = CreateButterfly2D(); break;
	default:
		assert(false);
	}

	return pm;
}

//-----------------------------------------------------------------------------
// Build a 3D butterfly mesh
FEMesh* FEBox::CreateButterfly3D()
{
	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double w = param[GBox::WIDTH ].GetFloatValue();
	double h = param[GBox::HEIGHT].GetFloatValue();
	double d = param[GBox::DEPTH ].GetFloatValue();

	// get mesh parameters
	m_r = GetFloatValue(RATIO);

	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);
	m_ns = GetIntValue(NSEG);

	m_bx = GetBoolValue(GX2);
	m_by = GetBoolValue(GY2);
	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	m_gx = GetFloatValue(GX);
	m_gy = GetFloatValue(GY);
	m_gz = GetFloatValue(GZ);
	m_gr = GetFloatValue(GR);

	// check parameters
	if (m_nx < 1) m_nx = 1;
	if (m_ny < 1) m_ny = 1;
	if (m_nz < 1) m_nz = 1;
	if (m_ns < 1) m_ns = 1;

	if (m_r < 0.001) m_r = .001;
	if (m_r > 0.999) m_r = .999;

	if (m_nx == 1) m_bx = false;
	if (m_ny == 1) m_by = false;
	if (m_nz == 1) m_bz = false;
	if (m_ns == 1) m_br = false;

	// create the MB nodes
	double r1 = 0.5;
	double r2 = m_r*r1;
	double d1 = d*(1-m_r)*0.5;
	double d2 = d - d1;
	m_MBNode.resize(16);
	m_MBNode[ 0].m_r = vec3d(-w*r1,-h*r1, 0);
	m_MBNode[ 1].m_r = vec3d( w*r1,-h*r1, 0);
	m_MBNode[ 2].m_r = vec3d( w*r1, h*r1, 0);
	m_MBNode[ 3].m_r = vec3d(-w*r1, h*r1, 0);
	m_MBNode[ 4].m_r = vec3d(-w*r1,-h*r1, d);
	m_MBNode[ 5].m_r = vec3d( w*r1,-h*r1, d);
	m_MBNode[ 6].m_r = vec3d( w*r1, h*r1, d);
	m_MBNode[ 7].m_r = vec3d(-w*r1, h*r1, d);
	m_MBNode[ 8].m_r = vec3d(-w*r2,-h*r2, d1);
	m_MBNode[ 9].m_r = vec3d( w*r2,-h*r2, d1);
	m_MBNode[10].m_r = vec3d( w*r2, h*r2, d1);
	m_MBNode[11].m_r = vec3d(-w*r2, h*r2, d1);
	m_MBNode[12].m_r = vec3d(-w*r2,-h*r2, d2);
	m_MBNode[13].m_r = vec3d( w*r2,-h*r2, d2);
	m_MBNode[14].m_r = vec3d( w*r2, h*r2, d2);
	m_MBNode[15].m_r = vec3d(-w*r2, h*r2, d2);

	// create the MB block
	m_MBlock.resize(7);

	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(8,9,10,11,12,13,14,15);
	b1.SetSizes(m_nx, m_ny, m_nz);
	b1.SetZoning(m_gx, m_gy, m_gz, m_bx, m_by, m_bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(8,0,1,9,12,4,5,13);
	b2.SetSizes(m_ns, m_nx, m_nz);
	b2.SetZoning(m_gr, m_gx, m_gz, m_br, m_bx, m_bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(9,1,2,10,13,5,6,14);
	b3.SetSizes(m_ns, m_ny, m_nz);
	b3.SetZoning(m_gr, m_gy, m_gz, m_br, m_by, m_bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(10,2,3,11,14,6,7,15);
	b4.SetSizes(m_ns, m_nx, m_nz);
	b4.SetZoning(m_gr, (m_bx?m_gx:1/m_gx), m_gz, m_br, m_bx, m_bz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(11,3,0,8,15,7,4,12);
	b5.SetSizes(m_ns, m_ny, m_nz);
	b5.SetZoning(m_gr, (m_by?m_gy:1/m_gy), m_gz, m_br, m_by, m_bz);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(11,10,9,8,3,2,1,0);
	b6.SetSizes(m_nx, m_ny, m_ns);
	b6.SetZoning(m_gx, (m_by?m_gy:1/m_gy), m_gr, m_bx, m_by, m_br);

	MBBlock& b7 = m_MBlock[6];
	b7.SetID(0);
	b7.SetNodes(12,13,14,15,4,5,6,7);
	b7.SetSizes(m_nx, m_ny, m_ns);
	b7.SetZoning(m_gx, m_gy, m_gr, m_bx, m_by, m_br);

	// update the MB data
	UpdateMB();

	// next, we assign the face ID's
	MBFace& F1 = GetBlockFace(1, 1); F1.SetID(0);
	MBFace& F2 = GetBlockFace(2, 1); F2.SetID(1);
	MBFace& F3 = GetBlockFace(3, 1); F3.SetID(2);
	MBFace& F4 = GetBlockFace(4, 1); F4.SetID(3);
	MBFace& F5 = GetBlockFace(5, 5); F5.SetID(4);
	MBFace& F6 = GetBlockFace(6, 5); F6.SetID(5);

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

	// assign the node ID's
	GetMBNode(0).SetID(0);
	GetMBNode(1).SetID(1);
	GetMBNode(2).SetID(2);
	GetMBNode(3).SetID(3);
	GetMBNode(4).SetID(4);
	GetMBNode(5).SetID(5);
	GetMBNode(6).SetID(6);
	GetMBNode(7).SetID(7);

	// create the MB
	return FEMultiBlockMesh::BuildMesh();
}

//-----------------------------------------------------------------------------
// Build a 2D butterfly mesh
FEMesh* FEBox::CreateButterfly2D()
{
	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double w = param[GBox::WIDTH ].GetFloatValue();
	double h = param[GBox::HEIGHT].GetFloatValue();
	double d = param[GBox::DEPTH ].GetFloatValue();

	// get mesh parameters
	m_r = GetFloatValue(RATIO);

	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);
	m_ns = GetIntValue(NSEG);

	m_bx = GetBoolValue(GX2);
	m_by = GetBoolValue(GY2);
	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	m_gx = GetFloatValue(GX);
	m_gy = GetFloatValue(GY);
	m_gz = GetFloatValue(GZ);
	m_gr = GetFloatValue(GR);

	// check parameters
	if (m_nx < 1) m_nx = 1;
	if (m_ny < 1) m_ny = 1;
	if (m_nz < 1) m_nz = 1;
	if (m_ns < 1) m_ns = 1;

	if (m_r < 0.001) m_r = .001;
	if (m_r > 0.999) m_r = .999;

	if (m_nx == 1) m_bx = false;
	if (m_ny == 1) m_by = false;
	if (m_nz == 1) m_bz = false;
	if (m_ns == 1) m_br = false;

	// create the MB nodes
	double r1 = 0.5;
	double r2 = m_r*r1;
	m_MBNode.resize(16);
	m_MBNode[ 0].m_r = vec3d(-w*r1,-h*r1, 0);
	m_MBNode[ 1].m_r = vec3d( w*r1,-h*r1, 0);
	m_MBNode[ 2].m_r = vec3d( w*r1, h*r1, 0);
	m_MBNode[ 3].m_r = vec3d(-w*r1, h*r1, 0);
	m_MBNode[ 4].m_r = vec3d(-w*r1,-h*r1, d);
	m_MBNode[ 5].m_r = vec3d( w*r1,-h*r1, d);
	m_MBNode[ 6].m_r = vec3d( w*r1, h*r1, d);
	m_MBNode[ 7].m_r = vec3d(-w*r1, h*r1, d);
	m_MBNode[ 8].m_r = vec3d(-w*r2,-h*r2, 0);
	m_MBNode[ 9].m_r = vec3d( w*r2,-h*r2, 0);
	m_MBNode[10].m_r = vec3d( w*r2, h*r2, 0);
	m_MBNode[11].m_r = vec3d(-w*r2, h*r2, 0);
	m_MBNode[12].m_r = vec3d(-w*r2,-h*r2, d);
	m_MBNode[13].m_r = vec3d( w*r2,-h*r2, d);
	m_MBNode[14].m_r = vec3d( w*r2, h*r2, d);
	m_MBNode[15].m_r = vec3d(-w*r2, h*r2, d);

	// create the MB block
	m_MBlock.resize(5);

	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(8,9,10,11,12,13,14,15);
	b1.SetSizes(m_nx, m_ny, m_nz);
	b1.SetZoning(m_gx, m_gy, m_gz, m_bx, m_by, m_bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(8,0,1,9,12,4,5,13);
	b2.SetSizes(m_ns, m_nx, m_nz);
	b2.SetZoning(m_gr, m_gx, m_gz, m_br, m_bx, m_bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(9,1,2,10,13,5,6,14);
	b3.SetSizes(m_ns, m_ny, m_nz);
	b3.SetZoning(m_gr, m_gy, m_gz, m_br, m_by, m_bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(10,2,3,11,14,6,7,15);
	b4.SetSizes(m_ns, m_nx, m_nz);
	b4.SetZoning(m_gr, (m_bx?m_gx:1/m_gx), m_gz, m_br, m_bx, m_bz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(11,3,0,8,15,7,4,12);
	b5.SetSizes(m_ns, m_ny, m_nz);
	b5.SetZoning(m_gr, (m_by?m_gy:1/m_gy), m_gz, m_br, m_by, m_bz);

	// update the MB data
	UpdateMB();

	// next, we assign the face ID's
	MBFace& F1 = GetBlockFace(1, 1); F1.SetID(0);
	MBFace& F2 = GetBlockFace(2, 1); F2.SetID(1);
	MBFace& F3 = GetBlockFace(3, 1); F3.SetID(2);
	MBFace& F4 = GetBlockFace(4, 1); F4.SetID(3);

	GetBlockFace(0,4).SetID(4);
	GetBlockFace(1,4).SetID(4);
	GetBlockFace(2,4).SetID(4);
	GetBlockFace(3,4).SetID(4);
	GetBlockFace(4,4).SetID(4);

	GetBlockFace(0,5).SetID(5);
	GetBlockFace(1,5).SetID(5);
	GetBlockFace(2,5).SetID(5);
	GetBlockFace(3,5).SetID(5);
	GetBlockFace(4,5).SetID(5);

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

	// assign the node ID's
	GetMBNode(0).SetID(0);
	GetMBNode(1).SetID(1);
	GetMBNode(2).SetID(2);
	GetMBNode(3).SetID(3);
	GetMBNode(4).SetID(4);
	GetMBNode(5).SetID(5);
	GetMBNode(6).SetID(6);
	GetMBNode(7).SetID(7);

	// create the MB
	return FEMultiBlockMesh::BuildMesh();
}

//-----------------------------------------------------------------------------
FEMesh* FEBox::CreateRegular()
{
	m_nelem = GetIntValue(NELEM);
	switch (m_nelem)
	{
	case 0: return CreateRegularHEX8 (); break;
	case 1: return CreateRegularHEX20(); break;
	case 2: return CreateRegularHEX27(); break;
	case 3: return CreateRegularTET4(); break;
	case 4: return CreateRegularTET10(); break;
	case 5: return CreateRegularTET15(); break;
	case 6: return CreateRegularTET20(); break;
	default:
		assert(false);
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Create a regular mesh
FEMesh* FEBox::CreateRegularHEX8()
{
	int i, j, k;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double w = param[GBox::WIDTH ].GetFloatValue();
	double h = param[GBox::HEIGHT].GetFloatValue();
	double d = param[GBox::DEPTH ].GetFloatValue();

	// get mesh parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);

	m_bx = GetBoolValue(GX2);
	m_by = GetBoolValue(GY2);
	m_bz = GetBoolValue(GZ2);

	m_gx = GetFloatValue(GX);
	m_gy = GetFloatValue(GY);
	m_gz = GetFloatValue(GZ);

	// check parameters
	if (m_nx < 1) m_nx = 1;
	if (m_ny < 1) m_ny = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nx == 1) m_bx = false;
	if (m_ny == 1) m_by = false;
	if (m_nz == 1) m_bz = false;

	int nodes = (m_nx+1)*(m_ny+1)*(m_nz+1);
	int elems = m_nx*m_ny*m_nz;

	// allocate storage
	FEMesh* pm = new FEMesh;
	pm->Create(nodes, elems);

	double gx = 1;
	double gy = 1;
	double gz = 1;

	double fx = m_gx;
	double fy = m_gy;
	double fz = m_gz;

	if (m_bx)
	{
		gx = 2; if (m_nx%2) gx += fx;
		for (i=0; i<m_nx/2-1; ++i) gx = fx*gx+2;
		gx = w / gx;
	}
	else 
	{
		for (i=0; i<m_nx-1; ++i) gx = fx*gx+1; 
		gx = w / gx;
	}

	if (m_by)
	{
		gy = 2; if (m_ny%2) gy += fy;
		for (i=0; i<m_ny/2-1; ++i) gy = fy*gy+2;
		gy = h / gy;
	}
	else 
	{
		for (i=0; i<m_ny-1; ++i) gy = fy*gy+1; 
		gy = h / gy;
	}

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = d / gz;
	}
	else 
	{
		for (i=0; i<m_nz-1; ++i) gz = fz*gz+1; 
		gz = d / gz;
	}

	// position the nodes
	double x, y, z;
	double dx, dy, dz;
	FENode* pn = pm->NodePtr();
	dx = gx;
	x = -w/2;
	for (i=0; i<=m_nx; i++)
	{
		dy = gy;
		y = -h/2;
		for (j=0; j<=m_ny; j++)
		{
			dz = gz;
			z = 0;
			for (k=0; k<=m_nz; k++, pn++)
			{
				pn->r = vec3d(x, y, z);

				z += dz;
				dz *= fz;
				if (m_bz && (k == m_nz/2-1))
				{
					if (m_nz%2 == 0) dz /= fz;
					fz = 1.0/fz;
				}
			}
			if (m_bz) fz = 1.0/fz;
			y += dy;
			dy *= fy;
			if (m_by && (j == m_ny/2-1))
			{
				if (m_ny%2 == 0) dy /= fy;
				fy = 1.0/fy;
			}
		}
		if (m_by) fy = 1.0/fy;
		x += dx;
		dx *= fx;
		if (m_bx && (i == m_nx/2-1))
		{
			if (m_nx%2 == 0) dx /= fx;
			fx = 1.0/fx;
		}
	}

	// assign the node ID's
	pm->Node( NodeIndex(   0,    0,    0) ).m_gid = 0;
	pm->Node( NodeIndex(m_nx,    0,    0) ).m_gid = 1;
	pm->Node( NodeIndex(m_nx, m_ny,    0) ).m_gid = 2;
	pm->Node( NodeIndex(   0, m_ny,    0) ).m_gid = 3;
	pm->Node( NodeIndex(   0,    0, m_nz) ).m_gid = 4;
	pm->Node( NodeIndex(m_nx,    0, m_nz) ).m_gid = 5;
	pm->Node( NodeIndex(m_nx, m_ny, m_nz) ).m_gid = 6;
	pm->Node( NodeIndex(   0, m_ny, m_nz) ).m_gid = 7;

	// create the connectivity
	int n = 0;
	for (i=0; i<m_nx; i++)
		for (j=0; j<m_ny; j++)
			for (k=0; k<m_nz; k++)
			{
				FEElement_* pe = pm->ElementPtr(n++);

				pe->m_node[0] = NodeIndex(i  ,j  ,k);
				pe->m_node[1] = NodeIndex(i+1,j  ,k);
				pe->m_node[2] = NodeIndex(i+1,j+1,k);
				pe->m_node[3] = NodeIndex(i  ,j+1,k);
				pe->m_node[4] = NodeIndex(i  ,j  ,k+1);
				pe->m_node[5] = NodeIndex(i+1,j  ,k+1);
				pe->m_node[6] = NodeIndex(i+1,j+1,k+1);
				pe->m_node[7] = NodeIndex(i  ,j+1,k+1);

				pe->m_gid = 0;
				pe->SetType(FE_HEX8);
			}
	// build faces
	BuildHexFaces(pm);

	// build edges
	BuildEdges(pm);

	// update mesh
	pm->BuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// Create a regular mesh
FEMesh* FEBox::CreateRegularTET4()
{
	int i, j, k;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double w = param[GBox::WIDTH ].GetFloatValue();
	double h = param[GBox::HEIGHT].GetFloatValue();
	double d = param[GBox::DEPTH ].GetFloatValue();

	// get mesh parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);

	m_bx = GetBoolValue(GX2);
	m_by = GetBoolValue(GY2);
	m_bz = GetBoolValue(GZ2);

	m_gx = GetFloatValue(GX);
	m_gy = GetFloatValue(GY);
	m_gz = GetFloatValue(GZ);

	m_nelem = GetIntValue(NELEM);

	// check parameters
	if (m_nx < 1) m_nx = 1;
	if (m_ny < 1) m_ny = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nx == 1) m_bx = false;
	if (m_ny == 1) m_by = false;
	if (m_nz == 1) m_bz = false;

	int nodes = (m_nx+1)*(m_ny+1)*(m_nz+1);
	int elems = 6*m_nx*m_ny*m_nz;

	// allocate storage
	FEMesh* pm = new FEMesh;
	pm->Create(nodes, elems);

	double gx = 1;
	double gy = 1;
	double gz = 1;

	double fx = m_gx;
	double fy = m_gy;
	double fz = m_gz;

	if (m_bx)
	{
		gx = 2; if (m_nx%2) gx += fx;
		for (i=0; i<m_nx/2-1; ++i) gx = fx*gx+2;
		gx = w / gx;
	}
	else 
	{
		for (i=0; i<m_nx-1; ++i) gx = fx*gx+1; 
		gx = w / gx;
	}

	if (m_by)
	{
		gy = 2; if (m_ny%2) gy += fy;
		for (i=0; i<m_ny/2-1; ++i) gy = fy*gy+2;
		gy = h / gy;
	}
	else 
	{
		for (i=0; i<m_ny-1; ++i) gy = fy*gy+1; 
		gy = h / gy;
	}

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = d / gz;
	}
	else 
	{
		for (i=0; i<m_nz-1; ++i) gz = fz*gz+1; 
		gz = d / gz;
	}

	// position the nodes
	double x, y, z;
	double dx, dy, dz;
	FENode* pn = pm->NodePtr();
	dx = gx;
	x = -w/2;
	for (i=0; i<=m_nx; i++)
	{
		dy = gy;
		y = -h/2;
		for (j=0; j<=m_ny; j++)
		{
			dz = gz;
			z = 0;
			for (k=0; k<=m_nz; k++, pn++)
			{
				pn->r = vec3d(x, y, z);

				z += dz;
				dz *= fz;
				if (m_bz && (k == m_nz/2-1))
				{
					if (m_nz%2 == 0) dz /= fz;
					fz = 1.0/fz;
				}
			}
			if (m_bz) fz = 1.0/fz;
			y += dy;
			dy *= fy;
			if (m_by && (j == m_ny/2-1))
			{
				if (m_ny%2 == 0) dy /= fy;
				fy = 1.0/fy;
			}
		}
		if (m_by) fy = 1.0/fy;
		x += dx;
		dx *= fx;
		if (m_bx && (i == m_nx/2-1))
		{
			if (m_nx%2 == 0) dx /= fx;
			fx = 1.0/fx;
		}
	}

	// assign the node ID's
	pm->Node( NodeIndex(   0,    0,    0) ).m_gid = 0;
	pm->Node( NodeIndex(m_nx,    0,    0) ).m_gid = 1;
	pm->Node( NodeIndex(m_nx, m_ny,    0) ).m_gid = 2;
	pm->Node( NodeIndex(   0, m_ny,    0) ).m_gid = 3;
	pm->Node( NodeIndex(   0,    0, m_nz) ).m_gid = 4;
	pm->Node( NodeIndex(m_nx,    0, m_nz) ).m_gid = 5;
	pm->Node( NodeIndex(m_nx, m_ny, m_nz) ).m_gid = 6;
	pm->Node( NodeIndex(   0, m_ny, m_nz) ).m_gid = 7;

	// create the connectivity
	int TET[6][4] = {
		{1, 2, 3, 6}, {0, 1, 3, 6}, {0, 6, 3, 7},
		{1, 5, 6, 0}, {4, 7, 5, 0}, {5, 7, 6, 0}};

	int n[8], eid = 0;
	for (i=0; i<m_nx; i++)
		for (j=0; j<m_ny; j++)
			for (k=0; k<m_nz; k++)
			{
				n[0] = NodeIndex(i  ,j  ,k);
				n[1] = NodeIndex(i+1,j  ,k);
				n[2] = NodeIndex(i+1,j+1,k);
				n[3] = NodeIndex(i  ,j+1,k);
				n[4] = NodeIndex(i  ,j  ,k+1);
				n[5] = NodeIndex(i+1,j  ,k+1);
				n[6] = NodeIndex(i+1,j+1,k+1);
				n[7] = NodeIndex(i  ,j+1,k+1);

				for (int l=0; l<6; ++l)
				{
					FEElement_* pe = pm->ElementPtr(eid++);

					pe->m_node[0] = n[TET[l][0]];
					pe->m_node[1] = n[TET[l][1]];
					pe->m_node[2] = n[TET[l][2]];
					pe->m_node[3] = n[TET[l][3]];

					pe->m_gid = 0;
					pe->SetType(FE_TET4);
				}
			}

	// build faces
	BuildTetFaces(pm);

	// build edges
	BuildEdges(pm);

	// update the mesh
	pm->BuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// Create a regular mesh
FEMesh* FEBox::CreateRegularHEX20()
{
	int i, j, k;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double w = param[GBox::WIDTH ].GetFloatValue();
	double h = param[GBox::HEIGHT].GetFloatValue();
	double d = param[GBox::DEPTH ].GetFloatValue();

	// get mesh parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);

	m_bx = GetBoolValue(GX2);
	m_by = GetBoolValue(GY2);
	m_bz = GetBoolValue(GZ2);

	m_gx = GetFloatValue(GX);
	m_gy = GetFloatValue(GY);
	m_gz = GetFloatValue(GZ);

	m_nelem = GetIntValue(NELEM);

	// check parameters
	if (m_nx < 1) m_nx = 1;
	if (m_ny < 1) m_ny = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nx == 1) m_bx = false;
	if (m_ny == 1) m_by = false;
	if (m_nz == 1) m_bz = false;

	int nodes = (2*m_nx+1)*(2*m_ny+1)*(2*m_nz+1) - m_nx*m_ny*m_nz - 6*m_nx*m_ny*m_nz + (m_nx-1)*m_ny*m_nz + (m_ny-1)*m_nx*m_nz + (m_nz-1)*m_nx*m_ny;
	int elems = m_nx*m_ny*m_nz;

	// allocate storage
	FEMesh* pm = new FEMesh;
	pm->Create(nodes, elems);

	double gx = 1;
	double gy = 1;
	double gz = 1;

	double fx = m_gx;
	double fy = m_gy;
	double fz = m_gz;

	if (m_bx)
	{
		gx = 2; if ((2*m_nx)%2) gx += fx;
		for (i=0; i<(2*m_nx)/2-1; ++i) gx = fx*gx+2;
		gx = w / gx;
	}
	else 
	{
		for (i=0; i<2*m_nx-1; ++i) gx = fx*gx+1; 
		gx = w / gx;
	}

	if (m_by)
	{
		gy = 2; if ((2*m_ny)%2) gy += fy;
		for (i=0; i<(2*m_ny)/2-1; ++i) gy = fy*gy+2;
		gy = h / gy;
	}
	else 
	{
		for (i=0; i<2*m_ny-1; ++i) gy = fy*gy+1; 
		gy = h / gy;
	}

	if (m_bz)
	{
		gz = 2; if ((2*m_nz)%2) gz += fz;
		for (i=0; i<(2*m_nz)/2-1; ++i) gz = fz*gz+2;
		gz = d / gz;
	}
	else 
	{
		for (i=0; i<2*m_nz-1; ++i) gz = fz*gz+1; 
		gz = d / gz;
	}

	// position the nodes
	double x, y, z;
	double dx, dy, dz;
	int NN = (2*m_nx + 1)*(2*m_ny + 1)*(2*m_nz + 1);
	vector<vec3d> r(NN);
	int nn = 0;
	dx = gx;
	x = -w/2;
	for (i=0; i<=2*m_nx; i++)
	{
		dy = gy;
		y = -h/2;
		for (j=0; j<=2*m_ny; j++)
		{
			dz = gz;
			z = 0;
			for (k=0; k<=2*m_nz; k++, nn++)
			{
				r[nn] = vec3d(x, y, z);

				z += dz;
				dz *= fz;
				if (m_bz && (k == m_nz-1))
				{
					if (m_nz%2 == 0) dz /= fz;
					fz = 1.0/fz;
				}
			}
			if (m_bz) fz = 1.0/fz;
			y += dy;
			dy *= fy;
			if (m_by && (j == m_ny-1))
			{
				if (m_ny%2 == 0) dy /= fy;
				fy = 1.0/fy;
			}
		}
		if (m_by) fy = 1.0/fy;
		x += dx;
		dx *= fx;
		if (m_bx && (i == m_nx-1))
		{
			if (m_nx%2 == 0) dx /= fx;
			fx = 1.0/fx;
		}
	}

	// Build the LUT
	vector<int> LUT(NN);
	nn = 0;
	int mx = 2*m_nx;
	int my = 2*m_ny;
	int mz = 2*m_nz;
	for (i=0; i<=mx;++i)
	{
		for (j=0; j<=my;++j)
		{
			for (k=0; k<=mz;++k)
			{
				int mm = k + j*(mz+1) + i*(my+1)*(mz+1);
				LUT[mm] = -1;

				if ((i%2==0)&&(j%2==0)&&(k%2==0)) LUT[mm] = nn++;
				else if ((i%2==0)&&(j%2==1)&&(k%2==0)) LUT[mm] = nn++;
				else if ((i%2==0)&&(j%2==0)&&(k%2==1)) LUT[mm] = nn++;
				else if ((j%2==0)&&(i%2==1)&&(k%2==0)) LUT[mm] = nn++;
				else if ((j%2==0)&&(i%2==0)&&(k%2==1)) LUT[mm] = nn++;
				else if ((k%2==0)&&(i%2==1)&&(j%2==0)) LUT[mm] = nn++;
				else if ((k%2==0)&&(i%2==0)&&(j%2==1)) LUT[mm] = nn++;
			}
		}
	}

	// assign nodal positions
	for (i=0; i<NN; ++i) if (LUT[i] != -1) pm->Node(LUT[i]).r = r[i];

	// assign the node ID's
	pm->Node( NodeIndex2( 0,  0,  0, LUT) ).m_gid = 0;
	pm->Node( NodeIndex2(mx,  0,  0, LUT) ).m_gid = 1;
	pm->Node( NodeIndex2(mx, my,  0, LUT) ).m_gid = 2;
	pm->Node( NodeIndex2( 0, my,  0, LUT) ).m_gid = 3;
	pm->Node( NodeIndex2( 0,  0, mz, LUT) ).m_gid = 4;
	pm->Node( NodeIndex2(mx,  0, mz, LUT) ).m_gid = 5;
	pm->Node( NodeIndex2(mx, my, mz, LUT) ).m_gid = 6;
	pm->Node( NodeIndex2( 0, my, mz, LUT) ).m_gid = 7;

	// create the connectivity
	int eid = 0;
	for (i=0; i<2*m_nx; i+=2)
		for (j=0; j<2*m_ny; j+=2)
			for (k=0; k<2*m_nz; k+=2)
			{
				FEElement_* pe = pm->ElementPtr(eid++);

				pe->m_node[0] = NodeIndex2(i  ,j  ,k, LUT);
				pe->m_node[1] = NodeIndex2(i+2,j  ,k, LUT);
				pe->m_node[2] = NodeIndex2(i+2,j+2,k, LUT);
				pe->m_node[3] = NodeIndex2(i  ,j+2,k, LUT);
				pe->m_node[4] = NodeIndex2(i  ,j  ,k+2, LUT);
				pe->m_node[5] = NodeIndex2(i+2,j  ,k+2, LUT);
				pe->m_node[6] = NodeIndex2(i+2,j+2,k+2, LUT);
				pe->m_node[7] = NodeIndex2(i  ,j+2,k+2, LUT);

				pe->m_node[ 8] = NodeIndex2(i+1,j  ,k, LUT);
				pe->m_node[ 9] = NodeIndex2(i+2,j+1,k, LUT);
				pe->m_node[10] = NodeIndex2(i+1,j+2,k, LUT);
				pe->m_node[11] = NodeIndex2(i  ,j+1,k, LUT);
				pe->m_node[12] = NodeIndex2(i+1,j  ,k+2, LUT);
				pe->m_node[13] = NodeIndex2(i+2,j+1,k+2, LUT);
				pe->m_node[14] = NodeIndex2(i+1,j+2,k+2, LUT);
				pe->m_node[15] = NodeIndex2(i  ,j+1,k+2, LUT);
				pe->m_node[16] = NodeIndex2(i  ,j  ,k+1, LUT);
				pe->m_node[17] = NodeIndex2(i+2,j  ,k+1, LUT);
				pe->m_node[18] = NodeIndex2(i+2,j+2,k+1, LUT);
				pe->m_node[19] = NodeIndex2(i  ,j+2,k+1, LUT);

				pe->m_gid = 0;
				pe->SetType(FE_HEX20);
			}

	// build faces
	BuildHex20Faces(pm, LUT);

	// build edges
	BuildHex20Edges(pm, LUT);

	// update mesh
	pm->BuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// Create a regular mesh
FEMesh* FEBox::CreateRegularHEX27()
{
	int i, j, k;

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double w = param[GBox::WIDTH ].GetFloatValue();
	double h = param[GBox::HEIGHT].GetFloatValue();
	double d = param[GBox::DEPTH ].GetFloatValue();

	// get mesh parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);

	m_bx = GetBoolValue(GX2);
	m_by = GetBoolValue(GY2);
	m_bz = GetBoolValue(GZ2);

	m_gx = GetFloatValue(GX);
	m_gy = GetFloatValue(GY);
	m_gz = GetFloatValue(GZ);

	m_nelem = GetIntValue(NELEM);

	// check parameters
	if (m_nx < 1) m_nx = 1;
	if (m_ny < 1) m_ny = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nx == 1) m_bx = false;
	if (m_ny == 1) m_by = false;
	if (m_nz == 1) m_bz = false;

	int nodes = (2*m_nx+1)*(2*m_ny+1)*(2*m_nz+1);
	int elems = m_nx*m_ny*m_nz;

	// allocate storage
	FEMesh* pm = new FEMesh;
	pm->Create(nodes, elems);

	double gx = 1;
	double gy = 1;
	double gz = 1;

	double fx = m_gx;
	double fy = m_gy;
	double fz = m_gz;

	if (m_bx)
	{
		gx = 2; if ((2*m_nx)%2) gx += fx;
		for (i=0; i<(2*m_nx)/2-1; ++i) gx = fx*gx+2;
		gx = w / gx;
	}
	else 
	{
		for (i=0; i<2*m_nx-1; ++i) gx = fx*gx+1; 
		gx = w / gx;
	}

	if (m_by)
	{
		gy = 2; if ((2*m_ny)%2) gy += fy;
		for (i=0; i<(2*m_ny)/2-1; ++i) gy = fy*gy+2;
		gy = h / gy;
	}
	else 
	{
		for (i=0; i<2*m_ny-1; ++i) gy = fy*gy+1; 
		gy = h / gy;
	}

	if (m_bz)
	{
		gz = 2; if ((2*m_nz)%2) gz += fz;
		for (i=0; i<(2*m_nz)/2-1; ++i) gz = fz*gz+2;
		gz = d / gz;
	}
	else 
	{
		for (i=0; i<2*m_nz-1; ++i) gz = fz*gz+1; 
		gz = d / gz;
	}

	// position the nodes
	double x, y, z;
	double dx, dy, dz;
	int nn = 0;
	dx = gx;
	x = -w/2;
	for (i=0; i<=2*m_nx; i++)
	{
		dy = gy;
		y = -h/2;
		for (j=0; j<=2*m_ny; j++)
		{
			dz = gz;
			z = 0;
			for (k=0; k<=2*m_nz; k++, nn++)
			{
				pm->Node(nn).r = vec3d(x, y, z);

				z += dz;
				dz *= fz;
				if (m_bz && (k == m_nz-1))
				{
					if (m_nz%2 == 0) dz /= fz;
					fz = 1.0/fz;
				}
			}
			if (m_bz) fz = 1.0/fz;
			y += dy;
			dy *= fy;
			if (m_by && (j == m_ny-1))
			{
				if (m_ny%2 == 0) dy /= fy;
				fy = 1.0/fy;
			}
		}
		if (m_by) fy = 1.0/fy;
		x += dx;
		dx *= fx;
		if (m_bx && (i == m_nx-1))
		{
			if (m_nx%2 == 0) dx /= fx;
			fx = 1.0/fx;
		}
	}

	// assign the node ID's
	int mx = 2*m_nx;
	int my = 2*m_ny;
	int mz = 2*m_nz;
	pm->Node( NodeIndex3( 0,  0,  0) ).m_gid = 0;
	pm->Node( NodeIndex3(mx,  0,  0) ).m_gid = 1;
	pm->Node( NodeIndex3(mx, my,  0) ).m_gid = 2;
	pm->Node( NodeIndex3( 0, my,  0) ).m_gid = 3;
	pm->Node( NodeIndex3( 0,  0, mz) ).m_gid = 4;
	pm->Node( NodeIndex3(mx,  0, mz) ).m_gid = 5;
	pm->Node( NodeIndex3(mx, my, mz) ).m_gid = 6;
	pm->Node( NodeIndex3( 0, my, mz) ).m_gid = 7;

	// create the connectivity
	int eid = 0;
	for (i=0; i<mx; i+=2)
		for (j=0; j<my; j+=2)
			for (k=0; k<mz; k+=2)
			{
				FEElement_* pe = pm->ElementPtr(eid++);

				pe->m_node[0] = NodeIndex3(i  ,j  ,k);
				pe->m_node[1] = NodeIndex3(i+2,j  ,k);
				pe->m_node[2] = NodeIndex3(i+2,j+2,k);
				pe->m_node[3] = NodeIndex3(i  ,j+2,k);
				pe->m_node[4] = NodeIndex3(i  ,j  ,k+2);
				pe->m_node[5] = NodeIndex3(i+2,j  ,k+2);
				pe->m_node[6] = NodeIndex3(i+2,j+2,k+2);
				pe->m_node[7] = NodeIndex3(i  ,j+2,k+2);

				pe->m_node[ 8] = NodeIndex3(i+1,j  ,k);
				pe->m_node[ 9] = NodeIndex3(i+2,j+1,k);
				pe->m_node[10] = NodeIndex3(i+1,j+2,k);
				pe->m_node[11] = NodeIndex3(i  ,j+1,k);
				pe->m_node[12] = NodeIndex3(i+1,j  ,k+2);
				pe->m_node[13] = NodeIndex3(i+2,j+1,k+2);
				pe->m_node[14] = NodeIndex3(i+1,j+2,k+2);
				pe->m_node[15] = NodeIndex3(i  ,j+1,k+2);
				pe->m_node[16] = NodeIndex3(i  ,j  ,k+1);
				pe->m_node[17] = NodeIndex3(i+2,j  ,k+1);
				pe->m_node[18] = NodeIndex3(i+2,j+2,k+1);
				pe->m_node[19] = NodeIndex3(i  ,j+2,k+1);

				pe->m_node[20] = NodeIndex3(i+1,j  ,k+1);
				pe->m_node[21] = NodeIndex3(i+2,j+1,k+1);
				pe->m_node[22] = NodeIndex3(i+1,j+2,k+1);
				pe->m_node[23] = NodeIndex3(i  ,j+1,k+1);
				pe->m_node[24] = NodeIndex3(i+1,j+1,k  );
				pe->m_node[25] = NodeIndex3(i+1,j+1,k+2);
				pe->m_node[26] = NodeIndex3(i+1,j+1,k+1);

				pe->m_gid = 0;
				pe->SetType(FE_HEX27);
			}

	// build faces
	BuildHex27Faces(pm);

	// build edges
	BuildHex27Edges(pm);

	// update mesh
	pm->BuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// Build faces of a regular hex mesh
void FEBox::BuildHexFaces(FEMesh* pm)
{
	int i, j, k;

	// calculate the nr of faces
	int faces = 2*(m_nx*m_ny + m_ny*m_nz + m_nx*m_nz);
	pm->Create(0,0,faces);

	// build the faces
	FEFace* pf = pm->FacePtr();

	// -Y face
	for (i=0; i<m_nx; ++i)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 0;
			f.m_sid = 0;
			f.n[0] = NodeIndex(i  , 0, k  );
			f.n[1] = NodeIndex(i+1, 0, k  );
			f.n[2] = NodeIndex(i+1, 0, k+1);
			f.n[3] = NodeIndex(i  , 0, k+1);
		}

	// +X face
	for (j=0; j<m_ny; ++j)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 1;
			f.m_sid = 1;
			f.n[0] = NodeIndex(m_nx, j  , k  );
			f.n[1] = NodeIndex(m_nx, j+1, k  );
			f.n[2] = NodeIndex(m_nx, j+1, k+1);
			f.n[3] = NodeIndex(m_nx, j  , k+1);
		}

	// +Y face
	for (i=m_nx-1; i>=0; --i)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 2;
			f.m_sid = 2;
			f.n[0] = NodeIndex(i+1, m_ny, k  );
			f.n[1] = NodeIndex(i  , m_ny, k  );
			f.n[2] = NodeIndex(i  , m_ny, k+1);
			f.n[3] = NodeIndex(i+1, m_ny, k+1);
		}

	// -X face
	for (j=m_ny-1; j>=0; --j)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 3;
			f.m_sid = 3;
			f.n[0] = NodeIndex(0, j+1, k  );
			f.n[1] = NodeIndex(0, j  , k  );
			f.n[2] = NodeIndex(0, j  , k+1);
			f.n[3] = NodeIndex(0, j+1, k+1);
		}

	// -Z face
	for (i=0; i<m_nx; ++i)
		for (j=m_ny-1; j>=0; --j, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 4;
			f.m_sid = 4;
			f.n[0] = NodeIndex(i  , j  , 0);
			f.n[1] = NodeIndex(i  , j+1, 0);
			f.n[2] = NodeIndex(i+1, j+1, 0);
			f.n[3] = NodeIndex(i+1, j  , 0);
		}

	// +Z face
	for (i=0; i<m_nx; ++i)
		for (j=0; j<m_ny; ++j, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 5;
			f.m_sid = 5;
			f.n[0] = NodeIndex(i  , j  , m_nz);
			f.n[1] = NodeIndex(i+1, j  , m_nz);
			f.n[2] = NodeIndex(i+1, j+1, m_nz);
			f.n[3] = NodeIndex(i  , j+1, m_nz);
		}
}

//-----------------------------------------------------------------------------
// Build faces of a regular hex mesh
void FEBox::BuildHex20Faces(FEMesh* pm, vector<int>& LUT)
{
	int i, j, k;

	// calculate the nr of faces
	int faces = 2*(m_nx*m_ny + m_ny*m_nz + m_nx*m_nz);
	pm->Create(0,0,faces);

	// build the faces
	FEFace* pf = pm->FacePtr();

	// -Y face
	for (i=0; i<m_nx; ++i)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD8);
			f.m_gid = 0;
			f.m_sid = 0;
			f.n[0] = NodeIndex2(2*i  , 0, 2*k  , LUT);
			f.n[1] = NodeIndex2(2*i+2, 0, 2*k  , LUT);
			f.n[2] = NodeIndex2(2*i+2, 0, 2*k+2, LUT);
			f.n[3] = NodeIndex2(2*i  , 0, 2*k+2, LUT);

			f.n[4] = NodeIndex2(2*i+1  , 0, 2*k  , LUT);
			f.n[5] = NodeIndex2(2*i+2  , 0, 2*k+1, LUT);
			f.n[6] = NodeIndex2(2*i+1  , 0, 2*k+2, LUT);
			f.n[7] = NodeIndex2(2*i    , 0, 2*k+1, LUT);
		}

	// +X face
	for (j=0; j<m_ny; ++j)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD8);
			f.m_gid = 1;
			f.m_sid = 1;
			f.n[0] = NodeIndex2(2*m_nx, 2*j  , 2*k  , LUT);
			f.n[1] = NodeIndex2(2*m_nx, 2*j+2, 2*k  , LUT);
			f.n[2] = NodeIndex2(2*m_nx, 2*j+2, 2*k+2, LUT);
			f.n[3] = NodeIndex2(2*m_nx, 2*j  , 2*k+2, LUT);

			f.n[4] = NodeIndex2(2*m_nx, 2*j+1, 2*k  , LUT);
			f.n[5] = NodeIndex2(2*m_nx, 2*j+2, 2*k+1, LUT);
			f.n[6] = NodeIndex2(2*m_nx, 2*j+1, 2*k+2, LUT);
			f.n[7] = NodeIndex2(2*m_nx, 2*j  , 2*k+1, LUT);
		}

	// +Y face
	for (i=m_nx-1; i>=0; --i)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD8);
			f.m_gid = 2;
			f.m_sid = 2;
			f.n[0] = NodeIndex2(2*i+2, 2*m_ny, 2*k  , LUT);
			f.n[1] = NodeIndex2(2*i  , 2*m_ny, 2*k  , LUT);
			f.n[2] = NodeIndex2(2*i  , 2*m_ny, 2*k+2, LUT);
			f.n[3] = NodeIndex2(2*i+2, 2*m_ny, 2*k+2, LUT);

			f.n[4] = NodeIndex2(2*i+1, 2*m_ny, 2*k  , LUT);
			f.n[5] = NodeIndex2(2*i  , 2*m_ny, 2*k+1, LUT);
			f.n[6] = NodeIndex2(2*i+1, 2*m_ny, 2*k+2, LUT);
			f.n[7] = NodeIndex2(2*i+2, 2*m_ny, 2*k+1, LUT);
		}

	// -X face
	for (j=m_ny-1; j>=0; --j)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD8);
			f.m_gid = 3;
			f.m_sid = 3;
			f.n[0] = NodeIndex2(0, 2*j+2, 2*k  , LUT);
			f.n[1] = NodeIndex2(0, 2*j  , 2*k  , LUT);
			f.n[2] = NodeIndex2(0, 2*j  , 2*k+2, LUT);
			f.n[3] = NodeIndex2(0, 2*j+2, 2*k+2, LUT);

			f.n[4] = NodeIndex2(0, 2*j+1, 2*k  , LUT);
			f.n[5] = NodeIndex2(0, 2*j  , 2*k+1, LUT);
			f.n[6] = NodeIndex2(0, 2*j+1, 2*k+2, LUT);
			f.n[7] = NodeIndex2(0, 2*j+2, 2*k+1, LUT);
		}

	// -Z face
	for (i=0; i<m_nx; ++i)
		for (j=m_ny-1; j>=0; --j, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD8);
			f.m_gid = 4;
			f.m_sid = 4;
			f.n[0] = NodeIndex2(2*i  , 2*j  , 0, LUT);
			f.n[1] = NodeIndex2(2*i  , 2*j+2, 0, LUT);
			f.n[2] = NodeIndex2(2*i+2, 2*j+2, 0, LUT);
			f.n[3] = NodeIndex2(2*i+2, 2*j  , 0, LUT);

			f.n[4] = NodeIndex2(2*i  , 2*j+1, 0, LUT);
			f.n[5] = NodeIndex2(2*i+1, 2*j+2, 0, LUT);
			f.n[6] = NodeIndex2(2*i+2, 2*j+1, 0, LUT);
			f.n[7] = NodeIndex2(2*i+1, 2*j  , 0, LUT);
		}

	// +Z face
	for (i=0; i<m_nx; ++i)
		for (j=0; j<m_ny; ++j, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD8);
			f.m_gid = 5;
			f.m_sid = 5;
			f.n[0] = NodeIndex2(2*i  , 2*j  , 2*m_nz, LUT);
			f.n[1] = NodeIndex2(2*i+2, 2*j  , 2*m_nz, LUT);
			f.n[2] = NodeIndex2(2*i+2, 2*j+2, 2*m_nz, LUT);
			f.n[3] = NodeIndex2(2*i  , 2*j+2, 2*m_nz, LUT);

			f.n[4] = NodeIndex2(2*i+1, 2*j  , 2*m_nz, LUT);
			f.n[5] = NodeIndex2(2*i+2, 2*j+1, 2*m_nz, LUT);
			f.n[6] = NodeIndex2(2*i+1, 2*j+2, 2*m_nz, LUT);
			f.n[7] = NodeIndex2(2*i  , 2*j+1, 2*m_nz, LUT);
		}
}


//-----------------------------------------------------------------------------
// Build the faces of a regular tet mesh
void FEBox::BuildTetFaces(FEMesh* pm)
{
	int i, j, k;

	// calculate the nr of faces
	int faces = 4*(m_nx*m_ny + m_ny*m_nz + m_nx*m_nz);
	pm->Create(0,0,faces);

	// build the faces
	FEFace* pf = pm->FacePtr();

	// -Y face
	for (i=0; i<m_nx; ++i)
		for (k=0; k<m_nz; ++k)
		{
			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 0;
			pf->m_sid = 0;
			pf->n[0] = NodeIndex(i  , 0, k  );
			pf->n[1] = NodeIndex(i+1, 0, k  );
			pf->n[2] = NodeIndex(i+1, 0, k+1);
			pf->n[3] = pf->n[2];
			++pf;

			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 0;
			pf->m_sid = 0;
			pf->n[0] = NodeIndex(i+1, 0, k+1);
			pf->n[1] = NodeIndex(i  , 0, k+1);
			pf->n[2] = NodeIndex(i  , 0, k  );
			pf->n[3] = pf->n[2];
			++pf;
		}

	// +X face
	for (j=0; j<m_ny; ++j)
		for (k=0; k<m_nz; ++k)
		{
			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 1;
			pf->m_sid = 1;
			pf->n[0] = NodeIndex(m_nx, j  , k  );
			pf->n[1] = NodeIndex(m_nx, j+1, k  );
			pf->n[2] = NodeIndex(m_nx, j+1, k+1);
			pf->n[3] = pf->n[2];
			++pf;

			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 1;
			pf->m_sid = 1;
			pf->n[0] = NodeIndex(m_nx, j+1, k+1);
			pf->n[1] = NodeIndex(m_nx, j  , k+1);
			pf->n[2] = NodeIndex(m_nx, j  , k  );
			pf->n[3] = pf->n[2];
			++pf;
		}

	// +Y face
	for (i=m_nx-1; i>=0; --i)
		for (k=0; k<m_nz; ++k)
		{
			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 2;
			pf->m_sid = 2;
			pf->n[0] = NodeIndex(i+1, m_ny, k  );
			pf->n[1] = NodeIndex(i  , m_ny, k  );
			pf->n[2] = NodeIndex(i+1, m_ny, k+1);
			pf->n[3] = pf->n[2];
			++pf;

			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 2;
			pf->m_sid = 2;
			pf->n[0] = NodeIndex(i  , m_ny, k  );
			pf->n[1] = NodeIndex(i  , m_ny, k+1);
			pf->n[2] = NodeIndex(i+1, m_ny, k+1);
			pf->n[3] = pf->n[2];
			++pf;
		}

	// -X face
	for (j=m_ny-1; j>=0; --j)
		for (k=0; k<m_nz; ++k)
		{
			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 3;
			pf->m_sid = 3;
			pf->n[0] = NodeIndex(0, j+1, k  );
			pf->n[1] = NodeIndex(0, j  , k  );
			pf->n[2] = NodeIndex(0, j+1, k+1);
			pf->n[3] = pf->n[2];
			++pf;

			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 3;
			pf->m_sid = 3;
			pf->n[0] = NodeIndex(0, j  , k  );
			pf->n[1] = NodeIndex(0, j  , k+1);
			pf->n[2] = NodeIndex(0, j+1, k+1);
			pf->n[3] = pf->n[2];
			++pf;
		}

	// -Z face
	for (i=0; i<m_nx; ++i)
		for (j=m_ny-1; j>=0; --j)
		{
			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 4;
			pf->m_sid = 4;
			pf->n[0] = NodeIndex(i  , j  , 0);
			pf->n[1] = NodeIndex(i  , j+1, 0);
			pf->n[2] = NodeIndex(i+1, j  , 0);
			pf->n[3] = pf->n[2];
			++pf;

			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 4;
			pf->m_sid = 4;
			pf->n[0] = NodeIndex(i+1, j  , 0);
			pf->n[1] = NodeIndex(i  , j+1, 0);
			pf->n[2] = NodeIndex(i+1, j+1, 0);
			pf->n[3] = pf->n[2];
			++pf;
		}

	// +Z face
	for (i=0; i<m_nx; ++i)
		for (j=0; j<m_ny; ++j)
		{
			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 5;
			pf->m_sid = 5;
			pf->n[0] = NodeIndex(i  , j  , m_nz);
			pf->n[1] = NodeIndex(i+1, j  , m_nz);
			pf->n[2] = NodeIndex(i  , j+1, m_nz);
			pf->n[3] = pf->n[2];
			++pf;

			pf->SetType(FE_FACE_TRI3);
			pf->m_gid = 5;
			pf->m_sid = 5;
			pf->n[0] = NodeIndex(i+1, j  , m_nz);
			pf->n[1] = NodeIndex(i+1, j+1, m_nz);
			pf->n[2] = NodeIndex(i  , j+1, m_nz);
			pf->n[3] = pf->n[2];
			++pf;
		}
}

//-----------------------------------------------------------------------------
// Build the edges of a box mesh
void FEBox::BuildEdges(FEMesh* pm)
{
	int i;

	// calculate the nr of edges
	int edges = 4*(m_nx + m_ny + m_nz);
	pm->Create(0,0,0,edges);
	FEEdge* pe = pm->EdgePtr();

	for (i=   0; i<m_nx; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  0; pe->n[0] = NodeIndex(   i,    0, 0); pe->n[1] = NodeIndex(i+1 ,    0, 0); }
	for (i=   0; i<m_ny; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  1; pe->n[0] = NodeIndex(m_nx,    i, 0); pe->n[1] = NodeIndex(m_nx,  i+1, 0); }
	for (i=m_nx; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  2; pe->n[0] = NodeIndex(   i, m_ny, 0); pe->n[1] = NodeIndex(i-1 , m_ny, 0); }
	for (i=m_ny; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  3; pe->n[0] = NodeIndex(  0,     i, 0); pe->n[1] = NodeIndex(   0,  i-1, 0); }

	for (i=   0; i<m_nx; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  4; pe->n[0] = NodeIndex(   i,    0, m_nz); pe->n[1] = NodeIndex(i+1 ,    0, m_nz); }
	for (i=   0; i<m_ny; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  5; pe->n[0] = NodeIndex(m_nx,    i, m_nz); pe->n[1] = NodeIndex(m_nx,  i+1, m_nz); }
	for (i=m_nx; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  6; pe->n[0] = NodeIndex(   i, m_ny, m_nz); pe->n[1] = NodeIndex(i-1 , m_ny, m_nz); }
	for (i=m_ny; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  7; pe->n[0] = NodeIndex(   0,    i, m_nz); pe->n[1] = NodeIndex(   0,  i-1, m_nz); }

	for (i=0   ; i<m_nz; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  8; pe->n[0] = NodeIndex(   0,    0, i); pe->n[1] = NodeIndex(   0,    0,  i+1); }
	for (i=0   ; i<m_nz; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid =  9; pe->n[0] = NodeIndex(m_nx,    0, i); pe->n[1] = NodeIndex(m_nx,    0,  i+1); }
	for (i=0   ; i<m_nz; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid = 10; pe->n[0] = NodeIndex(m_nx, m_ny, i); pe->n[1] = NodeIndex(m_nx, m_ny,  i+1); }
	for (i=0   ; i<m_nz; ++i, ++pe)  { pe->SetType(FE_EDGE2); pe->m_gid = 11; pe->n[0] = NodeIndex(   0, m_ny, i); pe->n[1] = NodeIndex(   0, m_ny,  i+1); }
}

//-----------------------------------------------------------------------------
// Build the edges of a box mesh
void FEBox::BuildHex20Edges(FEMesh* pm, vector<int>& LUT)
{
	int i;

	int nx = m_nx;
	int ny = m_ny;
	int nz = m_nz;

	// calculate the nr of edges
	int edges = 4*(nx + ny + nz);
	pm->Create(0,0,0,edges);
	FEEdge* pe = pm->EdgePtr();

	for (i=   0; i<nx; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  0; pe->n[0] = NodeIndex2( 2*i,    0, 0, LUT); pe->n[1] = NodeIndex2(2*i+2,    0, 0, LUT); pe->n[2] = NodeIndex2(2*i+1,    0, 0, LUT); }
	for (i=   0; i<ny; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  1; pe->n[0] = NodeIndex2(2*nx,  2*i, 0, LUT); pe->n[1] = NodeIndex2(2*nx ,2*i+2, 0, LUT); pe->n[2] = NodeIndex2(2*nx , 2*i+1, 0, LUT);}
	for (i=nx; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  2; pe->n[0] = NodeIndex2( 2*i, 2*ny, 0, LUT); pe->n[1] = NodeIndex2(2*i-2,2*ny , 0, LUT); pe->n[2] = NodeIndex2(2*i-1, 2*ny , 0, LUT);}
	for (i=ny; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  3; pe->n[0] = NodeIndex2(   0,  2*i, 0, LUT); pe->n[1] = NodeIndex2(    0,2*i-2, 0, LUT); pe->n[2] = NodeIndex2(    0, 2*i-1, 0, LUT);}

	for (i=   0; i<nx; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  4; pe->n[0] = NodeIndex2( 2*i,    0, 2*nz, LUT); pe->n[1] = NodeIndex2(2*i+2,    0, 2*nz, LUT); pe->n[2] = NodeIndex2(2*i+1,    0, 2*nz, LUT);}
	for (i=   0; i<ny; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  5; pe->n[0] = NodeIndex2(2*nx,  2*i, 2*nz, LUT); pe->n[1] = NodeIndex2(2*nx ,2*i+2, 2*nz, LUT); pe->n[2] = NodeIndex2(2*nx ,2*i+1, 2*nz, LUT);}
	for (i=nx; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  6; pe->n[0] = NodeIndex2( 2*i, 2*ny, 2*nz, LUT); pe->n[1] = NodeIndex2(2*i-2,2* ny, 2*nz, LUT); pe->n[2] = NodeIndex2(2*i-1,2* ny, 2*nz, LUT);}
	for (i=ny; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  7; pe->n[0] = NodeIndex2(   0,  2*i, 2*nz, LUT); pe->n[1] = NodeIndex2(    0,2*i-2, 2*nz, LUT); pe->n[2] = NodeIndex2(    0,2*i-1, 2*nz, LUT);}

	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  8; pe->n[0] = NodeIndex2(   0,    0, 2*i, LUT); pe->n[1] = NodeIndex2(   0,    0, 2*i+2, LUT); pe->n[2] = NodeIndex2(   0,    0, 2*i+1, LUT);}
	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  9; pe->n[0] = NodeIndex2(2*nx,    0, 2*i, LUT); pe->n[1] = NodeIndex2(2*nx,    0, 2*i+2, LUT); pe->n[2] = NodeIndex2(2*nx,    0, 2*i+1, LUT);}
	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid = 10; pe->n[0] = NodeIndex2(2*nx, 2*ny, 2*i, LUT); pe->n[1] = NodeIndex2(2*nx, 2*ny, 2*i+2, LUT); pe->n[2] = NodeIndex2(2*nx, 2*ny, 2*i+1, LUT);}
	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid = 11; pe->n[0] = NodeIndex2(   0, 2*ny, 2*i, LUT); pe->n[1] = NodeIndex2(   0, 2*ny, 2*i+2, LUT); pe->n[2] = NodeIndex2(   0, 2*ny, 2*i+1, LUT);}
}

//-----------------------------------------------------------------------------
// Build faces of a regular hex mesh
void FEBox::BuildHex27Faces(FEMesh* pm)
{
	int i, j, k;

	// calculate the nr of faces
	int faces = 2*(m_nx*m_ny + m_ny*m_nz + m_nx*m_nz);
	pm->Create(0,0,faces);

	// build the faces
	FEFace* pf = pm->FacePtr();

	// -Y face
	for (i=0; i<m_nx; ++i)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD9);
			f.m_gid = 0;
			f.m_sid = 0;
			f.n[0] = NodeIndex3(2*i  , 0, 2*k  );
			f.n[1] = NodeIndex3(2*i+2, 0, 2*k  );
			f.n[2] = NodeIndex3(2*i+2, 0, 2*k+2);
			f.n[3] = NodeIndex3(2*i  , 0, 2*k+2);

			f.n[4] = NodeIndex3(2*i+1  , 0, 2*k  );
			f.n[5] = NodeIndex3(2*i+2  , 0, 2*k+1);
			f.n[6] = NodeIndex3(2*i+1  , 0, 2*k+2);
			f.n[7] = NodeIndex3(2*i    , 0, 2*k+1);

			f.n[8] = NodeIndex3(2*i+1, 0, 2*k+1);
		}

	// +X face
	for (j=0; j<m_ny; ++j)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD9);
			f.m_gid = 1;
			f.m_sid = 1;
			f.n[0] = NodeIndex3(2*m_nx, 2*j  , 2*k  );
			f.n[1] = NodeIndex3(2*m_nx, 2*j+2, 2*k  );
			f.n[2] = NodeIndex3(2*m_nx, 2*j+2, 2*k+2);
			f.n[3] = NodeIndex3(2*m_nx, 2*j  , 2*k+2);

			f.n[4] = NodeIndex3(2*m_nx, 2*j+1, 2*k  );
			f.n[5] = NodeIndex3(2*m_nx, 2*j+2, 2*k+1);
			f.n[6] = NodeIndex3(2*m_nx, 2*j+1, 2*k+2);
			f.n[7] = NodeIndex3(2*m_nx, 2*j  , 2*k+1);

			f.n[8] = NodeIndex3(2*m_nx, 2*j+1, 2*k+1);
		}

	// +Y face
	for (i=m_nx-1; i>=0; --i)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD9);
			f.m_gid = 2;
			f.m_sid = 2;
			f.n[0] = NodeIndex3(2*i+2, 2*m_ny, 2*k  );
			f.n[1] = NodeIndex3(2*i  , 2*m_ny, 2*k  );
			f.n[2] = NodeIndex3(2*i  , 2*m_ny, 2*k+2);
			f.n[3] = NodeIndex3(2*i+2, 2*m_ny, 2*k+2);

			f.n[4] = NodeIndex3(2*i+1, 2*m_ny, 2*k  );
			f.n[5] = NodeIndex3(2*i  , 2*m_ny, 2*k+1);
			f.n[6] = NodeIndex3(2*i+1, 2*m_ny, 2*k+2);
			f.n[7] = NodeIndex3(2*i+2, 2*m_ny, 2*k+1);

			f.n[8] = NodeIndex3(2*i+1, 2*m_ny, 2*k+1);
		}

	// -X face
	for (j=m_ny-1; j>=0; --j)
		for (k=0; k<m_nz; ++k, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD9);
			f.m_gid = 3;
			f.m_sid = 3;
			f.n[0] = NodeIndex3(0, 2*j+2, 2*k  );
			f.n[1] = NodeIndex3(0, 2*j  , 2*k  );
			f.n[2] = NodeIndex3(0, 2*j  , 2*k+2);
			f.n[3] = NodeIndex3(0, 2*j+2, 2*k+2);

			f.n[4] = NodeIndex3(0, 2*j+1, 2*k  );
			f.n[5] = NodeIndex3(0, 2*j  , 2*k+1);
			f.n[6] = NodeIndex3(0, 2*j+1, 2*k+2);
			f.n[7] = NodeIndex3(0, 2*j+2, 2*k+1);

			f.n[8] = NodeIndex3(0, 2*j+1, 2*k+1);
		}

	// -Z face
	for (i=0; i<m_nx; ++i)
		for (j=m_ny-1; j>=0; --j, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD9);
			f.m_gid = 4;
			f.m_sid = 4;
			f.n[0] = NodeIndex3(2*i  , 2*j  , 0);
			f.n[1] = NodeIndex3(2*i  , 2*j+2, 0);
			f.n[2] = NodeIndex3(2*i+2, 2*j+2, 0);
			f.n[3] = NodeIndex3(2*i+2, 2*j  , 0);

			f.n[4] = NodeIndex3(2*i  , 2*j+1, 0);
			f.n[5] = NodeIndex3(2*i+1, 2*j+2, 0);
			f.n[6] = NodeIndex3(2*i+2, 2*j+1, 0);
			f.n[7] = NodeIndex3(2*i+1, 2*j  , 0);

			f.n[8] = NodeIndex3(2*i+1, 2*j+1, 0);
		}

	// +Z face
	for (i=0; i<m_nx; ++i)
		for (j=0; j<m_ny; ++j, ++pf)
		{
			FEFace& f = *pf;
			f.SetType(FE_FACE_QUAD9);
			f.m_gid = 5;
			f.m_sid = 5;
			f.n[0] = NodeIndex3(2*i  , 2*j  , 2*m_nz);
			f.n[1] = NodeIndex3(2*i+2, 2*j  , 2*m_nz);
			f.n[2] = NodeIndex3(2*i+2, 2*j+2, 2*m_nz);
			f.n[3] = NodeIndex3(2*i  , 2*j+2, 2*m_nz);

			f.n[4] = NodeIndex3(2*i+1, 2*j  , 2*m_nz);
			f.n[5] = NodeIndex3(2*i+2, 2*j+1, 2*m_nz);
			f.n[6] = NodeIndex3(2*i+1, 2*j+2, 2*m_nz);
			f.n[7] = NodeIndex3(2*i  , 2*j+1, 2*m_nz);

			f.n[8] = NodeIndex3(2*i+1, 2*j+1, 2*m_nz);
		}
}

//-----------------------------------------------------------------------------
// Build the edges of a box mesh
void FEBox::BuildHex27Edges(FEMesh* pm)
{
	int i;

	int nx = m_nx;
	int ny = m_ny;
	int nz = m_nz;

	// calculate the nr of edges
	int edges = 4*(nx + ny + nz);
	pm->Create(0,0,0,edges);
	FEEdge* pe = pm->EdgePtr();

	for (i=   0; i<nx; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  0; pe->n[0] = NodeIndex3( 2*i,    0, 0); pe->n[1] = NodeIndex3(2*i+2,    0, 0); pe->n[2] = NodeIndex3(2*i+1,     0, 0); }
	for (i=   0; i<ny; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  1; pe->n[0] = NodeIndex3(2*nx,  2*i, 0); pe->n[1] = NodeIndex3(2*nx ,2*i+2, 0); pe->n[2] = NodeIndex3(2*nx , 2*i+1, 0);}
	for (i=nx; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  2; pe->n[0] = NodeIndex3( 2*i, 2*ny, 0); pe->n[1] = NodeIndex3(2*i-2,2*ny , 0); pe->n[2] = NodeIndex3(2*i-1, 2*ny , 0);}
	for (i=ny; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  3; pe->n[0] = NodeIndex3(   0,  2*i, 0); pe->n[1] = NodeIndex3(    0,2*i-2, 0); pe->n[2] = NodeIndex3(    0, 2*i-1, 0);}

	for (i=   0; i<nx; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  4; pe->n[0] = NodeIndex3( 2*i,    0, 2*nz); pe->n[1] = NodeIndex3(2*i+2,    0, 2*nz); pe->n[2] = NodeIndex3(2*i+1,    0, 2*nz);}
	for (i=   0; i<ny; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  5; pe->n[0] = NodeIndex3(2*nx,  2*i, 2*nz); pe->n[1] = NodeIndex3(2*nx ,2*i+2, 2*nz); pe->n[2] = NodeIndex3(2*nx ,2*i+1, 2*nz);}
	for (i=nx; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  6; pe->n[0] = NodeIndex3( 2*i, 2*ny, 2*nz); pe->n[1] = NodeIndex3(2*i-2,2* ny, 2*nz); pe->n[2] = NodeIndex3(2*i-1,2* ny, 2*nz);}
	for (i=ny; i>=  1; --i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  7; pe->n[0] = NodeIndex3(   0,  2*i, 2*nz); pe->n[1] = NodeIndex3(    0,2*i-2, 2*nz); pe->n[2] = NodeIndex3(    0,2*i-1, 2*nz);}

	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  8; pe->n[0] = NodeIndex3(   0,    0, 2*i); pe->n[1] = NodeIndex3(   0,    0, 2*i+2); pe->n[2] = NodeIndex3(   0,    0, 2*i+1);}
	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid =  9; pe->n[0] = NodeIndex3(2*nx,    0, 2*i); pe->n[1] = NodeIndex3(2*nx,    0, 2*i+2); pe->n[2] = NodeIndex3(2*nx,    0, 2*i+1);}
	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid = 10; pe->n[0] = NodeIndex3(2*nx, 2*ny, 2*i); pe->n[1] = NodeIndex3(2*nx, 2*ny, 2*i+2); pe->n[2] = NodeIndex3(2*nx, 2*ny, 2*i+1);}
	for (i=0   ; i<nz; ++i, ++pe)  { pe->SetType(FE_EDGE3); pe->m_gid = 11; pe->n[0] = NodeIndex3(   0, 2*ny, 2*i); pe->n[1] = NodeIndex3(   0, 2*ny, 2*i+2); pe->n[2] = NodeIndex3(   0, 2*ny, 2*i+1);}
}

FEMesh* FEBox::CreateRegularTET10()
{
	FEMesh* tet4 = CreateRegularTET4();
	FETet4ToTet10 mod;
	FEMesh* tet10 = mod.Apply(tet4);
	delete tet4;
	return tet10;
}

FEMesh* FEBox::CreateRegularTET15()
{
	FEMesh* tet4 = CreateRegularTET4();
	FETet4ToTet15 mod;
	FEMesh* tet15 = mod.Apply(tet4);
	delete tet4;
	return tet15;
}

FEMesh* FEBox::CreateRegularTET20()
{
	FEMesh* tet4 = CreateRegularTET4();
	FETet4ToTet20 mod;
	FEMesh* tet20 = mod.Apply(tet4);
	delete tet4;
	return tet20;
}
