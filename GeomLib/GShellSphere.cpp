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

#include "GPrimitive.h"
#include <MeshTools/FEShellSphere.h>

/*
//=============================================================================
// GShellSphere
//=============================================================================

GShellSphere::GShellSphere(FSModel* ps) : GPrimitive(ps, GSHELL_SPHERE)
{
	m_R = 1;

	m_Param.AddDoubleParam(m_R, "R", "radius");

	m_pMesher = new FEShellSphere(this);

	Create();
}

//-----------------------------------------------------------------------------
void GShellSphere::Update(bool b)
{
	double R = m_Param.GetFloatValue(RAD);

	m_Node[0].m_r = vec3d( R,  0, 0);
	m_Node[1].m_r = vec3d( 0,  R, 0);
	m_Node[2].m_r = vec3d(-R,  0, 0);
	m_Node[3].m_r = vec3d( 0, -R, 0);
	m_Node[4].m_r = vec3d( 0,  0,-R);
	m_Node[5].m_r = vec3d( 0,  0, R);

	BuildGMesh();
}

//-----------------------------------------------------------------------------
void GShellSphere::Create()
{
	assert(m_pGMesh == 0);
	m_pGMesh = new GLMesh();

	int i;
	assert(m_Node.empty());
	m_Node.reserve(6);
	for (i=0; i<6; ++i)
	{
		GNode n(this);
		n.SetID(GNode::CreateUniqueID());
		n.Select(false);
		n.SetLocalID(i);
		m_Node.push_back(n);
	}

	// build the edges
	int ET[12][2] = {{0,1},{1,2},{2,3},{3,0},{0,5},{1,5},{2,5},{3,5},{0,4},{1,4},{2,4},{3,4}};
	assert(m_Edge.empty());
	m_Edge.reserve(12);
	for (i=0; i<12; ++i)
	{
		GEdge e(this);
		e.SetID(GEdge::CreateUniqueID());
		e.Select(false);
		e.SetLocalID(i);
		e.m_node[0] = ET[i][0];
		e.m_node[1] = ET[i][1];
		m_Edge.push_back(e);
	}

	// build the parts
	assert(m_Part.empty());
	m_Part.reserve(1);
	GPart p(this);
	p.SetID(GPart::CreateUniqueID());
	p.SetMaterial(0);
	p.Select(false);
	p.SetLocalID(0);
	m_Part.push_back(p);

	// build the faces
	int FT[8][3] = {
		{0, 4, 1},
		{1, 4, 2},
		{2, 4, 3},
		{3, 4, 0},
		{0, 1, 5},
		{1, 2, 5},
		{2, 3, 5},
		{3, 0, 5},
	};
	assert(m_Face.empty());
	m_Face.reserve(8);
	for (i=0; i<8; ++i)
	{
		GFace f(this);
		f.SetID(GFace::CreateUniqueID());
		f.Select(false);
		f.SetLocalID(i);
		f.m_node.resize(4);
		f.m_node[0] = FT[i][0];
		f.m_node[1] = FT[i][1];
		f.m_node[2] = FT[i][2];
		f.m_node[3] = -1;
		f.m_nPID[0] = 0;
		f.m_nPID[1] = -1;
		m_Face.push_back(f);
	}

	Update();
}

//-----------------------------------------------------------------------------
int GShellSphere::NodeIndex(int i, int j, int ND, int NZ)
{
	if (i==0) return 0;
	if (i==NZ) return 1+ND*(NZ-1);
	return (1 + (i-1)*ND + j%ND);
}

//-----------------------------------------------------------------------------
void GShellSphere::BuildGMesh()
{
	double R = m_Param.GetFloatValue(RAD);

	// MAKE SURE NZ and ND are EVEN!
	int NZ = 64;
	int ND = 32;

	int NN = 2 + ND*(NZ-1);
	int NF = 2*ND + (NZ-2)*(2*ND);
	int NE = ND + 4*NZ;

	GLMesh& m = *m_pGMesh;
	bool bempty = m.IsEmpty();
	m.Create(NN, NF, NE);

	// ------- nodes --------
	// top/bottom
	m.Node(0).r = vec3d(0,0,-R);
	m.Node(NN-1).r = vec3d(0,0,R);

	int i, j;
	for (i=1; i<NZ; ++i)
	{
		double z = -1 + i*(2.0)/NZ;
		for (j=0; j<ND; ++j)
		{
			double w = 2*j*PI/ND;
			double cw = cos(w);
			double sw = sin(w);
			double d = R*sqrt(1 - z*z);

			GLMesh::NODE& n = m.Node((i-1)*ND + j + 1);
			n.r = vec3d(d*cw, d*sw, R*z);
		}
	}

	if (bempty)
	{
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

				f2.n[0] = m[3];
				f2.n[1] = m[2];
				f2.n[2] = m[0];
				f2.pid = nb + 4*j/ND;
			}
		}

		// top
		for (i=0; i<ND; ++i)
		{
			GLMesh::FACE& f = m.Face(n++);
			f.n[0] = NN-1;
			f.n[1] = 1 + (NZ-2)*ND + i;
			f.n[2] = 1 + (NZ-2)*ND + (i+1)%ND;
			f.pid = 4 + 4*i/ND;
		}

		assert(n == NF);

		// ---- edges -----
		n = 0;
		for (i=0; i<ND; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(NZ/2, i, ND, NZ);
			e.n[1] = NodeIndex(NZ/2, i+1, ND, NZ);
			e.pid = 4*i/ND;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(NZ/2+i, 0, ND, NZ);
			e.n[1] = NodeIndex(NZ/2+i+1, 0, ND, NZ);
			e.pid = 4;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(NZ/2+i  , ND/4, ND, NZ);
			e.n[1] = NodeIndex(NZ/2+i+1, ND/4, ND, NZ);
			e.pid = 5;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(NZ/2+i  , 2*ND/4, ND, NZ);
			e.n[1] = NodeIndex(NZ/2+i+1, 2*ND/4, ND, NZ);
			e.pid = 6;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(NZ/2+i  , 3*ND/4, ND, NZ);
			e.n[1] = NodeIndex(NZ/2+i+1, 3*ND/4, ND, NZ);
			e.pid = 7;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , 0, ND, NZ);
			e.n[1] = NodeIndex(i+1, 0, ND, NZ);
			e.pid = 8;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , ND/4, ND, NZ);
			e.n[1] = NodeIndex(i+1, ND/4, ND, NZ);
			e.pid = 9;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , 2*ND/4, ND, NZ);
			e.n[1] = NodeIndex(i+1, 2*ND/4, ND, NZ);
			e.pid = 10;
		}

		for (i=0; i<NZ/2; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , 3*ND/4, ND, NZ);
			e.n[1] = NodeIndex(i+1, 3*ND/4, ND, NZ);
			e.pid = 11;
		}
	}

	m.UpdateNormals();
	m.Update();
}
*/
