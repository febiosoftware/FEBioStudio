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
#include <MeshTools/FEBox.h>
#include <GeomLib/GObject.h>

class GBoxManipulator : public GObjectManipulator
{
public:
	GBoxManipulator(GBox& box) : GObjectManipulator(&box), m_box(box) {}

	void TransformNode(GNode* pn, const Transform& T) override
	{
		assert(pn->Object() == &m_box);

		int m = pn->GetLocalID();

		vec3d r[2];
		r[0] = T.LocalToGlobal(m_box.Node(m)->Position());

		// get the opposite corner
		const int LUT[8] = {6, 7, 4, 5, 2, 3, 0, 1};
		r[1] = m_box.Node(LUT[m])->Position();

		BOX box;
		for (int i = 0; i < 2; ++i)
		{
			vec3d ri = m_box.GetTransform().GlobalToLocal(r[i]);
			box += ri;
		}

		double w = box.Width();
		double h = box.Height();
		double d = box.Depth();

		m_box.SetFloatValue(GBox::WIDTH , w);
		m_box.SetFloatValue(GBox::HEIGHT, h);
		m_box.SetFloatValue(GBox::DEPTH , d);

		vec3d c = box.Center();
		c.z -= d * 0.5;

		Transform& P = m_box.GetTransform();
		c = P.LocalToGlobal(c);
		m_box.GetTransform().SetPosition(c);

		m_box.Update();
	}

private:
	GBox& m_box;
};

//=============================================================================
// GBox
//=============================================================================

GBox::GBox() : GPrimitive(GBOX)
{
	m_w = m_h = m_d = 1.0;

	AddDoubleParam(m_w, "w", "Width (X)" );
	AddDoubleParam(m_h, "h", "Height (Y)");
	AddDoubleParam(m_d, "d", "Depth (Z)" );
	
	SetFEMesher(CreateDefaultMesher());

	SetManipulator(new GBoxManipulator(*this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GBox::CreateDefaultMesher()
{
	return new FEBoxMesher();
}

//-----------------------------------------------------------------------------
bool GBox::Update(bool b)
{
	double w = GetFloatValue(WIDTH);
	double h = GetFloatValue(HEIGHT);
	double d = GetFloatValue(DEPTH);

	if (w <= 0.0) return false;
	if (h <= 0.0) return false;
	if (d <= 0.0) return false;

	m_w = w;
	m_h = h;
	m_d = d;

	w = m_w*0.5;
	h = m_h*0.5;
	d = m_d;

	double x[8] = {-w,  w,  w, -w, -w,  w, w, -w};
	double y[8] = {-h, -h,  h,  h, -h, -h, h,  h};
	double z[8] = { 0,  0,  0,  0,  d,  d, d,  d};

	for (int i=0; i<8; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	return GObject::Update(b);
}

//-----------------------------------------------------------------------------
// Define the Box geometry.
void GBox::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// 2. build the edges
	//-------------------
	int ET[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	assert(m_Edge.empty());
	for (i=0; i<12; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	AddSolidPart();

	// 4. build the faces
	//-------------------
	int FET[6][4] = {
		{ 0, 9, 4, 8},{ 1,10, 5, 9},{ 2,11, 6,10},
		{ 3, 8, 7,11},{ 3, 2, 1, 0},{ 4, 5, 6, 7}
	};
	assert(m_Face.empty());
	std::vector<int> edge;
	for (i=0; i<6; ++i)
	{
		edge.resize(4);
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_QUAD);
	}

	Update();
}
