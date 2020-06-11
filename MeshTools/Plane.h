/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
