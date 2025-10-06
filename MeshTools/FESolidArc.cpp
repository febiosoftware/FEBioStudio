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
#include "FESolidArc.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>

FESolidArc::FESolidArc(GObject& o) : FEMultiBlockMesh(o)
{
	m_nd = 8;
	m_ns = 4;
	m_nz = 8;

	m_gz = m_gr = 1;
	m_bz = false;
	m_br = false;
	
	// define the tube parameters
	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments");
	AddIntParam(m_nz, "nz", "Stacks");

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

FSMesh* FESolidArc::BuildMesh()
{
//	return BuildMeshLegacy();
	return BuildMultiBlockMesh();
}

bool FESolidArc::BuildMultiBlock()
{
	GSolidArc* po = dynamic_cast<GSolidArc*>(&m_o);
	if (po == nullptr) return false;

	double R0 = po->GetFloatValue(GSolidArc::RIN);
	double R1 = po->GetFloatValue(GSolidArc::ROUT);
	double H = po->GetFloatValue(GSolidArc::HEIGHT);
	double w = po->GetFloatValue(GSolidArc::ARC);

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);

	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(ZR);

	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nz == 1) m_bz = false;
	if (m_ns == 1) m_br = false;

	// build nodes
	m_MBNode.clear();
	AddNode(po->Node(0)->LocalPosition()).SetID(0);
	AddNode(po->Node(1)->LocalPosition()).SetID(1);
	AddNode(po->Node(2)->LocalPosition()).SetID(2);
	AddNode(po->Node(3)->LocalPosition()).SetID(3);
	AddNode(po->Node(4)->LocalPosition()).SetID(4);
	AddNode(po->Node(5)->LocalPosition()).SetID(5);
	AddNode(po->Node(6)->LocalPosition()).SetID(6);
	AddNode(po->Node(7)->LocalPosition()).SetID(7);

	// build block
	m_MBlock.resize(1);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0, 1, 2, 3, 4, 5, 6, 7);
	b1.SetSizes(m_nd, m_ns, m_nz);
	b1.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	// build MB data structures
	BuildMB();

	// set IDs of faces and edges
	SetBlockFaceID(b1, 0, 1, 2, 3, 4, 5);

	MBFace& F1 = GetBlockFace(0, 0); SetFaceEdgeID(F1, 0, 9, 4, 8);
	MBFace& F2 = GetBlockFace(0, 1); SetFaceEdgeID(F2, 1, 10, 5, 9);
	MBFace& F3 = GetBlockFace(0, 2); SetFaceEdgeID(F3, 2, 11, 6, 10);
	MBFace& F4 = GetBlockFace(0, 3); SetFaceEdgeID(F4, 3, 8, 7, 11);
	MBFace& F5 = GetBlockFace(0, 4); SetFaceEdgeID(F5, 2, 1, 0, 3);
	MBFace& F6 = GetBlockFace(0, 5); SetFaceEdgeID(F6, 4, 5, 6, 7);

	// set edge types
	GetFaceEdge(F1, 0).SetWinding(1).m_ntype = EDGE_ZARC;
	GetFaceEdge(F1, 2).SetWinding(-1).m_ntype = EDGE_ZARC;
	GetFaceEdge(F3, 0).SetWinding(-1).m_ntype = EDGE_ZARC;
	GetFaceEdge(F3, 2).SetWinding(1).m_ntype = EDGE_ZARC;

	UpdateMB();

	return true;
}

FSMesh* FESolidArc::BuildMultiBlockMesh()
{
	if (!BuildMultiBlock()) return nullptr;

	// set element type
	int nelem = GetIntValue(ELEM_TYPE);
	switch (nelem)
	{
	case 0: SetElementType(FE_HEX8); break;
	case 1: SetElementType(FE_HEX20); break;
	case 2: SetElementType(FE_HEX27); break;
	}

	// build mesh and return
	return FEMultiBlockMesh::BuildMBMesh();
}

