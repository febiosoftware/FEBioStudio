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
#include <FSCore/math3d.h>
#include <MeshLib/FEMeshBase.h>
#include <MeshLib/FEMesh.h>
#include <MeshLib/GMesh.h>

// tools for finding intersections
struct Ray
{
	vec3d	origin;			// origin of ray
	vec3d	direction;		// direction of ray (must be unit vector!)
};

//-----------------------------------------------------------------------------
struct Intersection
{
	vec3d	point;			// point of intersection
	float	r[2];			// natural coordinates
	int		m_index;		// index of item that was intersected (context dependent)
	int		m_faceIndex;	// index of face that was intersected (context dependent)
};

//-----------------------------------------------------------------------------
struct Triangle
{
	vec3d	r0;
	vec3d	r1;
	vec3d	r2;
	vec3d	fn;	// face normal
};

//-----------------------------------------------------------------------------
struct Quad
{
	vec3d	r0;
	vec3d	r1;
	vec3d	r2;
	vec3d	r3;
};

//-----------------------------------------------------------------------------
// Find intersection of a ray with a triangle
// To evaluate the normal automatically, set evalNormal to true. Otherwise, the normal in Triangle is used
bool IntersectTriangle(const Ray& ray, const Triangle& tri, Intersection& q, bool evalNormal = true);

//-----------------------------------------------------------------------------
// Find intersection of a ray with a quad
bool IntersectQuad(const Ray& ray, const Quad& quad, Intersection& q);
bool FastIntersectQuad(const Ray& ray, const Quad& quad, Intersection& q);

//-----------------------------------------------------------------------------
bool FindFaceIntersection(const Ray& ray, const FSMeshBase& mesh, Intersection& q);
bool FindFaceIntersection(const Ray& ray, const GMesh& mesh, Intersection& q);
bool FindFaceIntersection(const Ray& ray, const FSMeshBase& mesh, const FSFace& face, Intersection& q);

bool RayIntersectFace(const Ray& ray, int faceType, vec3d* rn, Intersection& q);

//-----------------------------------------------------------------------------
bool FindElementIntersection(const Ray& ray, const FSMesh& mesh, Intersection& q, bool selectionState = false);
