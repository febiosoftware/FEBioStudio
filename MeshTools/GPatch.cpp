#include "stdafx.h"
#include <GeomLib/GPrimitive.h>
#include "FEShellPatch.h"
#include "FEAdvancingFrontMesher2D.h"

//=============================================================================
// GPatch
//=============================================================================

GPatch::GPatch() : GPrimitive(GPATCH)
{
	m_w = m_h = 1;

	AddDoubleParam(m_w, "w", "Width" );
	AddDoubleParam(m_h, "h", "Height");
	
	SetFEMesher(new FEShellPatch(this));

	Create();
}

//-----------------------------------------------------------------------------
FEMesher* GPatch::CreateDefaultMesher()
{
	return new FEShellPatch(this);
}

//-----------------------------------------------------------------------------

bool GPatch::Update(bool b)
{
	double w = 0.5*GetFloatValue(W);
	double h = 0.5*GetFloatValue(H);

	double x[4] = {-w, w, w, -w};
	double y[4] = {-h, -h, h, h};

	for (int i=0; i<4; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], 0);
	}

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------
void GPatch::Create()
{
	int i;

	// create nodes
	assert(m_Node.empty());
	for (i=0; i<4; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// build the edges
	int ET[4][2] = {{0,1},{1,2},{2,3},{3,0}};
	assert(m_Edge.empty());
	for (i=0; i<4; ++i) AddLine(ET[i][0], ET[i][1]);

	// build the parts
	assert(m_Part.empty());
	AddPart();

	// build the faces
	assert(m_Face.empty());
	vector<int> edge; edge.resize(4);
	edge[0] = 0;
	edge[1] = 1;
	edge[2] = 2;
	edge[3] = 3;
	AddFacet(edge, FACE_QUAD);

	Update();
}