FSMesh* FESolidArc::BuildMeshLegacy()
{
	GSolidArc* po = dynamic_cast<GSolidArc*>(&m_o);
	if (po == nullptr) return nullptr;

	int i, j, k, n;

	// get object parameters
	ParamBlock& param = po->GetParamBlock();
	double R0 = param.GetFloatValue(GSolidArc::RIN);
	double R1 = param.GetFloatValue(GSolidArc::ROUT);
	double h  = param.GetFloatValue(GSolidArc::HEIGHT);
	double w  = PI*param.GetFloatValue(GSolidArc::ARC)/180.0;

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_nz = GetIntValue(NSTACK);

	m_gz = GetFloatValue(ZZ);
	m_gr = GetFloatValue(ZR);

	m_bz = GetBoolValue(GZ2);
	m_br = GetBoolValue(GR2);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;
	if (m_nz < 1) m_nz = 1;

	if (m_nz == 1) m_bz = false;
	if (m_ns == 1) m_br = false;

	// get the parameters
	int nd = m_nd;
	int nr = m_ns;
	int nz = m_nz;

	double fz = m_gz;
	double fr = m_gr;

	int nodes = (nd+1)*(nr+1)*(nz+1);
	int elems = nd*nr*nz;

	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	double cosa, sina;
	double x, y, z, R;

	double gz = 1;
	double gr = 1;

	if (m_bz)
	{
		gz = 2; if (m_nz%2) gz += fz;
		for (i=0; i<m_nz/2-1; ++i) gz = fz*gz+2;
		gz = h / gz;
	}
	else
	{
		for (i=0; i<nz-1; ++i) gz = fz*gz+1; 
		gz = h / gz;
	}

	if (m_br)
	{
		gr = 2; if (m_ns%2) gr += fr;
		for (i=0; i<m_ns/2-1; ++i) gr = fr*gr+2;
		gr = (R1 - R0) / gr;
	}
	else
	{
		for (i=0; i<nr-1; ++i) gr = fr*gr+1; 
		gr = (R1 - R0) / gr;
	}


	// create nodes
	n = 0;
	double dr;
	double dz = gz;
	z = 0;
	for (k=0; k<=nz; ++k)
	{
		for (j=0; j<=nd; ++j)
		{
			cosa = (double) cos(w*j/nd);
			sina = (double) sin(w*j/nd);

			dr = gr;
			R = R0;
			for (i=0; i<=nr; ++i, ++n)
			{
				x = R*cosa;
				y = R*sina;

				FSNode& node = pm->Node(n);

				node.r = vec3d(x, y, z);

				R += dr;
				dr *= fr;
				if (m_br && (i == m_ns/2-1))
				{
					if (m_ns%2 == 0) dr /= fr;
					fr = 1.0/fr;
				}
			}
			if (m_br) fr = 1.0/fr;
		}

		z += dz;
		dz *= fz;
		if (m_bz && (k == m_nz/2-1))
		{
			if (m_nz%2 == 0) dz /= fz;
			fz = 1.0/fz;
		}
	}

	// assign node ID's
	pm->Node( NodeIndex(nr,    0, 0) ).m_gid = 0;
	pm->Node( NodeIndex(nr,   nd, 0) ).m_gid = 1;
	pm->Node( NodeIndex( 0,   nd, 0) ).m_gid = 2;
	pm->Node( NodeIndex( 0,    0, 0) ).m_gid = 3;
	pm->Node( NodeIndex(nr,    0, nz) ).m_gid = 4;
	pm->Node( NodeIndex(nr,   nd, nz) ).m_gid = 5;
	pm->Node( NodeIndex( 0,   nd, nz) ).m_gid = 6;
	pm->Node( NodeIndex( 0,    0, nz) ).m_gid = 7;

	// create elements
	n = 0;
	for (k=0; k<nz; ++k)
		for (j=0; j<nd; ++j)
			for (i=0; i<nr; ++i, ++n)
			{
				FSElement& e = pm->Element(n);
				e.SetType(FE_HEX8);
				e.m_gid = 0;

				e.m_node[0] = k*(nd+1)*(nr+1) + j*(nr+1) + i;
				e.m_node[1] = k*(nd+1)*(nr+1) + j*(nr+1) + i+1;
				e.m_node[2] = k*(nd+1)*(nr+1) + (j+1)*(nr+1) + i+1;
				e.m_node[3] = k*(nd+1)*(nr+1) + (j+1)*(nr+1) + i;
				e.m_node[4] = (k+1)*(nd+1)*(nr+1) + j*(nr+1) + i;
				e.m_node[5] = (k+1)*(nd+1)*(nr+1) + j*(nr+1) + i+1;
				e.m_node[6] = (k+1)*(nd+1)*(nr+1) + (j+1)*(nr+1) + i+1;
				e.m_node[7] = (k+1)*(nd+1)*(nr+1) + (j+1)*(nr+1) + i;
			}

	BuildFaces(pm);
	BuildEdges(pm);

	pm->BuildMesh();

	return pm;
}

