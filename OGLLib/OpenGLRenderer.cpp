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
#include "OpenGLRenderer.h"
#include <GLLib/GLMesh.h>
#include <QImage>
#include "OGLMesh.h"
#include "OGLProgram.h"
#include <map>

const char* shadertxt_8bit = \
"#version 120\n"\
"uniform sampler3D sampler;                               \n"\
"uniform float Imin;                                      \n"\
"uniform float Imax;                                      \n"\
"uniform float Iscl;                                      \n"\
"uniform float IsclMin;                                   \n"\
"uniform float Amin;                                      \n"\
"uniform float Amax;                                      \n"\
"uniform float gamma;                                     \n"\
"uniform int cmap;                                        \n"\
"vec3 grayScale(const float f);                           \n"\
"vec3 red(const float f);                                 \n"\
"vec3 green(const float f);                               \n"\
"vec3 blue(const float f);                                \n"\
"vec3 fire(const float f);                                \n"\
"void main(void)                                          \n"\
"{                                                        \n"\
"	vec4 t = texture3D(sampler, gl_TexCoord[0].xyz);      \n"\
"   float f = (t.x-IsclMin)*Iscl;                         \n"\
"   f = (f - Imin) / (Imax - Imin);                       \n"\
"   f = clamp(f, 0.0, 1.0);                               \n"\
"   if (f <= 0.0) discard;                                \n"\
"   float a = Amin + f*(Amax - Amin);                     \n"\
"   if (gamma != 1.0) f = pow(f, gamma);                  \n"\
"   vec3 c3;                                              \n"\
"   if      (cmap == 0) { c3 = grayScale(f); }            \n"\
"   else if (cmap == 1) { c3 = red(f); }                  \n"\
"   else if (cmap == 2) { c3 = green(f); }                \n"\
"   else if (cmap == 3) { c3 = blue(f); }                 \n"\
"   else if (cmap == 4) { c3 = fire(f); }                 \n"\
"   else c3 = vec3(0,0,0);                                \n"\
"   vec4 c4 = vec4(c3.x, c3.y, c3.z, a);                  \n"\
"   gl_FragColor = gl_Color*c4;                           \n "\
"}                                                        \n"\
"                                                         \n"\
"vec3 grayScale(const float f) { return vec3(f, f, f);}   \n"\
"vec3 red(const float f)   { return vec3(f, 0, 0);}       \n"\
"vec3 green(const float f) { return vec3(0, f, 0);}       \n"\
"vec3 blue(const float f)  { return vec3(0, 0, f);}       \n"\
"vec3 fire(const float f)  {                              \n"\
"   vec3 c1 = vec3(0.0, 0., 0.);                          \n"\
"   vec3 c2 = vec3(0.5, 0., 1.);                          \n"\
"   vec3 c3 = vec3(1.0, 0., 0.);                          \n"\
"   vec3 c4 = vec3(1.0, 1., 0.);                          \n"\
"   vec3 c5 = vec3(1.0, 1., 1.);                          \n"\
"   vec3 c = vec3(0.0,0.,0.);                             \n"\
"   float wa, wb;                                         \n"\
"   if      (f >= 0.75) { wb = 2.0*(f - 0.75); wa = 1.0 - wb; c = c4*vec3(wa,wa,wa) + c5*vec3(wb,wb,wb); }\n"\
"   else if (f >= 0.50) { wb = 2.0*(f - 0.50); wa = 1.0 - wb; c = c3*vec3(wa,wa,wa) + c4*vec3(wb,wb,wb); }\n"\
"   else if (f >= 0.25) { wb = 2.0*(f - 0.25); wa = 1.0 - wb; c = c2*vec3(wa,wa,wa) + c3*vec3(wb,wb,wb); }\n"\
"   else if (f >= 0.00) { wb = 2.0*(f - 0.00); wa = 1.0 - wb; c = c1*vec3(wa,wa,wa) + c2*vec3(wb,wb,wb); }\n"\
"  return vec3(c.x, c.y, c.z);                            \n"\
"}                                                        \n"\
"";


