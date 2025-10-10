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
#include <MeshTools/FETube.h>

class GTubeManipulator : public GObjectManipulator
{
public:
	GTubeManipulator(GTube& tube) : GObjectManipulator(&tube), m_tube(tube) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		vec3d r = T.LocalToGlobal(pn->Position());
		r = m_tube.GetTransform().GlobalToLocal(r);

		vec3d c0 = m_tube.Node(16)->LocalPosition();
		vec3d c1 = m_tube.Node(17)->LocalPosition();

		double z = r.z; r.z = 0;
		double R = (c0 - r).Length();

		if (m >= 8)
		{
			if (m < 12)
				m_tube.SetOuterRadius(R);
			else
				m_tube.SetInnerRadius(R);

			m_tube.SetHeight(z);
		}
		else
		{
			if (m < 4)
				m_tube.SetOuterRadius(R);
			else
				m_tube.SetInnerRadius(R);

			m_tube.GetTransform().Translate(vec3d(0, 0, z));
			m_tube.SetHeight(m_tube.Height() - z);
		}
		m_tube.Update();
	}

private:
	GTube& m_tube;
};

//=============================================================================
// GTube
//=============================================================================

GTube::GTube() : GPrimitive(GTUBE)
{
	m_Ri = 0.5;
	m_Ro = 1;
	m_h = 1;

	AddDoubleParam(m_Ri, "Ri", "inner radius");	// inner radius
	AddDoubleParam(m_Ro, "Ro", "outer radius");	// outer radius
	AddDoubleParam(m_h, "h", "height");	// height

	SetFEMesher(CreateDefaultMesher());
	SetManipulator(new GTubeManipulator(*this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GTube::CreateDefaultMesher()
{
	return new FETube(*this);
}

double GTube::InnerRadius() const { return GetFloatValue(RIN); }
double GTube::OuterRadius() const { return GetFloatValue(ROUT); }
double GTube::Height() const { return GetFloatValue(HEIGHT); }

void GTube::SetInnerRadius(double Ri) { SetFloatValue(RIN, Ri);}
void GTube::SetOuterRadius(double Ro) { SetFloatValue(ROUT, Ro);}
void GTube::SetHeight(double h) { SetFloatValue(HEIGHT, h); }

//-----------------------------------------------------------------------------
bool GTube::Update(bool b)
{
	double R0 = GetFloatValue(RIN);
	double R1 = GetFloatValue(ROUT);
	double H  = GetFloatValue(HEIGHT);

	if (R0 <= 0) R0 = 1e-5;
	if (R1 <= 0) R1 = 1e-5;
	if (R1 <= R0) R1 = R0 + 1e-5;

	m_Node[0]->LocalPosition() = vec3d( R1,   0, 0);
	m_Node[1]->LocalPosition() = vec3d(  0,  R1, 0);
	m_Node[2]->LocalPosition() = vec3d(-R1,   0, 0);
	m_Node[3]->LocalPosition() = vec3d(  0, -R1, 0);

	m_Node[4]->LocalPosition() = vec3d( R0,   0, 0);
	m_Node[5]->LocalPosition() = vec3d(  0,  R0, 0);
	m_Node[6]->LocalPosition() = vec3d(-R0,   0, 0);
	m_Node[7]->LocalPosition() = vec3d(  0, -R0, 0);

	m_Node[ 8]->LocalPosition() = vec3d( R1,   0, H);
	m_Node[ 9]->LocalPosition() = vec3d(  0,  R1, H);
	m_Node[10]->LocalPosition() = vec3d(-R1,   0, H);
	m_Node[11]->LocalPosition() = vec3d(  0, -R1, H);

	m_Node[12]->LocalPosition() = vec3d( R0,   0, H);
	m_Node[13]->LocalPosition() = vec3d(  0,  R0, H);
	m_Node[14]->LocalPosition() = vec3d(-R0,   0, H);
	m_Node[15]->LocalPosition() = vec3d(  0, -R0, H);

	m_Node[16]->LocalPosition() = vec3d(  0, 0, 0);
	m_Node[17]->LocalPosition() = vec3d(  0, 0, H);

	return GObject::Update();
}

//-----------------------------------------------------------------------------
void GTube::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<16; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add the center nodes
	for (i=16; i<18; ++i) AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	//-------------------
	int ET[32][3] = {
		{ 0, 1, 16},{ 1, 2, 16},{ 2, 3, 16},{ 3, 0, 16},
		{ 4, 5, 16},{ 5, 6, 16},{ 6, 7, 16},{ 7, 4, 16},
		{ 8, 9, 17},{ 9,10, 17},{10,11, 17},{11, 8, 17},
		{12,13, 17},{13,14, 17},{14,15, 17},{15,12, 17},
		{ 0, 8, -1},{ 1, 9, -1},{ 2,10, -1},{ 3,11, -1},
		{ 4,12, -1},{ 5,13, -1},{ 6,14, -1},{ 7,15, -1},
		{ 0, 4, -1},{ 1, 5, -1},{ 2, 6, -1},{ 3, 7, -1},
		{ 8,12, -1},{ 9,13, -1},{10,14, -1},{11,15, -1}
	};
	assert(m_Edge.empty());
	for (i=0; i<16; ++i) AddCircularArc(ET[i][2], ET[i][0], ET[i][1]);
	for (i=16; i<32; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddSolidPart();

	// 4. build the faces
	//-------------------
	// face-edge table
	int FE[16][4] = {
		{24,  4, 25,  0}, {25,  5, 26,  1}, {26,  6, 27,  2}, {27,  7, 24,  3},
		{ 0, 17,  8, 16}, { 1, 18,  9, 17}, { 2, 19, 10, 18}, { 3, 16, 11, 19},
		{12, 21,  4, 20}, {13, 22,  5, 21}, {14, 23,  6, 22}, {15, 20,  7, 23},
		{ 8, 29, 12, 28}, { 9, 30, 13, 29}, {10, 31, 14, 30}, {11, 28, 15, 31},
	};

	assert(m_Face.empty());
	std::vector<int> edge;
	for (i=0; i<16; ++i)
	{
		edge.resize(4);
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];
		if ((i<4)||(i>=12)) AddFacet(edge, FACE_POLYGON);
		else AddFacet(edge, FACE_EXTRUDE);
	}

	Update();
}

//=============================================================================
// GTube2
//=============================================================================

GTube2::GTube2() : GPrimitive(GTUBE2)
{
	m_Rix = m_Riy = 0.5;
	m_Rox = m_Roy = 1;
	m_h = 1;

	AddDoubleParam(m_Rix, "Rix", "inner x-radius");	// inner x-radius
	AddDoubleParam(m_Riy, "Riy", "inner y-radius");	// inner y-radius
	AddDoubleParam(m_Rox, "Rox", "outer x-radius");	// outer x-radius
	AddDoubleParam(m_Roy, "Roy", "outer y-radius");	// outer y-radius
	AddDoubleParam(m_h, "h", "height");	// height

	SetFEMesher(CreateDefaultMesher());

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GTube2::CreateDefaultMesher()
{
	return new FETube2(*this);
}

//-----------------------------------------------------------------------------
bool GTube2::Update(bool b)
{
	double R0x = GetFloatValue(RINX);
	double R0y = GetFloatValue(RINY);
	double R1x = GetFloatValue(ROUTX);
	double R1y = GetFloatValue(ROUTY);
	double H  = GetFloatValue(HEIGHT);

	if (R0x <= 0) R0x = 1e-5;
	if (R0y <= 0) R0y = 1e-5;
	if (R1x <= 0) R1x = 1e-5;
	if (R1y <= 0) R1y = 1e-5;
	if (R1x <= R0x) R1x = R0x + 1e-5;
	if (R1y <= R0y) R1y = R0y + 1e-5;

	m_Node[0]->LocalPosition() = vec3d( R1x,    0, 0);
	m_Node[1]->LocalPosition() = vec3d(   0,  R1y, 0);
	m_Node[2]->LocalPosition() = vec3d(-R1x,    0, 0);
	m_Node[3]->LocalPosition() = vec3d(   0, -R1y, 0);

	m_Node[4]->LocalPosition() = vec3d( R0x,    0, 0);
	m_Node[5]->LocalPosition() = vec3d(   0,  R0y, 0);
	m_Node[6]->LocalPosition() = vec3d(-R0x,    0, 0);
	m_Node[7]->LocalPosition() = vec3d(   0, -R0y, 0);

	m_Node[ 8]->LocalPosition() = vec3d( R1x,    0, H);
	m_Node[ 9]->LocalPosition() = vec3d(   0,  R1y, H);
	m_Node[10]->LocalPosition() = vec3d(-R1x,    0, H);
	m_Node[11]->LocalPosition() = vec3d(   0, -R1y, H);

	m_Node[12]->LocalPosition() = vec3d( R0x,    0, H);
	m_Node[13]->LocalPosition() = vec3d(   0,  R0y, H);
	m_Node[14]->LocalPosition() = vec3d(-R0x,    0, H);
	m_Node[15]->LocalPosition() = vec3d(   0, -R0y, H);

	m_Node[16]->LocalPosition() = vec3d(  0, 0, 0);
	m_Node[17]->LocalPosition() = vec3d(  0, 0, H);

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------
void GTube2::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<16; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add the center nodes
	for (i=16; i<18; ++i) AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	//-------------------
	int ET[32][3] = {
		{ 0, 1, 16},{ 1, 2, 16},{ 2, 3, 16},{ 3, 0, 16},
		{ 4, 5, 16},{ 5, 6, 16},{ 6, 7, 16},{ 7, 4, 16},
		{ 8, 9, 17},{ 9,10, 17},{10,11, 17},{11, 8, 17},
		{12,13, 17},{13,14, 17},{14,15, 17},{15,12, 17},
		{ 0, 8, -1},{ 1, 9, -1},{ 2,10, -1},{ 3,11, -1},
		{ 4,12, -1},{ 5,13, -1},{ 6,14, -1},{ 7,15, -1},
		{ 0, 4, -1},{ 1, 5, -1},{ 2, 6, -1},{ 3, 7, -1},
		{ 8,12, -1},{ 9,13, -1},{10,14, -1},{11,15, -1}
	};
	assert(m_Edge.empty());
	for (i=0; i<16; ++i) AddArcSection(ET[i][2], ET[i][0], ET[i][1]);
	for (i=16; i<32; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddSolidPart();

	// 4. build the faces
	//-------------------
	// face-edge table
	int FE[16][4] = {
		{ 0, 25,  4, 24}, { 1, 26,  5, 25}, { 2, 27,  6, 26}, { 3, 24,  7, 27},
		{ 0, 17,  8, 16}, { 1, 18,  9, 17}, { 2, 19, 10, 18}, { 3, 16, 11, 19},
		{ 4, 21, 12, 20}, { 5, 22, 13, 21}, { 6, 23, 14, 22}, { 7, 20, 15, 23},
		{ 8, 29, 12, 28}, { 9, 30, 13, 29}, {10, 31, 14, 30}, {11, 28, 15, 31},
	};

	assert(m_Face.empty());
	std::vector<int> edge;
	for (i=0; i<16; ++i)
	{
		edge.resize(4);
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];
		if ((i<4)||(i>=12)) AddFacet(edge, FACE_POLYGON);
		else AddFacet(edge, FACE_EXTRUDE);
	}

	Update();
}
