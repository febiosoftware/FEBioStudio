#include "GPrimitive.h"
#include <MeshTools/FETetGenMesher.h>

GHexagon::GHexagon() : GPrimitive(GHEXAGON)
{	
	AddDoubleParam(1.0, "R", "Radius");
	AddDoubleParam(1.0, "H", "Height");

	SetFEMesher(new FETetGenMesher(this));

	Create();
}

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

	BuildGMesh();

	return true;
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
	AddPart();

	//4. build the faces
	vector<int> edge;
	edge.resize(6);
	for (int i = 0; i<6; ++i) edge[i] = i;
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
