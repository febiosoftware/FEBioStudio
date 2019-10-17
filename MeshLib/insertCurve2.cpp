#include "insertCurve2.h"
#include "FESurfaceMesh.h"
#include "FECurveMesh.h"
#include <GeomLib/GObject.h>
#include "MeshTools.h"
#include "TriMesh.h"

InsertCurves2::InsertCurves2()
{
}

FESurfaceMesh* InsertCurves2::Apply(FESurfaceMesh* pm, vector<GEdge*>& curveList, bool binsertEdges)
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
		if (ps == 0) return 0;
		ps->Sort();

		// get the node count for this curve
		int N = ps->Nodes();
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
	FESurfaceMesh* newMesh = new FESurfaceMesh(mesh);

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
