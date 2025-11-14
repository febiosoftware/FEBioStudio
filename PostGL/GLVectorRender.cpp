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
#include "GLVectorRender.h"
#include <GLLib/glx.h>
#include <GLLib/GLRenderEngine.h>
#include <GLLib/GLMeshBuilder.h>

class GLVectorRenderer::Imp
{
public:
	int		lineStyle = 0;
	double	lineWidth = 1.0;
	double	scale = 1.0;
	double	density = 1.0;

	GLMesh* mesh = nullptr; // the mesh that will be used to render the vectors

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

void GLVectorRenderer::SetScaleFactor(double s) 
{ 
	if (s != m.scale)
	{
		delete m.mesh;
		m.mesh = nullptr;
		m.scale = s;
	}
}

void GLVectorRenderer::SetLineStyle(int n) 
{ 
	if (n != m.lineStyle)
	{
		delete m.mesh;
		m.mesh = nullptr;
		m.lineStyle = n;
	}
}
void GLVectorRenderer::SetLineWidth(double l) 
{ 
	if (l != m.lineWidth)
	{
		delete m.mesh;
		m.mesh = nullptr;
		m.lineWidth = l;
	}
}
void GLVectorRenderer::SetDensity(double d) 
{ 
	if (d != m.density)
	{
		delete m.mesh;
		m.mesh = nullptr;
		m.density = d;
	}
}

void GLVectorRenderer::Init(GLRenderEngine& re)
{
	re.beginShape();
	if (m.lineStyle == 0)
	{
		re.setMaterial(GLMaterial::OVERLAY, GLColor::White(), GLMaterial::VERTEX_COLOR);
		re.begin(GLRenderEngine::LINES);
	}
	else
	{
		re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::VERTEX_COLOR);
	}
}

void GLVectorRenderer::RenderVectors(GLRenderEngine& re)
{
	if (m.mesh == nullptr)
	{
		GLMeshBuilder mb;
		Init(mb);
		srand(0);
		for (auto& vector : m.vectors)
		{
			double r = (double)rand() / (double)RAND_MAX;
			if (r < m.density)
				RenderVector(mb, vector);
		}
		Finish(mb);

		m.mesh = mb.takeMesh();

		// just to be sure in case m.mesh was allocated at the same address as the last one
		if (m.mesh) re.deleteCachedMesh(m.mesh);
	}

	if (m.mesh)
	{
		re.pushTransform();
		if (m.lineStyle == 0)
		{
			re.setMaterial(GLMaterial::OVERLAY, GLColor::White(), GLMaterial::VERTEX_COLOR);
			re.renderGMeshEdges(*m.mesh);
		}
		else
		{
			re.setMaterial(GLMaterial::PLASTIC, GLColor::White(), GLMaterial::VERTEX_COLOR);
			re.renderGMesh(*m.mesh);
		}

		re.popTransform();
	}
}

void GLVectorRenderer::RenderVector(GLRenderEngine& re, const GLVectorRenderer::VECTOR& vector)
{
	vec3d p0 = vector.r - vector.n * (m.scale * 0.5);
	vec3d p1 = vector.r + vector.n * (m.scale * 0.5);

	re.setColor(vector.c);
	if (m.lineStyle == 0)
	{
		re.vertex(p0);
		re.vertex(p1);
	}
	else
	{
		re.pushTransform();

		re.transform(p0, quatd(vec3d(0, 0, 1), vector.n));
		glx::drawCylinder(re, (float)m.lineWidth, (float)m.scale, 10);
		re.popTransform();
	}
}

void GLVectorRenderer::Finish(GLRenderEngine& re)
{
	if (m.lineStyle == 0)
	{
		re.end(); // LINES
	}
	re.endShape();
}
