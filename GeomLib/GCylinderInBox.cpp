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
#include <MeshTools/FECylinderInBox.h>

class GCylinderInBoxManipulator : public GObjectManipulator
{
public:
	GCylinderInBoxManipulator(GCylinderInBox& cyl) : GObjectManipulator(&cyl), m_cyl(cyl) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		if (m < 8)
		{
			vec3d r[2];
			r[0] = T.LocalToGlobal(m_cyl.Node(m)->Position());

			// get the opposite corner
			const int LUT[8] = { 6, 7, 4, 5, 2, 3, 0, 1 };
			r[1] = m_cyl.Node(LUT[m])->Position();

			BOX box;
			for (int i = 0; i < 2; ++i)
			{
				vec3d ri = m_cyl.GetTransform().GlobalToLocal(r[i]);
				box += ri;
			}

			double w = box.Width();
			double h = box.Height();
			double d = box.Depth();

			m_cyl.SetFloatValue(GCylinderInBox::WIDTH , w);
			m_cyl.SetFloatValue(GCylinderInBox::HEIGHT, h);
			m_cyl.SetFloatValue(GCylinderInBox::DEPTH , d);

			vec3d c = box.Center();
			c.z -= d * 0.5;

			Transform& P = m_cyl.GetTransform();
			c = P.LocalToGlobal(c);
			m_cyl.GetTransform().SetPosition(c);
		}
		else
		{
			vec3d r = T.LocalToGlobal(pn->Position());
			r = m_cyl.GetTransform().GlobalToLocal(r);
			double z = r.z; r.z = 0;
			double R = r.Length();

			m_cyl.SetFloatValue(GCylinderInBox::RADIUS, R);
			if (m >= 12)
			{
				m_cyl.SetFloatValue(GCylinderInBox::DEPTH, z);
			}
			else
			{
				double H = m_cyl.GetFloatValue(GCylinderInBox::DEPTH);
				m_cyl.GetTransform().Translate(vec3d(0, 0, z));
				m_cyl.SetFloatValue(GCylinderInBox::DEPTH, H - z);
			}
		}
		m_cyl.Update();
	}

private:
	GCylinderInBox& m_cyl;
};

//-----------------------------------------------------------------------------
GCylinderInBox::GCylinderInBox() : GPrimitive(GCYLINDER_IN_BOX)
{
	m_W = m_H = m_D = 1.0;
	m_R = 0.25;

	AddDoubleParam(m_W, "w", "width" );
	AddDoubleParam(m_H, "h", "height");
	AddDoubleParam(m_D, "d", "depth" );
	AddDoubleParam(m_R, "R", "radius");
	
	SetFEMesher(new FECylinderInBox(this));
	SetManipulator(new GCylinderInBoxManipulator(*this));


	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GCylinderInBox::CreateDefaultMesher()
{
	return new FECylinderInBox(this);
}

//-----------------------------------------------------------------------------
bool GCylinderInBox::Update(bool b)
{
	m_W = GetFloatValue(WIDTH);
	m_H = GetFloatValue(HEIGHT);
	m_D = GetFloatValue(DEPTH);
	m_R = GetFloatValue(RADIUS);

	double w = m_W*0.5;
	double h = m_H*0.5;
	double d = m_D;
	double r = m_R*1.0/sqrt(2.0);

	double x[18] = {-w,  w,  w, -w, -w,  w, w, -w, -r,  r,  r, -r, -r,  r, r, -r, 0, 0};
	double y[18] = {-h, -h,  h,  h, -h, -h, h,  h, -r, -r,  r,  r, -r, -r, r,  r, 0, 0};
	double z[18] = { 0,  0,  0,  0,  d,  d, d,  d,  0,  0,  0,  0,  d,  d, d,  d, 0, d};

	for (int i=0; i<18; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	return GObject::Update();
}

//-----------------------------------------------------------------------------
// Define the Box geometry.
void GCylinderInBox::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<16; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);
	AddNode(vec3d(0,0,0), NODE_SHAPE, true);
	AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	//-------------------
	int ET[32][2] = {
		{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7},
		{8,9},{9,10},{10,11},{11,8},{12,13},{13,14},{14,15},{15,12},{8,12},{9,13},{10,14},{11,15},
		{0,8},{1,9},{2,10},{3,11},
		{4,12},{5,13},{6,14},{7,15}
	};
	assert(m_Edge.empty());
	for (i= 0; i<12; ++i) AddLine       (ET[i][0], ET[i][1]);
	for (i=12; i<16; ++i) AddCircularArc(16, ET[i][0], ET[i][1]);
	for (i=16; i<20; ++i) AddCircularArc(17, ET[i][0], ET[i][1]);
	for (i=20; i<32; ++i) AddLine	    (ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	AddSolidPart();

	// 4. build the faces
	//-------------------
	int FET[16][4] = {
		{ 0,  9,  4,  8}, { 1, 10,  5,  9}, { 2, 11,  6, 10}, { 3,  8,  7, 11},
		{12, 20, 16, 21}, {13, 21, 17, 22}, {14, 22, 18, 23}, {15, 23, 19, 20},
		{24, 12, 25,  0}, {25, 13, 26,  1}, {26, 14, 27,  2}, {27, 15, 24,  3},
		{ 4, 29, 16, 28}, { 5, 30, 17, 29}, { 6, 31, 18, 30}, { 7, 28, 19, 31}
	};

	assert(m_Face.empty());
	std::vector<int> edge;
	edge.resize(4);
	for (i=0; i<4; ++i)
	{
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_QUAD);
	}

	edge.resize(4);
	for (i=4; i<8; ++i)
	{
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_EXTRUDE);
	}

	edge.resize(4);
	for (i=8; i<16; ++i)
	{
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_POLYGON);
	}

	Update();
}
