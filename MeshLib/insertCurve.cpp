#include "insertCurve.h"
#include "TriMesh.h"
#include "FESurfaceMesh.h"
#include "FECurveMesh.h"
#include <GeomLib/GObject.h>

InsertCurves::InsertCurves()
{
}

FESurfaceMesh* InsertCurves::Apply(FESurfaceMesh* pm, vector<GEdge*>& curveList, bool insertEdges, double tol)
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
		if (ps == 0) return 0;

		// insert all the nodes
		int N = ps->Nodes();
		vector<TriMesh::NODEP> nodeList;
		for (int i = 0; i<N; ++i)
		{
			vec3d r = ps->Node(i).r;
			if (pd) r = pd->Transform().GlobalToLocal(r);
			TriMesh::NODEP node = insertPoint(dyna, r, tol);
			nodeList.push_back(node);
		}

		// insert all the edges
		if (insertEdges)
		{
			int NE = ps->Edges();
			for (int i=0; i<NE; ++i)
			{
				FEEdge& e = ps->Edge(i);
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
	return new FESurfaceMesh(dyna);
}
