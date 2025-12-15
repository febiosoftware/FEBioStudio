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
#include "FEGregoryPatch.h"
#include <MeshLib/FSMesh.h>

FEGregoryPatch::FEGregoryPatch(GObject& o) : FEMesher(o)
{
	m_w = m_h = 1;
	m_nx = m_ny = 5;
	m_mx = m_my = 5;

	AddDoubleParam(m_w, "w", "Width");
	AddDoubleParam(m_h, "h", "Height");
	AddIntParam(m_nx, "nx", "Nx"); // number of patches in x-direction
	AddIntParam(m_ny, "ny", "Ny"); // number of patches in y-direction
	AddIntParam(m_mx, "mx", "Mx"); // number of elements per patch in x-direction
	AddIntParam(m_my, "my", "My"); // number of elements per patch in y-direction
}

FEGregoryPatch::~FEGregoryPatch(void)
{
}

FSMesh* FEGregoryPatch::BuildMesh()
{
	// first, build the patches
	BuildPatches();

	// calcaulte the patch control data
	BuildPatchData();

	// next, we build the FE mesh
	return BuildFEMesh();
}

FSMesh* FEGregoryPatch::BuildFEMesh()
{
	// get the parameters
	m_w = GetFloatValue(W);
	m_h = GetFloatValue(H);
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_mx = GetIntValue(MX);
	m_my = GetIntValue(MY);

	if (m_nx<1) m_nx = 1;
	if (m_ny<1) m_ny = 1;
	if (m_mx<1) m_mx = 1;
	if (m_my<1) m_my = 1;

	int nodes = (m_nx*m_mx+1)*(m_ny*m_my+1);
	int elems = m_nx*m_mx*m_ny*m_my;

	FSMesh* pm = new FSMesh();
	pm->Create(nodes, elems);

	// build the nodes
	int i, j, k, l, m, n;
	double r, s;
	FSNode *pn = pm->NodePtr();
	for (j=0; j<=m_ny*m_my; ++j)
		for (i=0; i<=m_nx*m_mx; ++i, ++pn)
		{
			k = j/m_my;
			if (k==m_ny) { k = m_ny-1; m = m_my; } else m = j%m_my;

			l = i/m_mx;
			if (l==m_nx) { l = m_nx-1; n = m_mx; } else n = i%m_mx;

			GPatch& p = m_GPatch[k*m_nx+l];

			r = (double) n / (double) m_mx;
			s = (double) m / (double) m_my;

			pn->r = EvalPatch(p, r, s);
		}

	// assign group ID's to nodes
	for (j=0; j<=m_ny; ++j)
		for (i=0; i<=m_nx; ++i)
		{
			FSNode& n = pm->Node(j*(m_nx*m_mx+1)*(m_my) + i*m_mx);
			n.m_gid = j*(m_nx+1)+i;
		}

	// build the elements
	int eid = 0;
	for (j=0; j<m_ny*m_my; ++j)
		for (i=0; i<m_nx*m_mx; ++i)
		{
			FSElement_* pe = pm->ElementPtr(eid++);

			pe->m_node[0] = NodeIndex(i  ,j);
			pe->m_node[1] = NodeIndex(i+1,j);
			pe->m_node[2] = NodeIndex(i+1,j+1);
			pe->m_node[3] = NodeIndex(i  ,j+1);

			pe->SetType(FE_QUAD4);
			pe->m_gid = 0;
		}

	BuildFaces(pm);

	pm->BuildMesh();

	return pm;
}

void FEGregoryPatch::BuildPatches()
{
	double w = GetFloatValue(W);
	double h = GetFloatValue(H);
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);

	if (m_nx<1) m_nx = 1;
	if (m_ny<1) m_ny = 1;

	int NN = (m_nx+1)*(m_ny+1);
	int NP = m_nx*m_ny;

	m_GNode.resize(NN);
	m_GPatch.resize(NP);

	// calculate nodes
	int i, j, n;
	n = 0;
	double x, y, z;
	for (j=0; j<=m_ny; ++j)
		for (i=0; i<=m_nx; ++i, ++n)
		{
			x = -w*0.5 + i*w/m_nx;
			y = -h*0.5 + j*h/m_ny;
			z = 0;
			m_GNode[n].m_r = vec3d(x, y, z);
		}

	// create patches
	n = 0;
	for (j=0; j<m_ny; ++j)
		for (i=0; i<m_nx; ++i, ++n)
		{
			int* m = m_GPatch[n].m_node;
			m[0] = j*(m_nx+1) + i;
			m[1] = j*(m_nx+1) + i+1;
			m[2] = (j+1)*(m_nx+1) + i+1;
			m[3] = (j+1)*(m_nx+1) + i;
		}
}

