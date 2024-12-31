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
#include <GeomLib/GPrimitive.h>
#include <GLLib/GLMesh.h>
#include "FEHollowSphere.h"

//=============================================================================
// GHollowSphere
//=============================================================================

GHollowSphere::GHollowSphere() : GPrimitive(GHOLLOW_SPHERE)
{
	m_Ri = 0.5;
	m_Ro = 1;

	AddDoubleParam(m_Ri, "Ri", "Inner radius");
	AddDoubleParam(m_Ro, "Ro", "Outer radius");

	SetFEMesher(new FEHollowSphere(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GHollowSphere::CreateDefaultMesher()
{
	return new FEHollowSphere(this);
}

//-----------------------------------------------------------------------------
bool GHollowSphere::Update(bool b)
{
	double R0 = GetFloatValue(RIN);
	double R1 = GetFloatValue(ROUT);

	m_Node[0]->LocalPosition() = vec3d( R1,   0,  0);
	m_Node[1]->LocalPosition() = vec3d(  0,  R1,  0);
	m_Node[2]->LocalPosition() = vec3d(-R1,   0,  0);
	m_Node[3]->LocalPosition() = vec3d(  0, -R1,  0);
	m_Node[4]->LocalPosition() = vec3d(  0,   0,-R1);
	m_Node[5]->LocalPosition() = vec3d(  0,   0, R1);

	m_Node[ 6]->LocalPosition() = vec3d( R0,   0,  0);
	m_Node[ 7]->LocalPosition() = vec3d(  0,  R0,  0);
	m_Node[ 8]->LocalPosition() = vec3d(-R0,   0,  0);
	m_Node[ 9]->LocalPosition() = vec3d(  0, -R0,  0);
	m_Node[10]->LocalPosition() = vec3d(  0,   0,-R0);
	m_Node[11]->LocalPosition() = vec3d(  0,   0, R0);

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------
void GHollowSphere::Create()
{
	SetRenderMesh(new GLMesh());

	assert(m_Node.empty());
	for (int i=0; i<12; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// build the edges
	int ET[24][2] = 
	{
		{0,1},{1,2},{2,3},{3,0},{0,5},{1,5},{2,5},{3,5},{0,4},{1,4},{2,4},{3,4},
		{6,7},{7,8},{8,9},{9,6},{6,11},{7,11},{8,11},{9,11},{6,10},{7,10},{8,10},{9,10}
	};
	assert(m_Edge.empty());
	for (int i=0; i<24; ++i)
	{
		GEdge* e = new GEdge(this);
		e->m_node[0] = ET[i][0];
		e->m_node[1] = ET[i][1];
		AddEdge(e);
	}

	// build the parts
	assert(m_Part.empty());
	AddSolidPart();

	// build the faces
	int FT[16][3] = {
		{0, 4, 1},
		{1, 4, 2},
		{2, 4, 3},
		{3, 4, 0},
		{0, 1, 5},
		{1, 2, 5},
		{2, 3, 5},
		{3, 0, 5},
		{6,10, 7},
		{7,10, 8},
		{8,10, 9},
		{9,10, 6},
		{6, 7,11},
		{7, 8,11},
		{8, 9,11},
		{9, 6,11}
	};
	assert(m_Face.empty());
	m_Face.reserve(16);
	for (int i=0; i<16; ++i)
	{
		GFace* f = new GFace(this);
		f->m_node.resize(4);
		f->m_node[0] = FT[i][0];
		f->m_node[1] = FT[i][1];
		f->m_node[2] = FT[i][2];
		f->m_node[3] = -1;
		f->m_nPID[0] = 0;
		AddSurface(f);
	}

	Update();
}

//-----------------------------------------------------------------------------
int GHollowSphere::NodeIndex(int i, int j, int ND, int NZ)
{
	if (i==0) return 0;
	if (i==NZ) return 1+ND*(NZ-1);
	return (1 + (i-1)*ND + j%ND);
}

//-----------------------------------------------------------------------------
void GHollowSphere::BuildGMesh()
{
	double R0 = GetFloatValue(RIN);
	double R1 = GetFloatValue(ROUT);

	// MAKE SURE NZ and ND are EVEN!
	int NZ = 64;
	int ND = 32;

	int NN = 2*(2 + ND*(NZ-1));
	int NF = 2*(2*ND + (NZ-2)*(2*ND));
	int NE = 2*(ND + 4*NZ);

	GLMesh& m = *GetRenderMesh();
	bool bempty = m.IsEmpty();
	m.Create(NN, NF, NE);

	// ------- nodes --------
	// top/bottom
	m.Node(0     ).r = vec3f(0,0,-R1);
	m.Node(NN/2-1).r = vec3f(0,0, R1);
	m.Node(NN/2  ).r = vec3f(0,0,-R0);
	m.Node(NN-1  ).r = vec3f(0,0, R0);

	int i, j;
	for (i=1; i<NZ; ++i)
	{
		double z = -1 + i*(2.0)/NZ;
		for (j=0; j<ND; ++j)
		{
			double w = 2*j*PI/ND;
			double cw = cos(w);
			double sw = sin(w);
			double d = R1*sqrt(1 - z*z);

			GLMesh::NODE& n = m.Node((i-1)*ND + j + 1);
			n.r = vec3f(d*cw, d*sw, R1*z);
		}
	}

	for (i=1; i<NZ; ++i)
	{
		double z = -1 + i*(2.0)/NZ;
		for (j=0; j<ND; ++j)
		{
			double w = 2*j*PI/ND;
			double cw = cos(w);
			double sw = sin(w);
			double d = R0*sqrt(1 - z*z);

			GLMesh::NODE& n = m.Node(NN/2 + (i-1)*ND + j + 1);
			n.r = vec3f(d*cw, d*sw, R0*z);
		}
	}


	if (bempty)
	{
		// ============ outer =================
		// ------- faces --------
		// bottom
		int n = 0;
		for (i=0; i<ND; ++i)
		{
			GLMesh::FACE& f = m.Face(n++);
			f.n[0] = 0;
			f.n[1] = (i+1)%ND + 1;
			f.n[2] = i + 1;
			f.pid = 4*i/ND;
			f.sid = 0;
		}
	
		// middle faces
		for (i=0; i<NZ-2; ++i)
		{
			for (j=0; j<ND; ++j)
			{
				GLMesh::FACE& f1 = m.Face(n++);
				GLMesh::FACE& f2 = m.Face(n++);

				int m[4] = {1+i*ND+j, 1+i*ND+(j+1)%ND, 1+(i+1)*ND+j, 1+(i+1)*ND+(j+1)%ND};

				int nb = 4*(2*(i+1)/NZ);

				f1.n[0] = m[0];
				f1.n[1] = m[1];
				f1.n[2] = m[3];
				f1.pid = nb + 4*j/ND;
				f1.sid = 0;

				f2.n[0] = m[3];
				f2.n[1] = m[2];
				f2.n[2] = m[0];
				f2.pid = nb + 4*j/ND;
				f2.sid = 0;
			}
		}

		// top
		for (i=0; i<ND; ++i)
		{
			GLMesh::FACE& f = m.Face(n++);
			f.n[0] = NN/2-1;
			f.n[1] = 1 + (NZ-2)*ND + i;
			f.n[2] = 1 + (NZ-2)*ND + (i+1)%ND;
			f.pid = 4 + 4*i/ND;
			f.sid = 0;
		}

		// ============ inner =================
		// ------- faces --------
		// bottom
		for (i=0; i<ND; ++i)
		{
			GLMesh::FACE& f = m.Face(n++);
			f.n[2] = NN/2 + 0;
			f.n[1] = NN/2 + (i+1)%ND + 1;
			f.n[0] = NN/2 + i + 1;
			f.pid = 8 + 4*i/ND;
			f.sid = 1;
		}
	
		// middle faces
		for (i=0; i<NZ-2; ++i)
		{
			for (j=0; j<ND; ++j)
			{
				GLMesh::FACE& f1 = m.Face(n++);
				GLMesh::FACE& f2 = m.Face(n++);

				int m[4] = {NN/2 + 1+i*ND+j, NN/2 + 1+i*ND+(j+1)%ND, NN/2 + 1+(i+1)*ND+j, NN/2 + 1+(i+1)*ND+(j+1)%ND};

				int nb = 8 + 4*(2*(i+1)/NZ);

				f1.n[2] = m[0];
				f1.n[1] = m[1];
				f1.n[0] = m[3];
				f1.pid = nb + 4*j/ND;
				f1.sid = 1;

				f2.n[2] = m[3];
				f2.n[1] = m[2];
				f2.n[0] = m[0];
				f2.pid = nb + 4*j/ND;
				f2.sid = 1;
			}
		}

		// top
		for (i=0; i<ND; ++i)
		{
			GLMesh::FACE& f = m.Face(n++);
			f.n[2] = NN/2 + NN/2-1;
			f.n[1] = NN/2 + 1 + (NZ-2)*ND + i;
			f.n[0] = NN/2 + 1 + (NZ-2)*ND + (i+1)%ND;
			f.pid = 12 + 4*i/ND;
			f.sid = 1;
		}

	// ---- edges -----
		n = 0;
		for (int k=0; k<2; ++k)
		{
			int n0 = k*NN/2;
			int e0 = k*12;
			for (i=0; i<ND; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(NZ/2, i, ND, NZ);
				e.n[1] = n0 + NodeIndex(NZ/2, i+1, ND, NZ);
				e.pid = e0 + 4*i/ND;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(NZ/2+i, 0, ND, NZ);
				e.n[1] = n0 + NodeIndex(NZ/2+i+1, 0, ND, NZ);
				e.pid = e0 + 4;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(NZ/2+i  , ND/4, ND, NZ);
				e.n[1] = n0 + NodeIndex(NZ/2+i+1, ND/4, ND, NZ);
				e.pid = e0 + 5;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(NZ/2+i  , 2*ND/4, ND, NZ);
				e.n[1] = n0 + NodeIndex(NZ/2+i+1, 2*ND/4, ND, NZ);
				e.pid = e0 + 6;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(NZ/2+i  , 3*ND/4, ND, NZ);
				e.n[1] = n0 + NodeIndex(NZ/2+i+1, 3*ND/4, ND, NZ);
				e.pid = e0 + 7;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(i  , 0, ND, NZ);
				e.n[1] = n0 + NodeIndex(i+1, 0, ND, NZ);
				e.pid = e0 + 8;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(i  , ND/4, ND, NZ);
				e.n[1] = n0 + NodeIndex(i+1, ND/4, ND, NZ);
				e.pid = e0 + 9;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(i  , 2*ND/4, ND, NZ);
				e.n[1] = n0 + NodeIndex(i+1, 2*ND/4, ND, NZ);
				e.pid = e0 + 10;
			}

			for (i=0; i<NZ/2; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(i  , 3*ND/4, ND, NZ);
				e.n[1] = n0 + NodeIndex(i+1, 3*ND/4, ND, NZ);
				e.pid = e0 + 11;
			}
		}
	}
	m.Update();
}
