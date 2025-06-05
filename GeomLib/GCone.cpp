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
#include <MeshTools/FECone.h>
#include <MeshLib/GMesh.h>


class GConeManipulator : public GObjectManipulator
{
public:
	GConeManipulator(GCone& cone) : GObjectManipulator(&cone), m_cone(cone) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		vec3d r = T.LocalToGlobal(pn->Position());
		r = m_cone.GetTransform().GlobalToLocal(r);

		double z = r.z; r.z = 0;
		double R = r.Length();

		if (m < 4)
		{
			m_cone.GetTransform().Translate(vec3d(0, 0, z));
			m_cone.SetHeight(m_cone.Height() - z);

			m_cone.SetBottomRadius(R);
		}
		else
		{
			m_cone.SetTopRadius(R);
			m_cone.SetHeight(z);
		}
		m_cone.Update();
	}

private:
	GCone& m_cone;
};

//=============================================================================
// GCone
//=============================================================================

GCone::GCone() : GPrimitive(GCONE)
{
	m_R0 = 1;
	m_R1 = 1;
	m_h  = 1;

	AddDoubleParam(m_R0, "R0", "Bottom radius");
	AddDoubleParam(m_R1, "R1", "Top radius");
	AddDoubleParam(m_h , "h" , "Height");

	SetFEMesher(CreateDefaultMesher());
	SetManipulator(new GConeManipulator(*this));

	Create();
}

double GCone::BottomRadius() const { return GetFloatValue(R0);}
double GCone::TopRadius() const { return GetFloatValue(R1);}
double GCone::Height() const { return GetFloatValue(H); }

void GCone::SetBottomRadius(double r0) { SetFloatValue(R0, r0);}
void GCone::SetTopRadius(double r1) { SetFloatValue(R1, r1);}
void GCone::SetHeight(double h) { SetFloatValue(H, h); }

//-----------------------------------------------------------------------------
FEMesher* GCone::CreateDefaultMesher()
{
	return new FECone();
}