const char* shadertxt_rgb = \
"#version 120\n"\
"uniform sampler3D sampler;                               \n"\
"uniform float Iscl;                                      \n"\
"uniform float IsclMin;                                   \n"\
"uniform float Imin;                                      \n"\
"uniform float Imax;                                      \n"\
"uniform float Amin;                                      \n"\
"uniform float Amax;                                      \n"\
"uniform float gamma;                                     \n"\
"uniform vec3 col1;                                       \n"\
"uniform vec3 col2;                                       \n"\
"uniform vec3 col3;                                       \n"\
"uniform int cmap;                                        \n"\
"void main(void)                                          \n"\
"{                                                        \n"\
"   vec4 t = (texture3D(sampler, gl_TexCoord[0].xyz)-IsclMin)*Iscl;\n"\
"   t.x = (t.x - Imin) / (Imax - Imin);                   \n"\
"   t.x = clamp(t.x, 0.0, 1.0);                           \n"\
"   t.y = (t.y - Imin) / (Imax - Imin);                   \n"\
"   t.y = clamp(t.y, 0.0, 1.0);                           \n"\
"   t.z = (t.z - Imin) / (Imax - Imin);                   \n"\
"   t.z = clamp(t.z, 0.0, 1.0);                           \n"\
"   float f = t.x;                                        \n"\
"   if (t.y > f) f = t.y;                                 \n"\
"   if (t.z > f) f = t.z;                                 \n"\
"   if (f <= 0.0) discard;                                \n"\
"   if (gamma != 1.0) f = pow(f, gamma);                  \n"\
"   float a = Amin + f*(Amax - Amin);                     \n"\
"   vec3 c1 = vec3(t.x*col1.x, t.x*col1.y, t.x*col1.z);   \n"\
"   vec3 c2 = vec3(t.y*col2.x, t.y*col2.y, t.y*col2.z);   \n"\
"   vec3 c3 = vec3(t.z*col3.x, t.z*col3.y, t.z*col3.z);   \n"\
"   vec3 c4 = c1 + c2 + c3;                               \n"\
"   gl_FragColor = gl_Color*vec4(c4.x, c4.y, c4.z, a);    \n"\
"}                                                        \n"\
"";

const char* vertex_shader = \
"#version 130\n"\
"void main(void)                                          \n"\
"{                                                        \n"\
"   gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex; \n "\
"}                                                        \n"\
"";

OGLProgram* VRprg_8bit = nullptr;
OGLProgram* VRprg_rgb = nullptr;

class OpenGLRenderer::Imp {
public:
	bool useVertexColors = false;
	bool useTexture = false;

	bool shaderInit = false;

	OGLProgram* activeProgram = nullptr;

	std::map<const GLMesh*, OGLTriMesh*> triMesh;
	std::map<const GLMesh*, OGLLineMesh*> lineMesh;

	size_t cachedObjects() { return triMesh.size() + lineMesh.size(); }

	void InitShaders();
};

void OpenGLRenderer::Imp::InitShaders()
{
	if (shaderInit) return;

	if (VRprg_8bit == nullptr) VRprg_8bit = new OGLProgram;
	VRprg_8bit->Create(nullptr, shadertxt_8bit);

	if (VRprg_rgb == nullptr) VRprg_rgb = new OGLProgram;
	VRprg_rgb->Create(nullptr, shadertxt_rgb);

	shaderInit = true;
}

OpenGLRenderer::OpenGLRenderer() : m(*(new OpenGLRenderer::Imp))
{
}

OpenGLRenderer::~OpenGLRenderer() 
{
}

void OpenGLRenderer::start()
{
	for (auto it : m.triMesh) it.second->resetRef();
	for (auto it : m.lineMesh) it.second->resetRef();
	GLRenderEngine::start();
}

void OpenGLRenderer::finish()
{
	for (auto it = m.triMesh.begin(); it != m.triMesh.end();)
	{
		OGLMesh* gm = it->second;
		if (gm->refs() == 0)
		{
			delete gm;
			it = m.triMesh.erase(it);
		}
		else it++;
	}
	for (auto it = m.lineMesh.begin(); it != m.lineMesh.end();)
	{
		OGLMesh* gm = it->second;
		if (gm->refs() == 0)
		{
			delete gm;
			it = m.lineMesh.erase(it);
		}
		else it++;
	}
	m_stats.cachedObjects = m.cachedObjects();
}

