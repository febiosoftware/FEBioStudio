#pragma once
#include "math3d.h"

// extract components of tensor types
float component(const vec3f& v, int n);
float component2(const vec3f& v, int n);
float component(const Mat3d& m, int n);
float component(const mat3f& m, int n);
float component(const mat3fs& m, int n);
float component(const mat3fd& m, int n);
float component(const tens4fs& m, int n);
