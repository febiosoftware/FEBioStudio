#include "box.h"

//-----------------------------------------------------------------------------
// Default constructor
BOX::BOX()
{ 
	x0 = y0 = z0 = 0;
	x1 = y1 = z1 = 0; 
}

//-----------------------------------------------------------------------------
// Constructor from coordinates
BOX::BOX(double X0, double Y0, double Z0, double X1, double Y1, double Z1)
{ 
	x0 = X0; y0 = Y0; z0 = Z0; 
	x1 = X1; y1 = Y1; z1 = Z1; 
}

//-----------------------------------------------------------------------------
// constructor from vectors
BOX::BOX(const vec3d& r0, const vec3d& r1)
{
	x0 = r0.x; x1 = r1.x;
	y0 = r0.y; y1 = r1.y;
	z0 = r0.z; z1 = r1.z;
}

//-----------------------------------------------------------------------------
// return largest dimension of box
double BOX::GetMaxExtent() const
{
	double w = Width();
	double h = Height();
	double d = Depth();

	if (w>=h && w>=d) return w;
	if (h>=w && h>=d) return h;
	
	return d;
}

//-----------------------------------------------------------------------------
// return the center of the box
vec3d BOX::Center() const
{ 
	return vec3d((x0+x1)*0.5f, (y0+y1)*0.5f, (z0+z1)*0.5f); 
}

//-----------------------------------------------------------------------------
// The radius is half the distance between the two corners.
// (I think this is also the radius of the circumscribed sphere)
double BOX::Radius() const
{
	return 0.5*sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) + (z1-z0)*(z1-z0));
}

//-----------------------------------------------------------------------------
void BOX::Range(vec3d& n, double& min, double& max)
{
	min = max = n*vec3d(x0, y0, z0);
	double val;

	val = n*vec3d(x1, y0, z0); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3d(x1, y1, z0); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3d(x0, y1, z0); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3d(x0, y0, z1); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3d(x1, y0, z1); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3d(x1, y1, z1); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3d(x0, y1, z1); if (val < min) min = val; else if (val > max) max = val;
}

//-----------------------------------------------------------------------------
void BOX::Range(vec3d& n, vec3d& r0, vec3d& r1)
{
	double min, max;
	min = max = n*vec3d(x0, y0, z0);
	r0 = r1 = vec3d(x0, y0, z0);
	double val;
	vec3d r;

	r = vec3d(x1, y0, z0); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3d(x1, y1, z0); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3d(x0, y1, z0); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3d(x0, y0, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3d(x1, y0, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3d(x1, y1, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3d(x0, y1, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
}

//-----------------------------------------------------------------------------
BOX BOX::operator + (const BOX& b)
{
	BOX a;
	a.x0 = MIN(x0, b.x0);
	a.x1 = MAX(x1, b.x1);
	a.y0 = MIN(y0, b.y0);
	a.y1 = MAX(y1, b.y1);
	a.z0 = MIN(z0, b.z0);
	a.z1 = MAX(z1, b.z1);

	return a;
}

//-----------------------------------------------------------------------------
BOX& BOX::operator += (const BOX& b)
{
	x0 = MIN(x0, b.x0);
	y0 = MIN(y0, b.y0);
	z0 = MIN(z0, b.z0);
	x1 = MAX(x1, b.x1);
	y1 = MAX(y1, b.y1);
	z1 = MAX(z1, b.z1);

	return (*this);
}

//-----------------------------------------------------------------------------
bool BOX::IsInside(const vec3d& r)
{
	if ((r.x >= x0) && (r.x <= x1) && 
		(r.y >= y0) && (r.y <= y1) && 
		(r.z >= z0) && (r.z <= z1)) return true;

	return false;
}

//-----------------------------------------------------------------------------
void BOX::operator += (const vec3d& r)
{
	if (r.x < x0) x0 = r.x; 
	if (r.y < y0) y0 = r.y; 
	if (r.z < z0) z0 = r.z; 

	if (r.x > x1) x1 = r.x; 
	if (r.y > y1) y1 = r.y; 
	if (r.z > z1) z1 = r.z; 
}

//-----------------------------------------------------------------------------
void BOX::Inflate(double dx, double dy, double dz)
{
	x0 -= dx; x1 += dx;
	y0 -= dy; y1 += dy;
	z0 -= dz; z1 += dz;
}
