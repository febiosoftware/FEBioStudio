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
#include <MeshTools/FEShellRing.h>

class GRingManipulator : public GObjectManipulator
{
public:
	GRingManipulator(GRing& ring) : GObjectManipulator(&ring), m_ring(ring) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		vec3d r = T.LocalToGlobal(pn->Position());
		r = m_ring.GetTransform().GlobalToLocal(r);

		double z = r.z; r.z = 0;
		double R = r.Length();

		if (m < 4)
		{
			m_ring.SetOuterRadius(R);
		}
		else
		{
			m_ring.SetInnerRadius(R);
		}
		m_ring.Update();
	}

private:
	GRing& m_ring;
};

//=============================================================================
// GRing
//=============================================================================

GRing::GRing() : GShellPrimitive(GRING)
{
	m_Ri = 0.5;
	m_Ro = 1;

	AddDoubleParam(m_Ri, "Ri", "Inner radius");
	AddDoubleParam(m_Ro, "Ro", "Outer radius");

	SetFEMesher(CreateDefaultMesher());
	SetManipulator(new GRingManipulator(*this));

	Create();
}

void GRing::SetInnerRadius(double ri) { SetFloatValue(RIN, ri); }
void GRing::SetOuterRadius(double ro) { SetFloatValue(ROUT, ro); }

//-----------------------------------------------------------------------------
FEMesher* GRing::CreateDefaultMesher()
{
	return new FEShellRing(*this);
}

//-----------------------------------------------------------------------------
void GRing::Create()
{
	int i;

	// 1. create nodes
	assert(m_Node.empty());
	m_Node.reserve(9);
	for (i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add the center node
	AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	int ET[12][3] = {{0,1,8},{1,2,8},{2,3,8},{3,0,8},{4,5,8},{5,6,8},{6,7,8},{7,4,8},{0,4,-1},{1,5,-1},{2,6,-1},{3,7,-1}};
	assert(m_Edge.empty());
	m_Edge.reserve(12);
	for (i=0; i<8; ++i) AddCircularArc(ET[i][2], ET[i][0], ET[i][1]);
	for (i=8; i<12; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddShellPart();

	// 4. build the faces
	int FT[4][4] = {{ 0, 1, 5, 4}, { 1, 2, 6, 5}, { 2, 3, 7, 6}, { 3, 0, 4, 7}};
	int FE[4][4] = {{ 0, 9, 4, 8}, { 1,10, 5, 9}, { 2,11, 6,10}, { 3, 8, 7,11}};

	assert(m_Face.empty());
	std::vector<int> edge;
	for (i=0; i<4; ++i)
	{
		edge.resize(4);
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];
		AddFacet(edge, FACE_POLYGON);
	}

	Update();
}

//-----------------------------------------------------------------------------
bool GRing::Update(bool b)
{
	double R0 = GetFloatValue(RIN);
	double R1 = GetFloatValue(ROUT);

	if (R0 <= 0.0) R0 = 1e-5;
	if (R1 <= 0.0) R1 = 1.-5;
	if (R0 >= R1) R1 = R0 + 1e-5;

	m_Node[0]->LocalPosition() = vec3d( R1,  0, 0);
	m_Node[1]->LocalPosition() = vec3d( 0,  R1, 0);
	m_Node[2]->LocalPosition() = vec3d(-R1,  0, 0);
	m_Node[3]->LocalPosition() = vec3d( 0, -R1, 0);

	m_Node[4]->LocalPosition() = vec3d( R0,  0, 0);
	m_Node[5]->LocalPosition() = vec3d( 0,  R0, 0);
	m_Node[6]->LocalPosition() = vec3d(-R0,  0, 0);
	m_Node[7]->LocalPosition() = vec3d( 0, -R0, 0);

	return GObject::Update();
}
