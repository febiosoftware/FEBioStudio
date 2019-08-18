#pragma once

//=============================================================================
//! This class implements a full matrix

//! that is a matrix that stores all its elements.

class DenseMatrix
{
public:
	// con/de-structor
	DenseMatrix();
	~DenseMatrix();

	// clear all data
	void Clear()
	{
		if (m_pd) delete [] m_pd; m_pd = 0;
		if (m_pr) delete [] m_pr; m_pr = 0;
		m_ndim = 0;
	}

	// create a matrix of size N x N
	void Create(int N);
    
    // return matrix size
    int Size() { return m_ndim; }

	// retrieve matrix data
	double& operator () (int i, int j) { return m_pr[i][j]; }

	void add(int i, int j, double v) { m_pr[i][j] += v; }
	void set(int i, int j, double v) { m_pr[i][j] = v;  }

	double diag(int i) { return m_pr[i][i]; }

protected:
    int         m_ndim;
    int         m_nsize;
    double*     m_pd;
	double**	m_pr;	//!< pointers to rows
};
