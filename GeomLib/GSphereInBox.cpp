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
#include <MeshTools/FESphereInBox.h>
#include <GLLib/GLMesh.h>

//-----------------------------------------------------------------------------
GSphereInBox::GSphereInBox() : GPrimitive(GSPHERE_IN_BOX)
{
	AddDoubleParam(1.0, "w", "width" ); // width
	AddDoubleParam(1.0, "h", "height"); // height
	AddDoubleParam(1.0, "d", "depth"); // depth
	AddDoubleParam(0.25, "R", "radius"); // radius (of cavity)
	
	SetFEMesher(new FESphereInBox(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GSphereInBox::CreateDefaultMesher()
{
	return new FESphereInBox(this);
}

//-----------------------------------------------------------------------------
bool GSphereInBox::Update(bool b)
{
	double W = GetFloatValue(WIDTH);
	double H = GetFloatValue(HEIGHT);
	double D = GetFloatValue(DEPTH);
	double R = GetFloatValue(RADIUS);

	double w = W*0.5;
	double h = H*0.5;
	double d = D*0.5;
	double r = R*1.0/sqrt(3.0);

	double x[16] = {-w,  w,  w, -w, -w,  w, w, -w, -1,  1,  1, -1, -1,  1, 1, -1};
	double y[16] = {-h, -h,  h,  h, -h, -h, h,  h, -1, -1,  1,  1, -1, -1, 1,  1};
	double z[16] = {-d, -d, -d, -d,  d,  d, d,  d, -1, -1, -1, -1,  1,  1, 1,  1};

	for (int i=0; i<16; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	for (int i=8; i<16; ++i)
	{
		GNode& n = *m_Node[i];
		vec3d newPos = n.LocalPosition() * r;
		n.LocalPosition() = newPos;
	}

	for (int i=0; i<16; ++i)
	{
		GNode& n = *m_Node[i];
		vec3d pos = n.LocalPosition();
		pos.z += d;
		n.LocalPosition() = pos;
	}

	SetRenderMesh(nullptr);

	return true;
}

//-----------------------------------------------------------------------------
// Define the Box geometry.
void GSphereInBox::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<16; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// 2. build the edges
	//-------------------
	int ET[24][2] = {
		{0,1},{1, 2},{ 2, 3},{ 3,0},{ 4, 5},{ 5, 6},{ 6, 7},{ 7, 4},{0, 4},{1, 5},{ 2, 6},{ 3, 7},
		{8,9},{9,10},{10,11},{11,8},{12,13},{13,14},{14,15},{15,12},{8,12},{9,13},{10,14},{11,15}
	};
	assert(m_Edge.empty());
	for (i= 0; i<24; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	AddSolidPart();

	// 4. build the faces
	//-------------------
	int FET[12][4] = {
		{ 0,  9,  4,  8}, { 1, 10,  5,  9}, { 2, 11,  6, 10}, { 3,  8,  7, 11}, {3, 2, 1, 0 }, { 4,  5,  6,  7},
		{20, 16, 21, 12}, {21, 17, 22, 13}, {22, 18, 23, 14}, {23, 19, 20, 15}, {12, 13, 14, 15}, {19, 18, 17, 16}
	};

	assert(m_Face.empty());
	vector<int> edge;
	edge.resize(4);
	for (i=0; i<12; ++i)
	{
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_QUAD);
	}

	Update();
}

void GSphereInBox::BuildGMesh()
{
	double D = GetFloatValue(DEPTH);
	double R = GetFloatValue(RADIUS);
	double d = D * 0.5;

	GPrimitive::BuildGMesh();

	// project nodes onto surface
	GLMesh* pm = GetRenderMesh();

	// find all nodes for the inner surface and project to a sphere
	for (int i = 0; i < pm->Nodes(); ++i) pm->Node(i).tag = 0;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		GLMesh::FACE& f = pm->Face(i);
		if (f.pid >= 6)
		{
			pm->Node(f.n[0]).tag = 1;
			pm->Node(f.n[1]).tag = 1;
			pm->Node(f.n[2]).tag = 1;
			f.sid = 6;
		}
	}
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		GLMesh::NODE& n = pm->Node(i);
		if (n.tag == 1)
		{
			n.r.z -= d;
			n.r.Normalize();
			n.r *= R;
			n.r.z += d;
		}
	}

	pm->Update();
}
