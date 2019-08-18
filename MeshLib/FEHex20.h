#pragma once
#include <MathLib/mat3d.h>

class FEHex20
{
public:
	static void shape_fnc(double* H, double r, double s, double t);
	static vec3d eval(const vec3d* x, double r, double s, double t);
};

class FEQuad8
{
public:
	static void shape_fnc(double* H, double r, double s);
	static vec3d eval(const vec3d* x, double r, double s);
};
