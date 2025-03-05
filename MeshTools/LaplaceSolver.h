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

#pragma once
#include <vector>
//using namespace std;

using std::vector;

class FSMesh;

//-----------------------------------------------------------------------------
//! This class solves the Laplace equation using an iterative method
class LaplaceSolver
{
public:
	LaplaceSolver();

	void SetMaxIterations(int n);
	void SetTolerance(double a);
	void SetRelaxation(double w);

	// Solves the Laplace equation on the mesh.
	// Input: val = initial values for all nodes
	//        bn  = boundary flags: 0 = free, 1 = fixed
	// Output: val = solution
	bool Solve(FSMesh* pm, vector<double>& val, vector<int>& bn, int elemTag = 0);

	bool Solve(FSMesh* pm, vector<double>& val, vector<int>& bn, const vector<double>& weights, int elemTag = 0);

public: // output
	int GetIterationCount() const;
	double GetRelativeNorm() const;

private:
	// input parameters
	int		m_maxIters;	//!< max nr of iterations
	double	m_tol;	//!< convergence tolerance
	double	m_w;	//!< relaxation parameter

	// output variables
	int		m_niters;		//!< nr of iterations
	double	m_relNorm;		//!< final relative convergence norm
};
