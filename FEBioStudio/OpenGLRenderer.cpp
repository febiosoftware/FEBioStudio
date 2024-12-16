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
#include <GLLib/glx.h>

class OpenGLRenderer::Imp {
public:
	CGLSceneView* glv;

	std::map<const GMesh*, GLTriMesh*> triMesh;
	std::map<const GMesh*, GLLineMesh*> lineMesh;
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
	case GLRenderEngine::CLIPPLANE: glEnable(GL_CLIP_PLANE0); break;
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

void OpenGLRenderer::setMaterial(GLMaterial::Type mat, GLColor c)
{
	switch (mat)
	{
	case GLMaterial::PLASTIC:
	{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glColor4ub(c.r, c.g, c.b, c.a);

		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_POLYGON_STIPPLE);

		GLfloat rev[] = { 0.8f, 0.6f, 0.6f, 1.f };
		GLfloat spc[] = { 0.5f, 0.5f, 0.5f, 1.f };
		GLfloat emi[] = { 0.0f, 0.0f, 0.0f, 1.f };

		glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, rev);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spc);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emi);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 64);
	}
	break;
	case GLMaterial::HIGHLIGHT:
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_STIPPLE);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

		glColor4ub(c.r, c.g, c.b, c.a);
	}
	break;
	case GLMaterial::CONSTANT:
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glColor4ub(c.r, c.g, c.b, c.a);
	}
	break;
	}
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
	auto it = m.triMesh.find(&mesh);
	if (it == m.triMesh.end())
	{
		glm = new GLTriMesh;
		glm->SetRenderMode(GLMesh::VBOMode);
		glm->CreateFromGMesh(mesh, GLMesh::FLAG_NORMAL);
		m.triMesh[&mesh] = glm;
	}
	else
	{
		glm = it->second;
	}

	if (glm)
	{
		glm->Render();
		m_stats.triangles += glm->Vertices() / 3;
	}
}

void OpenGLRenderer::renderGMesh(const GMesh& mesh, int surfId)
{
	GLTriMesh* glm = nullptr;
	auto it = m.triMesh.find(&mesh);
	if (it == m.triMesh.end())
	{
		glm = new GLTriMesh;
		glm->SetRenderMode(GLMesh::VBOMode);
		glm->CreateFromGMesh(mesh, GLMesh::FLAG_NORMAL);
		m.triMesh[&mesh] = glm;
	}
	else
	{
		glm = it->second;
	}

	if (glm)
	{
		const GMesh::PARTITION& p = mesh.Partition(surfId);
		glm->Render(3*p.n0, 3*p.nf);
		m_stats.triangles += p.nf;
	}
}

void OpenGLRenderer::renderGMeshEdges(const GMesh& mesh)
{
	GLLineMesh* glm = nullptr;
	auto it = m.lineMesh.find(&mesh);
	if (it == m.lineMesh.end())
	{
		glm = new GLLineMesh;
		glm->SetRenderMode(GLMesh::VBOMode);
		glm->CreateFromGMesh(mesh);
		m.lineMesh[&mesh] = glm;
	}
	else
	{
		glm = it->second;
	}

	if (glm)
	{
		glm->Render();
		m_stats.lines += glm->Vertices() / 2;
	}
}

void OpenGLRenderer::renderGMeshEdges(const GMesh& mesh, int edgeId)
{
	GLLineMesh* glm = nullptr;
	auto it = m.lineMesh.find(&mesh);
	if (it == m.lineMesh.end())
	{
		glm = new GLLineMesh;
		glm->SetRenderMode(GLMesh::VBOMode);
		glm->CreateFromGMesh(mesh);
		m.lineMesh[&mesh] = glm;
	}
	else
	{
		glm = it->second;
	}

	if (glm)
	{
		const auto& p = mesh.EIL(edgeId);
		glm->Render(2 * p.first, 2 * p.second);
		m_stats.lines += p.second;
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

void OpenGLRenderer::setClipPlane(unsigned int n, const double* v)
{
	glClipPlane(GL_CLIP_PLANE0 + n, v);
}

void OpenGLRenderer::renderGlyph(GlyphType glyph, float scale, GLColor c)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColor3ub(c.r, c.g, c.b);

	switch (glyph)
	{
	case GlyphType::RIGID_BODY       : glx::renderRigidBody(scale); break;
	case GlyphType::RIGID_WALL       : glx::renderRigidWall(scale); break;
	case GlyphType::RIGID_JOINT      : glx::renderJoint(scale); break;
	case GlyphType::REVOLUTE_JOINT   : glx::renderRevoluteJoint(scale); break;
	case GlyphType::PRISMATIC_JOINT  : glx::renderPrismaticJoint(scale); break;
	case GlyphType::CYLINDRICAL_JOINT: glx::renderCylindricalJoint(scale); break;
	case GlyphType::PLANAR_JOINT     : glx::renderPlanarJoint(scale); break;
	case GlyphType::RIGID_LOCK       : glx::renderRigidLock(scale); break;
	}

	glPopAttrib();
}
