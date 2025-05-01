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
#include <MeshTools/FESolidArc.h>

class GSolidArcManipulator : public GObjectManipulator
{
public:
	GSolidArcManipulator(GSolidArc& arc) : GObjectManipulator(&arc), m_arc(arc) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		vec3d r = T.LocalToGlobal(pn->Position());
		r = m_arc.GetTransform().GlobalToLocal(r);
		double z = r.z; r.z = 0;

		double Ri = m_arc.GetFloatValue(GSolidArc::RIN);
		double Ro = m_arc.GetFloatValue(GSolidArc::ROUT);
		double H = m_arc.GetFloatValue(GSolidArc::HEIGHT);
		double w = m_arc.GetFloatValue(GSolidArc::ARC);

		if ((m == 1) || (m == 5))
		{
			Ro = r.Length();
			w = 180.0 * atan2(r.y, r.x) / PI;
		}
		else if ((m == 2) || (m == 6))
		{
			Ri = r.Length();
			w = 180.0 * atan2(r.y, r.x) / PI;
		}
		else if ((m == 0) || (m == 4))
		{
			Ro = r.Length();
		}
		else if ((m == 3) || (m == 7))
		{
			Ri = r.Length();
		}

		if (m < 4)
		{
			m_arc.GetTransform().Translate(vec3d(0, 0, z));
			H = H - z;
		}
		else
		{
			H = z;
		}

		m_arc.SetFloatValue(GSolidArc::RIN, Ri);
		m_arc.SetFloatValue(GSolidArc::ROUT, Ro);
		m_arc.SetFloatValue(GSolidArc::HEIGHT, H);
		m_arc.SetFloatValue(GSolidArc::ARC, w);

		m_arc.Update();
	}

private:
	GSolidArc& m_arc;
};

//=============================================================================
// GSolidArc
//=============================================================================

GSolidArc::GSolidArc() : GPrimitive(GSOLIDARC)
{
	AddDoubleParam(0.5, "Ri", "Inner radius");	// inner radius
	AddDoubleParam(1.0, "Ro", "Outer radius");	// outer radius
	AddDoubleParam(1.0, "h", "Height");				// height
	AddDoubleParam(90., "alpha", "Angle");				// alpha angle (in degrees)

	SetFEMesher(CreateDefaultMesher());
	SetManipulator(new GSolidArcManipulator(*this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GSolidArc::CreateDefaultMesher()
{
	return new FESolidArc();
}

//-----------------------------------------------------------------------------
bool GSolidArc::Update(bool b)
{
	double R0 = GetFloatValue(RIN);
	double R1 = GetFloatValue(ROUT);
	double H  = GetFloatValue(HEIGHT);
	double w = PI*GetFloatValue(ARC)/180.0;

	if (R0 <= 0) R0 = 1e-5;
	if (R1 <= 0) R1 = 1e-5;
	if (R1 <= R0) R1 = R0 + 1e-5;
	if (w == 0.0) w = 1e-5;

	double x0 = R0*cos(w), x1 = R1*cos(w);
	double y0 = R0*sin(w), y1 = R1*sin(w);

	// the vertices
	m_Node[0]->LocalPosition() = vec3d( R1,  0, 0);
	m_Node[1]->LocalPosition() = vec3d( x1, y1, 0);
	m_Node[2]->LocalPosition() = vec3d( x0, y0, 0);
	m_Node[3]->LocalPosition() = vec3d( R0,  0, 0);

	m_Node[4]->LocalPosition() = vec3d( R1,  0, H);
	m_Node[5]->LocalPosition() = vec3d( x1, y1, H);
	m_Node[6]->LocalPosition() = vec3d( x0, y0, H);
	m_Node[7]->LocalPosition() = vec3d( R0,  0, H);

	// the control points
	m_Node[8]->LocalPosition() = vec3d(  0, 0, 0);
	m_Node[9]->LocalPosition() = vec3d(  0, 0, H);

	return GObject::Update();
}

//-----------------------------------------------------------------------------
void GSolidArc::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add the center nodes
	for (i=8; i<10; ++i) AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	//-------------------
	int ET[12][3] = {
		{ 0, 1, 8},{ 1, 2, -1},{ 3, 2, 8},{ 3, 0, -1},
		{ 4, 5, 9},{ 5, 6, -1},{ 7, 6, 9},{ 7, 4, -1},
		{ 0, 4,-1},{ 1, 5, -1},{ 2, 6,-1},{ 3, 7, -1}
	};
	assert(m_Edge.empty());
	AddCircularArc (ET[0 ][2], ET[0 ][0], ET[0][1]);
	AddLine(ET[1 ][0], ET[1 ][1]);
	AddCircularArc (ET[2 ][2], ET[2 ][0], ET[2][1]);
	AddLine(ET[3 ][0], ET[3 ][1]);
	AddCircularArc (ET[4 ][2], ET[4 ][0], ET[4][1]);
	AddLine(ET[5 ][0], ET[5 ][1]);
	AddCircularArc (ET[6 ][2], ET[6 ][0], ET[6][1]);
	AddLine(ET[7 ][0], ET[7 ][1]);
	AddLine(ET[8 ][0], ET[8 ][1]);
	AddLine(ET[9 ][0], ET[9 ][1]);
	AddLine(ET[10][0], ET[10][1]);
	AddLine(ET[11][0], ET[11][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddSolidPart();

	// 4. build the faces
	//-------------------
	// face-edge table
	int FE[6][4] = {
		{ 0, 9,  4, 8}, { 1, 10,  5, 9}, { 2, 11,  6, 10}, { 3, 8, 7, 11},
		{ 3, 2, 1, 0}, { 4, 5,  6, 7}
	};

	assert(m_Face.empty());
	std::vector<int> edge;
	for (i=0; i<6; ++i)
	{
		edge.resize(4);
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];
		if (i<4) AddFacet(edge, FACE_EXTRUDE);
		else AddFacet(edge, FACE_POLYGON);
	}

	Update();
}
