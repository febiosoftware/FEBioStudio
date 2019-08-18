#pragma once
#include <vector>
using namespace std;

class FEMesh;

//-----------------------------------------------------------------------------
//! This class solves the Laplace equation using an iterative method
class LaplaceSolver
{
public:
	LaplaceSolver();

	// Solves the Laplace equation on the mesh.
	// Input: val = initial values for all nodes
	//        bn  = boundary flags: 0 = free, 1 = fixed
	// Output: val = solution
	bool Solve(FEMesh* pm, vector<double>& val, vector<int>& bn);
};
