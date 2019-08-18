#pragma once
#include <vector>
#include "TriMesh.h"
using namespace std;

class FESurfaceMesh;
class GEdge;
class vec3d;

class InsertCurves2
{
public:
	InsertCurves2();

	FESurfaceMesh* Apply(FESurfaceMesh* pm, vector<GEdge*>& curveList, bool binsertEdges);

private:
	void ProjectCurve(FESurfaceMesh* mesh, vector<vec3d>& curve);
	void insertEdge(TriMesh& mesh, TriMesh::NODEP pa, TriMesh::NODEP pb, int ntag);
};
