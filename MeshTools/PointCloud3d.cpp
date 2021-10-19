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

#include "PointCloud3d.h"
#include <math.h>
#include <stdio.h>

//-------------------------------------------------------------------------------
// constructor
PointCloud3d::PointCloud3d(PointCloud3d* pc)
{
    m_p = pc->m_p;
    m_pmin = pc->m_pmin;
    m_pmax = pc->m_pmax;
    m_u = pc->m_u;
    m_umin = pc->m_umin;
    m_umax = pc->m_umax;
}

//-------------------------------------------------------------------------------
// copy constructor
PointCloud3d::PointCloud3d(const PointCloud3d& pc)
{
    m_p = pc.m_p;
    m_pmin = pc.m_pmin;
    m_pmax = pc.m_pmax;
    m_u = pc.m_u;
    m_umin = pc.m_umin;
    m_umax = pc.m_umax;
}

//-------------------------------------------------------------------------------
void PointCloud3d::Clear()
{
    m_p.clear();
    m_pmin = m_pmax = vec3d(0,0,0);
    m_u.clear();
}

//-------------------------------------------------------------------------------
vec3d PointCloud3d::Centroid()
{
    // number of points
    int n = Points();
    
    // initialize centroid
    vec3d c(0,0,0);

    // evaluate centroid
    for (int i=0; i<n; ++i)
        c += m_p[i];
    c /= n;
    
    return c;
}

//-------------------------------------------------------------------------------
void PointCloud3d::BoundingBox()
{
    int n = Points();
    
    if (n > 0)
    {
        m_pmin = m_pmax = m_p[0];
        for (int i=1; i<n; ++i) {
            m_pmin.x = fmin(m_pmin.x, m_p[i].x);
            m_pmax.x = fmax(m_pmax.x, m_p[i].x);
            m_pmin.y = fmin(m_pmin.y, m_p[i].y);
            m_pmax.y = fmax(m_pmax.y, m_p[i].y);
            m_pmin.z = fmin(m_pmin.z, m_p[i].z);
            m_pmax.z = fmax(m_pmax.z, m_p[i].z);
        }
    }
}

//--------------------------------------------------------------------------------------
// read point cloud data from file in simple x y z format
bool PointCloud3d::Read(char* szname)
{
    FILE* fp;
    char szline[256];
    double x,y,z;
    
    fp = fopen(szname, "r");
    if (fp == NULL) return false;
    
    Clear();

    while (fgets(szline, 255, fp) != NULL) {
        sscanf(szline, "%la %la %la", &x,&y,&z);
        vec3d p(x,y,z);
        AddPoint(p);
    }
    
    fclose(fp);
    
    BoundingBox();
    
    return true;
}

//--------------------------------------------------------------------------------------
// write point cloud data to file in simple x y z format
bool PointCloud3d::Write(char* szname)
{
    FILE* fp;
    char szline[256];
    
    fp = fopen(szname, "w");
    if (fp == NULL) return false;
    
    int np = Points();
    for (int i=0; i<np; ++i) {
        sprintf(szline," %14.7e %14.7e %14.7e\n", m_p[i].x, m_p[i].y, m_p[i].z);
        if (fputs(szline, fp) < 0) return false;
    }
    
    fclose(fp);
    
    return true;
}

//--------------------------------------------------------------------------------------
// generate parametric coordinates from spatial coordinates
void PointCloud3d::ParametricCoordinatesFromXYZ(const int udir, const int vdir)
{
    // get bounding box
    BoundingBox();
    
    // get surface extents
    vec3d extnt = m_pmax - m_pmin;
    
    // get number of points
    int np = Points();
    
    // allocate memory for parametric coordinates
    m_u.clear();
    
    // assign parametric coordinates from spatial coordinates
    for (int i=0; i<np; ++i) {
        vec3d p = m_p[i];
        double u = (p(udir) - m_pmin(udir))/extnt(udir);
        double v = (p(vdir) - m_pmin(vdir))/extnt(vdir);
        vec2d uv(u,v);
        m_u.push_back(uv);
    }
    
    ParametricBoundingBox();
}

