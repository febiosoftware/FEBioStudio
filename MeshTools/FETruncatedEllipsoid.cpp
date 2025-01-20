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
#include "FETruncatedEllipsoid.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>

FETruncatedEllipsoid::FETruncatedEllipsoid(GTruncatedEllipsoid* po)
{
	m_pobj = po;
	m_ns = 6;
	m_nz = 6;
	m_nr = 1;
	m_gr = 1;
	m_br = false;

	AddIntParam(m_ns, "ns", "Slices");
	AddIntParam(m_nz, "nz", "Stacks");
	AddIntParam(m_nr, "nr", "Divisions");

	AddDoubleParam(m_gr, "gr", "R-bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddIntParam(0, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0Hex27\0");
}

extern double gain2(double x, double r, double n);

FSMesh* FETruncatedEllipsoid::BuildMesh()
{
	assert(m_pobj);

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double Ra = param.GetFloatValue(GTruncatedEllipsoid::RA);
	double Rb = param.GetFloatValue(GTruncatedEllipsoid::RB);
	double Rc = param.GetFloatValue(GTruncatedEllipsoid::RC);
	double wt = param.GetFloatValue(GTruncatedEllipsoid::WT);
	double uend = param.GetFloatValue(GTruncatedEllipsoid::VEND);
	uend = PI*uend/180.0;

	// get mesh parameters
	m_ns = GetIntValue(NSLICE);
	m_nz = GetIntValue(NSTACK);
	m_nr = GetIntValue(NDIV);
	m_gr = GetFloatValue(GR);
	m_br = GetBoolValue(GR2);

	double h = 1.0;
	double w0 = 0.5;
	double w1 = 1.0;

	m_MBNode.resize(34);

	m_MBNode[ 0].m_r = vec3d(-w0, -w0, 0.0);
	m_MBNode[ 1].m_r = vec3d( 0., -w0, 0.0);
	m_MBNode[ 2].m_r = vec3d( w0, -w0, 0.0);
	m_MBNode[ 3].m_r = vec3d(-w0,  0., 0.0);
	m_MBNode[ 4].m_r = vec3d( 0.,  0., 0.0);
	m_MBNode[ 5].m_r = vec3d( w0,  0., 0.0);
	m_MBNode[ 6].m_r = vec3d(-w0,  w0, 0.0);
	m_MBNode[ 7].m_r = vec3d( 0.,  w0, 0.0);
	m_MBNode[ 8].m_r = vec3d( w0,  w0, 0.0);

	m_MBNode[ 9].m_r = vec3d(-w1, -w1, 0.0);
	m_MBNode[10].m_r = vec3d( 0., -w1, 0.0);
	m_MBNode[11].m_r = vec3d( w1, -w1, 0.0);
	m_MBNode[12].m_r = vec3d( w1,  0., 0.0);
	m_MBNode[13].m_r = vec3d( w1,  w1, 0.0);
	m_MBNode[14].m_r = vec3d( 0.,  w1, 0.0);
	m_MBNode[15].m_r = vec3d(-w1,  w1, 0.0);
	m_MBNode[16].m_r = vec3d(-w1,  0., 0.0);

	m_MBNode[17].m_r = vec3d(-w0, -w0, h);
	m_MBNode[18].m_r = vec3d( 0., -w0, h);
	m_MBNode[19].m_r = vec3d( w0, -w0, h);
	m_MBNode[20].m_r = vec3d(-w0,  0., h);
	m_MBNode[21].m_r = vec3d( 0.,  0., h);
	m_MBNode[22].m_r = vec3d( w0,  0., h);
	m_MBNode[23].m_r = vec3d(-w0,  w0, h);
	m_MBNode[24].m_r = vec3d( 0.,  w0, h);
	m_MBNode[25].m_r = vec3d( w0,  w0, h);

	m_MBNode[26].m_r = vec3d(-w1, -w1, h);
	m_MBNode[27].m_r = vec3d( 0., -w1, h);
	m_MBNode[28].m_r = vec3d( w1, -w1, h);
	m_MBNode[29].m_r = vec3d( w1,  0., h);
	m_MBNode[30].m_r = vec3d( w1,  w1, h);
	m_MBNode[31].m_r = vec3d( 0.,  w1, h);
	m_MBNode[32].m_r = vec3d(-w1,  w1, h);
	m_MBNode[33].m_r = vec3d(-w1,  0., h);

	// create the MB block
	m_MBlock.resize(12);
	int MB[12][8] = {
		{ 0,  1, 4, 3, 17, 18, 21, 20},	// 0
		{ 1,  2, 5, 4, 18, 19, 22, 21},	// 1
		{ 3,  4, 7, 6, 20, 21, 24, 23},	// 2
		{ 4,  5, 8, 7, 21, 22, 25, 24},	// 3
		{ 9, 10, 1, 0, 26, 27, 18, 17},	// 4
		{10, 11, 2, 1, 27, 28, 19, 18},	// 5
		{11, 12, 5, 2, 28, 29, 22, 19},	// 6
		{12, 13, 8, 5, 29, 30, 25, 22}, // 7 
		{13, 14, 7, 8, 30, 31, 24, 25},	// 8
		{14, 15, 6, 7, 31, 32, 23, 24}, // 9
		{15, 16, 3, 6, 32, 33, 20, 23}, // 10
		{16,  9, 0, 3, 33, 26, 17, 20}  // 11
	};

	int BS[12][3] = {
		{m_ns, m_ns, m_nr},
		{m_ns, m_ns, m_nr},
		{m_ns, m_ns, m_nr},
		{m_ns, m_ns, m_nr},
		{m_ns, m_nz, m_nr},
		{m_ns, m_nz, m_nr},
		{m_ns, m_nz, m_nr},
		{m_ns, m_nz, m_nr},
		{m_ns, m_nz, m_nr},
		{m_ns, m_nz, m_nr},
		{m_ns, m_nz, m_nr},
		{m_ns, m_nz, m_nr}
	};

	int i;
	for (i=0; i<12; ++i)
	{
		int* n = MB[i];
		int* s = BS[i];
		m_MBlock[i].SetNodes(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
		m_MBlock[i].SetSizes(s[0],s[1],s[2]);
		m_MBlock[i].SetID(0);
	}

	// update the MB data
	BuildMB();

	// assign face ID's
	GetBlockFace(3, 4).SetID(0);
	GetBlockFace(7, 4).SetID(0);
	GetBlockFace(8, 4).SetID(0);

	GetBlockFace( 2, 4).SetID(1);
	GetBlockFace( 9, 4).SetID(1);
	GetBlockFace(10, 4).SetID(1);

	GetBlockFace( 0, 4).SetID(2);
	GetBlockFace(11, 4).SetID(2);
	GetBlockFace( 4, 4).SetID(2);

	GetBlockFace( 1, 4).SetID(3);
	GetBlockFace( 5, 4).SetID(3);
	GetBlockFace( 6, 4).SetID(3);

	GetBlockFace(3, 5).SetID(4);
	GetBlockFace(7, 5).SetID(4);
	GetBlockFace(8, 5).SetID(4);

	GetBlockFace( 2, 5).SetID(5);
	GetBlockFace( 9, 5).SetID(5);
	GetBlockFace(10, 5).SetID(5);

	GetBlockFace( 0, 5).SetID(6);
	GetBlockFace(11, 5).SetID(6);
	GetBlockFace( 4, 5).SetID(6);

	GetBlockFace( 1, 5).SetID(7);
	GetBlockFace( 5, 5).SetID(7);
	GetBlockFace( 6, 5).SetID(7);

	GetBlockFace( 7, 0).SetID(8);
	GetBlockFace( 8, 0).SetID(8);

	GetBlockFace( 9, 0).SetID(9);
	GetBlockFace(10, 0).SetID(9);

	GetBlockFace(11, 0).SetID(10);
	GetBlockFace( 4, 0).SetID(10);

	GetBlockFace(5, 0).SetID(11);
	GetBlockFace(6, 0).SetID(11);

	// edge ID's
	GetFaceEdge(GetBlockFace(3,0),0).SetID(0);
	GetFaceEdge(GetBlockFace(7,3),0).SetID(0);

	GetFaceEdge(GetBlockFace(3,3),0).SetID(1);
	GetFaceEdge(GetBlockFace(8,1),0).SetID(1);

	GetFaceEdge(GetBlockFace( 2,0),0).SetID(2);
	GetFaceEdge(GetBlockFace(10,1),0).SetID(2);

	GetFaceEdge(GetBlockFace(1,3),0).SetID(3);
	GetFaceEdge(GetBlockFace(5,3),0).SetID(3);

	GetFaceEdge(GetBlockFace(3,0),2).SetID(4);
	GetFaceEdge(GetBlockFace(7,3),2).SetID(4);

	GetFaceEdge(GetBlockFace(3,3),2).SetID(5);
	GetFaceEdge(GetBlockFace(8,1),2).SetID(5);

	GetFaceEdge(GetBlockFace( 2,0),2).SetID(6);
	GetFaceEdge(GetBlockFace(10,1),2).SetID(6);

	GetFaceEdge(GetBlockFace(1,3),2).SetID(7);
	GetFaceEdge(GetBlockFace(5,3),2).SetID(7);

	GetFaceEdge(GetBlockFace( 7,0),3).SetID( 8);
	GetFaceEdge(GetBlockFace( 9,0),3).SetID( 9);
	GetFaceEdge(GetBlockFace(11,0),3).SetID(10);
	GetFaceEdge(GetBlockFace( 5,0),3).SetID(11);

	GetFaceEdge(GetBlockFace(7,0),0).SetID(12);
	GetFaceEdge(GetBlockFace(8,0),0).SetID(12);

	GetFaceEdge(GetBlockFace( 9,0),0).SetID(13);
	GetFaceEdge(GetBlockFace(10,0),0).SetID(13);

	GetFaceEdge(GetBlockFace(11,0),0).SetID(14);
	GetFaceEdge(GetBlockFace( 4,0),0).SetID(14);

	GetFaceEdge(GetBlockFace(5,0),0).SetID(15);
	GetFaceEdge(GetBlockFace(6,0),0).SetID(15);

	GetFaceEdge(GetBlockFace(7,0),2).SetID(16);
	GetFaceEdge(GetBlockFace(8,0),2).SetID(16);

	GetFaceEdge(GetBlockFace( 9,0),2).SetID(17);
	GetFaceEdge(GetBlockFace(10,0),2).SetID(17);

	GetFaceEdge(GetBlockFace(11,0),2).SetID(18);
	GetFaceEdge(GetBlockFace( 4,0),2).SetID(18);

	GetFaceEdge(GetBlockFace(5,0),2).SetID(19);
	GetFaceEdge(GetBlockFace(6,0),2).SetID(19);

	// Node ID's
	m_MBNode[ 4].SetID(0);
	m_MBNode[21].SetID(1);
	m_MBNode[12].SetID(2);
	m_MBNode[29].SetID(3);
	m_MBNode[14].SetID(4);
	m_MBNode[31].SetID(5);
	m_MBNode[16].SetID(6);
	m_MBNode[33].SetID(7);
	m_MBNode[10].SetID(8);
	m_MBNode[27].SetID(9);

	// set element type
	int nelem = GetIntValue(ELEM_TYPE);
	switch (nelem)
	{
	case 0: SetElementType(FE_HEX8); break;
	case 1: SetElementType(FE_HEX20); break;
	case 2: SetElementType(FE_HEX27); break;
	}

	// create the MB
	FSMesh* pm = FEMultiBlockMesh::BuildMesh();

	// project nodes to geometry
	int NN = pm->Nodes();
	for (int i=0; i<NN; ++i)
	{
		vec3d& r = pm->Node(i).r;
		double d1 = fmax(fabs(r.x), fabs(r.y));
		double d2 = sqrt(r.x*r.x+r.y*r.y)/sqrt(2.0);
		double d = (d1 <= w0 ? d2 : d2 + (d1 - w0)*(d1 - d2)/(1 - w0));
		double v = -atan2(r.x, r.y) + PI/2;
		double u = -PI/2 + d*(uend + PI/2);

		double h = r.z;
		if (m_br)
		{
			if (h <= 0.5)
				h = 0.5*gain2(2*h, m_gr, m_ns);
			else
				h = 1 - 0.5*gain2(2 - 2*h, m_gr, m_ns);
		}
		else h = gain2(h, m_gr, m_ns); 

		double w = -wt + h*wt*2.0;
		r.x = (Ra - w)*cos(u)*cos(v);
		r.y = (Rb - w)*cos(u)*sin(v);
		r.z = (Rc - w)*sin(u);
	}

	// update the mesh
	pm->UpdateMesh();

	return pm;
}