void FESolidArc::BuildFaces(FSMesh* pm)
{
	int i, j;

	int nd = m_nd;
	int nz = m_nz;
	int nr = m_ns;

	// count the faces
	int NF = 2*nz*nd + 2*nr*nd + 2*nz*nr;
	pm->Create(0,0,NF);

	// build the faces
	FSFace* pf = pm->FacePtr();

	// face 0
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nd; ++i, ++pf)
		{
			FSFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 0;
			f.m_sid = 0;
			f.n[0] = NodeIndex(nr,   i,   j);
			f.n[1] = NodeIndex(nr, i+1,   j);
			f.n[2] = NodeIndex(nr, i+1, j+1);
			f.n[3] = NodeIndex(nr,   i, j+1);
		}
	}

	// face 1
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FSFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 1;
			f.m_sid = 1;
			f.n[0] = NodeIndex(i  , nd,   j);
			f.n[1] = NodeIndex(i+1, nd,   j);
			f.n[2] = NodeIndex(i+1, nd, j+1);
			f.n[3] = NodeIndex(i  , nd, j+1);
		}
	}

	// face 2
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nd; ++i, ++pf)
		{
			FSFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 2;
			f.m_sid = 2;
			f.n[0] = NodeIndex(0, i+1, j  );
			f.n[1] = NodeIndex(0, i  , j  );
			f.n[2] = NodeIndex(0, i  , j+1);
			f.n[3] = NodeIndex(0, i+1, j+1);
		}
	}

	// face 3
	for (j=0; j<nz; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FSFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 3;
			f.m_sid = 3;
			f.n[0] = NodeIndex(i  , 0,   j);
			f.n[1] = NodeIndex(i+1, 0,   j);
			f.n[2] = NodeIndex(i+1, 0, j+1);
			f.n[3] = NodeIndex(i  , 0, j+1);
		}
	}

	// face 4
	for (j=0; j<nd; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FSFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 4;
			f.m_sid = 4;
			f.n[0] = NodeIndex(i+1, j,   0);
			f.n[1] = NodeIndex(i  , j,   0);
			f.n[2] = NodeIndex(i  , j+1, 0);
			f.n[3] = NodeIndex(i+1, j+1, 0);
		}
	}

	// face 5
	for (j=0; j<nd; ++j)
	{
		for (i=0; i<nr; ++i, ++pf)
		{
			FSFace& f = *pf;
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 5;
			f.m_sid = 5;
			f.n[0] = NodeIndex(i  , j,   nz);
			f.n[1] = NodeIndex(i+1, j,   nz);
			f.n[2] = NodeIndex(i+1, j+1, nz);
			f.n[3] = NodeIndex(i  , j+1, nz);
		}
	}

}

void FESolidArc::BuildEdges(FSMesh* pm)
{
	int i;

	int nd = m_nd;
	int nz = m_nz;
	int nr = m_ns;

	// count edges
	int nedges = 4*nd + 4*nz + 4*nr;
	pm->Create(0,0,0,nedges);
	FSEdge* pe = pm->EdgePtr();

	for (i= 0; i<  nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 0; pe->n[0] = NodeIndex(nr, i,  0); pe->n[1] = NodeIndex(nr, i+1,  0); }
	for (i= 0; i<  nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 2; pe->n[0] = NodeIndex(0 , i,  0); pe->n[1] = NodeIndex(0 , i+1,  0); }
	for (i= 0; i<  nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 4; pe->n[0] = NodeIndex(nr, i, nz); pe->n[1] = NodeIndex(nr, i+1, nz); }
	for (i= 0; i<  nd; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 6; pe->n[0] = NodeIndex(0 , i, nz); pe->n[1] = NodeIndex(0 , i+1, nz); }

	for (i= 0; i<  nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 1; pe->n[0] = NodeIndex(i , nd,  0); pe->n[1] = NodeIndex(i+1 , nd, 0); }
	for (i= 0; i<  nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 3; pe->n[0] = NodeIndex(i ,  0,  0); pe->n[1] = NodeIndex(i+1 ,  0, 0); }
	for (i= 0; i<  nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 5; pe->n[0] = NodeIndex(i , nd, nz); pe->n[1] = NodeIndex(i+1 , nd, nz); }
	for (i= 0; i<  nr; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 7; pe->n[0] = NodeIndex(i ,  0, nz); pe->n[1] = NodeIndex(i+1 ,  0, nz); }

	for (i= 0; i<  nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  8; pe->n[0] = NodeIndex(nr,  0, i); pe->n[1] = NodeIndex(nr,  0, i+1); }
	for (i= 0; i<  nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid =  9; pe->n[0] = NodeIndex(nr, nd, i); pe->n[1] = NodeIndex(nr, nd, i+1); }
	for (i= 0; i<  nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 10; pe->n[0] = NodeIndex( 0, nd, i); pe->n[1] = NodeIndex( 0, nd, i+1); }
	for (i= 0; i<  nz; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 11; pe->n[0] = NodeIndex( 0,  0, i); pe->n[1] = NodeIndex( 0,  0, i+1); }
}
