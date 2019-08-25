#pragma once
#include "PointCloud3d.h"

class BivariateSpline
{
public:
    BivariateSpline() {}
    ~BivariateSpline() {}
    
public:
    virtual bool GetSplineCoeficients() = 0;
    
    // Evaluate a point on the surface at parametric coordinates (u,v)
    virtual vec3d SurfacePoint(const double u, const double v) = 0;
    
    // Evaluate the nu-th derivative along u and nv-th derivative along v at parametric coordinates (u,v)
    virtual vec3d SurfaceDerivative(const double u, const double v, const int nu, const int nv) = 0;
    
    // assign a point cloud to this bivariate spline object
    void SetPointCloud3d(PointCloud3d* pc) { m_pc = pc; }
    
    // Evaluate the surface normal at parametric coordinates (u,v)
    vec3d SurfaceNormal(const double u, const double v);
    
    // Evaluate the surface principal curvatures kappa and directions theta at parametric coordinates (u,v)
    void SurfaceCurvature(const double u, const double v, const vec3d pn, vec2d& kappa, vec3d* theta);
    
    // Evaluate surface points at parametric coordinates of point cloud data
    void FittedPoints(PointCloud3d& pc, double& rmsres);
    
public:
    PointCloud3d*               m_pc;   // pointer to point cloud
};
