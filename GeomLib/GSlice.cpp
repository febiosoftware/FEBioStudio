#include "GPrimitive.h"
#include <MeshTools/FESlice.h>

//-----------------------------------------------------------------------------
GSlice::GSlice() : GPrimitive(GSLICE)
{
	m_R = 1.0;
	m_H = 1.0;
	m_w = 90;

	AddDoubleParam(m_R, "R", "Radius");
	AddDoubleParam(m_H, "h", "height");
	AddDoubleParam(m_w, "w", "angle" );

	SetFEMesher(new FESlice(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GSlice::CreateDefaultMesher()
{
	return new FESlice(this);
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

	BuildGMesh();

	return true;
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
	AddPart();

	// 4. build the faces
	//-------------------
	// face-edge table
	int FE[5][4] = {
		{0, 7, 3,  6}, {1,8,4,7}, {2,6,5,8},
		{2, 1, 0, -1},
		{3, 4, 5, -1}
	};

	assert(m_Face.empty());
	vector<int> edge;
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
