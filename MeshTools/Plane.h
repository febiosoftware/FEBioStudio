//
//  Plane.h
//  libspg
//
//  Created by Gerard Ateshian on 2/26/15.
//
//

#ifndef __libspg__Plane__
#define __libspg__Plane__

#include "MathLib/mat3d.h"
#include "MathLib/math3d.h"

class Plane
{
public:
    Plane() { p_n = vec3d(0,0,0); p_d = 0; }
    Plane(vec3d n, double d) { n.Normalize(); p_n = n; p_d = d; }
    ~Plane() {}

public:
    void SetNormal(vec3d n) { n.Normalize(); p_n = n; }
    void SetDistance(double d) { p_d = d; }
    vec3d GetNormal() { return p_n; }
    double GetDistance() { return p_d; }
    void EigenPlane();
    
public:
    double PointDistance(vec3d q) { return q*p_n - p_d; }
    vec3d PointProjection(vec3d q) { return q - p_n*PointDistance(q); }
    vec2d ParametricPointProjection(vec3d q);

    // Rotate
    void Rotate(mat3d R) { p_n = R*p_n; }
    
    // Translate
    void Translate(vec3d c) { p_d += c*p_n; }
    
private:
    vec3d   p_n;        // plane unit normal
    double  p_d;        // closest distance to origin
public:
    double  p_eval[3];  // eigenvalues of plane (optional)
    vec3d   p_evec[3];  // eigenvectors of plane (optional)

};
#endif /* defined(__libspg__Plane__) */
