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

#include "BivariatePolynomialSpline.h"
#include <FECore/LUSolver.h>
#include <stdio.h>

inline int Factorial(int n) {
    return (n == 1 || n == 0) ? 1 : Factorial(n - 1) * n;
}
inline int DerivFactorial(int n, int k) {
    return (n < k ? 1 : Factorial(n)/Factorial(n-k));
}

//--------------------------------------------------------------------------------------
bool BivariatePolynomialSpline::GetSplineCoeficients()
{
    if (m_pc->Points() == 0) {
        printf("No point cloud defined for BivariatePolynomialSpline fit\n");
        return false;
    }
    if (m_pc->m_u.size() == 0) {
        printf("No parametric coordinates defined for BivariatePolynomialSpline fit\n");
        return false;
    }
    
    // create solver and matrix
    LUSolver solver;
    FECore::DenseMatrix     matrix;
    solver.SetMatrix(&matrix);
    int N = m_n + 1;
    matrix.Create(N*N, N*N);
    
    // get summations of the powers of u and v
    int np = m_pc->Points();
    vector< vector<double> > sum(2*m_n+1, vector<double>(2*m_n+1));
    for (int i=0; i<=2*m_n; ++i) {
        for (int j=0; j<=2*m_n; ++j) {
            sum[i][j] = 0;
            for (int k=0; k<np; ++k)
                sum[i][j] += pow(m_pc->m_u[k].x(), i)*pow(m_pc->m_u[k].y(), j);
        }
    }
    
    // evaluate the coefficient matrix
    int irow, icol;
    for (int i=0; i<N; ++i) {
        for (int j=0; j<N; ++j) {
            irow = i + j*N;
            for (int l=0; l<N; ++l) {
                for (int m=0; m<N; ++m) {
                    icol = l + m*N;
                    matrix.set(irow, icol, sum[i+l][j+m]);
                }
            }
        }
    }
    
    // perform the factorization
    if (!solver.Factor()) {
        printf("Matrix is singular in BivariatePolynomialSpline fit\n");
        return false;
    }
    
    double uv;
    m_c.assign(3,vector<double>(N*N,0));
    for (int i=0; i<N; ++i) {
        for (int j=0; j<N; ++j) {
            irow = i + j*N;
            for (int l=0; l<np; ++l) {
                uv = pow(m_pc->m_u[l].x(), i)*pow(m_pc->m_u[l].y(), j);
                m_c[0][irow] += m_pc->m_p[l].x*uv;
                m_c[1][irow] += m_pc->m_p[l].y*uv;
                m_c[2][irow] += m_pc->m_p[l].z*uv;
            }
        }
    }
    
    // perform the back substitution
    solver.BackSolve(m_c[0].data(), m_c[0].data());
    solver.BackSolve(m_c[1].data(), m_c[1].data());
    solver.BackSolve(m_c[2].data(), m_c[2].data());
    
    return true;
}

//--------------------------------------------------------------------------------------
// Evaluate a point on the surface at parametric coordinates (u,v)
vec3d BivariatePolynomialSpline::SurfacePoint(const double u, const double v)
{
    int N = m_n + 1;
    double uc, vc, uv;
    int irow;
    
    vec3d   p(0,0,0);
    
    for (int i=0; i<N; ++i) {
        uc = pow(u, i);
        for (int j=0; j<N; ++j) {
            vc = pow(v, j);
            uv = uc*vc;
            irow = i + j*N;
            p.x += m_c[0][irow]*uv;
            p.y += m_c[1][irow]*uv;
            p.z += m_c[2][irow]*uv;
        }
    }
    
    return p;
}

//--------------------------------------------------------------------------------------
// Evaluate the nu-th derivative along u and nv-th derivative along v at parametric coordinates (u,v)
vec3d BivariatePolynomialSpline::SurfaceDerivative(const double u, const double v, const int nu, const int nv)
{
    vec3d   p(0,0,0);
    if ((nu > m_n) || (nv > m_n)) return p;
    
    int N = m_n + 1;
    double uc,vc, uv;
    int irow;
    
    for (int i=nu; i<N; ++i) {
        uc = DerivFactorial(i,nu)*pow(u, i-nu);
        for (int j=nv; j<N; ++j) {
            vc = DerivFactorial(j,nv)*pow(v, j-nv);
            uv = uc*vc;
            irow = i + j*N;
            p.x += m_c[0][irow]*uv;
            p.y += m_c[1][irow]*uv;
            p.z += m_c[2][irow]*uv;
        }
    }
    
    return p;
}
