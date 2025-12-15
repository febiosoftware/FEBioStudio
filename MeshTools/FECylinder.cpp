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

// FECylinder.cpp: implementation of the FECylinder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FECylinder.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>

extern double gain2(double x, double r, double n);

//-----------------------------------------------------------------------------
// Constructor
FECylinder::FECylinder(GObject& o) : FEMultiBlockMesh(o)
{
	m_r = 0.5;
	m_nd = m_ns = 4;
	m_nz = 8;
	m_gz = 1;
	m_gr = 1;
	m_ctype = 0;
	m_bz = false;
	m_br = false;

	AddDoubleParam(m_r, "r", "Ratio");
	AddIntParam(m_nd, "nd", "Slices");
	AddIntParam(m_ns, "ns", "Segments");
	AddIntParam(m_nz, "nz", "Stacks");

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddIntParam(m_ctype, "ctype", "Mesh Type")->SetEnumNames("Butterfly center\0Wedge center\0");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddChoiceParam(0, "elem_type", "Element Type")->SetEnumNames("HEX8\0HEX20\0HEX27\0");
}

//-----------------------------------------------------------------------------
// Build the mesh
FSMesh* FECylinder::BuildMesh()
{
	m_ctype = GetIntValue(CTYPE);
	switch (m_ctype)
	{
	case BUTTERFLY: return BuildButterfly();
	case WEDGED   : return BuildWedged();
	default:
		assert(false);
	}

	return 0;
}