//--------------------------------------------------------------------------------------
// generate parametric coordinates from spatial coordinates
void PointCloud3d::ParametricCoordinatesFromPlane()
{
    // fit a plane to the point cloud
    Plane plane;
    FitPlane(plane);
    plane.EigenPlane();
    vec3d p = plane.GetNormal()*plane.GetDistance();
    
    // get number of points
    int np = Points();
    
    // allocate memory for parametric coordinates
    m_u.resize(np);
    
    // assign parametric coordinates
    for (int i=0; i<np; ++i) {
        vec3d q = m_p[i] - p;
        double u = (q*plane.p_evec[2])/plane.p_eval[2];
        double v = (q*plane.p_evec[1])/plane.p_eval[1];
        vec2d uv(u,v);
        m_u[i] = uv;
    }
    
    ParametricBoundingBox();
}

//-------------------------------------------------------------------------------
void PointCloud3d::ParametricBoundingBox()
{
    int n = Points();
    
    if (n > 0)
    {
        m_umin = m_umax = m_u[0];
        for (int i=1; i<n; ++i) {
            m_umin.x = fmin(m_umin.x, m_u[i].x);
            m_umax.x = fmax(m_umax.x, m_u[i].x);
            m_umin.y = fmin(m_umin.y, m_u[i].y);
            m_umax.y = fmax(m_umax.y, m_u[i].y);
        }
    }
}

//-------------------------------------------------------------------------------
// Fit plane
double PointCloud3d::FitPlane(Plane& p)
{
    int np = Points();
    vec3d c = Centroid();
    mat3ds A;
    A.zero();
    
    for (int i=0; i<np; ++i)
        A += dyad(m_p[i]-c);
    
    double lam[3];
    vec3d v[3];
    A.eigen2(lam,v);
    
    vec3d n = v[0];
    p.SetNormal(n);
    p.SetDistance(c*n);
    
    double rmsres = 0;
    for (int i=0; i<np; ++i) {
        double d = (m_p[i] - c)*n;
        rmsres += d*d;
    }
    if (np > 0) rmsres = sqrt(rmsres/np);
    
    return rmsres;
}

//-------------------------------------------------------------------------------
// Extract points closest to plane (within dist)
void PointCloud3d::ExtractPlanarPoints(Plane plane, double dist,
                                       PointCloud3d& pcex, vector<bool>& slct)
{
    pcex.Clear();
    if (slct.size() == 0) slct.resize(Points(),false);
    for (int i=0; i<Points(); ++i) {
        vec3d q = Point(i);
        double qdist = fabs(plane.PointDistance(q));
        if (qdist <= dist) {
            pcex.AddPoint(q);
            slct[i] = true;
        }
    }
}

//-------------------------------------------------------------------------------
// Get principal axes
// c is the centroid, l are the eigenvalues, v are the eigenvectors
void PointCloud3d::PrincipalAxes(vec3d& c, double l[3], vec3d v[3])
{
    int np = Points();
    c = Centroid();
    mat3ds A;
    A.zero();
    
    for (int i=0; i<np; ++i)
        A += dyad(m_p[i]-c);
    
    A.eigen2(l,v);
}

//-------------------------------------------------------------------------------
// Rotate
void PointCloud3d::Rotate(mat3d R)
{
    // rotate the point cloud
    for (int i=0; i<Points(); ++i)
        m_p[i] = R*m_p[i];
    
    // rotate the bounding box
    m_pmin = R*m_pmin;
    m_pmax = R*m_pmax;
}

//-------------------------------------------------------------------------------
// Translate
void PointCloud3d::Translate(vec3d c)
{
    // translate the point cloud
    for (int i=0; i<Points(); ++i)
        m_p[i] += c;
    
    // rotate the bounding box
    m_pmin += c;
    m_pmax += c;
}