void OpenGLRenderer::deleteCachedMesh(GLMesh* gm)
{
	auto it1 = m.triMesh.find(gm);
	if (it1 != m.triMesh.end())
	{
		delete it1->second;
		m.triMesh.erase(it1);
	}

	auto it2 = m.lineMesh.find(gm);
	if (it2 != m.lineMesh.end())
	{
		delete it2->second;
		m.lineMesh.erase(it2);
	}
}

void OpenGLRenderer::pushState()
{
	glPushAttrib(GL_ENABLE_BIT);
}

void OpenGLRenderer::popState()
{
	glPopAttrib();
}

void OpenGLRenderer::pushTransform()
{
	// This assumes that the matrix mode is MODELVIEW
	glPushMatrix();
}

void OpenGLRenderer::popTransform()
{
	// This assumes that the matrix mode is MODELVIEW
	glPopMatrix();
}

void OpenGLRenderer::translate(const vec3d& r)
{
	glTranslated(r.x, r.y, r.z);
}

void OpenGLRenderer::rotate(const quatd& rot)
{
	double w = rot.GetAngle();
	if (w != 0.0)
	{
		vec3d r = rot.GetVector();
		glRotated(w * RAD2DEG, r.x, r.y, r.z);
	}
}

void OpenGLRenderer::rotate(double angleDeg, double x, double y, double z)
{
	glRotated(angleDeg, x, y, z);
}

void OpenGLRenderer::scale(double x, double y, double z)
{
	glScaled(x, y, z);
}

void OpenGLRenderer::transform(const vec3d& pos, const quatd& rot)
{
	translate(pos);
	rotate(rot);
}

void OpenGLRenderer::multTransform(const double* m)
{
	glMultMatrixd(m);
}

void OpenGLRenderer::enable(GLRenderEngine::StateFlag flag)
{
	switch (flag)
	{
	case GLRenderEngine::LIGHTING  : glEnable(GL_LIGHTING); break;
	case GLRenderEngine::DEPTHTEST : glEnable(GL_DEPTH_TEST); break;
	case GLRenderEngine::CULLFACE  : glEnable(GL_CULL_FACE); break;
	case GLRenderEngine::BLENDING  : glEnable(GL_BLEND); break;
	}
}

void OpenGLRenderer::disable(GLRenderEngine::StateFlag flag)
{
	switch (flag)
	{
	case GLRenderEngine::LIGHTING  : glDisable(GL_LIGHTING); break;
	case GLRenderEngine::DEPTHTEST : glDisable(GL_DEPTH_TEST); break;
	case GLRenderEngine::CULLFACE  : glDisable(GL_CULL_FACE); break;
	case GLRenderEngine::BLENDING  : glDisable(GL_BLEND); break;
	}
}

void OpenGLRenderer::setColor(GLColor c)
{
	glColor4ub(c.r, c.g, c.b, c.a);
}

void OpenGLRenderer::setMaterial(const GLMaterial& mat)
{
	setMaterial(mat.type, mat.diffuse);
}

