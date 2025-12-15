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
#include <MeshTools/FETorus.h>

//=============================================================================
// GTorus
//=============================================================================

GTorus::GTorus() : GPrimitive(GTORUS)
{
	m_R0 = 1;
	m_R1 = 0.2;

	AddDoubleParam(m_R0, "R0", "outer radius");
	AddDoubleParam(m_R1, "R1", "inner radius");

	SetFEMesher(CreateDefaultMesher());

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GTorus::CreateDefaultMesher()
{
	return new FETorus(*this);
}

//----------------------------------------------------------------------------
bool GTorus::Update(bool b)
{
	double R0 = GetFloatValue(RIN);
	double R1 = GetFloatValue(ROUT);

	m_Node[0]->LocalPosition() = vec3d(0, -R0-R1,   0);
	m_Node[1]->LocalPosition() = vec3d(0, -R0   , -R1);
	m_Node[2]->LocalPosition() = vec3d(0, -R0+R1,   0);
	m_Node[3]->LocalPosition() = vec3d(0, -R0   ,  R1);

	m_Node[4]->LocalPosition() = vec3d(R0+R1, 0,   0);
	m_Node[5]->LocalPosition() = vec3d(R0   , 0, -R1);
	m_Node[6]->LocalPosition() = vec3d(R0-R1, 0,   0);
	m_Node[7]->LocalPosition() = vec3d(R0   , 0,  R1);

	m_Node[ 8]->LocalPosition() = vec3d(0, R0+R1,   0);
	m_Node[ 9]->LocalPosition() = vec3d(0, R0   , -R1);
	m_Node[10]->LocalPosition() = vec3d(0, R0-R1,   0);
	m_Node[11]->LocalPosition() = vec3d(0, R0   ,  R1);

	m_Node[12]->LocalPosition() = vec3d(-R0-R1, 0,   0);
	m_Node[13]->LocalPosition() = vec3d(-R0   , 0, -R1);
	m_Node[14]->LocalPosition() = vec3d(-R0+R1, 0,   0);
	m_Node[15]->LocalPosition() = vec3d(-R0   , 0,  R1);

	m_Node[16]->LocalPosition() = vec3d( 0   , -R0,  0);
	m_Node[17]->LocalPosition() = vec3d( R0  ,   0,  0);
	m_Node[18]->LocalPosition() = vec3d( 0   ,  R0,  0);
	m_Node[19]->LocalPosition() = vec3d(-R0  ,   0,  0);

	return GObject::Update();
}

//----------------------------------------------------------------------------
void GTorus::Create()
{
	int i;

	// 1. define the nodes
	//--------------------
	// Add the vertices
	// We'll position them later in the Update function
	assert(m_Node.empty());
	for (i=0; i<16; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add additional nodes for defining the geometry
	for (i=16; i<20; ++i) AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	//-------------------
	int ET[32][3] = {
		{ 0, 4, -1},{ 4, 8, -1},{ 8,12, -1},{12, 0, -1},
		{ 1, 5, -1},{ 5, 9, -1},{ 9,13, -1},{13, 1, -1},
		{ 2, 6, -1},{ 6,10, -1},{10,14, -1},{14, 2, -1},
		{ 3, 7, -1},{ 7,11, -1},{11,15, -1},{15, 3, -1},
		{ 0, 1, 16},{ 1, 2, 16},{ 2, 3, 16},{ 3, 0, 16},
		{ 4, 5, 17},{ 5, 6, 17},{ 6, 7, 17},{ 7, 4, 17},
		{ 8, 9, 18},{ 9,10, 18},{10,11, 18},{11, 8, 18},
		{12,13, 19},{13,14, 19},{14,15, 19},{15,12, 19}
	};

	assert(m_Edge.empty());
	
	// add the revolved edges
	for (i=0; i<16; ++i) AddZArc(ET[i][0], ET[i][1]);

	// add the cirucular arcs
	for (i=16; i<32; ++i) AddCircularArc(ET[i][2], ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	AddSolidPart();
	
	// 4. build the faces
	//-------------------

	// node table
	int FT[16][4] = {
		{ 0,  1,  5,  4}, { 1,  2,  6,  5}, { 2,  3,  7,  6}, { 3,  0,  4,  7},
		{ 4,  5,  9,  8}, { 5,  6, 10,  9}, { 6,  7, 11, 10}, { 7,  4,  8, 11},
		{ 8,  9, 13, 12}, { 9, 10, 14, 13}, {10, 11, 15, 14}, {11,  8, 12, 15},
		{12, 13,  1,  0}, {13, 14,  2,  1}, {14, 15,  3,  2}, {15, 12,  0,  3}
	};

	// edge table
	int FE[16][4] = {
		{16,  4, 20,  0}, {17,  8, 21,  4}, {18, 12, 22,  8}, {19,  0, 23, 12},
		{20,  5, 24,  1}, {21,  9, 25,  5}, {22, 13, 26,  9}, {23,  1, 27, 13},
		{24,  6, 28,  2}, {25, 10, 29,  6}, {26, 14, 30, 10}, {27,  2, 31, 14},
		{28,  7, 16,  3}, {29, 11, 17,  7}, {30, 15, 18, 11}, {31,  3, 19, 15}
	};

	assert(m_Face.empty());
	m_Face.reserve(16);
	std::vector<int> edge;
	for (i=0; i<16; ++i)
	{
		edge.resize(4);
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];

		AddFacet(edge, FACE_REVOLVE);
	}

	Update();
}