//-----------------------------------------------------------------------------
bool FECylinder::BuildMultiBlock()
{
	GCylinder* po = dynamic_cast<GCylinder*>(&m_o);
	if (po == nullptr) return false;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double h = param.GetFloatValue(GCylinder::HEIGHT);

	// get the mesh parameters
	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(GR);
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);

	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// get parameters
	int nd = m_nd;
	int ns = m_ns;
	int nz = m_nz;
	double fz = m_gz;
	double fr = m_gr;

	// check parameters
	if (nd < 1) nd = 1;
	if (ns < 1) ns = 1;
	if (nz < 1) nz = 1;
	if (nz == 1) m_bz = false;

	double R1 = param.GetFloatValue(GCylinder::RADIUS);
	m_r = GetFloatValue(RATIO);

	double b = R1 * sqrt(2.0) / 2.0;
	double a = m_r * b;
	double c = R1;

	ClearMB();

	// create the MB nodes
	m_MBNode.resize(34);
	m_MBNode[0].m_r = vec3d(-a, -a, 0);
	m_MBNode[1].m_r = vec3d(0, -a, 0);
	m_MBNode[2].m_r = vec3d(a, -a, 0);
	m_MBNode[3].m_r = vec3d(-a, 0, 0);
	m_MBNode[4].m_r = vec3d(0, 0, 0);
	m_MBNode[5].m_r = vec3d(a, 0, 0);
	m_MBNode[6].m_r = vec3d(-a, a, 0);
	m_MBNode[7].m_r = vec3d(0, a, 0);
	m_MBNode[8].m_r = vec3d(a, a, 0);

	m_MBNode[9].m_r = vec3d(-a, -a, h);
	m_MBNode[10].m_r = vec3d(0, -a, h);
	m_MBNode[11].m_r = vec3d(a, -a, h);
	m_MBNode[12].m_r = vec3d(-a, 0, h);
	m_MBNode[13].m_r = vec3d(0, 0, h);
	m_MBNode[14].m_r = vec3d(a, 0, h);
	m_MBNode[15].m_r = vec3d(-a, a, h);
	m_MBNode[16].m_r = vec3d(0, a, h);
	m_MBNode[17].m_r = vec3d(a, a, h);

	m_MBNode[18].m_r = vec3d(-b, -b, 0);
	m_MBNode[19].m_r = vec3d(0, -c, 0);
	m_MBNode[20].m_r = vec3d(b, -b, 0);
	m_MBNode[21].m_r = vec3d(c, 0, 0);
	m_MBNode[22].m_r = vec3d(b, b, 0);
	m_MBNode[23].m_r = vec3d(0, c, 0);
	m_MBNode[24].m_r = vec3d(-b, b, 0);
	m_MBNode[25].m_r = vec3d(-c, 0, 0);

	m_MBNode[26].m_r = vec3d(-b, -b, h);
	m_MBNode[27].m_r = vec3d(0, -c, h);
	m_MBNode[28].m_r = vec3d(b, -b, h);
	m_MBNode[29].m_r = vec3d(c, 0, h);
	m_MBNode[30].m_r = vec3d(b, b, h);
	m_MBNode[31].m_r = vec3d(0, c, h);
	m_MBNode[32].m_r = vec3d(-b, b, h);
	m_MBNode[33].m_r = vec3d(-c, 0, h);

	// create the MB blocks
	m_MBlock.resize(12);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0, 1, 4, 3, 9, 10, 13, 12);
	b1.SetSizes(nd, nd, nz);
	b1.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1, 2, 5, 4, 10, 11, 14, 13);
	b2.SetSizes(nd, nd, nz);
	b2.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(3, 4, 7, 6, 12, 13, 16, 15);
	b3.SetSizes(nd, nd, nz);
	b3.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(4, 5, 8, 7, 13, 14, 17, 16);
	b4.SetSizes(nd, nd, nz);
	b4.SetZoning(1, 1, fz, false, false, m_bz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(0, 18, 19, 1, 9, 26, 27, 10);
	b5.SetSizes(ns, nd, nz);
	b5.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(1, 19, 20, 2, 10, 27, 28, 11);
	b6.SetSizes(ns, nd, nz);
	b6.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b7 = m_MBlock[6];
	b7.SetID(0);
	b7.SetNodes(2, 20, 21, 5, 11, 28, 29, 14);
	b7.SetSizes(ns, nd, nz);
	b7.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b8 = m_MBlock[7];
	b8.SetID(0);
	b8.SetNodes(5, 21, 22, 8, 14, 29, 30, 17);
	b8.SetSizes(ns, nd, nz);
	b8.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b9 = m_MBlock[8];
	b9.SetID(0);
	b9.SetNodes(8, 22, 23, 7, 17, 30, 31, 16);
	b9.SetSizes(ns, nd, nz);
	b9.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b10 = m_MBlock[9];
	b10.SetID(0);
	b10.SetNodes(7, 23, 24, 6, 16, 31, 32, 15);
	b10.SetSizes(ns, nd, nz);
	b10.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b11 = m_MBlock[10];
	b11.SetID(0);
	b11.SetNodes(6, 24, 25, 3, 15, 32, 33, 12);
	b11.SetSizes(ns, nd, nz);
	b11.SetZoning(fr, 1, fz, false, false, m_bz);

	MBBlock& b12 = m_MBlock[11];
	b12.SetID(0);
	b12.SetNodes(3, 25, 18, 0, 12, 33, 26, 9);
	b12.SetSizes(ns, nd, nz);
	b12.SetZoning(fr, 1, fz, false, false, m_bz);

	// update the MB data
	BuildMB();

	// assign face ID's
/*
	// order before new CAD engine
	SetBlockFaceID(b1, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b2, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b3, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b4, -1, -1, -1, -1, 4, 5);
	SetBlockFaceID(b5, -1,  2, -1, -1, 4, 5);
	SetBlockFaceID(b6, -1,  3, -1, -1, 4, 5);
	SetBlockFaceID(b7, -1,  3, -1, -1, 4, 5);
	SetBlockFaceID(b8, -1,  0, -1, -1, 4, 5);
	SetBlockFaceID(b9, -1,  0, -1, -1, 4, 5);
	SetBlockFaceID(b10, -1,  1, -1, -1, 4, 5);
	SetBlockFaceID(b11, -1,  1, -1, -1, 4, 5);
	SetBlockFaceID(b12, -1,  2, -1, -1, 4, 5);
*/
	SetBlockFaceID(b1, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b2, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b3, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b4, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b5, -1, 3, -1, -1, 0, 5);
	SetBlockFaceID(b6, -1, 4, -1, -1, 0, 5);
	SetBlockFaceID(b7, -1, 4, -1, -1, 0, 5);
	SetBlockFaceID(b8, -1, 1, -1, -1, 0, 5);
	SetBlockFaceID(b9, -1, 1, -1, -1, 0, 5);
	SetBlockFaceID(b10, -1, 2, -1, -1, 0, 5);
	SetBlockFaceID(b11, -1, 2, -1, -1, 0, 5);
	SetBlockFaceID(b12, -1, 3, -1, -1, 0, 5);

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

//-----------------------------------------------------------------------------
// Build a butterfly mesh
FSMesh* FECylinder::BuildButterfly()
{
	// create the multiblock data
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

	return pm;
}

//-----------------------------------------------------------------------------
// Build a wedged mesh
FSMesh* FECylinder::BuildWedged()
{
	GCylinder* po = dynamic_cast<GCylinder*>(&m_o);
	if (po == nullptr) return nullptr;

	int i, j, k;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double R1 = param.GetFloatValue(GCylinder::RADIUS);
	double h = param.GetFloatValue(GCylinder::HEIGHT);

	// get the mesh parameters
	m_r  = GetFloatValue(RATIO);
	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(GR);
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);
	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_ns == 1) { m_gr = 1; m_br = false; }

	// make sure the divisions is a multiple of 4
	// so that we can always make four sides
	m_nd *= 4;

	// calculate storage
	int nodes = (m_nz+1)*(1 + m_nd*m_ns);
	int elems = m_nz*m_ns*m_nd;
	int faces = 2*m_ns*m_nd + m_nz*m_nd;
	int edges = 2*m_nd + 4*m_nz;

	// create mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems, faces, edges);

	// --- A. Create the nodes ---
	FSNode* pn = pm->NodePtr();
	double x, y, z, R;

	double gz = 1;
	double gr = 1;

	double fz = m_gz;
	double fr = m_gr;

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = h / gz;
	}
	else 
	{
		for (i=0; i<m_nz-1; ++i) gz = fz*gz+1; 
		gz = h / gz;
	}

	if (m_br)
	{
		gr = 2; if (m_ns%2) gr += fr;
		for (i=0; i<m_ns/2-1; ++i) gr = fr*gr+2;
		gr = R1 / gr;
	}
	else 
	{
		for (i=0; i<m_ns-1; ++i) gr = fr*gr+1; 
		gr = R1 / gr;
	}

	double dz = gz;
	z = 0;
	for (i=0; i<=m_nz; ++i)
	{
		double dr = gr;
		R = 0;

		// create the center node
		pn->r = vec3d(0, 0, z);
		pn->m_gid = -1;
		++pn;

		R += dr;
		dr *= fr;
		if (m_br && (m_ns == 2))
		{
			dr /= fr;
			fr = 1.0/fr;
		}

		// create the other nodes
		for (j=0; j<m_ns; ++j)
		{
			for (k=0; k<m_nd; ++k)
			{
				x = R*cos(k*2.0*PI / (m_nd));
				y = R*sin(k*2.0*PI / (m_nd));

				pn->r = vec3d(x, y, z);
				pn->m_gid = -1;
				pn++;
			}

			R += dr;
			dr *= fr;
			if (m_br && (j+1 == m_ns/2-1))
			{
				if (m_ns%2 == 0) dr /= fr;
				fr = 1.0/fr;
			}
		}
		if (m_br) fr = 1.0/fr;

		z += dz;
		dz *= fz;
		if (m_bz && (i == m_nz/2-1))
		{
			if (m_nz%2 == 0) dz /= fz;
			fz = 1.0/fz;
		}
	}

	pm->Node(NodeIndex(0, m_ns, 0       )).m_gid = 0;
	pm->Node(NodeIndex(0, m_ns, m_nd/4  )).m_gid = 1;
	pm->Node(NodeIndex(0, m_ns, m_nd/2  )).m_gid = 2;
	pm->Node(NodeIndex(0, m_ns, 3*m_nd/4)).m_gid = 3;
	pm->Node(NodeIndex(m_nz, m_ns, 0       )).m_gid = 4;
	pm->Node(NodeIndex(m_nz, m_ns, m_nd/4  )).m_gid = 5;
	pm->Node(NodeIndex(m_nz, m_ns, m_nd/2  )).m_gid = 6;
	pm->Node(NodeIndex(m_nz, m_ns, 3*m_nd/4)).m_gid = 7;

	// --- B. Create the elements ---

	// create the inner wedge elements
	int eid = 0;
	int nlevel = 1+m_nd*m_ns;
	for (i=0; i<m_nz; i++)
	{
		// wedge elements
		for (k=0; k<m_nd; ++k)
		{
			FSElement_* ph = pm->ElementPtr(eid++);

			ph->SetType(FE_PENTA6);
			ph->m_gid = 0;
			ph->m_node[0] = i*nlevel;
			ph->m_node[1] = i*nlevel + 1 + k;
			ph->m_node[2] = i*nlevel + 1 + (k+1)%m_nd;

			ph->m_node[3] = (i+1)*nlevel;
			ph->m_node[4] = (i+1)*nlevel + 1 + k;
			ph->m_node[5] = (i+1)*nlevel + 1 + (k+1)%m_nd;
		}

		// hex elements
		for (j=1; j<m_ns; ++j)
		{
			for (k=0; k<m_nd; ++k)
			{
				FSElement_* ph = pm->ElementPtr(eid++);

				ph->SetType(FE_HEX8);
				ph->m_gid = 0;

				ph->m_node[0] = i*nlevel + 1 + (j-1)*m_nd + k;
				ph->m_node[1] = i*nlevel + 1 + (j  )*m_nd + k;
				ph->m_node[2] = i*nlevel + 1 + (j  )*m_nd + (k+1)%m_nd;
				ph->m_node[3] = i*nlevel + 1 + (j-1)*m_nd + (k+1)%m_nd;

				ph->m_node[4] = (i+1)*nlevel + 1 + (j-1)*m_nd + k;
				ph->m_node[5] = (i+1)*nlevel + 1 + (j  )*m_nd + k;
				ph->m_node[6] = (i+1)*nlevel + 1 + (j  )*m_nd + (k+1)%m_nd;
				ph->m_node[7] = (i+1)*nlevel + 1 + (j-1)*m_nd + (k+1)%m_nd;
			}
		}
	}

	// --- C. Create faces ---
	// side faces
	FSFace* pf = pm->FacePtr();
	for (i=0; i<m_nz; ++i)
	{
		for (k=0; k<m_nd; ++k, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
//			pf->m_gid = k/(m_nd/4);
			pf->m_gid = k/(m_nd/4) + 1;
			pf->n[0] = NodeIndex(i  , m_ns, k  );
			pf->n[1] = NodeIndex(i  , m_ns, k+1);
			pf->n[2] = NodeIndex(i+1, m_ns, k+1);
			pf->n[3] = NodeIndex(i+1, m_ns, k  );
		}
	}

	// bottom faces
	for (k=0; k<m_nd; ++k)
	{
		pf->SetType(FE_FACE_TRI3);
//		pf->m_gid = 4;
		pf->m_gid = 0;
		pf->n[0] = NodeIndex(0, 0, 0);
		pf->n[1] = NodeIndex(0, 1, k+1);
		pf->n[2] = NodeIndex(0, 1, k);
		pf->n[3] = pf->n[2];
		++pf;

		for (j=1; j<m_ns; ++j, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 0;
//			pf->m_gid = 4;
			pf->n[0] = NodeIndex(0,j  ,k+1);
			pf->n[1] = NodeIndex(0,j+1,k+1);
			pf->n[2] = NodeIndex(0,j+1,k  );
			pf->n[3] = NodeIndex(0,j  ,k  );
		}
	}

	// top faces
	for (k=0; k<m_nd; ++k)
	{
		pf->SetType(FE_FACE_TRI3);
		pf->m_gid = 5;
		pf->n[0] = NodeIndex(m_nz, 0, 0);
		pf->n[1] = NodeIndex(m_nz, 1, k);
		pf->n[2] = NodeIndex(m_nz, 1, k+1);
		pf->n[3] = pf->n[2];
		++pf;

		for (j=1; j<m_ns; ++j, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 5;
			pf->n[0] = NodeIndex(m_nz,j  ,k  );
			pf->n[1] = NodeIndex(m_nz,j+1,k  );
			pf->n[2] = NodeIndex(m_nz,j+1,k+1);
			pf->n[3] = NodeIndex(m_nz,j  ,k+1);
		}
	}

	// --- D. Create edges ---
	FSEdge* pe = pm->EdgePtr();
	for (k=0; k<m_nd; ++k, ++pe)
	{
		pe->m_gid = k/(m_nd/4);
		pe->SetType(FE_EDGE2);
		pe->n[0] = NodeIndex(0, m_ns, k);
		pe->n[1] = NodeIndex(0, m_ns, k+1);
	}

	for (k=0; k<m_nd; ++k, ++pe)
	{
		pe->m_gid = 4+k/(m_nd/4);
		pe->SetType(FE_EDGE2);
		pe->n[0] = NodeIndex(m_nz, m_ns, k);
		pe->n[1] = NodeIndex(m_nz, m_ns, k+1);
	}

	for (k=0; k<4; ++k)
	{
		for (i=0; i<m_nz; ++i, ++pe)
		{
			pe->m_gid = 8 + k;
			pe->SetType(FE_EDGE2);
			pe->n[0] = NodeIndex(i, m_ns, k*m_nd / 4);
			pe->n[1] = NodeIndex(i+1, m_ns, k*m_nd/4);
		}
	}

	pm->BuildMesh();

	return pm;
}