void OpenGLRenderer::setMaterial(GLMaterial::Type mat, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
{
	if (map == GLMaterial::DiffuseMap::VERTEX_COLOR) m.useVertexColors = true;
	else m.useVertexColors = false;

	m.useTexture = false;
	if (map == GLMaterial::DiffuseMap::TEXTURE_1D) m.useTexture = true;
	if (map == GLMaterial::DiffuseMap::TEXTURE_2D) m.useTexture = true;
	if (map == GLMaterial::DiffuseMap::TEXTURE_3D) m.useTexture = true;

	if (map == GLMaterial::DiffuseMap::TEXTURE_1D) glEnable(GL_TEXTURE_1D);
	else glDisable(GL_TEXTURE_1D);

	if (map == GLMaterial::DiffuseMap::TEXTURE_2D) glEnable(GL_TEXTURE_2D);
	else glDisable(GL_TEXTURE_2D);

	if (map == GLMaterial::DiffuseMap::TEXTURE_3D) glEnable(GL_TEXTURE_3D);
	else glDisable(GL_TEXTURE_3D);

	switch (mat)
	{
	case GLMaterial::INVALID:
		UseProgram(nullptr);
		break;
	case GLMaterial::PLASTIC:
	case GLMaterial::GLASS:
	{
		glEnable(GL_COLOR_MATERIAL);
		if (frontOnly)
			glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		else
			glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glColor4ub(c.r, c.g, c.b, c.a);

		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);

		if (mat == GLMaterial::PLASTIC)
			glDisable(GL_POLYGON_STIPPLE);
		else
			glEnable(GL_POLYGON_STIPPLE);

		GLfloat rev[] = { 0.8f, 0.6f, 0.6f, c.a/255.f };
		GLfloat spc[] = { 0.5f, 0.5f, 0.5f, 1.f };
		GLfloat emi[] = { 0.0f, 0.0f, 0.0f, 1.f };

		if (frontOnly) glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, rev);
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

void OpenGLRenderer::UseProgram(OGLProgram* prg)
{
	if (prg)
	{
		prg->Use();
	}
	else glUseProgram(0);
	m.activeProgram = prg;
}

void OpenGLRenderer::setPointSize(float f)
{
	glPointSize(f);
}

float OpenGLRenderer::pointSize()
{
	float pointSize;
	glGetFloatv(GL_POINT_SIZE, &pointSize);
	return pointSize;
}

float OpenGLRenderer::lineWidth()
{
	float lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	return lineWidth;
}

void OpenGLRenderer::setLineWidth(float f)
{
	glLineWidth(f);
}

GLRenderEngine::FrontFace OpenGLRenderer::frontFace()
{
	int frontFace;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);
	if (frontFace == GL_CCW) return GLRenderEngine::COUNTER_CLOCKWISE;
	else return GLRenderEngine::CLOCKWISE;
}

void OpenGLRenderer::setFrontFace(GLRenderEngine::FrontFace f)
{
	int frontFace = GL_CCW;
	if (f == GLRenderEngine::CLOCKWISE) frontFace = GL_CW;
	glFrontFace(frontFace);
}

void OpenGLRenderer::positionCamera(const GLCamera& cam)
{
	// reset the modelview matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// target in camera coordinates
	vec3d r = cam.Target();

	// zoom-in a little when in decal mode
	if (cam.m_bdecal)
		glPolygonOffset(0, 0);
	else
		glPolygonOffset(1, 1);

	// position the target in camera coordinates
	translate(-r);

	// orient the camera
	rotate(cam.m_rot.Value());

	// translate to world coordinates
	translate(-cam.GetPosition());
}

void OpenGLRenderer::setLightPosition(unsigned int lightIndex, const vec3f& p)
{
	GLfloat fv[4] = { 0 };
	fv[0] = p.x; fv[1] = p.y; fv[2] = p.z;
	glLightfv(GL_LIGHT0 + lightIndex, GL_POSITION, fv);
}

void OpenGLRenderer::begin(PrimitiveType prim)
{
	switch (prim)
	{
	case PrimitiveType::POINTS     : glBegin(GL_POINTS); break;
	case PrimitiveType::LINES      : glBegin(GL_LINES); break;
	case PrimitiveType::LINELOOP   : glBegin(GL_LINE_LOOP); break;
	case PrimitiveType::LINESTRIP  : glBegin(GL_LINE_STRIP); break;
	case PrimitiveType::TRIANGLES  : glBegin(GL_TRIANGLES); break;
	case PrimitiveType::TRIANGLEFAN: glBegin(GL_TRIANGLE_FAN); break;
	case PrimitiveType::QUADS      : glBegin(GL_QUADS); break;
	case PrimitiveType::QUADSTRIP  : glBegin(GL_QUAD_STRIP); break;
	default:
		assert(false);
	}
}

void OpenGLRenderer::end()
{
	glEnd();
}

void OpenGLRenderer::vertex(const vec3d& r)
{
	glVertex3d(r.x, r.y, r.z);
}

void OpenGLRenderer::normal(const vec3d& r)
{
	glNormal3d(r.x, r.y, r.z);
}

void OpenGLRenderer::texCoord1d(double t)
{
	glTexCoord1d(t);
}

void OpenGLRenderer::texCoord2d(double r, double s)
{
	glTexCoord2d(r, s);
}

void OpenGLRenderer::renderGMesh(const GLMesh& mesh, bool cacheMesh)
{
	OGLTriMesh* glm = nullptr;
	if (cacheMesh)
	{
		auto it = m.triMesh.find(&mesh);
		if (it == m.triMesh.end())
		{
			glm = new OGLTriMesh;
			glm->SetRenderMode(OGLMesh::VBOMode);
			glm->CreateFromGMesh(mesh, OGLMesh::FLAG_NORMAL | OGLMesh::FLAG_COLOR | OGLMesh::FLAG_TEXTURE);
			m.triMesh[&mesh] = glm;
		}
		else
		{
			glm = it->second;
		}
	}
	else
	{
		glm = new OGLTriMesh;
		glm->SetRenderMode(OGLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh, OGLMesh::FLAG_NORMAL | OGLMesh::FLAG_COLOR | OGLMesh::FLAG_TEXTURE);
	}

	if (glm)
	{
		unsigned int flags = OGLMesh::FLAG_NORMAL;
		if (m.useVertexColors) flags |= OGLMesh::FLAG_COLOR;
		if (m.useTexture) flags |= OGLMesh::FLAG_TEXTURE;
		glm->Render(flags);
		m_stats.triangles += glm->Vertices() / 3;

		glm->incRef();
		if (!cacheMesh) delete glm;
	}
}

void OpenGLRenderer::renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh)
{
	if ((surfId < 0) || (surfId >= mesh.Partitions())) return;

	if (cacheMesh)
	{
		OGLTriMesh* glm = nullptr;
		auto it = m.triMesh.find(&mesh);
		if (it == m.triMesh.end())
		{
			glm = new OGLTriMesh;
			glm->SetRenderMode(OGLMesh::VBOMode);
			glm->CreateFromGMesh(mesh, OGLMesh::FLAG_NORMAL | OGLMesh::FLAG_COLOR | OGLMesh::FLAG_TEXTURE);
			m.triMesh[&mesh] = glm;
		}
		else
		{
			glm = it->second;
		}

		if (glm)
		{
			unsigned int flags = OGLMesh::FLAG_NORMAL;
			if (m.useVertexColors) flags |= OGLMesh::FLAG_COLOR;
			if (m.useTexture) flags |= OGLMesh::FLAG_TEXTURE;

			const GLMesh::PARTITION& p = mesh.Partition(surfId);
			glm->Render(3 * p.n0, 3 * p.nf, flags);
			m_stats.triangles += p.nf;

			glm->incRef();
		}
	}
	else
	{
		OGLTriMesh* glm = new OGLTriMesh;
		glm->SetRenderMode(OGLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh, surfId, OGLMesh::FLAG_NORMAL | OGLMesh::FLAG_COLOR | OGLMesh::FLAG_TEXTURE);

		unsigned int flags = OGLMesh::FLAG_NORMAL;
		if (m.useVertexColors) flags |= OGLMesh::FLAG_COLOR;
		if (m.useTexture) flags |= OGLMesh::FLAG_TEXTURE;

		glm->Render(flags);

		const GLMesh::PARTITION& p = mesh.Partition(surfId);
		m_stats.triangles += p.nf;

		delete glm;
	}
}

