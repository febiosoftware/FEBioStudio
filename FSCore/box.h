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
#include <FECore/FETransform.h>

//-----------------------------------------------------------------------------
// Class that describes a bounding box structure
class BOX
{
public:
	double x0, y0, z0;	
	double x1, y1, z1;
	bool	m_valid;

public:
	// default constructor
	BOX();

	// constructor from coordinates
	BOX(double X0, double Y0, double Z0, double X1, double Y1, double Z1);

	// constructor from vectors
	BOX(const vec3d& r0, const vec3d& r1);

	// is the box valid
	bool IsValid() const { return m_valid; }

	// size of box
	double Width () const { return x1 - x0; }
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
	bool IsInside(const vec3d& r) const;

	// see if this box intersects another box
	bool Intersects(const BOX& b) const;

	// add a point to the box
	void operator += (const vec3d& r);

	// inflate the box
	void Inflate(double dx, double dy, double dz);
	void InflateTo(double fx, double fy, double fz);
	void Inflate(double f);
	void Scale(double f);

	// get the coordinates of the box
	vec3d r0() const;
	vec3d r1() const;
};

inline bool BOX::Intersects(const BOX& b) const
{
	if ((b.x0 > x1) || (b.x1 < x0)) return false;
	if ((b.y0 > y1) || (b.y1 < y0)) return false;
	if ((b.z0 > z1) || (b.z1 < z0)) return false;
	return true;
}

BOX LocalToGlobalBox(const BOX& box, const Transform& T);
