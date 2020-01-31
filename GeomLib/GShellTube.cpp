#include "GPrimitive.h"
#include <MeshTools/FEShellTube.h>
#include <MeshTools/FEShellPatch.h>

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
FEMesher* GThinTube::CreateDefaultMesher()
{
	return new FEShellTube(this);
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

//-----------------------------------------------------------------------------
GCylindricalPatch::GCylindricalPatch() : GPrimitive(GCYLINDRICAL_PATCH)
{
	AddDoubleParam(1.0, "w", "width");
	AddDoubleParam(1.0, "h", "height");
	AddDoubleParam(1.0, "r", "radius");

	SetFEMesher(new FECylndricalPatch(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GCylindricalPatch::CreateDefaultMesher()
{
	return new FECylndricalPatch(this);
}

double GCylindricalPatch::Width() const { return GetFloatValue(W); }
double GCylindricalPatch::Height() const { return GetFloatValue(H); }
double GCylindricalPatch::Radius() const { return GetFloatValue(R); };

bool GCylindricalPatch::Update(bool b)
{
	double w = Width() / 2;
	double h = Height();
	double r = Radius();

	if (w > r) return false;

	double l = sqrt(r*r - w*w);

	m_Node[0]->LocalPosition() = vec3d( w, 0, 0);
	m_Node[1]->LocalPosition() = vec3d(-w, 0, 0);
	m_Node[2]->LocalPosition() = vec3d(-w, 0, h);
	m_Node[3]->LocalPosition() = vec3d( w, 0, h);

	m_Node[4]->LocalPosition() = vec3d( 0, -l, 0);
	m_Node[5]->LocalPosition() = vec3d( 0, -l, h);

	BuildGMesh();

	return true;
}

void GCylindricalPatch::Create()
{
	// 1. create nodes
	assert(m_Node.empty());
	for (int i = 0; i<4; ++i) AddNode(vec3d(0, 0, 0), NODE_VERTEX, true);

	// add center nodes
	AddNode(vec3d(0, 0, 0), NODE_SHAPE, true);
	AddNode(vec3d(0, 0, 0), NODE_SHAPE, true);

	// 2. build the edges
	int ET[4][3] = { { 0,1,4 },{ 3,2,5 },{ 0,3,-1 },{ 1,2,-1 }};
	assert(m_Edge.empty());
	for (int i = 0; i<2; ++i) AddCircularArc(ET[i][2], ET[i][0], ET[i][1]);
	for (int i = 2; i<4; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	assert(m_Part.empty());
	AddPart();

	// 4. build the faces
	int FE[1][4] = { { 0, 3, 1, 2 }};
	assert(m_Face.empty());
	vector<int> edge;
	for (int i = 0; i<1; ++i)
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
