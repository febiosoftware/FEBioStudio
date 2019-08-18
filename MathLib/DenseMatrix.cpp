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
