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
#include "FEShellPatch.h"
#include "FEAdvancingFrontMesher2D.h"

//=============================================================================
// GPatch
//=============================================================================

GPatch::GPatch() : GPrimitive(GPATCH)
{
	m_w = m_h = 1;

	AddDoubleParam(m_w, "w", "Width" );
	AddDoubleParam(m_h, "h", "Height");
	
	SetFEMesher(new FEShellPatch(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GPatch::CreateDefaultMesher()
{
	return new FEShellPatch(this);
}

//-----------------------------------------------------------------------------

bool GPatch::Update(bool b)
{
	double w = 0.5*GetFloatValue(W);
	double h = 0.5*GetFloatValue(H);

	double x[4] = {-w, w, w, -w};
	double y[4] = {-h, -h, h, h};

	for (int i=0; i<4; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], 0);
	}

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------
void GPatch::Create()
{
	int i;

	// create nodes
	assert(m_Node.empty());
	for (i=0; i<4; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// build the edges
	int ET[4][2] = {{0,1},{1,2},{2,3},{3,0}};
	assert(m_Edge.empty());
	for (i=0; i<4; ++i) AddLine(ET[i][0], ET[i][1]);

	// build the parts
	assert(m_Part.empty());
	AddPart();

	// build the faces
	assert(m_Face.empty());
	vector<int> edge; edge.resize(4);
	edge[0] = 0;
	edge[1] = 1;
	edge[2] = 2;
	edge[3] = 3;
	AddFacet(edge, FACE_QUAD);

	Update();
}
