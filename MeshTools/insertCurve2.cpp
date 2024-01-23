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

#include "insertCurve2.h"
#include <MeshLib/FESurfaceMesh.h>
#include <MeshLib/FECurveMesh.h>
#include <GeomLib/GObject.h>
#include <MeshLib/MeshTools.h>
#include "FECurveMesher.h"
#include <MeshLib/TriMesh.h>
using namespace std;

InsertCurves2::InsertCurves2()
{
}

FSSurfaceMesh* InsertCurves2::Apply(FSSurfaceMesh* pm, vector<GEdge*>& curveList, bool binsertEdges)
{
	// make sure we have at least one curve
	if (curveList.empty()) return 0;

	TriMesh mesh;
	BuildTriMesh(mesh, pm);

	// get the parent object so we can convert from global to local coordinates
	GObject* pd = pm->GetGObject();

	// create a list of unique points to be inserted (in local coordinates)
	vector<vec3d> pt;

	// each curve will then store the index in the point list
	vector< vector<int> > curve(curveList.size());

	// repeat for all curves
	for (int n = 0; n < curveList.size(); ++n)
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
				curveMesher.SetElementSize(0);
				ps = curveMesher.BuildMesh(pc);
				if (ps == 0) return 0;
			}
			else return 0;
		}
		ps->Sort();

		// convert the curve mesh to global coordinates
		int N = ps->Nodes();
		for (int i = 0; i < N; ++i)
		{
			vec3d r0 = ps->Node(i).r;
			vec3d r1 = pco->GetTransform().LocalToGlobal(r0);
			ps->Node(i).r = r1;
		}

		// get the node count for this curve
		curve[n].assign(N, -1);

		// create the point list (in local coordinates)
		for (int i = 0; i<N; ++i)
		{
			vec3d r = ps->Node(i).r;
			if (pd) r = pd->GetTransform().GlobalToLocal(r);

			// see if it already exists in the point list
			int nj = -1;
			for (int j=0; j<(int)pt.size(); ++j)
			{
				if ((r - pt[j]).SqrLength() < 1e-16)
				{
					nj = j;
					break;
				}
			}

			// if we did not find it, let's store it
			if (nj == -1) 
			{
				pt.push_back(r);
				nj = (int)pt.size() - 1;
			}

			curve[n][i] = nj;
		}

		// if the curve is closed, repeat the first node
		if (ps->Type() == FECurveMesh::CLOSED_CURVE)
		{
			curve[n].push_back(curve[n][0]);
		}
	}

	// now, project all the points on the mesh
	vector< TriMesh::NODEP > proj(pt.size());
	int np = (int)pt.size();
	for (int i=0; i<np; ++i)
	{
		TriMesh::NODEP pi = insertDelaunyPoint(mesh, pt[i]);
		pi->gid = 0;
		assert(pi != mesh.m_Node.end());
		proj[i] = pi;
	}

	// insert all the edges
	if (binsertEdges)
	{
		// get the current edge partitions
		// we'll need this for ID'ing the new edges
		int ng = pm->CountEdgePartitions() + 1;

		// repeat for all curves
		for (int n = 0; n < curve.size(); ++n)
		{
			vector<int> cn = curve[n];
			int N = (int)cn.size();

			for (int i = 0; i<N-1; ++i)
			{
				TriMesh::NODEP pa = proj[cn[i    ]];
				TriMesh::NODEP pb = proj[cn[i + 1]];

				insertEdge(mesh, pa, pb, ng);
			}

			// increment edge groupd ID
			++ng;
		}
	}

	mesh.PartitionSurface();

	// create the new surface mesh from the tri mesh
	FSSurfaceMesh* newMesh = new FSSurfaceMesh(mesh);

	return newMesh;
}

void InsertCurves2::insertEdge(TriMesh& mesh, TriMesh::NODEP pa, TriMesh::NODEP pb, int ngid)
{
	if (pa == pb) return;

	static int cn = 0; cn++;
	static int l = 0;
	l++;

	// search the edge
	TriMesh::EDGEP edge = mesh.findEdge(pa, pb);
	if (edge != mesh.m_Edge.end())
	{
		edge->gid = ngid;
	}
	else
	{
		if (l < 7)
		{
			// If we get here we did not find it
			vec3d c = (pa->r + pb->r)*0.5;

			TriMesh::NODEP pc = insertDelaunyPoint(mesh, c, true);
			if (pc != mesh.m_Node.end())
			{
				pc->gid = 0;

				// recursively insert the edges
				insertEdge(mesh, pa, pc, ngid);
				insertEdge(mesh, pc, pb, ngid);
			}
		}
	}

	l--;
}
