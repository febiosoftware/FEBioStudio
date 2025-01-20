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
#include "FSMesh.h"
#include <FSCore/math3d.h>
#include "FSMeshBase.h"
#include "FSCoreMesh.h"

// project a node to a particular edge
vec3d projectToEdge(const FSMeshBase& m, const vec3d& p, int gid);

//-------------------------------------------------------------------------------------------------
// project a node to a particular surface of a mesh
// Input:
// - m   = the mesh
// - p   = the point that will be projected
// - gid = if gid != -1, the point will only be projected on the facets that have the same gid
// Output:
// - faceID = the ID of the face where the projection occurred
// Return: a point that falls on the projection
// 
struct Intersection; // definde in Intersection.h
vec3d projectToSurface(const FSMeshBase& m, const vec3d& p, int gid = -1, int* faceID = nullptr, Intersection* intersect = nullptr);

// prjoect to a patch of a surface
vec3d projectToPatch(const FSMeshBase& m, const vec3d& p, int gid, int faceID, int l);

// calculate area of triangle
double area_triangle(vec3d r[3]);
double area_triangle(const vec3d& a, const vec3d& b, const vec3d& c);

//fit a circle to a triangle
void fitCircle(const vec3d a[3], vec3d& o, double& R);

// calculate intersection with triangle
bool IntersectTri(vec3d* y, vec3d x, vec3d n, vec3d& q, double& g);

// calculate intersection with quad
bool IntersectQuad(vec3d* y, vec3d x, vec3d n, vec3d& q, double& g);

// finds the closest surface node to point r that is facing r. The vector t defines
// the direction in which point r is looking.
vec3d ClosestNodeOnSurface(FSMesh& mesh, const vec3d& r, const vec3d& t);

vec3d ProjectToFace(FSMesh& mesh, vec3d p, FSFace& f, double& r, double& s, bool bedge = true);
vec3d ProjectToEdge(vec3d e1, vec3d e2, vec3d p, double& r);

// project to a triangle
bool projectToTriangle(const vec3d& p, const vec3d& r0, const vec3d& r1, const vec3d& r2, vec3d& q, Intersection* intersect = nullptr);

// project to a quad
bool projectToQuad(const vec3d& p, const vec3d y[4], vec3d& q, Intersection* intersect = nullptr);

bool FindIntersection(FSMeshBase& mesh, const vec3d& x, const vec3d& n, vec3d& q, bool snap = false);
bool FindIntersection(FSMeshBase& mesh, FSFace& f, const vec3d& x, const vec3d& n, vec3d& q, double& g);

std::vector<vec3d> FindAllIntersections(FSMeshBase& mesh, const vec3d& x, const vec3d& n, bool forwardOnly = true);

// calculate edge intersection
int edgeIntersect(const vec3d& r0, const vec3d& r1, const vec3d& r2, const vec3d& r3, vec3d N, vec3d& q, double& L, const double eps);

int edgeIntersect(const vec3d& r0, const vec3d& r1, const vec3d& r2, const vec3d& r3, vec3d& q, double& L, const double eps);

// this calculates the ratio of the shortest diagonal to the longest edge
double TriangleQuality(vec3d r[3]);

// calculates the max dihedral angle of a face
double TriMaxDihedralAngle(const FSMeshBase& mesh, const FSFace& face);

// normal to face
inline vec3d faceNormal(const vec3d& a, const vec3d& b, const vec3d& c)
{
	vec3d N = (b - a)^(c - a); N.Normalize();
	return N;
}

// normal to face
inline vec3d faceNormal(const vec3d r[3])
{
	vec3d N = (r[1] - r[0]) ^ (r[2] - r[0]); N.Normalize();
	return N;
}


// find the element and the iso-parametric coordinates of a point inside the mesh
bool FindElementRef(FSCoreMesh& m, const vec3f& x, int& nelem, double r[3]);

// project the point p in the current frame of element el. This returns the iso-parametric coordinates in r.
// The return value is true or false depending if the point is actually inside the element
bool ProjectInsideElement(FSCoreMesh& m, FSElement_& el, const vec3f& p, double r[3]);

bool ProjectToElement(FSElement& el, const vec3f& p, vec3f* x0, vec3f* xt, vec3f& q);

bool IsInsideElement(FSElement_& el, double r[3], const double tol);

void project_inside_element(FSElement_& el, const vec3f& p, double r[3], vec3f* x);

bool project_inside_element2d(FSElement_& el, vec3d* x, const vec2d& p, double q[2]);

// project the point p in the reference frame of element el. This returns the iso-parametric coordinates in r.
// The return value is true or false depending if the point is actually inside the element
bool ProjectInsideReferenceElement(FSCoreMesh& m, FSElement_& el, const vec3f& p, double r[3]);

// find the shortest path on a mesh (along the mesh' edges)
std::vector<vec3d> FindShortestPath(FSMesh& mesh, int m0, int m1);

// find the element and the iso-parametric coordinates of a point inside the mesh
// the x coordinates is assumed to be in reference frame
bool FindElementInReferenceFrame(FSCoreMesh& m, const vec3f& x, int& nelem, double r[3]);
