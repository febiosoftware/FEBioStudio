#pragma once
#include <vector>
using namespace std;

class FESurfaceMesh;
class FECurveMesh;

class FELoftMesher
{
public:
	FELoftMesher();

	void setElementType(int elem) { m_elem = elem; }

	void setDivisions(int n) { m_ndivs = n; }

	FESurfaceMesh* Apply(vector<FECurveMesh*> curve);

private:
	FESurfaceMesh* BuildTriMesh(vector<FECurveMesh*> curve);
	FESurfaceMesh* BuildQuadMesh(vector<FECurveMesh*> curve);

private:
	int m_elem;
	int	m_ndivs;
};
