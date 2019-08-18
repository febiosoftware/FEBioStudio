//
//  BivariatePolynomialSpline.h
//
//  Created by Gerard Ateshian on 9/10/14.
//
//

#ifndef __libspg__BivariatePolynomialSpline__
#define __libspg__BivariatePolynomialSpline__

#include "BivariateSpline.h"
#include "PointCloud3d.h"

class BivariatePolynomialSpline : public BivariateSpline
{
public:
    BivariatePolynomialSpline() {}
    ~BivariatePolynomialSpline() {}
    
public:
    void SetSplineDegree(const int n) { m_n = n; }
    bool GetSplineCoeficients();
    
    // Evaluate a point on the surface at parametric coordinates (u,v)
    vec3d SurfacePoint(const double u, const double v);
    
    // Evaluate the nu-th derivative along u and nv-th derivative along v at parametric coordinates (u,v)
    vec3d SurfaceDerivative(const double u, const double v, const int nu, const int nv);
    
public:
    vector< vector<double> >    m_c;    // spline coefficients
    int                         m_n;    // spline degree
};

#endif /* defined(__libspg__BivariatePolynomialSpline__) */
