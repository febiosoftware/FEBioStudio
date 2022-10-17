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

#pragma once
#include <vector>
#include <FECore/vec3d.h>
#include "Plane.h"

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
    void ExtractPlanarPoints(Plane plane, double dist, PointCloud3d& pcex, std::vector<bool>& slct);
    
    // Get principal axes
    void PrincipalAxes(vec3d& c, double l[3], vec3d v[3]);
    
    // Rotate
    void Rotate(mat3d R);
    
    // Translate
    void Translate(vec3d c);
    
public:
    std::vector <vec3d>  m_p;        // point coordinates
    vec3d           m_pmin;     // bounding box min
    vec3d           m_pmax;     // bounding box max
    std::vector <vec2d>  m_u;        // parametric coordinates of points
    vec2d           m_umin;     // parametric bounding box min
    vec2d           m_umax;     // parametric bounding box max
};
