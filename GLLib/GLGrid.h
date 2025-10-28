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
#include <FSCore/color.h>

class GLRenderEngine;
class GLCamera;

class GLGrid
{
public:
	GLGrid();

	vec3d Intersect(vec3d r, vec3d t, bool bsnap);

	double GetScale() { return m_scale; }

	void Render(GLRenderEngine& re, const GLCamera& cam);

	vec3d WorldToPlane(const vec3d& r) const
	{
		return m_q.Inverse() * (r - m_o);
	}

	void SetAxesColors(GLColor cx, GLColor cy) { m_colx = cx; m_coly = cy; }

	void SetOrientation(const quatd& q) { m_q = q; }

protected:
	vec3d Snap(vec3d r);

public:
	vec3d	m_o;	// plane origin
	quatd	m_q;	// plane orientation
	int m_ndiv = 2;

	double	m_scale;	// scale of grid (ie. distance between lines)

	GLColor m_colx, m_coly;
};
