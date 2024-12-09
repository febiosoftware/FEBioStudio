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
#include "OpenGLRenderer.h"
#include <MeshLib/GMesh.h>
#include <qopengl.h>
#include <QImage>
#include <GLLib/GLMesh.h>
#include <map>

class OpenGLRenderer::Imp {
public:
	CGLSceneView* glv;

	std::map<const GMesh*, GLMesh*> mesh;
};

OpenGLRenderer::OpenGLRenderer(CGLSceneView* view) : m(*(new OpenGLRenderer::Imp))
{
	m.glv = view;
}

OpenGLRenderer::~OpenGLRenderer() 
{
}

void OpenGLRenderer::pushState()
{
	glPushAttrib(GL_ENABLE_BIT);
}

void OpenGLRenderer::popState()
{
	glPopAttrib();
}

void OpenGLRenderer::enable(GLRenderEngine::StateFlag flag)
{
	switch (flag)
	{
	case GLRenderEngine::LIGHTING: glEnable(GL_LIGHTING); break;
	}
}

void OpenGLRenderer::disable(GLRenderEngine::StateFlag flag)
{
	switch (flag)
	{
	case GLRenderEngine::LIGHTING: glDisable(GL_LIGHTING); break;
	}
}

void OpenGLRenderer::setColor(GLColor c)
{
	glColor4ub(c.r, c.g, c.b, c.a);
}

void OpenGLRenderer::setMaterial(GLRenderEngine::MaterialType mat, GLColor c)
{
	glEnable(GL_COLOR_MATERIAL);
	glColor4ub(c.r, c.g, c.b, c.a);
}

void OpenGLRenderer::renderPoint(const vec3d& r)
{
	glBegin(GL_POINTS);
	{
		glVertex3d(r.x, r.y, r.z);
	}
	glEnd();
}

void OpenGLRenderer::renderLine(const vec3d& a, const vec3d& b)
{
	glBegin(GL_LINES);
	{
		glVertex3d(a.x, a.y, a.z);
		glVertex3d(b.x, b.y, b.z);
	}
	glEnd();
}

void OpenGLRenderer::renderGMesh(const GMesh& mesh)
{
	GLTriMesh* glm = nullptr;
	auto it = m.mesh.find(&mesh);
	if (it == m.mesh.end())
	{
		glm = new GLTriMesh;
		glm->SetRenderMode(GLMesh::VBOMode);
		glm->CreateFromGMesh(mesh, GLMesh::FLAG_NORMAL);
		m.mesh[&mesh] = glm;
	}
	else
	{
		GLMesh* glm = it->second;
	}

	if (glm) glm->Render();

/*
	glBegin(GL_TRIANGLES);
	{
		for (int i = 0; i < mesh.Faces(); ++i)
		{
			const GMesh::FACE& f = mesh.Face(i);
			glNormal3fv(&f.vn[0].x); glVertex3fv(&f.vr[0].x);
			glNormal3fv(&f.vn[1].x); glVertex3fv(&f.vr[1].x);
			glNormal3fv(&f.vn[2].x); glVertex3fv(&f.vr[2].x);
		}
	}
	glEnd();
*/
}

void OpenGLRenderer::renderGMesh(const GMesh& mesh, int surfId)
{
	GLMesh* glm = nullptr;
	auto it = m.mesh.find(&mesh);
	if (it == m.mesh.end())
	{
		glm = new GLTriMesh;
		glm->SetRenderMode(GLMesh::VBOMode);
		glm->CreateFromGMesh(mesh, GLMesh::FLAG_NORMAL);
		m.mesh[&mesh] = glm;
	}
	else
	{
		glm = it->second;
	}

	if (glm)
	{
		const GMesh::PARTITION& p = mesh.Partition(surfId);
		glm->Render(3*p.n0, 3*p.nf);
	}
}

unsigned int OpenGLRenderer::LoadEnvironmentMap(const std::string& fileName)
{
	// try to load the image
	QImage img;
	bool berr = img.load(QString::fromStdString(fileName));
	if (berr == false) return -1;

	uchar* d = img.bits();
	int nx = img.width();
	int ny = img.height();

	// we need to flip and invert colors
	GLubyte* buf = new GLubyte[nx * ny * 3];

	GLubyte* b = buf;
	for (int j = ny - 1; j >= 0; --j)
		for (int i = 0; i < nx; ++i, b += 3)
		{
			GLubyte* s = d + (j * (4 * nx) + 4 * i);
			b[0] = s[2];
			b[1] = s[1];
			b[2] = s[0];
		}

	unsigned int texid = 0;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	// set texture parameter for 2D textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nx, ny, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);

	delete[] buf;

	return (int)texid;
}

void OpenGLRenderer::ActivateEnvironmentMap(unsigned int mapid)
{
	if (mapid <= 0) return;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mapid);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
}

void OpenGLRenderer::DeactivateEnvironmentMap(unsigned int mapid)
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D);
}
