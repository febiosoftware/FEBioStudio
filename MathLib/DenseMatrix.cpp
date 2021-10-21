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

#include "DenseMatrix.h"

//-----------------------------------------------------------------------------
DenseMatrix::DenseMatrix()
{
    m_ndim = m_nsize = 0;
    m_pd = 0;
	m_pr = 0;
}

//-----------------------------------------------------------------------------
DenseMatrix::~DenseMatrix()
{
	delete [] m_pd; m_pd = 0;
	delete [] m_pr; m_pr = 0;
}

//-----------------------------------------------------------------------------
// Creat a dense matrix of size N x N
void DenseMatrix::Create(int N)
{
	if (N != m_ndim)
	{
		if (m_pd) delete [] m_pd;
		if (m_pr) delete [] m_pr;

		m_pd = new double[N*N];
		m_pr = new double*[N];

		for (int i=0; i<N; ++i) m_pr[i] = m_pd + i*N;

		m_ndim = N;
		m_nsize = N*N;
	}
}
