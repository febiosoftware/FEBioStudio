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

#include "insertCurve.h"
#include <MeshLib/TriMesh.h>
#include <MeshLib/FESurfaceMesh.h>
#include <MeshLib/FECurveMesh.h>
#include <GeomLib/GObject.h>
#include "FECurveMesher.h"

InsertCurves::InsertCurves()
{
}

FSSurfaceMesh* InsertCurves::Apply(FSSurfaceMesh* pm, std::vector<GEdge*>& curveList, bool insertEdges, double tol)
{
	// make sure we have at least one curve
	if (curveList.empty()) return 0;

	// Build the TriMesh we need for inserting nodes, edges
	TriMesh dyna;
	BuildTriMesh(dyna, pm);

	// get the parent object so we can convert from global to local coordinates
	GObject* pd = pm->GetGObject();

	// repeat for all curves
	for (int n=0; n < curveList.size(); ++n)
	{
		GEdge* pc = curveList[n];

		// get a mesh for this curve
		GObject* pco = dynamic_cast<GObject*>(pc->Object());
		FECurveMesh* ps = pco->GetFECurveMesh(pc->GetLocalID());
		if (ps == 0)
		{
			if (pco->GetType() == GCURVE)
			{
				FECurveMesher curveMesher;
				ps = curveMesher.BuildMesh(pc);
				if (ps == 0) return 0;
			}
			else return 0;
		}

		// insert all the nodes
		int N = ps->Nodes();
		std::vector<TriMesh::NODEP> nodeList;
		for (int i = 0; i<N; ++i)
		{
			vec3d r = ps->Node(i).r;
			if (pd) r = pd->GetTransform().GlobalToLocal(r);
			TriMesh::NODEP node = insertPoint(dyna, r, tol);
			nodeList.push_back(node);
		}

		// insert all the edges
		if (insertEdges)
		{
			int NE = ps->Edges();
			for (int i=0; i<NE; ++i)
			{
				FSEdge& e = ps->Edge(i);
				insertEdge(dyna, nodeList[e.n[0]], nodeList[e.n[1]], nodeList, n + 1, tol);
			}
		}

		// tag all new nodes
		int nodes = pm->Nodes();
		for (size_t i = 0; i < nodeList.size(); ++i) 
		{
			// Note that some nodes will already have their tag set
			// These nodes are nodes that coincide with nodes of the original mesh
			if (nodeList[i]->ntag == 0)
				nodeList[i]->ntag = nodes++;
		}

		// clean up
		delete ps;
	}

	if (insertEdges)
	{
		int ng = pm->CountEdgePartitions();
		TriMesh::EdgeIterator edgePtr(dyna);
		while (edgePtr.isValid())
		{
			if (edgePtr->ntag > 0) edgePtr->gid = ng + edgePtr->ntag - 1;
			++edgePtr;
		}
	}	

	// create and return new surface mesh object from the triMesh
	return new FSSurfaceMesh(dyna);
}
