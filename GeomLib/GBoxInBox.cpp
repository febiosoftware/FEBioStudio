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
#include <MeshTools/FEBoxInBox.h>

GBoxInBox::GBoxInBox() : GPrimitive(GBOX_IN_BOX)
{
	AddDoubleParam(1, "Outer Width (X)");
	AddDoubleParam(1, "Outer Height (Y)");
	AddDoubleParam(1, "Outer Depth (Z)");
	AddDoubleParam(0.5, "Inner Width (X)");
	AddDoubleParam(0.5, "Inner Height (Y)");
	AddDoubleParam(0.5, "Inner Depth (Z)");

	SetFEMesher(CreateDefaultMesher());

	Create();
}

FEMesher* GBoxInBox::CreateDefaultMesher()
{
	FEBoxInBox* mesher = new FEBoxInBox(*this);
	return mesher;
}

double GBoxInBox::OuterWidth () const { return GetFloatValue(0); }
double GBoxInBox::OuterHeight() const { return GetFloatValue(1); }
double GBoxInBox::OuterDepth () const { return GetFloatValue(2); }

double GBoxInBox::InnerWidth () const { return GetFloatValue(3); }
double GBoxInBox::InnerHeight() const { return GetFloatValue(4); }
double GBoxInBox::InnerDepth () const { return GetFloatValue(5); }

bool GBoxInBox::Update(bool b)
{
	double W0 = OuterWidth();
	double H0 = OuterHeight();
	double D0 = OuterDepth();

	double W1 = InnerWidth();
	double H1 = InnerHeight();
	double D1 = InnerDepth();

	if ((W0 <= 0.0)||(W1 <= 0.0)) return false;
	if ((H0 <= 0.0)||(H1 <= 0.0)) return false;
	if ((D0 <= 0.0)||(D1 <= 0.0)) return false;
	if (W1 >= W0) return false;
	if (H1 >= H0) return false;
	if (D1 >= D0) return false;

	double w0 = W0 * 0.5;
	double h0 = H0 * 0.5;
	double w1 = W1 * 0.5;
	double h1 = H1 * 0.5;

	double z0 = 0.5 * (D0 - D1);
	double z1 = 0.5 * (D0 + D1);

	double x[16] = { -w0,  w0,  w0, -w0, -w0,  w0, w0, -w0, -w1,  w1,  w1, -w1, -w1,  w1, w1, -w1 };
	double y[16] = { -h0, -h0,  h0,  h0, -h0, -h0, h0,  h0, -h1, -h1,  h1,  h1, -h1, -h1, h1,  h1 };
	double z[16] = { 0,  0,  0,  0,  D0,  D0, D0,  D0,  z0, z0, z0, z0, z1, z1, z1, z1 };

	for (int i = 0; i < 16; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	return GObject::Update();
}

void GBoxInBox::Create()
{
	// build the nodes
	for (int i = 0; i < 16; ++i) AddNode(vec3d(0, 0, 0), NODE_VERTEX, true);

	// build the edges
	const int NE = 24;
	int ET[NE][2] = { 
		{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}, 
		{8,9},{9,10},{10,11},{11,8},{12,13},{13,14},{14,15},{15,12},{8,12},{9,13},{10,14},{11,15}
	};
	for (int i = 0; i < NE; ++i) AddLine(ET[i][0], ET[i][1]);

	// build the facets
	const int NF = 12;
	int FET[NF][4] = {
		{ 0, 9, 4, 8},{ 1,10, 5, 9},{ 2,11, 6,10}, { 3, 8, 7,11},{ 3, 2, 1, 0},{ 4, 5, 6, 7},
		{20, 16, 21, 12},{ 21, 17, 22, 13},{ 22,18, 23,14}, { 23, 19, 20,15},{ 12, 13, 14, 15},{ 19, 18, 17, 16}
	};
	for (int i = 0; i < NF; ++i)
	{
		std::vector<int> edge(4);
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_QUAD);
	}

	// add part
	AddSolidPart();
}
