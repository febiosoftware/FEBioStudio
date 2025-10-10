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
#include <MeshTools/FEQuartDogBone.h>

//-----------------------------------------------------------------------------
GQuartDogBone::GQuartDogBone() : GPrimitive(GQUART_DOG_BONE)
{
	AddDoubleParam(1.0, "CW", "Clamp Width" );
	AddDoubleParam(1.0, "CH", "Clamp Height");
	AddDoubleParam(0.5, "R", "Radius");
	AddDoubleParam(1.0, "L", "Length");
	AddDoubleParam(1.0, "D", "Depth");
	AddDoubleParam(0.1, "W", "Wing" );

	SetFEMesher(CreateDefaultMesher());

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GQuartDogBone::CreateDefaultMesher()
{
	return new FEQuartDogBone(*this);
}

//-----------------------------------------------------------------------------
bool GQuartDogBone::Update(bool b)
{
	double cw = GetFloatValue(CWIDTH );
	double ch = GetFloatValue(CHEIGHT);
	double R  = GetFloatValue(RADIUS );
	double h  = GetFloatValue(DEPTH  );
	double L  = GetFloatValue(LENGTH );
	double W  = GetFloatValue(WING   );

	m_Node[0]->LocalPosition() = vec3d(   0  ,      0, 0);
	m_Node[1]->LocalPosition() = vec3d(  cw  ,      0, 0);
	m_Node[2]->LocalPosition() = vec3d(  cw  , ch    , 0);
	m_Node[3]->LocalPosition() = vec3d(cw-W  , ch    , 0);
	m_Node[4]->LocalPosition() = vec3d(cw-W-R, ch+R  , 0);
	m_Node[5]->LocalPosition() = vec3d(cw-W-R, ch+R+L, 0);
	m_Node[6]->LocalPosition() = vec3d(     0, ch+R+L, 0);
	m_Node[7]->LocalPosition() = vec3d(cw-W  , ch+R  , 0);

	m_Node[ 8]->LocalPosition() = vec3d(     0,      0, h);
	m_Node[ 9]->LocalPosition() = vec3d(    cw,      0, h);
	m_Node[10]->LocalPosition() = vec3d(    cw, ch    , h);
	m_Node[11]->LocalPosition() = vec3d(cw-W  , ch    , h);
	m_Node[12]->LocalPosition() = vec3d(cw-W-R, ch+R  , h);
	m_Node[13]->LocalPosition() = vec3d(cw-W-R, ch+R+L, h);
	m_Node[14]->LocalPosition() = vec3d(     0, ch+R+L, h);
	m_Node[15]->LocalPosition() = vec3d(  cw-W, ch+R  , h);

	return GObject::Update();
}

//-----------------------------------------------------------------------------
void GQuartDogBone::Create()
{
	// 1. Create the nodes
	assert(m_Node.empty());

	// bottom nodes
	for (int i=0; i<7; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);
	AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// top nodes
	for (int i=0; i<7; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);
	AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	int ET[][3] = {
		{0 ,1,-1},{ 1, 2,-1},{ 2, 3,-1},{ 3, 4, 7},{ 4, 5,-1},{ 5, 6,-1},{ 6, 0,-1},
		{8 ,9,-1},{ 9,10,-1},{10,11,-1},{11,12,15},{12,13,-1},{13,14,-1},{14, 8,-1},
		{0, 8,-1},{ 1, 9,-1},{ 2,10,-1},{ 3,11,-1},{ 4,12,-1},{ 5,13,-1},{ 6,14,-1}
	};

	AddLine(ET[0][0], ET[0][1]);
	AddLine(ET[1][0], ET[1][1]);
	AddLine(ET[2][0], ET[2][1]);
	AddCircularArc (ET[3][2], ET[3][1], ET[3][0]);
	AddLine(ET[4][0], ET[4][1]);
	AddLine(ET[5][0], ET[5][1]);
	AddLine(ET[6][0], ET[6][1]);

	AddLine(ET[ 7][0], ET[ 7][1]);
	AddLine(ET[ 8][0], ET[ 8][1]);
	AddLine(ET[ 9][0], ET[ 9][1]);
	AddCircularArc (ET[10][2], ET[10][1], ET[10][0]);
	AddLine(ET[11][0], ET[11][1]);
	AddLine(ET[12][0], ET[12][1]);
	AddLine(ET[13][0], ET[13][1]);

	AddLine(ET[14][0], ET[14][1]);
	AddLine(ET[15][0], ET[15][1]);
	AddLine(ET[16][0], ET[16][1]);
	AddLine(ET[17][0], ET[17][1]);
	AddLine(ET[18][0], ET[18][1]);
	AddLine(ET[19][0], ET[19][1]);
	AddLine(ET[20][0], ET[20][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddSolidPart();

	// 4. build the faces
	//-------------------
	std::vector<int> edge;

	// bottom face
	edge.resize(7);
	for (int i=0; i<7; ++i) edge[6 - i] = i;
	AddFacet(edge, FACE_POLYGON);

	// sides
	int FE[][4] = {
		{0, 15,  7, 14}, {1, 16,  8, 15}, {2, 17,  9, 16},
		{3, 18, 10, 17}, {4, 19, 11, 18}, {5, 20, 12, 19},
		{6, 14, 13, 20},
	};

	edge.resize(4);
	for (int i=0; i<7; ++i)
	{
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];
		AddFacet(edge, FACE_EXTRUDE);
	}

	// top face
	edge.resize(7);
	for (int i=0; i<7; ++i) edge[i] = i + 7;
	AddFacet(edge, FACE_POLYGON);

	Update();
}

