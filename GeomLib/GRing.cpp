#include "GPrimitive.h"
#include <MeshTools/FEShellRing.h>

//=============================================================================
// GRing
//=============================================================================

GRing::GRing() : GPrimitive(GRING)
{
	m_Ri = 0.5;
	m_Ro = 1;

	AddDoubleParam(m_Ri, "Ri", "Inner radius");
	AddDoubleParam(m_Ro, "Ro", "Outer radius");

	SetFEMesher(new FEShellRing(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GRing::CreateDefaultMesher()
{
	return new FEShellRing(this);
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
	AddPart();

	// 4. build the faces
	int FT[4][4] = {{ 0, 1, 5, 4}, { 1, 2, 6, 5}, { 2, 3, 7, 6}, { 3, 0, 4, 7}};
	int FE[4][4] = {{ 0, 9, 4, 8}, { 1,10, 5, 9}, { 2,11, 6,10}, { 3, 8, 7,11}};

	assert(m_Face.empty());
	vector<int> edge;
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

	BuildGMesh();

	return true;
}
