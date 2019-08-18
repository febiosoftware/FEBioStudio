#pragma once
#include "MathLib/math3d.h"

//-----------------------------------------------------------------------------
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

//-----------------------------------------------------------------------------
// Class that describes a bounding box structure
class BOX
{
public:
	double x0, y0, z0;	
	double x1, y1, z1;

public:
	// default constructor
	BOX();

	// constructor from coordinates
	BOX(double X0, double Y0, double Z0, double X1, double Y1, double Z1);

	// constructor from vectors
	BOX(const vec3d& r0, const vec3d& r1);

	// size of box
	double Width () const { return x1-x0; }
	double Height() const { return y1 - y0; }
	double Depth () const { return z1 - z0; }

	// return largest dimension of box
	double GetMaxExtent() const;

	// return the center of the box
	vec3d Center() const;

	// return the radius of the box
	double Radius() const;

	// range of box
	void Range(vec3d& n, double& min, double& max);
	void Range(vec3d& n, vec3d& r0, vec3d& r1);

	// box operations
	BOX operator + (const BOX& b);

	// box operations
	BOX& operator += (const BOX& b);

	// see if a point is inside the box
	bool IsInside(const vec3d& r);

	// add a point to the box
	void operator += (const vec3d& r);

	// inflate the box
	void Inflate(double dx, double dy, double dz);
};
