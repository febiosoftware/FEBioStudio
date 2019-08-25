#include "Plane.h"

void Plane::EigenPlane()
{
    // evaluate A = I - n (x) n
    mat3ds A = mat3dd(1.0) - dyad(p_n);
    
    // get eigenvalues and eigenvectors of A
    // eigenvalues are sorted from smallest to largest
    A.eigen2(p_eval,p_evec);
}

vec2d Plane::ParametricPointProjection(vec3d x)
{
    vec3d p = p_n*p_d;
    
    vec3d q = x - p;
    double u = (q*p_evec[2])/p_eval[2];
    double v = (q*p_evec[1])/p_eval[1];
    vec2d uv(u,v);

    return uv;
}
