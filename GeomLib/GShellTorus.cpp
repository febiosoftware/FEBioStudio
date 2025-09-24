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
#include <MeshTools/FEShellTorus.h>

/*
//=============================================================================
// GShellTorus
//=============================================================================

GShellTorus::GShellTorus(FSModel* ps) : GPrimitive(ps, GSHELL_TORUS)
{
	m_R0 = 1;
	m_R1 = 0.02;

	m_Param.AddDoubleParam(m_R0, "R0", "outer radius");
	m_Param.AddDoubleParam(m_R1, "R1", "inner radius");

	m_pMesher = new FEShellTorus(this);

	Create();
}

//----------------------------------------------------------------------------
void GShellTorus::Update(bool b)
{
	double R0 = m_Param.GetFloatValue(RIN);
	double R1 = m_Param.GetFloatValue(ROUT);

	m_Node[0].m_r = vec3d(0, -R0-R1,   0);
	m_Node[1].m_r = vec3d(0, -R0   , -R1);
	m_Node[2].m_r = vec3d(0, -R0+R1,   0);
	m_Node[3].m_r = vec3d(0, -R0   ,  R1);

	m_Node[4].m_r = vec3d(R0+R1, 0,   0);
	m_Node[5].m_r = vec3d(R0   , 0, -R1);
	m_Node[6].m_r = vec3d(R0-R1, 0,   0);
	m_Node[7].m_r = vec3d(R0   , 0,  R1);

	m_Node[ 8].m_r = vec3d(0, R0+R1,   0);
	m_Node[ 9].m_r = vec3d(0, R0   , -R1);
	m_Node[10].m_r = vec3d(0, R0-R1,   0);
	m_Node[11].m_r = vec3d(0, R0   ,  R1);

	m_Node[12].m_r = vec3d(-R0-R1, 0,   0);
	m_Node[13].m_r = vec3d(-R0   , 0, -R1);
	m_Node[14].m_r = vec3d(-R0+R1, 0,   0);
	m_Node[15].m_r = vec3d(-R0   , 0,  R1);

	BuildGMesh();
}

//----------------------------------------------------------------------------
void GShellTorus::Create()
{
	assert(m_pGMesh == 0);
	m_pGMesh = new GLMesh();

	int i;
	assert(m_Node.empty());
	m_Node.reserve(16);
	for (i=0; i<16; ++i)
	{
		GNode n(this);
		n.SetID(GNode::CreateUniqueID());
		n.Select(false);
		n.SetLocalID(i);
		m_Node.push_back(n);
	}

	// build the edges
	int ET[32][2] = {
		{ 0, 4},{ 4, 8},{ 8,12},{12, 0},
		{ 1, 5},{ 5, 9},{ 9,13},{13, 1},
		{ 2, 6},{ 6,10},{10,14},{14, 2},
		{ 3, 7},{ 7,11},{11,15},{15, 3},
		{ 0, 1},{ 1, 2},{ 2, 3},{ 3, 0},
		{ 4, 5},{ 5, 6},{ 6, 7},{ 7, 4},
		{ 8, 9},{ 9,10},{10,11},{11, 8},
		{12,13},{13,14},{14,15},{15,12}
	};

	assert(m_Edge.empty());
	m_Edge.reserve(32);
	for (i=0; i<32; ++i)
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
	int FT[16][4] = {
		{0, 1, 5, 4},
		{1, 2, 6, 5},
		{2, 3, 7, 6},
		{3, 0, 4, 7},

		{4, 5,  9,  8},
		{5, 6, 10,  9},
		{6, 7, 11, 10},
		{7, 4,  8, 11},

		{ 8,  9, 13, 12},
		{ 9, 10, 14, 13},
		{10, 11, 15, 14},
		{11,  8, 12, 15},

		{12, 13, 1, 0},
		{13, 14, 2, 1},
		{14, 15, 3, 2},
		{15, 12, 0, 3}
	};
	assert(m_Face.empty());
	m_Face.reserve(16);
	for (i=0; i<16; ++i)
	{
		GFace f(this);
		f.SetID(GFace::CreateUniqueID());
		f.Select(false);
		f.SetLocalID(i);
		f.m_node.resize(4);
		f.m_node[0] = FT[i][0];
		f.m_node[1] = FT[i][1];
		f.m_node[2] = FT[i][2];
		f.m_node[3] = FT[i][3];
		f.m_nPID[0] = 0;
		f.m_nPID[1] = -1;
		m_Face.push_back(f);
	}

	Update();
}

//----------------------------------------------------------------------------
int GShellTorus::NodeIndex(int i, int j, int N0, int N1)
{
	return (i%N0)*N1 + j%N1;
}

//----------------------------------------------------------------------------
void GShellTorus::BuildGMesh()
{
	double R0 = m_Param.GetFloatValue(RIN);
	double R1 = m_Param.GetFloatValue(ROUT);

	// MAKE SURE N0 and N1 are EVEN!
	int N0 = 64;
	int N1 = 32;

	int NN = N0*N1;
	int NF = 2*N0*N1;
	int NE = 4*N0 + 4*N1;

	GLMesh& m = *m_pGMesh;
	bool bempty = m.IsEmpty();
	m.Create(NN, NF, NE);

	// ------- nodes --------
	int i, j;
	vec3d r;
	for (i=0; i<N0; ++i)
	{
		double w0 = 2.0*i*PI/N0 - PI*0.5;
		double cw0 = cos(w0), sw0 = sin(w0);
		for (j=0; j<N1; ++j)
		{
			GLMesh::NODE& n = m.Node(i*N1+j);

			double w1 = 2.0*PI - 2.0*j*PI/N1;
			
			r.x = R1*cos(w1) + R0;
			r.y = 0;
			r.z = R1*sin(w1);

			n.r.x = cw0*r.x;
			n.r.y = sw0*r.x;
			n.r.z = r.z;
		}
	}

	if (bempty)
	{
		// -------- faces --------
		int n = 0;
		for (i=0; i<N0; ++i)
		{
			for (j=0; j<N1; ++j)
			{
				GLMesh::FACE& f1 = m.Face(n++);
				GLMesh::FACE& f2 = m.Face(n++);

				int m[4] = {
					NodeIndex(i  ,j  , N0, N1), 
					NodeIndex(i+1,j  , N0, N1),
					NodeIndex(i  ,j+1, N0, N1),
					NodeIndex(i+1,j+1, N0, N1)
				};

				int nb = 4*(4*i/N0);

				f1.n[0] = m[0];
				f1.n[1] = m[2];
				f1.n[2] = m[3];
				f1.pid = nb + 4*j/N1;

				f2.n[0] = m[3];
				f2.n[1] = m[1];
				f2.n[2] = m[0];
				f2.pid = nb + 4*j/N1;
			}
		}

		// ------ edges ------
		n = 0;
		for (i=0; i<N0; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , 0, N0, N1);
			e.n[1] = NodeIndex(i+1, 0, N0, N1);
			e.pid = 4*i/N0;
		}

		for (i=0; i<N0; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , 3*N1/4, N0, N1);
			e.n[1] = NodeIndex(i+1, 3*N1/4, N0, N1);
			e.pid = 12 + 4*i/N0;
		}

		for (i=0; i<N0; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , 2*N1/4, N0, N1);
			e.n[1] = NodeIndex(i+1, 2*N1/4, N0, N1);
			e.pid = 8 + 4*i/N0;
		}

		for (i=0; i<N0; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i  , N1/4, N0, N1);
			e.n[1] = NodeIndex(i+1, N1/4, N0, N1);
			e.pid = 4 + 4*i/N0;
		}

		for (i=0; i<N1; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(0, i  , N0, N1);
			e.n[1] = NodeIndex(0, i+1, N0, N1);
			e.pid = 16 + 4*i/N1;
		}

		for (i=0; i<N1; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(N0/4, i  , N0, N1);
			e.n[1] = NodeIndex(N0/4, i+1, N0, N1);
			e.pid = 20 + 4*i/N1;
		}

		for (i=0; i<N1; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(2*N0/4, i  , N0, N1);
			e.n[1] = NodeIndex(2*N0/4, i+1, N0, N1);
			e.pid = 24 + 4*i/N1;
		}

		for (i=0; i<N1; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(3*N0/4, i  , N0, N1);
			e.n[1] = NodeIndex(3*N0/4, i+1, N0, N1);
			e.pid = 28 + 4*i/N1;
		}
	}

	m.UpdateNormals();
	m.Update();
}
*/
