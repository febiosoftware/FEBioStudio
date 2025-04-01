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
#include <MeshTools/FETruncatedEllipsoid.h>
#include <GLLib/GLMesh.h>

GTruncatedEllipsoid::GTruncatedEllipsoid() : GPrimitive(GTRUNC_ELLIPSOID)
{
	SetFEMesher(new FETruncatedEllipsoid(this));
	m_Ra = m_Rb = m_Rc = 1.0;
	m_wt = 0.1;
	m_vend = 0.0;

	AddDoubleParam(m_Ra, "a", "a-radius");
	AddDoubleParam(m_Rb, "b", "b-radius");
	AddDoubleParam(m_Rc, "c", "c-radius");
	AddDoubleParam(m_wt, "wt", "wt-radius");
	AddDoubleParam(m_vend, "vend", "end-radius");

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GTruncatedEllipsoid::CreateDefaultMesher()
{
	return new FETruncatedEllipsoid(this);
}

bool GTruncatedEllipsoid::Update(bool b)
{
	double Ra = GetFloatValue(RA);
	double Rb = GetFloatValue(RB);
	double Rc = GetFloatValue(RC);
	double wt = GetFloatValue(WT);
	double uend = GetFloatValue(VEND);

	double w = PI*uend/180.0;
	double cw = cos(uend);
	double sw = sin(uend);

	m_Node[0]->LocalPosition() = vec3d( 0,   0,  -Rc-wt);
	m_Node[1]->LocalPosition() = vec3d( 0,   0,  -Rc+wt);
	m_Node[2]->LocalPosition() = vec3d( (Ra+wt)*cw,  0,  (Rc+wt)*sw);
	m_Node[3]->LocalPosition() = vec3d( (Ra-wt)*cw,  0,  (Rc-wt)*sw);
	m_Node[4]->LocalPosition() = vec3d( 0, (Rb+wt)*cw,  (Rc+wt)*sw);
	m_Node[5]->LocalPosition() = vec3d( 0, (Rb-wt)*cw,  (Rc-wt)*sw);
	m_Node[6]->LocalPosition() = vec3d( -(Ra+wt)*cw,  0,  (Rc+wt)*sw);
	m_Node[7]->LocalPosition() = vec3d( -(Ra-wt)*cw,  0,  (Rc-wt)*sw);
	m_Node[8]->LocalPosition() = vec3d( 0, -(Rb+wt)*cw,  (Rc+wt)*sw);
	m_Node[9]->LocalPosition() = vec3d( 0, -(Rb-wt)*cw,  (Rc-wt)*sw);

	return GObject::Update();
}

void GTruncatedEllipsoid::Create()
{
	SetRenderMesh(new GLMesh());

	assert(m_Node.empty());
	for (int i=0; i<10; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// build the edges
	int ET[20][2] = 
	{
		{0,2},{0,4},{0,6},{0,8},
		{1,3},{1,5},{1,7},{1,9},
		{2,3},{4,5},{6,7},{8,9},
		{2,4},{4,6},{6,8},{8,2},
		{3,5},{5,7},{7,9},{9,3}
	};
	assert(m_Edge.empty());
	for (int i=0; i<20; ++i)
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
	int FT[12][4] = {
		{0, 4, 2, -1},
		{0, 6, 4, -1},
		{0, 8, 6, -1},
		{0, 2, 8, -1},
		{1, 5, 3, -1},
		{1, 7, 5, -1},
		{1, 9, 7, -1},
		{1, 9, 3, -1},
		{2, 4, 5, 3},
		{4, 6, 7, 5},
		{6, 8, 9, 7},
		{8, 2, 3, 9},
	};
	assert(m_Face.empty());
	m_Face.reserve(12);
	for (int i=0; i<8; ++i)
	{
		GFace* f = new GFace(this);
		f->m_node.resize(3);
		f->m_node[0] = FT[i][0];
		f->m_node[1] = FT[i][1];
		f->m_node[2] = FT[i][2];
		f->m_nPID[0] = 0;
		AddSurface(f);
	}

	for (int i = 8; i < 12; ++i)
	{
		GFace* f = new GFace(this);
		f->m_node.resize(4);
		f->m_node[0] = FT[i][0];
		f->m_node[1] = FT[i][1];
		f->m_node[2] = FT[i][2];
		f->m_node[3] = FT[i][3];
		f->m_nPID[0] = 0;
		AddSurface(f);
	}

	Update();
}

void GTruncatedEllipsoid::BuildGMesh()
{
	double Ra = GetFloatValue(RA);
	double Rb = GetFloatValue(RB);
	double Rc = GetFloatValue(RC);
	double wt = GetFloatValue(WT);
	double uend = GetFloatValue(VEND);

	int NS = 64;	// must be divisable by 4
	int NZ = 32;

	int NN = 2*(1 + NS*NZ);
	int NF = 2*(NS+NS*2*(NZ-1)) + 2*NS;
	int NE = 8*NZ+2*NS+4;

	GLMesh& m = *GetRenderMesh();
	bool bempty = m.IsEmpty();
	m.Create(NN, NF, NE);

	// --- Nodes ---
	double u0 = -PI/2;
	double u1 = PI*uend/180;
	double v0 = 0;
	double v1 = 2*PI;

	m.Node(0   ).r = vec3f( 0,   0,  -Rc-wt);
	m.Node(NN/2).r = vec3f( 0,   0,  -Rc+wt);

	// outside
	for (int j=1; j<=NZ;++j)
	{
		double u = u0 + j*(u1 - u0)/NZ;
		double cu = cos(u);
		double su = sin(u);
		for (int i=0; i<NS; ++i)
		{
			double v = v0 + i*(v1 - v0)/NS;
			double cv = cos(v);
			double sv = sin(v);

			double x = (Ra + wt)*cu*cv;
			double y = (Rb + wt)*cu*sv;
			double z = (Rc + wt)*su;

			GLMesh::NODE& n = m.Node((j-1)*NS+i+1);
			n.r = vec3f(x, y, z);
		}
	}

	// inside
	for (int j=1; j<=NZ;++j)
	{
		double u = u0 + j*(u1 - u0)/NZ;
		double cu = cos(u);
		double su = sin(u);
		for (int i=0; i<NS; ++i)
		{
			double v = v0 + i*(v1 - v0)/NS;
			double cv = cos(v);
			double sv = sin(v);

			double x = (Ra - wt)*cu*cv;
			double y = (Rb - wt)*cu*sv;
			double z = (Rc - wt)*su;

			GLMesh::NODE& n = m.Node((j-1)*NS + i + 1 + NN/2);
			n.r = vec3f(x, y, z);
		}
	}


	if (bempty)
	{
		int n = 0;
		// outside-bottom
		for (int i=0; i<NS; ++i)
		{
			GLMesh::FACE& f = m.Face(n++);
			f.n[0] = NodeIndex(i, 0, NS); assert(f.n[0] < NN);
			f.n[2] = NodeIndex(i, 1, NS); assert(f.n[1] < NN);
			f.n[1] = NodeIndex(i+1, 1, NS); assert(f.n[2] < NN);
			f.pid = 4*i/NS;
			f.sid = 0;
		}

		// outside-side
		for (int i=1; i<NZ; ++i)
		{
			for (int j=0; j<NS; ++j)
			{
				GLMesh::FACE& f1 = m.Face(n++);
				GLMesh::FACE& f2 = m.Face(n++);

				f1.n[0] = NodeIndex(j  , i  , NS); assert(f1.n[0] < NN);
				f1.n[2] = NodeIndex(j  , i+1, NS); assert(f1.n[1] < NN);
				f1.n[1] = NodeIndex(j+1, i+1, NS); assert(f1.n[2] < NN);
				f1.pid = 4*j/NS;
				f1.sid = 0;

				f2.n[0] = NodeIndex(j  , i  , NS); assert(f2.n[0] < NN);
				f2.n[2] = NodeIndex(j+1, i+1, NS); assert(f2.n[1] < NN);
				f2.n[1] = NodeIndex(j+1, i  , NS); assert(f2.n[2] < NN);
				f2.pid = 4*j/NS;
				f2.sid = 0;
			}
		}

		// inside-bottom
		for (int i=0; i<NS; ++i)
		{
			GLMesh::FACE& f = m.Face(n++);
			f.n[0] = NodeIndex(i, 0, NS) + NN/2; assert(f.n[0] < NN);
			f.n[1] = NodeIndex(i, 1, NS) + NN/2; assert(f.n[1] < NN);
			f.n[2] = NodeIndex(i+1, 1, NS) + NN/2; assert(f.n[2] < NN);
			f.pid = 4*i/NS + 4;
			f.sid = 1;
		}

		// inside-side
		for (int i=1; i<NZ; ++i)
		{
			for (int j=0; j<NS; ++j)
			{
				GLMesh::FACE& f1 = m.Face(n++);
				GLMesh::FACE& f2 = m.Face(n++);

				f1.n[0] = NodeIndex(j  , i  , NS) + NN/2; assert(f1.n[0] < NN);
				f1.n[1] = NodeIndex(j  , i+1, NS) + NN/2; assert(f1.n[1] < NN);
				f1.n[2] = NodeIndex(j+1, i+1, NS) + NN/2; assert(f1.n[2] < NN);
				f1.pid = 4*j/NS + 4;
				f1.sid = 1;

				f2.n[0] = NodeIndex(j  , i  , NS) + NN/2; assert(f2.n[0] < NN);
				f2.n[1] = NodeIndex(j+1, i+1, NS) + NN/2; assert(f2.n[1] < NN);
				f2.n[2] = NodeIndex(j+1, i  , NS) + NN/2; assert(f2.n[2] < NN);
				f2.pid = 4*j/NS + 4;
				f2.sid = 1;
			}
		}

		// top
		for (int i=0; i<NS; ++i)
		{
			GLMesh::FACE& f1 = m.Face(n++);
			GLMesh::FACE& f2 = m.Face(n++);
			
			f1.n[0] = NodeIndex(i, NZ, NS); assert(f1.n[0] < NN);
			f1.n[1] = NodeIndex(i+1, NZ, NS) + NN/2; assert(f1.n[1] < NN);
			f1.n[2] = NodeIndex(i, NZ, NS) + NN/2; assert(f1.n[2] < NN);
			f1.pid = 4*i/NS + 8;
			f1.sid = 2;

			f2.n[0] = NodeIndex(i, NZ, NS); assert(f2.n[0] < NN);
			f2.n[1] = NodeIndex(i+1, NZ, NS); assert(f2.n[1] < NN);
			f2.n[2] = NodeIndex(i+1, NZ, NS) + NN/2; assert(f2.n[2] < NN);
			f2.pid = 4*i/NS + 8;
			f2.sid = 2;
		}

		// --- edges ---
		n = 0;
		for (int k=0; k<2; k++)
		{
			int n0 = k*NN/2;
			for (int j=0; j<4; j++)
			{
				for (int i=0; i<NZ; ++i)
				{
					GLMesh::EDGE& e = m.Edge(n++);
					e.n[0] = n0 + NodeIndex(j*NS/4, i, NS);
					e.n[1] = n0 + NodeIndex(j*NS/4, i+1, NS);
					e.pid = k*4+j;
				}
			}
		}

		for (int i=0; i<4; ++i)
		{
			GLMesh::EDGE& e = m.Edge(n++);
			e.n[0] = NodeIndex(i*NS/4, NZ, NS);
			e.n[1] = NodeIndex(i*NS/4, NZ, NS) + NN/2;
			e.pid = 8+i;
		}

		for (int k=0; k<2; k++)
		{
			int n0 = k*NN/2;
			for (int i=0; i<NS; ++i)
			{
				GLMesh::EDGE& e = m.Edge(n++);
				e.n[0] = n0 + NodeIndex(i, NZ, NS);
				e.n[1] = n0 + NodeIndex(i+1, NZ, NS);
				e.pid = 12+k*4+i/(NS/4);
			}
		}
	}
	m.Update();
}

int GTruncatedEllipsoid::NodeIndex(int i, int j, int NS)
{
	if (j==0) return 0;
	return (j-1)*NS+(i%NS)+1;
}

