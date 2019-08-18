#include "stdafx.h"
#include "FECurveMesher.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FECurveMesh.h>

//-----------------------------------------------------------------------------
// constructor
FECurveMesher::FECurveMesher()
{
	m_elemSize = 0.1;
}

//-----------------------------------------------------------------------------
void FECurveMesher::SetElementSize(double h)
{
	assert(h > 0);
	m_elemSize = h;
}

//-----------------------------------------------------------------------------
// create the mesh
FECurveMesh* FECurveMesher::BuildMesh(GEdge* edge)
{
	// make sure this edge is not null
	if (edge == 0) return 0;

	// make sure this edge has a parent object
	GObject* parentObject = dynamic_cast<GObject*>(edge->Object());
	assert(parentObject);
	if (parentObject == 0) return 0;

	assert(edge->Type() != EDGE_UNKNOWN);
	switch (edge->Type())
	{
	case EDGE_LINE: return BuildLineMesh(edge); break;
	case EDGE_MESH: return BuildEdgeMesh(edge); break;
	default:
		assert(false);
	}

	return 0;
}

//-----------------------------------------------------------------------------
FECurveMesh* FECurveMesher::BuildLineMesh(GEdge* edge)
{
	// get the parent object
	// we need this for getting the nodal coordinates
	GBaseObject* parentObject = edge->Object();

	// get the position of the edge nodes
	vec3d ra = parentObject->Node(edge->m_node[0])->LocalPosition();
	vec3d rb = parentObject->Node(edge->m_node[1])->LocalPosition();

	// get the number of elements, based on element size
	double L = (rb - ra).Length();
	int elems = (int) (L / m_elemSize);
	if (elems < 1) elems = 1;

	// generate the nodes and edges
	FECurveMesh* edgeMesh = new FECurveMesh;
	int nodes = elems + 1;
	for (int i=0; i<nodes; ++i)
	{
		vec3d ri = ra + (rb - ra)*((double)i / (nodes - 1.0));
		edgeMesh->AddNode(ri);
	}
	for (int i=0; i<elems; ++i)
	{
		edgeMesh->AddEdge(i, i+1);
	}

	return edgeMesh;
}

//-----------------------------------------------------------------------------
FECurveMesh* FECurveMesher::BuildEdgeMesh(GEdge* edge)
{
	// get the parent object
	// we need this for getting the nodal coordinates
	GObject* parentObject = dynamic_cast<GObject*>(edge->Object());

	// get the curve mesh from the edge
	// This will only work if a mesh is assigned to this object
	FECurveMesh* curve = parentObject->GetFECurveMesh(edge->GetLocalID());
	assert(curve);

	return curve;
}
