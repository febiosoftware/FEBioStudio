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
#include <MeshTools/FEShellDisc.h>

class GDiscManipulator : public GObjectManipulator
{
public:
	GDiscManipulator(GDisc& disc) : GObjectManipulator(&disc), m_disc(disc) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();
		if (m < 4)
		{
			vec3d r = T.LocalToGlobal(pn->Position());
			r = m_disc.GetTransform().GlobalToLocal(r);

			double R = r.Length();
			m_disc.SetRadius(R);
			m_disc.Update();
		}
	}

private:
	GDisc& m_disc;
};

//-----------------------------------------------------------------------------
// constructor
GDisc::GDisc() : GShellPrimitive(GDISC)
{
	// define parameters
	AddDoubleParam(1.0, "R", "radius");

	// assign default mesher
	SetFEMesher(new FEShellDisc(this));

	SetManipulator(new GDiscManipulator(*this));

	// build the object
	Create();
}

double GDisc::Radius() const { return GetFloatValue(RADIUS); }
void GDisc::SetRadius(double R) { SetFloatValue(RADIUS, R); }

//-----------------------------------------------------------------------------
FEMesher* GDisc::CreateDefaultMesher()
{
	return new FEShellDisc(this);
}

//-----------------------------------------------------------------------------
// create the disc object
void GDisc::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<5; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// 2. build the edges
	//-------------------
	int ET[8][2] = {
		{0,1},{1,2},{2,3},{3,0},
		{0,4},{1,4},{2,4},{3,4}
	};
	assert(m_Edge.empty());
	for (i=0; i<4; ++i) AddCircularArc(4, ET[i][0], ET[i][1]);
	for (i=4; i<8; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	AddShellPart();

	// 4. build the faces
	//------------------
	int FE[4][3] = {{0,5,4}, {1,6,5}, {2,7,6}, {3,4,7}};
	assert(m_Face.empty());
	std::vector<int> edge;
	for (i=0; i<4; ++i)
	{
		edge.resize(3);
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		AddFacet(edge, FACE_POLYGON);
	}

	Update();
}

//-----------------------------------------------------------------------------
// update the GDisc object
bool GDisc::Update(bool b)
{
	double R = GetFloatValue(RADIUS);
	if (R <= 0) R = 1e-5;

	m_Node[0]->LocalPosition() = vec3d( R, 0, 0);
	m_Node[1]->LocalPosition() = vec3d( 0, R, 0);
	m_Node[2]->LocalPosition() = vec3d(-R, 0, 0);
	m_Node[3]->LocalPosition() = vec3d( 0,-R, 0);
	m_Node[4]->LocalPosition() = vec3d( 0, 0, 0);

	return GObject::Update();
}
