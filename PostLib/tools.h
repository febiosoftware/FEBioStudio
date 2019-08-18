#pragma once
#include "math3d.h"

// closest point projection of x onto the triangle defined by y
bool ProjectToTriangle(vec3f* y, vec3f& x, vec3f& q, double tol);

// closest point projection of x onto a quadrilateral surface defined by y
bool ProjectToQuad(vec3f* y, vec3f& x, vec3f& q, double tol);

// Projection of x, along direction t, onto triangle defined by y
bool ProjectToTriangle(vec3f* y, vec3f& x, vec3f& t, vec3f& q, double tol);

// Projection of x, along direction t, onto quad defined by y
bool ProjectToQuad(vec3f* y, vec3f& x, vec3f& t, vec3f& q, double tol);
