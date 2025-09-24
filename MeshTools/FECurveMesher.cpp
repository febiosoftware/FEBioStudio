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

#include "stdafx.h"
#include "FECurveMesher.h"
#include <GeomLib/GObject.h>
#include <MeshLib/FSCurveMesh.h>

//-----------------------------------------------------------------------------
// constructor
FECurveMesher::FECurveMesher()
{
	m_elemSize = 0.1;
}

//-----------------------------------------------------------------------------
void FECurveMesher::SetElementSize(double h)
{
	assert(h >= 0);
	m_elemSize = h;
}

//-----------------------------------------------------------------------------
// create the mesh
FSCurveMesh* FECurveMesher::BuildMesh(GEdge* edge)
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
FSCurveMesh* FECurveMesher::BuildLineMesh(GEdge* edge)
{
	// get the parent object
	// we need this for getting the nodal coordinates
	GBaseObject* parentObject = edge->Object();

	// get the position of the edge nodes
	vec3d ra = parentObject->Node(edge->m_node[0])->LocalPosition();
	vec3d rb = parentObject->Node(edge->m_node[1])->LocalPosition();

	// get the number of elements, based on element size
	double L = (rb - ra).Length();
	int elems = 1;
	if (m_elemSize > 0) elems = (int)(L / m_elemSize);
	if (elems < 1) elems = 1;

	// generate the nodes and edges
	FSCurveMesh* edgeMesh = new FSCurveMesh;
	int nodes = elems + 1;
	for (int i=0; i<nodes; ++i)
	{
		vec3d ri = ra + (rb - ra)*((double)i / (nodes - 1.0));
		edgeMesh->AddNode(ri, false);
	}
	for (int i=0; i<elems; ++i)
	{
		edgeMesh->AddEdge(i, i+1);
	}

	edgeMesh->BuildMesh();

	return edgeMesh;
}

//-----------------------------------------------------------------------------
FSCurveMesh* FECurveMesher::BuildEdgeMesh(GEdge* edge)
{
	// get the parent object
	// we need this for getting the nodal coordinates
	GObject* parentObject = dynamic_cast<GObject*>(edge->Object());

	// get the curve mesh from the edge
	// This will only work if a mesh is assigned to this object
	FSCurveMesh* curve = parentObject->GetFECurveMesh(edge->GetLocalID());
	assert(curve);

	return curve;
}
