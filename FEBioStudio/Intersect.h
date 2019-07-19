#pragma once
#include <MathLib/math3d.h>
#include <MeshLib/FEMeshBase.h>
#include <MeshLib/FEMesh.h>
#include <PreViewLib/GLMesh.h>

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
bool IntersectTriangle(const Ray& ray, const Triangle& tri, Intersection& q);

//-----------------------------------------------------------------------------
// Find intersection of a ray with a quad
bool IntersectQuad(const Ray& ray, const Quad& quad, Intersection& q);
bool FastIntersectQuad(const Ray& ray, const Quad& quad, Intersection& q);

//-----------------------------------------------------------------------------
bool FindFaceIntersection(const Ray& ray, const FEMeshBase& mesh, Intersection& q);
bool FindFaceIntersection(const Ray& ray, const GLMesh& mesh, Intersection& q);

//-----------------------------------------------------------------------------
bool FindElementIntersection(const Ray& ray, const FEMesh& mesh, Intersection& q, bool selectionState = false);
