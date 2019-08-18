#pragma once
#include "DenseMatrix.h"
#include <vector>

using namespace std;

//-----------------------------------------------------------------------------
//! LU decomposition solver

//! This solver performs an LU decomposition and uses a backsolving algorithm
//! to solve the equations.
//! This solver uses the FullMatrix class and therefore is not the preferred
//! solver. It should only be used for small problems and only when the other
//! solvers are not adequate.

class LUSolver
{
public:
	//! constructor
	LUSolver();

	//! Factor matrix
	bool Factor();

	//! solve using factored matrix
	bool BackSolve(vector<double>& x, vector<double>& b);

	//! Clean-up
	void Destroy();

	//! Set the matrix
    void SetMatrix(DenseMatrix* pA) { m_pA = pA; }
    
protected:
	vector<int>		indx;	//!< indices
	DenseMatrix*	m_pA;	//!< sparse matrix
};
