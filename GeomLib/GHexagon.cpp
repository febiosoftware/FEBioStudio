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
#include <MeshTools/FETetGenMesher.h>

class GHexagonManipulator : public GObjectManipulator
{
public:
	GHexagonManipulator(GHexagon& hex) : GObjectManipulator(&hex), m_hex(hex) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		vec3d r = T.LocalToGlobal(pn->Position());
		r = m_hex.GetTransform().GlobalToLocal(r);

		double z = r.z; r.z = 0;
		double R = r.Length();

		m_hex.SetRadius(R);
		if (m >= 4)
		{
			m_hex.SetHeight(z);
		}
		else
		{
			m_hex.GetTransform().Translate(vec3d(0, 0, z));
			m_hex.SetHeight(m_hex.Height() - z);
		}
		m_hex.Update();
	}

private:
	GHexagon& m_hex;
};

GHexagon::GHexagon() : GPrimitive(GHEXAGON)
{	
	AddDoubleParam(1.0, "R", "Radius");
	AddDoubleParam(1.0, "H", "Height");

	SetFEMesher(new FETetGenMesher(this));
	SetManipulator(new GHexagonManipulator(*this));

	Create();
}

double GHexagon::Radius() const { return GetFloatValue(RADIUS); }
double GHexagon::Height() const { return GetFloatValue(HEIGHT); }

void GHexagon::SetRadius(double R) { SetFloatValue(RADIUS, R); }
void GHexagon::SetHeight(double H) { SetFloatValue(HEIGHT, H); }

bool GHexagon::Update(bool b)
{
	double R = GetFloatValue(RADIUS);
	double H = GetFloatValue(HEIGHT);

	for (int i=0; i<6; ++i)
	{
		double x = R*cos(i*PI/3.0);
		double y = R*sin(i*PI/3.0);
		m_Node[i    ]->LocalPosition() = vec3d(x, y, 0);
		m_Node[i + 6]->LocalPosition() = vec3d(x, y, H);
	}

	return GObject::Update();
}

void GHexagon::Create()
{
	// 1. Create the nodes
	assert(m_Node.empty());

	// bottom nodes
	for (int i = 0; i<6; ++i) AddNode(vec3d(0, 0, 0), NODE_VERTEX, true);

	// top nodes
	for (int i = 0; i<6; ++i) AddNode(vec3d(0, 0, 0), NODE_VERTEX, true);

	// 2. build the edges
	int ET[18][2] = {
		{0,1},{1,2},{2,3},{3, 4},{ 4, 5},{ 5, 0},
		{6,7},{7,8},{8,9},{9,10},{10,11},{11, 6},
		{0,6},{1,7},{2,8},{3, 9},{ 4,10},{ 5,11},
	};
	for (int i=0; i<18; ++i) AddLine(ET[i][0], ET[i][1]);

	//3. build part
	AddSolidPart();

	//4. build the faces
	std::vector<int> edge;
	edge.resize(6);
	for (int i = 0; i<6; ++i) edge[5 - i] = i;
	AddFacet(edge, FACE_POLYGON);

	int FE[][4] = {
		{ 0, 13, 6, 12 }, { 1, 14,  7, 13 }, { 2, 15,  8, 14 },
		{ 3, 16, 9, 15 }, { 4, 17, 10, 16 }, { 5, 12, 11, 17 }
	};

	edge.resize(4);
	for (int i = 0; i<6; ++i)
	{
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];
		AddFacet(edge, FACE_EXTRUDE);
	}

	edge.resize(6);
	for (int i = 0; i<6; ++i) edge[i] = i + 6;
	AddFacet(edge, FACE_POLYGON);

	Update();
}