//=============================================================================
// C Y L I N D E R 2
//=============================================================================
// Constructor
FECylinder2::FECylinder2(GObject& o) : FEMultiBlockMesh(o)
{
	m_r = 0.5;
	m_nd = m_ns = 4;
	m_nz = 8;
	m_gz = 1;
	m_gr = 1;
	m_ctype = 0;
	m_bz = false;
	m_br = false;

	AddDoubleParam(m_r, "r", "Ratio");
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments");
	AddIntParam(m_nz, "nz", "Stacks");

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddIntParam(m_ctype, "ctype", "Mesh Type")->SetEnumNames("Butterfly center\0Wedge center\0");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");
}

// Build the mesh
FSMesh* FECylinder2::BuildMesh()
{
	m_ctype = GetIntValue(CTYPE);
	switch (m_ctype)
	{
	case BUTTERFLY: return BuildButterfly();
	case WEDGED   : return BuildWedged();
	default:
		assert(false);
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Build a butterfly mesh
FSMesh* FECylinder2::BuildButterfly()
{
	GCylinder2* po = dynamic_cast<GCylinder2*>(&m_o);
	if (po == nullptr) return nullptr;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double Rx = param.GetFloatValue(GCylinder2::RADIUSX);
	double Ry = param.GetFloatValue(GCylinder2::RADIUSY);
	double h  = param.GetFloatValue(GCylinder2::HEIGHT );

	// get the mesh parameters
	m_r = GetFloatValue(RATIO);
	m_gz = GetFloatValue(ZZ);
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);

	m_bz = GetBoolValue(GZ2);

	// get parameters
	double R0x = m_r;
	double R0y = m_r;
	int nd = m_nd;
	int ns = m_ns;
	int nz = m_nz;
	double fz = m_gz;
	double fr = 1;

	// check parameters
	if (nd < 1) nd = 1;
	if (ns < 1) ns = 1;
	if (nz < 1) nz = 1;

	if (R0x < 0) R0x = 0; if (R0x > 1) R0x = 1;
	if (R0y < 0) R0y = 0; if (R0y > 1) R0y = 1;

	if (nz == 1) m_bz = false;

	R0x *= Rx;
	R0y *= Ry;

	double d0x = R0x/sqrt(2.0);
	double d1x = Rx /sqrt(2.0);
	double d0y = R0y/sqrt(2.0);
	double d1y = Ry /sqrt(2.0);

	// create the MB nodes
	m_MBNode.resize(34);
	m_MBNode[ 0].m_r = vec3d( -1, -1, 0);
	m_MBNode[ 1].m_r = vec3d(  0, -1, 0);
	m_MBNode[ 2].m_r = vec3d(  1, -1, 0);
	m_MBNode[ 3].m_r = vec3d( -1,  0, 0);
	m_MBNode[ 4].m_r = vec3d(  0,  0, 0);
	m_MBNode[ 5].m_r = vec3d(  1,  0, 0);
	m_MBNode[ 6].m_r = vec3d( -1,  1, 0);
	m_MBNode[ 7].m_r = vec3d(  0,  1, 0);
	m_MBNode[ 8].m_r = vec3d(  1,  1, 0);

	m_MBNode[ 9].m_r = vec3d(-1, -1, h);
	m_MBNode[10].m_r = vec3d( 0, -1, h);
	m_MBNode[11].m_r = vec3d( 1, -1, h);
	m_MBNode[12].m_r = vec3d(-1,  0, h);
	m_MBNode[13].m_r = vec3d( 0,  0, h);
	m_MBNode[14].m_r = vec3d( 1,  0, h);
	m_MBNode[15].m_r = vec3d(-1,  1, h);
	m_MBNode[16].m_r = vec3d( 0,  1, h);
	m_MBNode[17].m_r = vec3d( 1,  1, h);

	m_MBNode[18].m_r = vec3d(-2, -2, 0);
	m_MBNode[19].m_r = vec3d( 0, -2, 0);
	m_MBNode[20].m_r = vec3d( 2, -2, 0);
	m_MBNode[21].m_r = vec3d( 2,  0, 0);
	m_MBNode[22].m_r = vec3d( 2,  2, 0);
	m_MBNode[23].m_r = vec3d( 0,  2, 0);
	m_MBNode[24].m_r = vec3d(-2,  2, 0);
	m_MBNode[25].m_r = vec3d(-2,  0, 0);

	m_MBNode[26].m_r = vec3d(-2, -2, h);
	m_MBNode[27].m_r = vec3d( 0, -2, h);
	m_MBNode[28].m_r = vec3d( 2, -2, h);
	m_MBNode[29].m_r = vec3d( 2,  0, h);
	m_MBNode[30].m_r = vec3d( 2,  2, h);
	m_MBNode[31].m_r = vec3d( 0,  2, h);
	m_MBNode[32].m_r = vec3d(-2,  2, h);
	m_MBNode[33].m_r = vec3d(-2,  0, h);

	// create the MB blocks
	m_MBlock.resize(12);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0,1,4,3,9,10,13,12);
	b1.SetSizes(nd,nd,nz);
	b1.SetZoning(1,1,fz, false, false, m_bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1,2,5,4,10,11,14,13);
	b2.SetSizes(nd,nd,nz);
	b2.SetZoning(1,1,fz, false, false, m_bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(3,4,7,6,12,13,16,15);
	b3.SetSizes(nd,nd,nz);
	b3.SetZoning(1,1,fz, false, false, m_bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(4,5,8,7,13,14,17,16);
	b4.SetSizes(nd,nd,nz);
	b4.SetZoning(1,1,fz, false, false, m_bz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(0);
	b5.SetNodes(0,18,19,1,9,26,27,10);
	b5.SetSizes(ns,nd,nz);
	b5.SetZoning(fr,1,fz, false, false, m_bz);

	MBBlock& b6 = m_MBlock[5];
	b6.SetID(0);
	b6.SetNodes(1,19,20,2,10,27,28,11);
	b6.SetSizes(ns,nd,nz);
	b6.SetZoning(fr,1,fz, false, false, m_bz);

	MBBlock& b7 = m_MBlock[6];
	b7.SetID(0);
	b7.SetNodes(2,20,21,5,11,28,29,14);
	b7.SetSizes(ns,nd,nz);
	b7.SetZoning(fr,1,fz, false, false, m_bz);

	MBBlock& b8 = m_MBlock[7];
	b8.SetID(0);
	b8.SetNodes(5,21,22,8,14,29,30,17);
	b8.SetSizes(ns,nd,nz);
	b8.SetZoning(fr,1,fz, false, false, m_bz);

	MBBlock& b9 = m_MBlock[8];
	b9.SetID(0);
	b9.SetNodes(8,22,23,7,17,30,31,16);
	b9.SetSizes(ns,nd,nz);
	b9.SetZoning(fr,1,fz, false, false, m_bz);

	MBBlock& b10 = m_MBlock[9];
	b10.SetID(0);
	b10.SetNodes(7,23,24,6,16,31,32,15);
	b10.SetSizes(ns,nd,nz);
	b10.SetZoning(fr,1,fz, false, false, m_bz);

	MBBlock& b11 = m_MBlock[10];
	b11.SetID(0);
	b11.SetNodes(6,24,25,3,15,32,33,12);
	b11.SetSizes(ns,nd,nz);
	b11.SetZoning(fr,1,fz, false, false, m_bz);

	MBBlock& b12 = m_MBlock[11];
	b12.SetID(0);
	b12.SetNodes(3,25,18,0,12,33,26,9);
	b12.SetSizes(ns,nd,nz);
	b12.SetZoning(fr,1,fz, false, false, m_bz);

	// update the MB data
	BuildMB();

	// assign face ID's
	SetBlockFaceID(b1, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b2, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b3, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b4, -1, -1, -1, -1, 0, 5);
	SetBlockFaceID(b5, -1,  3, -1, -1, 0, 5);
	SetBlockFaceID(b6, -1,  4, -1, -1, 0, 5);
	SetBlockFaceID(b7, -1,  4, -1, -1, 0, 5);
	SetBlockFaceID(b8, -1,  1, -1, -1, 0, 5);
	SetBlockFaceID(b9, -1,  1, -1, -1, 0, 5);
	SetBlockFaceID(b10, -1,  2, -1, -1, 0, 5);
	SetBlockFaceID(b11, -1,  2, -1, -1, 0, 5);
	SetBlockFaceID(b12, -1,  3, -1, -1, 0, 5);

	MBFace& F1 = GetBlockFace( 7, 1); SetFaceEdgeID(F1, 0, -1, 4,  8);
	MBFace& F2 = GetBlockFace( 8, 1); SetFaceEdgeID(F2, 0,  9, 4, -1);
	MBFace& F3 = GetBlockFace( 9, 1); SetFaceEdgeID(F3, 1, -1, 5,  9);
	MBFace& F4 = GetBlockFace(10, 1); SetFaceEdgeID(F4, 1, 10, 5, -1);
	MBFace& F5 = GetBlockFace(11, 1); SetFaceEdgeID(F5, 2, -1, 6, 10);
	MBFace& F6 = GetBlockFace( 4, 1); SetFaceEdgeID(F6, 2, 11, 6, -1);
	MBFace& F7 = GetBlockFace( 5, 1); SetFaceEdgeID(F7, 3, -1, 7, 11);
	MBFace& F8 = GetBlockFace( 6, 1); SetFaceEdgeID(F8, 3,  8, 7, -1);

	m_MBNode[21].SetID(0);
	m_MBNode[23].SetID(1);
	m_MBNode[25].SetID(2);
	m_MBNode[19].SetID(3);
	m_MBNode[29].SetID(4);
	m_MBNode[31].SetID(5);
	m_MBNode[33].SetID(6);
	m_MBNode[27].SetID(7);

	// create the MB
	FSMesh* pm = FEMultiBlockMesh::BuildMBMesh();

	// project the nodes onto a cylinder
	vec3d r0, r1;
	for (int i=0; i<pm->Nodes(); ++i)
	{
		// get the nodal coordinate in the template
		vec3d& rn = pm->Node(i).r;
		double x = rn.x;
		double y = rn.y;

		// get the max-distance 
		double D = fmax(fabs(x),fabs(y));

		if (D <= 1)
		{
			rn.x *= d0x;
			rn.y *= d0y;
		}
		else
		{
			// "normalize" the coordinates
			// with respect to the max distance
			double r = x/D;
			double s = y/D;

			vec3d r0;
			if (fabs(x) >= fabs(y))
			{
				double u = x/fabs(x);
				r0.x = u*Rx*cos(PI*0.25*s);
				r0.y =   Ry*sin(PI*0.25*s);
			}
			else 
			{
				double u = y/fabs(y);
				r0.y = u*Ry*cos(PI*0.25*r);
				r0.x =   Rx*sin(PI*0.25*r);
			}

			vec3d r1(r*d0x, s*d0y, 0);
			double a = D - 1;

			rn.x = r0.x*a + r1.x*(1-a);
			rn.y = r0.y*a + r1.y*(1-a);
		}
	}

	// update the mesh
	pm->UpdateMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// Build a wedged mesh
FSMesh* FECylinder2::BuildWedged()
{
	GCylinder2* po = dynamic_cast<GCylinder2*>(&m_o);
	if (po == nullptr) return nullptr;

	int i, j, k;

	// get the object parameters
	ParamBlock& param = po->GetParamBlock();
	double Rx = param.GetFloatValue(GCylinder2::RADIUSX);
	double Ry = param.GetFloatValue(GCylinder2::RADIUSY);
	double h  = param.GetFloatValue(GCylinder2::HEIGHT );

	// get the mesh parameters
	m_r  = GetFloatValue(RATIO);
	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(GR);
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);
	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_ns == 1) { m_gr = 1; m_br = false; }

	// make sure the divisions is a multiple of 4
	// so that we can always make four sides
	m_nd *= 4;

	// calculate storage
	int nodes = (m_nz+1)*(1 + m_nd*m_ns);
	int elems = m_nz*m_ns*m_nd;
	int faces = 2*m_ns*m_nd + m_nz*m_nd;
	int edges = 2*m_nd + 4*m_nz;

	// create mesh
	FSMesh* pm = new FSMesh;
	pm->Create(nodes, elems, faces, edges);

	// --- A. Create the nodes ---
	FSNode* pn = pm->NodePtr();
	double x, y, z, R;

	double gz = 1;
	double gr = 1;

	double fz = m_gz;
	double fr = m_gr;

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = h / gz;
	}
	else 
	{
		for (i=0; i<m_nz-1; ++i) gz = fz*gz+1; 
		gz = h / gz;
	}

	if (m_br)
	{
		gr = 2; if (m_ns%2) gr += fr;
		for (i=0; i<m_ns/2-1; ++i) gr = fr*gr+2;
		gr = 1.0 / gr;
	}
	else 
	{
		for (i=0; i<m_ns-1; ++i) gr = fr*gr+1; 
		gr = 1.0 / gr;
	}

	double dz = gz;
	z = 0;
	for (i=0; i<=m_nz; ++i)
	{
		double dr = gr;
		R = 0;

		// create the center node
		pn->r = vec3d(0, 0, z);
		pn->m_gid = -1;
		++pn;

		R += dr;
		dr *= fr;
		if (m_br && (m_ns == 2))
		{
			dr /= fr;
			fr = 1.0/fr;
		}

		// create the other nodes
		for (j=0; j<m_ns; ++j)
		{
			for (k=0; k<m_nd; ++k)
			{
				x = Rx*R*cos(k*2.0*PI / (m_nd));
				y = Ry*R*sin(k*2.0*PI / (m_nd));

				pn->r = vec3d(x, y, z);
				pn->m_gid = -1;
				pn++;
			}

			R += dr;
			dr *= fr;
			if (m_br && (j+1 == m_ns/2-1))
			{
				if (m_ns%2 == 0) dr /= fr;
				fr = 1.0/fr;
			}
		}
		if (m_br) fr = 1.0/fr;

		z += dz;
		dz *= fz;
		if (m_bz && (i == m_nz/2-1))
		{
			if (m_nz%2 == 0) dz /= fz;
			fz = 1.0/fz;
		}
	}

	pm->Node(NodeIndex(0, m_ns, 0       )).m_gid = 0;
	pm->Node(NodeIndex(0, m_ns, m_nd/4  )).m_gid = 1;
	pm->Node(NodeIndex(0, m_ns, m_nd/2  )).m_gid = 2;
	pm->Node(NodeIndex(0, m_ns, 3*m_nd/4)).m_gid = 3;
	pm->Node(NodeIndex(1, m_ns, 0       )).m_gid = 4;
	pm->Node(NodeIndex(1, m_ns, m_nd/4  )).m_gid = 5;
	pm->Node(NodeIndex(1, m_ns, m_nd/2  )).m_gid = 6;
	pm->Node(NodeIndex(1, m_ns, 3*m_nd/4)).m_gid = 7;

	// --- B. Create the elements ---

	// create the inner wedge elements
	int eid = 0;
	int nlevel = 1+m_nd*m_ns;
	for (i=0; i<m_nz; i++)
	{
		// wedge elements
		for (k=0; k<m_nd; ++k)
		{
			FSElement_* ph = pm->ElementPtr(eid++);

			ph->SetType(FE_PENTA6);
			ph->m_gid = 0;
			ph->m_node[0] = i*nlevel;
			ph->m_node[1] = i*nlevel + 1 + k;
			ph->m_node[2] = i*nlevel + 1 + (k+1)%m_nd;

			ph->m_node[3] = (i+1)*nlevel;
			ph->m_node[4] = (i+1)*nlevel + 1 + k;
			ph->m_node[5] = (i+1)*nlevel + 1 + (k+1)%m_nd;
		}

		// hex elements
		for (j=1; j<m_ns; ++j)
		{
			for (k=0; k<m_nd; ++k)
			{
				FSElement_* ph = pm->ElementPtr(eid++);

				ph->SetType(FE_HEX8);
				ph->m_gid = 0;

				ph->m_node[0] = i*nlevel + 1 + (j-1)*m_nd + k;
				ph->m_node[1] = i*nlevel + 1 + (j  )*m_nd + k;
				ph->m_node[2] = i*nlevel + 1 + (j  )*m_nd + (k+1)%m_nd;
				ph->m_node[3] = i*nlevel + 1 + (j-1)*m_nd + (k+1)%m_nd;

				ph->m_node[4] = (i+1)*nlevel + 1 + (j-1)*m_nd + k;
				ph->m_node[5] = (i+1)*nlevel + 1 + (j  )*m_nd + k;
				ph->m_node[6] = (i+1)*nlevel + 1 + (j  )*m_nd + (k+1)%m_nd;
				ph->m_node[7] = (i+1)*nlevel + 1 + (j-1)*m_nd + (k+1)%m_nd;
			}
		}
	}

	// --- C. Create faces ---
	// side faces
	FSFace* pf = pm->FacePtr();
	for (i=0; i<m_nz; ++i)
	{
		for (k=0; k<m_nd; ++k, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
//			pf->m_gid = k/(m_nd/4);
			pf->m_gid = k/(m_nd/4) + 1;
			pf->n[0] = NodeIndex(i  , m_ns, k  );
			pf->n[1] = NodeIndex(i  , m_ns, k+1);
			pf->n[2] = NodeIndex(i+1, m_ns, k+1);
			pf->n[3] = NodeIndex(i+1, m_ns, k  );
		}
	}

	// bottom faces
	for (k=0; k<m_nd; ++k)
	{
		pf->SetType(FE_FACE_TRI3);
//		pf->m_gid = 4;
		pf->m_gid = 0;
		pf->n[0] = NodeIndex(0, 0, 0);
		pf->n[1] = NodeIndex(0, 1, k+1);
		pf->n[2] = NodeIndex(0, 1, k);
		pf->n[3] = pf->n[2];
		++pf;

		for (j=1; j<m_ns; ++j, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 0;
//			pf->m_gid = 4;
			pf->n[0] = NodeIndex(0,j  ,k+1);
			pf->n[1] = NodeIndex(0,j+1,k+1);
			pf->n[2] = NodeIndex(0,j+1,k  );
			pf->n[3] = NodeIndex(0,j  ,k  );
		}
	}

	// top faces
	for (k=0; k<m_nd; ++k)
	{
		pf->SetType(FE_FACE_TRI3);
		pf->m_gid = 5;
		pf->n[0] = NodeIndex(m_nz, 0, 0);
		pf->n[1] = NodeIndex(m_nz, 1, k);
		pf->n[2] = NodeIndex(m_nz, 1, k+1);
		pf->n[3] = pf->n[2];
		++pf;

		for (j=1; j<m_ns; ++j, ++pf)
		{
			pf->SetType(FE_FACE_QUAD4);
			pf->m_gid = 5;
			pf->n[0] = NodeIndex(m_nz,j  ,k  );
			pf->n[1] = NodeIndex(m_nz,j+1,k  );
			pf->n[2] = NodeIndex(m_nz,j+1,k+1);
			pf->n[3] = NodeIndex(m_nz,j  ,k+1);
		}
	}

	// --- D. Create edges ---
	FSEdge* pe = pm->EdgePtr();
	for (k=0; k<m_nd; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2);
		pe->m_gid = k / (m_nd / 4);
		pe->n[0] = NodeIndex(0, m_ns, k);
		pe->n[1] = NodeIndex(0, m_ns, k+1);
	}

	for (k=0; k<m_nd; ++k, ++pe)
	{
		pe->SetType(FE_EDGE2);
		pe->m_gid = 4 + k / (m_nd / 4);
		pe->n[0] = NodeIndex(m_nz, m_ns, k);
		pe->n[1] = NodeIndex(m_nz, m_ns, k+1);
	}

	for (k=0; k<4; ++k)
	{
		for (i=0; i<m_nz; ++i, ++pe)
		{
			pe->SetType(FE_EDGE2);
			pe->m_gid = 8 + k;
			pe->n[0] = NodeIndex(i  , m_ns, k*m_nd/4);
			pe->n[1] = NodeIndex(i+1, m_ns, k*m_nd/4);
		}
	}

	pm->BuildMesh();

	return pm;
}
