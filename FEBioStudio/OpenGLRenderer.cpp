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

	size_t cachedObjects() { return triMesh.size() + lineMesh.size(); }
};

OpenGLRenderer::OpenGLRenderer(CGLSceneView* view) : m(*(new OpenGLRenderer::Imp))
{
	m.glv = view;
}

OpenGLRenderer::~OpenGLRenderer() 
{
}

void OpenGLRenderer::start()
{
	GLRenderEngine::start();
}

void OpenGLRenderer::finish()
{
	m_stats.cachedObjects = m.cachedObjects();
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
	case GLRenderEngine::LIGHTING : glEnable(GL_LIGHTING); break;
	case GLRenderEngine::CLIPPLANE: glEnable(GL_CLIP_PLANE0); break;
	}
}

void OpenGLRenderer::disable(GLRenderEngine::StateFlag flag)
{
	switch (flag)
	{
	case GLRenderEngine::LIGHTING : glDisable(GL_LIGHTING); break;
	case GLRenderEngine::CLIPPLANE: glDisable(GL_CLIP_PLANE0); break;
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
	case GLMaterial::GLASS:
	{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glColor4ub(c.r, c.g, c.b, c.a);

		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);

		if (mat == GLMaterial::PLASTIC)
			glDisable(GL_POLYGON_STIPPLE);
		else
			glEnable(GL_POLYGON_STIPPLE);

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
	case GLMaterial::OVERLAY:
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_POLYGON_STIPPLE);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

		glColor4ub(c.r, c.g, c.b, c.a);
	}
	break;
	case GLMaterial::CONSTANT:
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_POLYGON_STIPPLE);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glColor4ub(c.r, c.g, c.b, c.a);
	}
	break;
	}
}

void OpenGLRenderer::setPointSize(float f)
{
	glPointSize(f);
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

void OpenGLRenderer::renderGMesh(const GMesh& mesh, bool cacheMesh)
{
	GLTriMesh* glm = nullptr;
	if (cacheMesh)
	{
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
	}
	else
	{
		glm = new GLTriMesh;
		glm->SetRenderMode(GLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh, GLMesh::FLAG_NORMAL);
	}

	if (glm)
	{
		glm->Render();
		m_stats.triangles += glm->Vertices() / 3;
		if (!cacheMesh) delete glm;
	}
}

void OpenGLRenderer::renderGMesh(const GMesh& mesh, int surfId, bool cacheMesh)
{
	if ((surfId < 0) || (surfId >= mesh.Partitions())) return;

	GLTriMesh* glm = nullptr;
	if (cacheMesh)
	{
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
	}
	else
	{
		glm = new GLTriMesh;
		glm->SetRenderMode(GLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh, GLMesh::FLAG_NORMAL | GLMesh::FLAG_COLOR);
	}

	if (glm)
	{
		const GMesh::PARTITION& p = mesh.Partition(surfId);
		glm->Render(3*p.n0, 3*p.nf);
		m_stats.triangles += p.nf;

		if (!cacheMesh) delete glm;
	}
}

void OpenGLRenderer::renderGMeshNodes(const GMesh& mesh, bool cacheMesh)
{
	// TODO: implement mesh caching for point meshes
	GLPointMesh points;
	points.SetRenderMode(GLMesh::VertexArrayMode);
	points.CreateFromGMesh(mesh);
	points.Render();

	m_stats.points += points.Vertices();
}

void OpenGLRenderer::renderGMeshEdges(const GMesh& mesh, bool cacheMesh)
{
	GLLineMesh* glm = nullptr;
	if (cacheMesh)
	{
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
	}
	else
	{
		glm = new GLLineMesh;
		glm->SetRenderMode(GLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh);
	}

	if (glm)
	{
		glm->Render();
		m_stats.lines += glm->Vertices() / 2;

		if (!cacheMesh) delete glm;
	}
}

void OpenGLRenderer::renderGMeshEdges(const GMesh& mesh, int edgeId, bool cacheMesh)
{
	if ((edgeId < 0) || (edgeId >= mesh.EILs())) return;

	GLLineMesh* glm = nullptr;
	if (cacheMesh)
	{
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
	}
	else
	{
		glm = new GLLineMesh;
		glm->SetRenderMode(GLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh);
	}

	if (glm)
	{
		const auto& p = mesh.EIL(edgeId);
		glm->Render(2 * p.first, 2 * p.second);
		m_stats.lines += p.second;

		if (!cacheMesh) delete glm;
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

void OpenGLRenderer::renderGMeshOutline(CGLCamera& cam, const GMesh& gmsh, const Transform& T, int surfID)
{
	// get some settings
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();

	// this array will collect all points to render
	vector<vec3f> points; points.reserve(1024);

	const GMesh::PARTITION& part = gmsh.Partition(surfID);
	int NF = part.nf;
	if (NF > 0)
	{
		// loop over all faces
		for (int i = 0; i < NF; ++i)
		{
			const GMesh::FACE& f = gmsh.Face(i + part.n0);
			for (int j = 0; j < 3; ++j)
			{
				bool bdraw = false;

				if (f.nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					const GMesh::FACE& f2 = gmsh.Face(f.nbr[j]);

					if (f.pid != f2.pid)
					{
						bdraw = true;
					}
					else
					{
						vec3d n1 = T.LocalToGlobalNormal(to_vec3d(f.fn));
						vec3d n2 = T.LocalToGlobalNormal(to_vec3d(f2.fn));

						if (cam.IsOrtho())
						{
							q.RotateVector(n1);
							q.RotateVector(n2);
							if (n1.z * n2.z <= 0) bdraw = true;
						}
						else
						{
							int a = j;
							int b = (j + 1) % 3;
							vec3d ra = T.LocalToGlobal(to_vec3d(gmsh.Node(f.n[a]).r));
							vec3d rb = T.LocalToGlobal(to_vec3d(gmsh.Node(f.n[b]).r));
							vec3d c = (ra + rb) * 0.5;
							vec3d pc = p - c;
							double d1 = pc * n1;
							double d2 = pc * n2;
							if (d1 * d2 <= 0) bdraw = true;
						}
					}
				}

				if (bdraw)
				{
					int a = f.n[j];
					int b = f.n[(j + 1) % 3];
					if (a > b) { a ^= b; b ^= a; a ^= b; }

					points.push_back(gmsh.Node(a).r);
					points.push_back(gmsh.Node(b).r);
				}
			}
		}
	}
	if (points.empty()) return;

	// build the line mesh
	GMesh lineMesh;
	int NN = (int)points.size();
	int NE = (int)points.size() / 2;
	lineMesh.Create(NN, 0, NE);
	for (int i = 0; i < NE; ++i)
	{
		lineMesh.Node(2 * i).r = points[2 * i];
		lineMesh.Node(2 * i + 1).r = points[2 * i + 1];
		lineMesh.Edge(i).n[0] = 2 * i;
		lineMesh.Edge(i).n[1] = 2 * i + 1;
	}
	lineMesh.Update();

	// render the active edges
	renderGMeshEdges(lineMesh, false);
}