void OpenGLRenderer::renderGMeshNodes(const GLMesh& mesh, bool cacheMesh)
{
	// TODO: implement mesh caching for point meshes
	OGLPointMesh points;
	points.SetRenderMode(OGLMesh::VertexArrayMode);
	points.CreateFromGMesh(mesh);
	points.Render();

	m_stats.points += points.Vertices();
}

void OpenGLRenderer::renderTaggedGMeshNodes(const GLMesh& mesh, int tag)
{
	OGLPointMesh points;
	points.SetRenderMode(OGLMesh::VertexArrayMode);
	points.CreateFromTaggedGMesh(mesh, tag);
	points.Render();

	m_stats.points += points.Vertices();
}

void OpenGLRenderer::renderGMeshEdges(const GLMesh& mesh, bool cacheMesh)
{
	OGLLineMesh* glm = nullptr;
	if (cacheMesh)
	{
		auto it = m.lineMesh.find(&mesh);
		if (it == m.lineMesh.end())
		{
			glm = new OGLLineMesh;
			glm->SetRenderMode(OGLMesh::VBOMode);
			glm->CreateFromGMesh(mesh, OGLMesh::FLAG_COLOR);
			m.lineMesh[&mesh] = glm;
		}
		else
		{
			glm = it->second;
		}
	}
	else
	{
		glm = new OGLLineMesh;
		glm->SetRenderMode(OGLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh, OGLMesh::FLAG_COLOR);
	}

	if (glm)
	{
		unsigned int flags = 0;
		if (m.useVertexColors) flags = OGLMesh::FLAG_COLOR;

		glm->Render(flags);
		m_stats.lines += glm->Vertices() / 2;

		glm->incRef();
		if (!cacheMesh) delete glm;
	}
}

