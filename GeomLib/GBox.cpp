#include "GPrimitive.h"
#include <MeshTools/FEBox.h>

//=============================================================================
// GBox
//=============================================================================

GBox::GBox() : GPrimitive(GBOX)
{
	m_w = m_h = m_d = 1.0;

	AddDoubleParam(m_w, "w", "Width (X)" );
	AddDoubleParam(m_h, "h", "Height (Y)");
	AddDoubleParam(m_d, "d", "Depth (Z)" );
	
	SetFEMesher(new FEBox(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GBox::CreateDefaultMesher()
{
	return new FEBox(this);
}

//-----------------------------------------------------------------------------
bool GBox::Update(bool b)
{
	double w = GetFloatValue(WIDTH);
	double h = GetFloatValue(HEIGHT);
	double d = GetFloatValue(DEPTH);

	if (w <= 0.0) return false;
	if (h <= 0.0) return false;
	if (d <= 0.0) return false;

	m_w = w;
	m_h = h;
	m_d = d;

	w = m_w*0.5;
	h = m_h*0.5;
	d = m_d;

	double x[8] = {-w,  w,  w, -w, -w,  w, w, -w};
	double y[8] = {-h, -h,  h,  h, -h, -h, h,  h};
	double z[8] = { 0,  0,  0,  0,  d,  d, d,  d};

	for (int i=0; i<8; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------
// Define the Box geometry.
void GBox::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<8; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// 2. build the edges
	//-------------------
	int ET[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
	assert(m_Edge.empty());
	for (i=0; i<12; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	AddPart();

	// 4. build the faces
	//-------------------
	int FET[6][4] = {
		{ 0, 9, 4, 8},{ 1,10, 5, 9},{ 2,11, 6,10},
		{ 3, 8, 7,11},{ 3, 0, 1, 2},{ 4, 5, 6, 7}
	};
	assert(m_Face.empty());
	vector<int> edge;
	for (i=0; i<6; ++i)
	{
		edge.resize(4);
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_QUAD);
	}

	Update();
}