//-----------------------------------------------------------------------------
void GCone::Create()
{
	GMesh* gmesh = new GMesh();
	SetRenderMesh(gmesh);

	// build the nodes
	assert(m_Node.empty());
	for (int i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// build the edges
	int ET[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	assert(m_Edge.empty());
	m_Edge.reserve(12);
	for (int i=0; i<12; ++i)
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
	int FT[6][4] = {
		{0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6},
		{3, 0, 4, 7}, {3, 2, 1, 0}, {4, 5, 6, 7}
	};
	assert(m_Face.empty());
	for (int i=0; i<6; ++i)
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

//-----------------------------------------------------------------------------
bool GCone::Update(bool b)
{
	double h = GetFloatValue(H);
	double r0 = GetFloatValue(R0);
	double r1 = GetFloatValue(R1);

	double x[8] = {r0,  0, -r0,  0,  r1,  0, -r1,  0};
	double y[8] = {0,  r0,  0, -r0,  0,  r1,  0, -r1};
	double z[8] = {0,  0,  0,  0,  h,  h,  h,  h};

	for (int i=0; i<8; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	return GObject::Update();
}

//-----------------------------------------------------------------------------
void GCone::BuildGMesh()
{
	int i, j;

	double h = GetFloatValue(H);
	double r0 = GetFloatValue(R0);
	double r1 = GetFloatValue(R1);
	double rmax = (r0>r1? r0:r1);

	int a = 1;
	if (rmax != 0.0) a = 1 + (int) (h/rmax);

	// ND must be a multiple of 4
	int ND = 36;
	int NH = 4*a;
	int NR = 12;

	GMesh& m = *GetRenderMesh();
	int NN0 = m.Nodes();
	int NF0 = m.Faces();
	int NE0 = m.Edges();

	int NN = 2 + 2*NR*ND + (NH+1)*ND;
	int NF = 2*ND + 4*(NR-1)*ND + 2*NH*ND;
	int NE = 2*ND + 4*NH;
	m.Create(NN, NF, NE);

	// --- create nodes ---
	m.Node(0   ).r = vec3f(0,0,0);
	m.Node(NN-1).r = vec3f(0,0,h);

	// bottom face
	int nn = 1;
	for (j=0; j<NR; ++j)
	{
		for (i=0; i<ND; ++i)
		{
			GMesh::NODE& n0 = m.Node(nn++);
			double w = 2.0*i*PI/ND;
			double cw = cos(w);
			double sw = sin(w);
			double r = (j+1)*r0/NR;
			n0.r = vec3f(r*cw, r*sw, 0);
		}
	}

	// side faces
	for (j=0; j<=NH; ++j)
	{
		double hj = j*h/NH;
		double rj = r0 + j*(r1 - r0)/NH;
		for (i=0; i<ND; ++i)
		{
			GMesh::NODE& n0 = m.Node(nn++);
		
			double w = 2.0*i*PI/ND;
			double cw = cos(w);
			double sw = sin(w);
			n0.r = vec3f(rj*cw, rj*sw, hj);
		}
	}

	// top face
	for (j=0; j<NR; ++j)
	{
		for (i=0; i<ND; ++i)
		{
			GMesh::NODE& n0 = m.Node(nn++);
		
			double w = 2.0*i*PI/ND;
			double cw = cos(w);
			double sw = sin(w);
			double r = (j+1)*r1/NR;
			n0.r = vec3f(r*cw, r*sw, h);
		}
	}
	assert(nn+1 == NN);

	if ((NN != NN0) || (NF != NF0) || (NE != NE0))
	{
		// --- create faces ---
		int nf = 0;

		// bottom face
		for (i=0; i<ND; ++i)
		{
			GMesh::FACE& f = m.Face(nf++);
			f.n[0] = 0;
			f.n[1] = 1 + (i+1)%ND;
			f.n[2] = 1 + i;
			f.pid = 4;
			f.sid = 0;
			f.fn = vec3f(0,0,-1);
			f.vn[0] = f.vn[1] = f.vn[2] = vec3f(0,0,-1);
		}

		for (j=0; j<NR-1; ++j)
		{
			for (i=0; i<ND; ++i)
			{
				GMesh::FACE& f1 = m.Face(nf++);
				f1.n[0] = 1 + j*ND + i;
				f1.n[1] = 1 + j*ND + (i+1)%ND;
				f1.n[2] = 1 + (j+1)*ND + (i+1)%ND;
				f1.pid = 4;
				f1.sid = 0;

				GMesh::FACE& f2 = m.Face(nf++);
				f2.n[0] = 1 + (j+1)*ND + (i+1)%ND;
				f2.n[1] = 1 + (j+1)*ND + i;
				f2.n[2] = 1 + j*ND + i;
				f2.pid = 4;
				f2.sid = 0;
			}
		}

		// side faces
		for (j=0; j<NH; ++j)
		{
			for (i=0; i<ND; ++i)
			{
				int n[4] = {i, (i+1)%ND, ND+i, ND+(i+1)%ND};

				GMesh::FACE& f1 = m.Face(nf++);
				f1.n[0] = NR*ND + j*ND + 1 + n[0];
				f1.n[1] = NR*ND + j*ND + 1 + n[1];
				f1.n[2] = NR*ND + j*ND + 1 + n[3];
				f1.pid = 4*i/ND;
				f1.sid = 1;

				GMesh::FACE& f2 = m.Face(nf++);
				f2.n[0] = NR*ND + j*ND + 1 + n[3];
				f2.n[1] = NR*ND + j*ND + 1 + n[2];
				f2.n[2] = NR*ND + j*ND + 1 + n[0];
				f2.pid = 4*i/ND;
				f2.sid = 1;
			}
		}

		// top face
		for (i=0; i<ND; ++i)
		{
			GMesh::FACE& f = m.Face(nf++);
			f.n[0] = 1 + NR*ND + (NH+1)*ND + (i+1)%ND;
			f.n[1] = NN-1;
			f.n[2] = 1 + NR * ND + (NH + 1) * ND + i;
			f.pid = 5;
			f.sid = 2;
		}

		for (j=0; j<NR-1; ++j)
		{
			for (i=0; i<ND; ++i)
			{
				GMesh::FACE& f1 = m.Face(nf++);
				GMesh::FACE& f2 = m.Face(nf++);
				f1.n[0] = 1 + NR*ND + (NH+1)*ND + j*ND + i;
				f1.n[2] = 1 + NR*ND + (NH+1)*ND + j*ND + (i+1)%ND;
				f1.n[1] = 1 + NR*ND + (NH+1)*ND + (j+1)*ND + (i+1)%ND;
				f1.pid = 5;
				f1.sid = 2;

				f2.n[0] = 1 + NR*ND + (NH+1)*ND + (j+1)*ND + (i+1)%ND;
				f2.n[2] = 1 + NR*ND + (NH+1)*ND + (j+1)*ND + i;
				f2.n[1] = 1 + NR*ND + (NH+1)*ND + j*ND + i;
				f2.pid = 5;
				f2.sid = 2;
			}
		}
		assert(nf == NF);

		// create edges
		int ne = 0;

		// bottom edges
		for (i=0; i<ND; ++i)
		{
			GMesh::EDGE& e = m.Edge(ne++);
			e.n[0] = 1 + NR*ND + i;
			e.n[1] = 1 + NR*ND + (i+1)%ND;
			e.pid = 4*i/ND;
		}

		// side edges
		for (j=0; j<NH; ++j)
		{
			for (i=0; i<4; ++i)
			{
				GMesh::EDGE& e = m.Edge(ne++);
				e.n[0] = 1 + NR*ND + j*ND + i*ND/4;
				e.n[1] = 1 + NR*ND + (j+1)*ND + i*ND/4;
				e.pid = 8 + i;
			}
		}

		// top edges
		for (i=0; i<ND; ++i)
		{
			GMesh::EDGE& e = m.Edge(ne++);
			e.n[0] = 1 + 2*NR*ND-ND + (NH+1)*ND + i;
			e.n[1] = 1 + 2*NR*ND-ND + (NH+1)*ND + (i+1)%ND;
			e.pid = 4 + 4*i/ND;
		}
		assert(ne == NE);
	}
	m.Update();
}
