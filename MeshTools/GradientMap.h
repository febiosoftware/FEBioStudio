#pragma once
#include <vector>
using namespace std;

//-----------------------------------------------------------------------------
class FENodeData;
class vec3d;

//-----------------------------------------------------------------------------
// Calculates gradient of a node data field
class GradientMap
{
public:
	GradientMap();

	void Apply(const FENodeData& data, vector<vec3d>& out, int niter);
};
