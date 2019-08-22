#include "GPrimitive.h"
#include <MeshTools/FEShellTube.h>

//=============================================================================
// GThinTube
//=============================================================================

GThinTube::GThinTube() : GPrimitive(GSHELL_TUBE)
{
	m_R = 1;
	m_h = 1;

	AddDoubleParam(m_R, "R", "radius");	// radius
	AddDoubleParam(m_h, "h", "height");	// height

	SetFEMesher(new FEShellTube(this));

	Create();
}

//-----------------------------------------------------------------------------

bool GThinTube::Update(bool b)
{
	double R = GetFloatValue(RAD);
	double h = GetFloatValue(H);

	m_Node[0]->LocalPosition() = vec3d( R,  0, 0);
	m_Node[1]->LocalPosition() = vec3d( 0,  R, 0);
	m_Node[2]->LocalPosition() = vec3d(-R,  0, 0);
	m_Node[3]->LocalPosition() = vec3d( 0, -R, 0);

	m_Node[4]->LocalPosition() = vec3d( R,  0, h);
	m_Node[5]->LocalPosition() = vec3d( 0,  R, h);
	m_Node[6]->LocalPosition() = vec3d(-R,  0, h);
	m_Node[7]->LocalPosition() = vec3d( 0, -R, h);

	m_Node[8]->LocalPosition() = vec3d(0, 0, 0);
	m_Node[9]->LocalPosition() = vec3d(0, 0, h);

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------

void GThinTube::Create()
{
	int i;

	// 1. create nodes
	assert(m_Node.empty());
	for (i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// add center nodes
	AddNode(vec3d(0,0,0), NODE_SHAPE, true);
	AddNode(vec3d(0,0,0), NODE_SHAPE, true);

	// 2. build the edges
	int ET[12][3] = {{0,1,8},{1,2,8},{2,3,8},{3,0,8},{4,5,9},{5,6,9},{6,7,9},{7,4,9},{0,4,-1},{1,5,-1},{2,6,-1},{3,7,-1}};
	assert(m_Edge.empty());
	for (i=0; i<4; ++i) AddCircularArc(ET[i][2], ET[i][0], ET[i][1]);
	for (i=4; i<8; ++i) AddCircularArc(ET[i][2], ET[i][0], ET[i][1]);
	for (i=8; i<12; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	assert(m_Part.empty());
	AddPart();

	// 4. build the faces
	int FE[4][4] = {{0, 9, 4, 8}, {1, 10, 5, 9}, {2, 11, 6, 10}, {3, 8, 7, 11}};
	assert(m_Face.empty());
	vector<int> edge;
	for (i=0; i<4; ++i)
	{
		edge.resize(4);
		edge[0] = FE[i][0];
		edge[1] = FE[i][1];
		edge[2] = FE[i][2];
		edge[3] = FE[i][3];
		AddFacet(edge, FACE_EXTRUDE);
	}

	Update();
}
