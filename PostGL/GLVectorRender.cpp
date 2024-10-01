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
#include "GLVectorRender.h"
#ifdef __APPLE__
#include <OpenGL/GLU.h>
#elif WIN32
#include <Windows.h>
#include <GL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <GLLib/glx.h>

class GLVectorRenderer::Imp
{
public:
	int		lineStyle = 0;
	double	lineWidth = 1.0;
	double	scale = 1.0;
	GLUquadricObj* glyph = nullptr;

	std::vector<VECTOR>	vectors;
};

GLVectorRenderer::GLVectorRenderer() : m(*(new Imp)) {
	m.vectors.reserve(4096);
}

void GLVectorRenderer::AddVector(const GLVectorRenderer::VECTOR& vector)
{
	m.vectors.push_back(vector);
}

void GLVectorRenderer::Clear() { m.vectors.clear(); }

void GLVectorRenderer::SetScaleFactor(double s) { m.scale = s; }
void GLVectorRenderer::SetLineStyle(int n) { m.lineStyle = n; }
void GLVectorRenderer::SetLineWidth(double l) { m.lineWidth = l; }

void GLVectorRenderer::Init()
{
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	if (m.lineStyle == 0)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_LINES);
	}
	else
	{
		m.glyph = gluNewQuadric();
		gluQuadricNormals(m.glyph, GLU_SMOOTH);
	}
}

void GLVectorRenderer::RenderVectors()
{
	for (auto& vector : m.vectors)
		RenderVector(vector);
}

void GLVectorRenderer::RenderVector(const GLVectorRenderer::VECTOR& vector)
{
	vec3d p0 = vector.r - vector.n * (m.scale * 0.5);
	vec3d p1 = vector.r + vector.n * (m.scale * 0.5);

	glColor3ub(vector.c.r, vector.c.g, vector.c.b);
	if (m.lineStyle == 0)
	{
		glVertex3d(p0.x, p0.y, p0.z);
		glVertex3d(p1.x, p1.y, p1.z);
	}
	else
	{
		glPushMatrix();

		glx::translate(p0);
		quatd Q(vec3d(0, 0, 1), vector.n);
		glx::rotate(Q);

		gluCylinder(m.glyph, m.lineWidth, m.lineWidth, m.scale, 10, 1);

		glPopMatrix();
	}
}

void GLVectorRenderer::Finish()
{
	if (m.lineStyle == 0)
	{
		glEnd(); // GL_LINES
	}
	else
	{
		gluDeleteQuadric(m.glyph);
	}
	glPopAttrib();
}
