//
//  PointCloud3d.h
//  libspg
//
//  Created by Gerard Ateshian on 9/9/14.
//
//

#ifndef __libspg__PointCloud3d__
#define __libspg__PointCloud3d__

#include <vector>
#include <MathLib/math3d.h>
#include "Plane.h"

using namespace std;

class PointCloud3d
{
public:
    PointCloud3d() { m_p.clear(); m_pmin = m_pmax = vec3d(0,0,0); m_u.clear(); }
    PointCloud3d(PointCloud3d* pc);
    PointCloud3d(const PointCloud3d& pc);
    ~PointCloud3d() {}
    
public:
    // Read point cloud data from file
    bool Read(char* szname);
    
    // Write point cloud data to file
    bool Write(char* szname);
    
    // Add point
    void AddPoint(vec3d p) { m_p.push_back(p); }
    void AddPoint(vec3d p, vec2d uv) { m_p.push_back(p); m_u.push_back(uv); }
    
    // Clear
    void Clear();
    
    // Get number of points in point cloud
    int Points() { return (int) m_p.size(); }
    
    // Get specific point in point cloud
    vec3d Point(const int i) { return m_p[i]; }
    
    // Get centroid of points in point cloud
    vec3d Centroid();
    
    // Evaluate bounding box
    void BoundingBox();
    
    // Generate parametric coordinates from spatial coordinates
    void ParametricCoordinatesFromXYZ(const int udir, const int vdir);
    
    // Generate parametric coordinates from planar projection
    void ParametricCoordinatesFromPlane();
    
    // Evaluate parametric bounding box
    void ParametricBoundingBox();
    
    // Fit plane
    double FitPlane(Plane& p);
    
    // Extract points closest to plane
    void ExtractPlanarPoints(Plane plane, double dist, PointCloud3d& pcex, vector<bool>& slct);
    
    // Get principal axes
    void PrincipalAxes(vec3d& c, double l[3], vec3d v[3]);
    
    // Rotate
    void Rotate(mat3d R);
    
    // Translate
    void Translate(vec3d c);
    
public:
    vector <vec3d>  m_p;        // point coordinates
    vec3d           m_pmin;     // bounding box min
    vec3d           m_pmax;     // bounding box max
    vector <vec2d>  m_u;        // parametric coordinates of points
    vec2d           m_umin;     // parametric bounding box min
    vec2d           m_umax;     // parametric bounding box max
};

#endif /* defined(__libspg__PointCloud3d__) */
