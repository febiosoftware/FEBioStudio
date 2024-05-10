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

#include "stdafx.h"
#include "box.h"
#include <FSCore/math3d.h>

//-----------------------------------------------------------------------------
// Default constructor
BOX::BOX()
{ 
	x0 = y0 = z0 = 0;
	x1 = y1 = z1 = 0; 
	m_valid = false;
}

//-----------------------------------------------------------------------------
// Constructor from coordinates
BOX::BOX(double X0, double Y0, double Z0, double X1, double Y1, double Z1)
{ 
	x0 = X0; y0 = Y0; z0 = Z0; 
	x1 = X1; y1 = Y1; z1 = Z1; 
	m_valid = true;
}

//-----------------------------------------------------------------------------
// constructor from vectors
BOX::BOX(const vec3d& r0, const vec3d& r1)
{
	x0 = r0.x; x1 = r1.x;
	y0 = r0.y; y1 = r1.y;
	z0 = r0.z; z1 = r1.z;
	m_valid = true;
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
	if (!b.IsValid()) return *this;
	if (m_valid)
	{
		x0 = MIN(x0, b.x0);
		y0 = MIN(y0, b.y0);
		z0 = MIN(z0, b.z0);
		x1 = MAX(x1, b.x1);
		y1 = MAX(y1, b.y1);
		z1 = MAX(z1, b.z1);
	}
	else
	{
		*this = b;
		m_valid = true;
	}

	return (*this);
}

//-----------------------------------------------------------------------------
bool BOX::IsInside(const vec3d& r) const
{
	if ((r.x >= x0) && (r.x <= x1) && 
		(r.y >= y0) && (r.y <= y1) && 
		(r.z >= z0) && (r.z <= z1)) return true;

	return false;
}

//-----------------------------------------------------------------------------
void BOX::operator += (const vec3d& r)
{
	if (m_valid == false)
	{
		x0 = x1 = r.x;
		y0 = y1 = r.y;
		z0 = z1 = r.z;
		m_valid = true;
	}
	else
	{
		if (r.x < x0) x0 = r.x;
		if (r.y < y0) y0 = r.y;
		if (r.z < z0) z0 = r.z;

		if (r.x > x1) x1 = r.x;
		if (r.y > y1) y1 = r.y;
		if (r.z > z1) z1 = r.z;
	}
}

//-----------------------------------------------------------------------------
void BOX::Inflate(double dx, double dy, double dz)
{
	x0 -= dx; x1 += dx;
	y0 -= dy; y1 += dy;
	z0 -= dz; z1 += dz;
}

//-----------------------------------------------------------------------------
void BOX::InflateTo(double fx, double fy, double fz)
{
	double xc = x0 + x1;
	double yc = y0 + y1;
	double zc = z0 + z1;
	x0 = (xc - fx)*0.5; x1 = (xc + fx)*0.5;
	y0 = (yc - fy)*0.5; y1 = (yc + fy)*0.5;
	z0 = (zc - fz)*0.5; z1 = (zc + fz)*0.5;
}

//-----------------------------------------------------------------------------
void BOX::Inflate(double f)
{
	x0 -= f; x1 += f;
	y0 -= f; y1 += f;
	z0 -= f; z1 += f;
}

//-----------------------------------------------------------------------------
void BOX::Scale(double s)
{
	vec3d c = Center();
	double dx = 0.5*s*Width();
	double dy = 0.5*s*Height();
	double dz = 0.5*s*Depth();
	x0 = c.x - dx; x1 = c.x + dx;
	y0 = c.y - dy; y1 = c.y + dy;
	z0 = c.z - dz; z1 = c.z + dz;
}

//-----------------------------------------------------------------------------
vec3d BOX::r0() const
{
	return vec3d(x0, y0, z0);
}

//-----------------------------------------------------------------------------
vec3d BOX::r1() const
{
	return vec3d(x1, y1, z1);
}

BOX LocalToGlobalBox(const BOX& box, const Transform& T)
{
	vec3d a = box.r0();
	vec3d b = box.r1();

	BOX globalBox;
	globalBox += T.LocalToGlobal(vec3d(a.x, a.y, a.z));
	globalBox += T.LocalToGlobal(vec3d(a.x, a.y, a.z));
	globalBox += T.LocalToGlobal(vec3d(b.x, a.y, a.z));
	globalBox += T.LocalToGlobal(vec3d(b.x, b.y, a.z));
	globalBox += T.LocalToGlobal(vec3d(a.x, b.y, a.z));
	globalBox += T.LocalToGlobal(vec3d(a.x, a.y, b.z));
	globalBox += T.LocalToGlobal(vec3d(b.x, a.y, b.z));
	globalBox += T.LocalToGlobal(vec3d(b.x, b.y, b.z));
	globalBox += T.LocalToGlobal(vec3d(a.x, b.y, b.z));
	return globalBox;
}
