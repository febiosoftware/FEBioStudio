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

//#include "stdafx.h"
#include <GeomLib/GPrimitive.h>
#include <MeshTools/FEShellPatch.h>
#include <MeshTools/FEAdvancingFrontMesher2D.h>
using namespace std;


class GPatchManipulator : public GObjectManipulator
{
public:
	GPatchManipulator(GPatch& patch) : GObjectManipulator(&patch), m_patch(patch) {}

	void TransformNode(GNode* pn, const Transform& T) override
	{
		int m = pn->GetLocalID();

		vec3d r[2];
		r[0] = T.LocalToGlobal(m_patch.Node(m)->Position());

		// get the opposite corner
		const int LUT[4] = { 2, 3, 0, 1 };
		r[1] = m_patch.Node(LUT[m])->Position();

		BOX box;
		for (int i = 0; i < 2; ++i)
		{
			vec3d ri = m_patch.GetTransform().GlobalToLocal(r[i]);
			box += ri;
		}

		double w = box.Width();
		double h = box.Height();
		double z = box.Depth();

		m_patch.SetFloatValue(GPatch::W, w);
		m_patch.SetFloatValue(GPatch::H, h);

		vec3d c = box.Center();
		c.z = 0;

		Transform& P = m_patch.GetTransform();
		c = P.LocalToGlobal(c);
		c.z = r[0].z;
		m_patch.GetTransform().SetPosition(c);

		m_patch.Update();
	}

private:
	GPatch& m_patch;
};

//=============================================================================
// GPatch
//=============================================================================

GPatch::GPatch() : GShellPrimitive(GPATCH)
{
	m_w = m_h = 1;

	AddDoubleParam(m_w, "w", "Width" );
	AddDoubleParam(m_h, "h", "Height");
	
	SetFEMesher(new FEShellPatch(this));
	SetManipulator(new GPatchManipulator(*this));

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

	return GObject::Update();
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
	AddShellPart();

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
