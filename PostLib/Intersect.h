#pragma once
#include "math3d.h"
#include "FEMesh.h"

namespace Post {

// tools for finding intersections
struct Ray
{
	vec3f	origin;			// origin of ray
	vec3f	direction;		// direction of ray (must be unit vector!)
};

//-----------------------------------------------------------------------------
struct Intersection
{
	vec3f	point;		// point of intersection
	float	r[2];		// natural coordinates
	int		m_index;	// index of item that was intersected (context dependent)
};

//-----------------------------------------------------------------------------
struct Triangle
{
	vec3f	r0;
	vec3f	r1;
	vec3f	r2;
	vec3f	fn;	// face normal
};

//-----------------------------------------------------------------------------
struct Quad
{
	vec3f	r0;
	vec3f	r1;
	vec3f	r2;
	vec3f	r3;
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
bool FindFaceIntersection(const Ray& ray, const Post::FEMeshBase& mesh, const Post::FEFace& face, Intersection& q);
}