void FEGregoryPatch::BuildPatchData()
{
	int i, j;

	int NN = (m_nx+1)*(m_ny+1);
	int NP = m_nx*m_ny;

	// zero node normals
	for (i=0; i<NN; ++i) m_GNode[i].m_n = vec3d(0,0,0);

	// calculate node normals
	vec3d r1, r2, y;
	for (i=0; i<NP; ++i)
	{
		GPatch& p = m_GPatch[i];
		for (j=0; j<4; ++j)
		{
			GNode& n0 = m_GNode[ p.m_node[j] ];
			GNode& n1 = m_GNode[ p.m_node[(j+1)%4] ];
			GNode& n2 = m_GNode[ p.m_node[(j+3)%4] ];

			r1 = n1.m_r - n0.m_r;
			r2 = n2.m_r - n0.m_r;

			y = r1^r2;
			y.Normalize();

			n0.m_n += y;
		}
	}

	// normalize normals
	for (i=0; i<NN; ++i) m_GNode[i].m_n.Normalize();

	// calculate the edge control points for each patch
	vec3d y0, y3, m0, m3, c0, c1, c2;
	for (i=0; i<NP; ++i)
	{
		GPatch& p = m_GPatch[i];
		for (j=0; j<4; ++j)
		{
			GNode& n0 = m_GNode[ p.m_node[j] ];
			GNode& n3 = m_GNode[ p.m_node[(j+1)%4] ];

			y0 = n0.m_r;
			m0 = n0.m_n;

			y3 = n3.m_r;
			m3 = n3.m_n;

			c0 = ((y3 - y0)-m0*((y3 - y0)*m0))/3.0;
			c2 = ((y3 - y0)-m3*((y3 - y0)*m3))/3.0;

			p.m_ye[j][0] = y0 + c0;
			p.m_ye[j][1] = y3 - c2;
		}
	}

	// calculate the interior control points for each patch
	vec3d y1, y2, a0, a3, b0, b1, b2, b3;
	double k0, k1, h0, h1;
	for (i=0; i<NP; ++i)
	{
		GPatch& p = m_GPatch[i];
		for (j=0; j<4; ++j)
		{
			GNode& n0 = m_GNode[ p.m_node[j] ];
			GNode& n3 = m_GNode[ p.m_node[(j+1)%4] ];
			y0 = n0.m_r; m0 = n0.m_n;
			y3 = n3.m_r; m3 = n3.m_n;
			y1 = p.m_ye[j][0];
			y2 = p.m_ye[j][1];

			a0 = m0^(y3-y0); a0.Normalize();
			a3 = m3^(y3-y0); a3.Normalize();

			b0 = p.m_ye[(j+3)%4][1] - y0;
			b3 = p.m_ye[(j+1)%4][0] - y3;

			c0 = y1 - y0;
			c1 = y2 - y1;
			c2 = y3 - y2;

			k0 = a0*b0;	h0 = (c0*b0)/(c0*c0);
			k1 = a3*b3;	h1 = (c2*b3)/(c2*c2);

			b1 = (a0*(k1 + k0) + a3*k0 + c1*(2.0*h0) + c0*h1)/3;
			b2 = (a3*(k1 + k0) + a0*k1 + c1*(2.0*h1) + c2*h0)/3;

			p.m_yi[j][0] = y1 + b1;
			p.m_yi[j][1] = y2 + b2;
		}
	}
}

vec3d FEGregoryPatch::EvalPatch(FEGregoryPatch::GPatch &p, double r, double s)
{
	// calculate internal control points
	vec3d x[4][4];
	x[0][0] = m_GNode[p.m_node[0]].m_r;
	x[3][0] = m_GNode[p.m_node[1]].m_r;
	x[3][3] = m_GNode[p.m_node[2]].m_r;
	x[0][3] = m_GNode[p.m_node[3]].m_r;

	x[1][0] = p.m_ye[0][0]; x[2][0] = p.m_ye[0][1];
	x[3][1] = p.m_ye[1][0]; x[3][2] = p.m_ye[1][1];
	x[2][3] = p.m_ye[2][0]; x[1][3] = p.m_ye[2][1];
	x[0][2] = p.m_ye[3][0]; x[0][1] = p.m_ye[3][1];

	x[1][1] = (r+s   == 0? vec3d(0,0,0) : (p.m_yi[0][0]*(r  ) + p.m_yi[3][1]*(s  ))/(r+s));
	x[2][1] = (1-r+s == 0? vec3d(0,0,0) : (p.m_yi[0][1]*(1-r) + p.m_yi[1][0]*(s  ))/(1-r+s));
	x[1][2] = (r+1-s == 0? vec3d(0,0,0) : (p.m_yi[2][1]*(r  ) + p.m_yi[3][0]*(1-s))/(r+1-s));
	x[2][2] = (2-r-s == 0? vec3d(0,0,0) : (p.m_yi[2][0]*(1-r) + p.m_yi[1][1]*(1-s))/(2-r-s));

	// calculate Bezier coefficients
	double B[2][4];
	B[0][0] = (1-r)*(1-r)*(1-r);
	B[0][1] = 3*r*(1-r)*(1-r);
	B[0][2] = 3*r*r*(1-r);
	B[0][3] = r*r*r;

	B[1][0] = (1-s)*(1-s)*(1-s);
	B[1][1] = 3*s*(1-s)*(1-s);
	B[1][2] = 3*s*s*(1-s);
	B[1][3] = s*s*s;

	// calculate the final position
	int i, j;
	vec3d y(0,0,0);
	for (i=0; i<4; ++i)
		for (j=0; j<4; ++j) y += x[i][j]*(B[0][i]*B[1][j]);

	return y;
}

void FEGregoryPatch::BuildFaces(FSMesh* pm)
{
	int faces = pm->Elements();
	pm->Create(0,0,faces);
	for (int i=0; i<faces; ++i)
	{
		FSFace& f = pm->Face(i);
		FSElement& e = pm->Element(i);
		f.SetType(FE_FACE_QUAD4);
		f.n[0] = e.m_node[0];
		f.n[1] = e.m_node[1];
		f.n[2] = e.m_node[2];
		f.n[3] = e.m_node[3];
		f.m_gid = e.m_gid;
		e.m_gid = 0;
	}
}