void OpenGLRenderer::renderGMeshEdges(const GLMesh& mesh, int edgeId, bool cacheMesh)
{
	if ((edgeId < 0) || (edgeId >= mesh.EILs())) return;

	OGLLineMesh* glm = nullptr;
	if (cacheMesh)
	{
		auto it = m.lineMesh.find(&mesh);
		if (it == m.lineMesh.end())
		{
			glm = new OGLLineMesh;
			glm->SetRenderMode(OGLMesh::VBOMode);
			glm->CreateFromGMesh(mesh, OGLMesh::FLAG_COLOR);
			m.lineMesh[&mesh] = glm;
		}
		else
		{
			glm = it->second;
		}
	}
	else
	{
		glm = new OGLLineMesh;
		glm->SetRenderMode(OGLMesh::VertexArrayMode);
		glm->CreateFromGMesh(mesh, OGLMesh::FLAG_COLOR);
	}

	if (glm)
	{
		unsigned int flags = 0;
		if (m.useVertexColors) flags = OGLMesh::FLAG_COLOR;

		const auto& p = mesh.EIL(edgeId);
		glm->Render(2 * p.first, 2 * p.second, flags);
		m_stats.lines += p.second;

		glm->incRef();
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

void OpenGLRenderer::enableClipPlane(unsigned int n)
{
	glEnable(GL_CLIP_PLANE0 + n);
}

void OpenGLRenderer::disableClipPlane(unsigned int n)
{
	glDisable(GL_CLIP_PLANE0 + n);
}

void OpenGLRenderer::renderGMeshOutline(GLCamera& cam, const GLMesh& gmsh, const Transform& T)
{
	// get some settings
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();

	// this array will collect all points to render
	vector<vec3f> points; points.reserve(1024);

	int NF = gmsh.Faces();
	if (NF > 0)
	{
		// loop over all faces
		for (int i = 0; i < NF; ++i)
		{
			const GLMesh::FACE& f = gmsh.Face(i);
			for (int j = 0; j < 3; ++j)
			{
				bool bdraw = false;

				if (f.nbr[j] >= 0)
				{
					const GLMesh::FACE& f2 = gmsh.Face(f.nbr[j]);
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
	GLMesh lineMesh;
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

void OpenGLRenderer::renderGMeshOutline(GLCamera& cam, const GLMesh& gmsh, const Transform& T, int surfID)
{
	// get some settings
	quatd q = cam.GetOrientation();
	vec3d p = cam.GlobalPosition();

	// this array will collect all points to render
	vector<vec3f> points; points.reserve(1024);

	const GLMesh::PARTITION& part = gmsh.Partition(surfID);
	int NF = part.nf;
	if (NF > 0)
	{
		// loop over all faces
		for (int i = 0; i < NF; ++i)
		{
			const GLMesh::FACE& f = gmsh.Face(i + part.n0);
			for (int j = 0; j < 3; ++j)
			{
				bool bdraw = false;

				if (f.nbr[j] < 0)
				{
					bdraw = true;
				}
				else
				{
					const GLMesh::FACE& f2 = gmsh.Face(f.nbr[j]);

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
	GLMesh lineMesh;
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

void OpenGLRenderer::setTexture(GLTexture1D& tex)
{
	unsigned int texID = tex.GetID();
	if (texID == 0)
	{
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_1D, texID);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //(m_bsmooth ? GL_LINEAR : GL_NEAREST));
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //(m_bsmooth ? GL_LINEAR : GL_NEAREST));

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

		tex.SetID(texID);
	}
	else glBindTexture(GL_TEXTURE_1D, texID);

	if (tex.DoUpdate())
	{
		glTexImage1D(GL_TEXTURE_1D, 0, 3, tex.Size(), 0, GL_RGB, GL_UNSIGNED_BYTE, tex.GetBytes());
		tex.Update(false);
	}
}

void OpenGLRenderer::setTexture(GLTexture2D& tex)
{
	CRGBAImage* im = tex.GetImage();
	if (im == nullptr) return;

	unsigned int texID = tex.GetTexID();
	if (texID == 0)
	{
		glGenTextures(1, &texID);
		tex.SetTexID(texID);
	}

	glBindTexture(GL_TEXTURE_2D, texID);

	if (tex.IsModified())
	{
		// set texture parameter for 2D textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		int nx = im->Width();
		int ny = im->Height();
		if (nx * ny > 0)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, 4, nx, ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, im->GetBytes());
		}
		tex.SetModified(false);
	}
}

void OpenGLRenderer::setTexture(GLTexture3D& tex)
{
	C3DImage* im = tex.Get3DImage();
	if (im == nullptr) return;

	unsigned int texID = tex.GetTexID();
	if (texID == 0)
	{
		glGenTextures(1, &texID);
		tex.SetTexID(texID);
	}

	if (tex.IsModified())
	{
		C3DImage& im3d = *im;
		
		// get the original image dimensions
		int nx = im3d.Width();
		int ny = im3d.Height();
		int nz = im3d.Depth();

		int pType = im3d.PixelType();

		// find the min and max values
		size_t N = nx * ny * nz;
		if (pType == CImage::INT_RGB8 || pType == CImage::UINT_RGB8 || pType == CImage::INT_RGB16 || pType == CImage::UINT_RGB16)
		{
			N *= 3;
		}

		constexpr int max8 = std::numeric_limits<unsigned char>::max();
		constexpr int max16 = std::numeric_limits<unsigned short>::max();
		constexpr uint32_t max32 = std::numeric_limits<unsigned int>::max();

		double min, max;
		im3d.GetMinMax(min, max);
		switch (pType)
		{
		case CImage::INT_8:
		case CImage::INT_RGB8:
		{
			max /= max8 / 2;
			min /= max8 / 2;
		}
		break;
		case CImage::UINT_8:
		case CImage::UINT_RGB8:
		{
			max /= max8;
			min /= max8;
		}
		break;
		case CImage::INT_RGB16:
		{
			max /= max16 / 2;
			min /= max16 / 2;
		}
		break;
		case CImage::UINT_16:
		case CImage::UINT_RGB16:
		{
			max /= max16;
			min /= max16;
		}
		break;
		case CImage::INT_16:
		case CImage::INT_32:
		case CImage::UINT_32:
		{
			// Don't do anything for these cases since we scale the data
			// before we send it to OpenGL
		}
		break;
		case CImage::REAL_32:
		case CImage::REAL_64:
		{
			// floating point images are copied and scaled when calling glTexImage3D, so we can just 
			// set the range to [0,1]
			max = 1.f;
			min = 0.f;
		}
		break;
		default:
			assert(false);
		}

		if (max == min) max++;
		tex.Iscale = 1.f / (max - min);
		tex.IscaleMin = min;

		glBindTexture(GL_TEXTURE_3D, texID);

		//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// set texture parameter for 2D textures
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

		int oldUnpackAlignment = 0;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		switch (im3d.PixelType())
		{
		case CImage::INT_8: glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_BYTE, im3d.GetBytes()); break;
		case CImage::INT_16:
		{
			GLbyte* d = new GLbyte[N];
			short* s = (short*)im3d.GetBytes();
			for (size_t i = 0; i < N; ++i) d[i] = (GLbyte)(255 * (s[i] - tex.IscaleMin) * tex.Iscale);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, d);
			delete[] d;

			tex.Iscale = 1.f;
			tex.IscaleMin = 0.f;
		}
		break;
		case CImage::INT_32:
		case CImage::UINT_32:
		{
			// We're doing the scaling here, because it appears
			// that OpenGL only maps the upper 16 bits to the range [0,1].
			// If the image only fills the lower 16 bits, we won't see anything 
			GLbyte* d = new GLbyte[N];
			int* s = (int*)im3d.GetBytes();
			for (size_t i = 0; i < N; ++i) d[i] = (GLbyte)(255 * (s[i] - tex.IscaleMin) * tex.Iscale);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, d);
			delete[] d;

			tex.Iscale = 1.f;
			tex.IscaleMin = 0.f;
		}
		break;
		case CImage::INT_RGB8: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_BYTE, im3d.GetBytes()); break;
		case CImage::INT_RGB16: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_SHORT, im3d.GetBytes()); break;
		case CImage::UINT_8: glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_BYTE, im3d.GetBytes()); break;
		case CImage::UINT_16: glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_UNSIGNED_SHORT, im3d.GetBytes()); break;
		case CImage::UINT_RGB8: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_UNSIGNED_BYTE, im3d.GetBytes()); break;
		case CImage::UINT_RGB16: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, nx, ny, nz, 0, GL_RGB, GL_UNSIGNED_SHORT, im3d.GetBytes()); break;
		case CImage::REAL_32:
		{
			// Opengl expects that values between 0 and 1, so we need to scale the buffer
			float* d = new float[N];
			float* s = (float*)im3d.GetBytes();
			float fmax = *std::max_element(s, s + N); if (fmax == 0.f) fmax = 1.f;
			for (size_t i = 0; i < N; ++i) d[i] = (s[i] / fmax);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_FLOAT, d);
			delete[] d;
		}
		break;
		case CImage::REAL_64:
		{
			// OpenGL doesn't have a 64-bit real (i.e. double precision), so we need to make a float copy.
			float* d = new float[N];
			double* s = (double*)im3d.GetBytes();
			double dmax = *std::max_element(s, s + N); if (dmax == 0.0) dmax = 1.0;
			for (size_t i = 0; i < N; ++i) d[i] = (float)(s[i] / dmax);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, nx, ny, nz, 0, GL_RED, GL_FLOAT, d);
			delete[] d;
		}
		break;
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);

		tex.SetModified(false);
	}
	else
		glBindTexture(GL_TEXTURE_3D, texID);

	if (m.shaderInit == false) m.InitShaders();

	OGLProgram* prg = nullptr;
	switch (im->PixelType())
	{
	case CImage::UINT_8 :
	case CImage::INT_8  :
	case CImage::UINT_16:
	case CImage::INT_16 :
	case CImage::UINT_32:
	case CImage::INT_32 :
	case CImage::REAL_32:
	case CImage::REAL_64:
		prg = VRprg_8bit; break;
		break;
	case CImage::UINT_RGB8 :
	case CImage::INT_RGB8  :
	case CImage::UINT_RGB16:
	case CImage::INT_RGB16 :
		prg = VRprg_rgb;
		break;
	default:
		return;
	}

	UseProgram(prg);

	if (prg)
	{
		prg->SetFloat("Imin", tex.Imin);
		prg->SetFloat("Imax", tex.Imax);
		prg->SetFloat("Amin", tex.Amin);
		prg->SetFloat("Amax", tex.Amax);
		prg->SetFloat("gamma", tex.gamma);
		prg->SetInt("cmap", tex.cmap);
		prg->SetFloat("Iscl", tex.Iscale);
		prg->SetFloat("IsclMin", tex.IscaleMin);

		float c1[4] = { 0.f }, c2[4] = { 0.f }, c3[4] = { 0.f };
		tex.col1.toFloat(c1);
		tex.col2.toFloat(c2);
		tex.col3.toFloat(c3);
		prg->SetFloat3("col1", c1);
		prg->SetFloat3("col2", c2);
		prg->SetFloat3("col3", c3);
	}
}
