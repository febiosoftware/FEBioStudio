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
#include "GLRenderEngine.h"

void GLRenderEngine::ResetStats()
{
	m_stats.clear();
}

GLRenderStats GLRenderEngine::GetRenderStats()
{
	return m_stats;
}

void GLRenderEngine::transform(const vec3d& r0, const vec3d& r1)
{
	translate(r0);
	rotate(r1 - r0);
}

void GLRenderEngine::rotate(const vec3d& r, vec3d ref)
{
	vec3d n = r; n.Normalize();
	vec3d z = ref; z.Normalize();
	if ((z * n) == -1)
	{
		vec3d e(1, 0, 0);
		if (fabs(e * n) > 0.9) e = vec3d(0, 1, 0);
		vec3d t = (n ^ e);
		rotate(180, t.x, t.y, t.z);
	}
	else if (z * n != 1)
	{
		quatd q(z, n);
		rotate(q);
	}
}

void GLRenderEngine::renderPoint(const vec3d& r)
{
	begin(POINTS);
	{
		vertex(r);
	}
	end();
}

void GLRenderEngine::renderLine(const vec3d& a, const vec3d& b)
{
	begin(LINES);
	{
		vertex(a);
		vertex(b);
	}
	end();
}

void GLRenderEngine::renderPath(const std::vector<vec3d>& points)
{
	if (points.size() < 2) return;
	begin(LINESTRIP);
	{
		for (const vec3d& p : points) vertex(p);
	}
	end();
}

void GLRenderEngine::renderRect(double x0, double y0, double x1, double y1)
{
	begin(QUADS);
	{
		vertex(vec3d(x0, y0, 0));
		vertex(vec3d(x1, y0, 0));
		vertex(vec3d(x1, y1, 0));
		vertex(vec3d(x0, y1, 0));
	}
	end();
}

void GLRenderEngine::renderLine(const vec3d& a, const vec3d& b, const vec3d& c)
{
	begin(LINESTRIP);
	{
		vertex(a);
		vertex(b);
		vertex(c);
	}
	end();
}
