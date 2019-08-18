#include "bbox.h"

//-----------------------------------------------------------------------------
// Default constructor
BOUNDINGBOX::BOUNDINGBOX()
{
	x0 = y0 = z0 = 0;
	x1 = y1 = z1 = 0;
	bvalid = false;
}

//-----------------------------------------------------------------------------
// constructor from coordinates
BOUNDINGBOX::BOUNDINGBOX(float X0, float Y0, float Z0, float X1, float Y1, float Z1)
{
	x0 = X0; y0 = Y0; z0 = Z0;
	x1 = X1; y1 = Y1; z1 = Z1;
	bvalid = true;
}

//-----------------------------------------------------------------------------
// constructor from vectors
BOUNDINGBOX::BOUNDINGBOX(const vec3f& r0, const vec3f& r1)
{
	x0 = r0.x; y0 = r0.y; z0 = r0.z;
	x1 = r1.x; y1 = r1.y; z1 = r1.z;
	bvalid = true;
}

//-----------------------------------------------------------------------------
float BOUNDINGBOX::GetMaxExtent() const
{
	float w = Width();
	float h = Height();
	float d = Depth();

	if (w>=h && w>=d) return w;
	if (h>=w && h>=d) return h;
	
	return d;
}

//-----------------------------------------------------------------------------
// return the center of the box
vec3f BOUNDINGBOX::Center() const
{
	return vec3f((x0+x1)*0.5f,(y0+y1)*0.5f,(z0+z1)*0.5f); 
}

//-----------------------------------------------------------------------------
// The radius is half the distance between the two corners.
// (I think this is also the radius of the circumscribed sphere)
float BOUNDINGBOX::Radius() const
{
	return 0.5f* (float) sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) + (z1-z0)*(z1-z0));
}

//-----------------------------------------------------------------------------
void BOUNDINGBOX::Range(vec3f& n, float& min, float& max)
{
	min = max = n*vec3f(x0, y0, z0);
	float val;

	val = n*vec3f(x1, y0, z0); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3f(x1, y1, z0); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3f(x0, y1, z0); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3f(x0, y0, z1); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3f(x1, y0, z1); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3f(x1, y1, z1); if (val < min) min = val; else if (val > max) max = val;
	val = n*vec3f(x0, y1, z1); if (val < min) min = val; else if (val > max) max = val;
}

//-----------------------------------------------------------------------------
void BOUNDINGBOX::Range(vec3f& n, vec3f& r0, vec3f& r1)
{
	float min, max;
	min = max = n*vec3f(x0, y0, z0);
	r0 = r1 = vec3f(x0, y0, z0);
	float val;
	vec3f r;

	r = vec3f(x1, y0, z0); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3f(x1, y1, z0); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3f(x0, y1, z0); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3f(x0, y0, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3f(x1, y0, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3f(x1, y1, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
	r = vec3f(x0, y1, z1); val = n*r; if (val < min) { r0 = r; min = val; } else if (val > max) { r1 = r; max = val; }
}

//-----------------------------------------------------------------------------
void BOUNDINGBOX::operator +=(const vec3f& r)
{
	if (bvalid == false)
	{
		x0 = x1 = r.x;
		y0 = y1 = r.y;
		z0 = z1 = r.z;
		bvalid = true;
	}
	else
	{
		if (r.x < x0) x0 = r.x; if (r.x > x1) x1 = r.x;
		if (r.y < y0) y0 = r.y; if (r.y > y1) y1 = r.y;
		if (r.z < z0) z0 = r.z; if (r.z > z1) z1 = r.z;
	}
}

//-----------------------------------------------------------------------------
void BOUNDINGBOX::InflateTo(float fx, float fy, float fz)
{
	if (bvalid)
	{
		float xc = x0 + x1;
		float yc = y0 + y1;
		float zc = z0 + z1;
		x0 = (xc - fx)*0.5f; x1 = (xc + fx)*0.5f;
		y0 = (yc - fy)*0.5f; y1 = (yc + fy)*0.5f;
		z0 = (zc - fz)*0.5f; z1 = (zc + fz)*0.5f;
	}
}

//-----------------------------------------------------------------------------
void BOUNDINGBOX::Inflate(float fx, float fy, float fz)
{
	if (bvalid)
	{
		x0 -= fx; x1 += fx;
		y0 -= fy; y1 += fy;
		z0 -= fz; z1 += fz;
	}
}

//-----------------------------------------------------------------------------
void BOUNDINGBOX::Inflate(float f)
{
	if (bvalid)
	{
		x0 -= f; x1 += f;
		y0 -= f; y1 += f;
		z0 -= f; z1 += f;
	}
}

//-----------------------------------------------------------------------------
vec3f BOUNDINGBOX::r0() const
{
	return vec3f(x0, y0, z0);
}

//-----------------------------------------------------------------------------
vec3f BOUNDINGBOX::r1() const
{
	return vec3f(x1, y1, z1);
}
