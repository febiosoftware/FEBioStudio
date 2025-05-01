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
#include <MeshTools/FESlice.h>

class GSliceManipulator : public GObjectManipulator
{
public:
	GSliceManipulator(GSlice& slice) : GObjectManipulator(&slice), m_slice(slice) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		vec3d r = T.LocalToGlobal(pn->Position());
		r = m_slice.GetTransform().GlobalToLocal(r);
		double z = r.z; r.z = 0;

		double R = m_slice.GetFloatValue(GSlice::RADIUS);
		double H = m_slice.GetFloatValue(GSlice::HEIGHT);
		double w = m_slice.GetFloatValue(GSlice::ANGLE);

		if ((m == 2) || (m == 5))
		{
			R = r.Length();
			w = 180.0 * atan2(r.y, r.x) / PI;
		}
		else if ((m == 1) || (m == 4))
		{
			R = r.Length();
		}

		if (m < 3)
		{
			m_slice.GetTransform().Translate(vec3d(0, 0, z));
			H = H - z;
		}
		else
		{
			H = z;
		}

		m_slice.SetFloatValue(GSlice::RADIUS, R);
		m_slice.SetFloatValue(GSlice::HEIGHT, H);
		m_slice.SetFloatValue(GSlice::ANGLE, w);

		m_slice.Update();
	}

private:
	GSlice& m_slice;
};

//-----------------------------------------------------------------------------
GSlice::GSlice() : GPrimitive(GSLICE)
{
	m_R = 1.0;
	m_H = 1.0;
	m_w = 90;

	AddDoubleParam(m_R, "R", "Radius");
	AddDoubleParam(m_H, "h", "height");
	AddDoubleParam(m_w, "w", "angle" );

	SetFEMesher(CreateDefaultMesher());
	SetManipulator(new GSliceManipulator(*this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GSlice::CreateDefaultMesher()
{
	return new FESlice();
}

//-----------------------------------------------------------------------------
bool GSlice::Update(bool b)
{
	m_R = GetFloatValue(RADIUS);
	m_H = GetFloatValue(HEIGHT);
	m_w = GetFloatValue(ANGLE);
	if (m_R < 0) m_R = 1e-5;

	double x = m_R*cos(PI*m_w/180.0);
	double y = m_R*sin(PI*m_w/180.0);

	m_Node[0]->LocalPosition() = vec3d(0  , 0, 0);
	m_Node[1]->LocalPosition() = vec3d(m_R, 0, 0);
	m_Node[2]->LocalPosition() = vec3d(x  , y, 0);

	m_Node[3]->LocalPosition() = vec3d(  0, 0, m_H);
	m_Node[4]->LocalPosition() = vec3d(m_R, 0, m_H);
	m_Node[5]->LocalPosition() = vec3d(  x, y, m_H);

	return GObject::Update();
}

//-----------------------------------------------------------------------------
void GSlice::Create()
{
	// 1. Build the nodes
	assert(m_Node.empty());
	for (int i=0; i<6; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// 2. Build the edges
	int ET[9][3] = {
		{0,1,-1},{1,2, 0},{2,0,-1},
		{3,4,-1},{4,5, 3},{5,3,-1},
		{0,3,-1},{1,4,-1},{2,5,-1}
	};

	AddLine(ET[0][0], ET[0][1]);
	AddZArc(ET[1][0], ET[1][1]);
	AddLine(ET[2][0], ET[2][1]);
	AddLine(ET[3][0], ET[3][1]);
	AddZArc(ET[4][0], ET[4][1]);
	AddLine(ET[5][0], ET[5][1]);
	AddLine(ET[6][0], ET[6][1]);
	AddLine(ET[7][0], ET[7][1]);
	AddLine(ET[8][0], ET[8][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddSolidPart();

	// 4. build the faces
	//-------------------
	// face-edge table
	int FE[5][4] = {
		{0, 7, 3,  6}, {1,8,4,7}, {2,6,5,8},
		{2, 1, 0, -1},
		{3, 4, 5, -1}
	};

	assert(m_Face.empty());
	std::vector<int> edge;
	for (int i=0; i<5; ++i)
	{
		if ((i==3)||(i==4))
		{
			edge.resize(3);
			edge[0] = FE[i][0];
			edge[1] = FE[i][1];
			edge[2] = FE[i][2];
			AddFacet(edge, FACE_REVOLVE_WEDGE);
		}
		else
		{
			edge.resize(4);
			edge[0] = FE[i][0];
			edge[1] = FE[i][1];
			edge[2] = FE[i][2];
			edge[3] = FE[i][3];
			AddFacet(edge, FACE_EXTRUDE);
		}
	}

	Update();
}
