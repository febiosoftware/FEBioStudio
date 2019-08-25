#pragma once
#include "PointCloud3d.h"

class Quadric
{
public:
    Quadric() {}
    Quadric(PointCloud3d* pc) { m_pc = pc; }
    ~Quadric() {}
    
public:
    // assign a point cloud to this bivariate spline object
    void SetPointCloud3d(PointCloud3d* pc) { m_pc = pc; }
    
    // fit the point cloud to get quadric surface coefficients
    bool GetQuadricCoeficients();
    
    // Evaluate the surface normal at point p
    vec3d SurfaceNormal(const vec3d p);
    
    // Evaluate the surface principal curvatures kappa and directions v at point p
    void SurfaceCurvature(const vec3d p, const vec3d n, vec2d& kappa, vec3d* v);
    
    // Find ray-quadric surface intersections x: p is point on ray, n is normal along ray
    void RayQuadricIntersection(const vec3d p, const vec3d n, vector<vec3d>* x, vector<double>* t = nullptr);
    
    // Find the point on the quadric closest to the point p
    vec3d ClosestPoint(const vec3d p);
    
    // Find the point on the quadric closest to the point p
    vec3d ClosestPoint(const vec3d p, const vec3d norm);

public:
    double          m_c[10];    // quadric surface coefficients
    PointCloud3d*   m_pc;       // pointer to point cloud
};
