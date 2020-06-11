/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
