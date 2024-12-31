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
#include <MeshTools/FECylinder.h>

class GCylinderManipulator : public GObjectManipulator
{
public:
	GCylinderManipulator(GCylinder& cyl) : GObjectManipulator(&cyl), m_cyl(cyl) {}

	void TransformNode(GNode* pn, const Transform& T)
	{
		int m = pn->GetLocalID();

		vec3d r = T.LocalToGlobal(pn->Position());
		r = m_cyl.GetTransform().GlobalToLocal(r);

		vec3d c0 = m_cyl.Node(8)->LocalPosition();
		vec3d c1 = m_cyl.Node(9)->LocalPosition();

		double z = r.z; r.z = 0;
		double R = (c0 - r).Length();

		m_cyl.SetRadius(R);
		if (m >= 4)
		{
			m_cyl.SetHeight(z);
		}
		else
		{
			m_cyl.GetTransform().Translate(vec3d(0, 0, z));
			m_cyl.SetHeight(m_cyl.Height() - z);
		}
	}

private:
	GCylinder& m_cyl;
};

//-----------------------------------------------------------------------------
// Constructor. 
GCylinder::GCylinder() : GPrimitive(GCYLINDER)
{
	m_R = 1;
	m_h = 1;

	AddDoubleParam(m_R, "R", "radius");
	AddDoubleParam(m_h, "h", "height");

	SetFEMesher(new FECylinder(this));
	SetManipulator(new GCylinderManipulator(*this));

	Create();
}

double GCylinder::Radius() const { return GetFloatValue(0); }
double GCylinder::Height() const { return GetFloatValue(1); }

void GCylinder::SetRadius(double R) { SetFloatValue(0, R); Update(); }
void GCylinder::SetHeight(double H) { SetFloatValue(1, H); Update(); }

//-----------------------------------------------------------------------------
FEMesher* GCylinder::CreateDefaultMesher()
{
	return new FECylinder(this);
}

//-----------------------------------------------------------------------------
// Create the cylinder geometry
void GCylinder::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());

	// add all the vertices
	// we'll position them later in the Update function
	for (i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add the center nodes for the top and bottom faces
	for (i=8; i<10; ++i) AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	//-------------------
	int ET[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	assert(m_Edge.empty());

	// circular edges
	for (i=0; i<4; ++i) AddCircularArc(8, ET[i][0], ET[i][1]);
	for (i=4; i<8; ++i) AddCircularArc(9, ET[i][0], ET[i][1]);

	// vertical edges
	for (i=8; i<12; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddSolidPart();

	// 4. build the faces
	//-------------------
	int FE[6][4] = {
		{ 3, 2, 1, 0},{ 0, 9, 4, 8},{ 1,10, 5, 9},
		{ 2,11, 6,10},{ 3, 8, 7,11},{ 4, 5, 6, 7}
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
		if ((i==0)||(i==5)) AddFacet(edge, FACE_POLYGON);
		else AddFacet(edge, FACE_EXTRUDE);
	}

	// update geometry data
	Update();
}

//-----------------------------------------------------------------------------
// Update the cylinder geometry
bool GCylinder::Update(bool b)
{
	// get parameters
	double H = GetFloatValue(HEIGHT);
	double R = GetFloatValue(RADIUS);
	if (R <= 0) R = 1e-5;

	// new nodal positions
	double x[10] = {R,  0, -R,  0,  R,  0, -R,  0, 0, 0};
	double y[10] = {0,  R,  0, -R,  0,  R,  0, -R, 0, 0};
	double z[10] = {0,  0,  0,  0,  H,  H,  H,  H, 0, H};

	// position the nodes
	for (int i=0; i<10; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	return GObject::Update();
}

//=============================================================================
// C Y L I N D E R 2
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor. 
GCylinder2::GCylinder2() : GPrimitive(GCYLINDER2)
{
	m_Rx = 1;
	m_Ry = 1;
	m_h = 1;

	AddDoubleParam(m_Rx, "Rx", "X-radius");
	AddDoubleParam(m_Ry, "Ry", "Y-radius");
	AddDoubleParam(m_h , "h" , "Height"  );

	SetFEMesher(new FECylinder2(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GCylinder2::CreateDefaultMesher()
{
	return new FECylinder2(this);
}

//-----------------------------------------------------------------------------
// Create the cylinder geometry
void GCylinder2::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());

	// add all the vertices
	// we'll position them later in the Update function
	for (i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add the center nodes for the top and bottom faces
	for (i=8; i<10; ++i) AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	//-------------------
	int ET[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	assert(m_Edge.empty());

	// arc edges
	for (i=0; i<4; ++i) AddArcSection(8, ET[i][0], ET[i][1]);
	for (i=4; i<8; ++i) AddArcSection(9, ET[i][0], ET[i][1]);

	// vertical edges
	for (i=8; i<12; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	m_Part.reserve(1);
	AddSolidPart();

	// 4. build the faces
	//-------------------
	int FE[6][4] = {
		{ 0, 1, 2, 3},{ 0, 9, 4, 8},{ 1,10, 5, 9},
		{ 2,11, 6,10},{ 3, 8, 7,11},{ 4, 5, 6, 7}
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
		if ((i==0)||(i==5)) AddFacet(edge, FACE_POLYGON);
		else AddFacet(edge, FACE_EXTRUDE);
	}

	// update geometry data
	Update();
}

//-----------------------------------------------------------------------------
// Update the cylinder geometry
bool GCylinder2::Update(bool b)
{
	// get parameters
	double H = GetFloatValue(HEIGHT);
	double Rx = GetFloatValue(RADIUSX);
	double Ry = GetFloatValue(RADIUSY);
	if (Rx <= 0) Rx = 1e-5;
	if (Ry <= 0) Ry = 1e-5;

	// new nodal positions
	double x[10] = {Rx,  0, -Rx,  0,  Rx,  0, -Rx,  0, 0, 0};
	double y[10] = {0,  Ry,  0, -Ry,  0,  Ry,  0, -Ry, 0, 0};
	double z[10] = {0,  0,  0,  0,  H,  H,  H,  H, 0, H};

	// position the nodes
	for (int i=0; i<10; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	// rebuild the GLMesh
	BuildGMesh();

	return true;
}
