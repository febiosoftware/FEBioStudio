#pragma once
#include <vector>
using namespace std;

class FESurfaceMesh;
class GEdge;

class InsertCurves
{
public:
	InsertCurves();

	FESurfaceMesh* Apply(FESurfaceMesh* mesh, vector<GEdge*>& curveList, bool insertEdges, double tol);
};
