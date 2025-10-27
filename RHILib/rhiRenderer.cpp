/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include "rhiRenderer.h"
#include "rhiUtil.h"
#include "rhiTriMesh.h"
#include "rhiShader.h"


void GlobalUniformBlock::create(QRhi* rhi)
{
	m_ub.create({
		{ rhi::UniformBlock::VEC4, "lightPos" },
		{ rhi::UniformBlock::VEC4, "specColor"},
		{ rhi::UniformBlock::VEC4, "clipPlane"}
	});

	m_ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_ub.size()));
	m_ubuf->create();
}

void GlobalUniformBlock::setLightPosition(const vec3f& lp) { m_ub.setVec4(0, lp); }
void GlobalUniformBlock::setSpecularColor(GLColor c) 
{ 
	float f[4] = { 0.f };
	c.toFloat(f);
	m_ub.setVec4(1, f[0], f[1], f[2], f[3]); 
}
void GlobalUniformBlock::setClipPlane(const float f[4]) { m_ub.setVec4(2, f[0], f[1], f[2], f[3]); }

void GlobalUniformBlock::update(QRhiResourceUpdateBatch* u)
{
	u->updateDynamicBuffer(m_ubuf.get(), 0, m_ub.size(), m_ub.data());
}

void CanvasUniformBlock::create(QRhi* rhi)
{
	m_ub.create({
		{ rhi::UniformBlock::VEC2, "viewport" }
		});

	m_ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_ub.size()));
	m_ubuf->create();
}

void CanvasUniformBlock::setViewPort(const vec2f& vp)
{
	m_ub.setVec2(0, vp);
}

void CanvasUniformBlock::update(QRhiResourceUpdateBatch* u)
{
	u->updateDynamicBuffer(m_ubuf.get(), 0, m_ub.size(), m_ub.data());
}

rhiRenderer::rhiRenderer(QRhi* rhi, QRhiSwapChain* sc, QRhiRenderPassDescriptor* rp) : m_rhi(rhi), m_sc(sc), m_rp(rp), m_texture(rhi)
{
}

rhiRenderer::~rhiRenderer()
{
	clearCache();
}

void rhiRenderer::init()
{
	m_initialUpdates = m_rhi->nextResourceUpdateBatch();

	// create global uniform (used by several shaders)
	m_global.create(m_rhi);

	// prep 1D texture
	QImage img(QSize(1024, 1), QImage::Format_RGBA8888);
	img.fill(Qt::white);
	m_texture.create(img);
	m_texture.upload(m_initialUpdates);

	// convenience class for passing around all the resources that are shared between shaders
	m_sharedResources = { m_global.get(), m_texture.texture.get(), m_texture.sampler.get()};

	// create all render passes
	m_solidPass.reset(new SolidRenderPass(m_rhi));
	m_solidPass->create(m_sc, &m_sharedResources);

	m_linePass.reset(new LineRenderPass(m_rhi));
	m_linePass->create(m_sc, &m_sharedResources);

	m_pointPass.reset(new PointRenderPass(m_rhi));
	m_pointPass->create(m_sc, &m_sharedResources);

	m_overlayPass.reset(new OverlayRenderPass(m_rhi));
	m_overlayPass->create(m_sc, &m_sharedResources);

	// create the canvas pass (for rendering fps)
	m_canvasPass.reset(new CanvasRenderPass(m_rhi));
	m_canvasPass->create(m_sc);

	// reset timing
	timing.m_tic = timing.m_toc = high_resolution_clock::now();
}

void rhiRenderer::clearCache()
{
	m_solidPass->clearCache();
	m_linePass->clearCache();
	m_pointPass->clearCache();
}

QSize rhiRenderer::pixelSize() const
{
	return m_sc->surfacePixelSize();
}

void rhiRenderer::clearUnusedCache()
{
	m_solidPass->clearUnusedCache();
	m_linePass->clearUnusedCache();
	m_pointPass->clearUnusedCache();
}

void rhiRenderer::setBackgroundColor(const GLColor& c)
{
	m_bgColor = QColor(c.r, c.g, c.b, c.a);
}

void rhiRenderer::setLightPosition(unsigned int n, const vec3f& lp)
{
	m_light = lp;
}

void rhiRenderer::setLightSpecularColor(unsigned int lightIndex, const GLColor& col)
{
	m_lightSpecular = col;
}

void rhiRenderer::positionCamera(const GLCamera& cam)
{
	m_viewMatrix.setToIdentity();

	// target in camera coordinates
	vec3d r = cam.Target();

	// position the target in camera coordinates
	m_viewMatrix.translate(-r.x, -r.y, -r.z);

	// orient the camera
	quatd q = cam.m_rot.Value();
	m_viewMatrix.rotate(QQuaternion(q.w, q.x, q.y, q.z));

	// translate to world coordinates
	vec3d p = cam.GetPosition();
	m_viewMatrix.translate(-p.x, -p.y, -p.z);
}

void rhiRenderer::setProjection(double fov, double fnear, double far)
{
	m_projMatrix = m_rhi->clipSpaceCorrMatrix();
	double W = m_viewport[2];
	double H = m_viewport[3];
	double ar = (H == 0 ? 1 : W / H);
	m_projMatrix.perspective(fov, ar, fnear, far);
}

void rhiRenderer::setOrthoProjection(double left, double right, double bottom, double top, double zNear, double zFar)
{
	m_projMatrix = m_rhi->clipSpaceCorrMatrix();
	m_projMatrix.ortho(left, right, bottom, top, zNear, zFar);
}

void rhiRenderer::pushTransform()
{
	m_transformStack.push(m_modelMatrix);
}

void rhiRenderer::popTransform()
{
	assert(!m_transformStack.empty());
	if (m_transformStack.empty()) return;
	m_modelMatrix = m_transformStack.top();
	m_transformStack.pop();
}

void rhiRenderer::translate(const vec3d& r)
{
	QMatrix4x4 T;
	T.translate(QVector3D(r.x, r.y, r.z));
	m_modelMatrix *= T;
}

void rhiRenderer::rotate(const quatd& rot)
{
	QMatrix4x4 R;
	R.rotate(QQuaternion(rot.w, rot.x, rot.y, rot.z));
	m_modelMatrix *= R;
}

void rhiRenderer::rotate(double deg, double x, double y, double z)
{
	quatd q(deg * DEG2RAD, vec3d(x, y, z));
	rotate(q);
}

void rhiRenderer::scale(double x, double y, double z)
{
	QMatrix4x4 S;
	S.scale(x, y, z);
	m_modelMatrix *= S;
}

void rhiRenderer::setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
{
	m_currentMat.diffuse = m_currentMat.ambient = c;
	m_currentMat.type = matType;
	m_currentMat.diffuseMap = map;
}

void rhiRenderer::setColor(GLColor c)
{
	m_currentMat.diffuse = m_currentMat.ambient = c;
	mb.setColor(c);
}

void rhiRenderer::setMaterial(const GLMaterial& mat)
{
	m_currentMat = mat;
}

void rhiRenderer::renderGMesh(const GLMesh& mesh, bool cacheMesh)
{
	rhi::Mesh* pm = m_solidPass->addGLMesh(mesh, cacheMesh);
	if (pm)
	{
		pm->SetMaterial(m_currentMat);
		pm->SetModelMatrix(m_modelMatrix);
		pm->doClipping = m_clipEnabled;
		pm->setActive(true);
	}
}

void rhiRenderer::renderGMeshEdges(const GLMesh& mesh, bool cacheMesh)
{
	rhi::Mesh* lineMesh = m_linePass->addGLMesh(mesh, cacheMesh);
	if (lineMesh)
	{
		lineMesh->SetMaterial(m_currentMat);
		lineMesh->SetModelMatrix(m_modelMatrix);
		lineMesh->doClipping = m_clipEnabled;
		lineMesh->setActive(true);
	}
}

void rhiRenderer::renderGMeshNodes(const GLMesh& mesh, bool cacheMesh)
{
	rhi::Mesh* pointMesh = m_pointPass->addGLMesh(mesh, cacheMesh);
	if (pointMesh)
	{
		pointMesh->SetMaterial(m_currentMat);
		pointMesh->SetModelMatrix(m_modelMatrix);
		pointMesh->doClipping = m_clipEnabled;
		pointMesh->setActive(true);
	}
}

void rhiRenderer::setTexture(GLTexture1D& tex)
{
	if (tex.DoUpdate())
	{
		// update texture data
		QImage img(tex.Size(), 1, QImage::Format_RGBA8888);
		for (int i = 0; i < tex.Size(); ++i)
		{
			GLColor c = tex.sample((float)i / (tex.Size() - 1.f));
			img.setPixelColor(i, 0, QColor(c.r, c.g, c.b, c.a));
		}
		m_texture.image = img;
		m_texture.needsUpload = true;
		tex.Update(false);
	}
}

void rhiRenderer::setClipPlane(unsigned int n, const double* v)
{
	if (n == 0)
	{
		// Transform to eye coordinates
		QVector4D N(v[0], v[1], v[2], 0);
		QMatrix4x4 modelView = m_viewMatrix * m_modelMatrix;
		QVector4D Np = modelView * N;
		// TODO: Do I need transpose of modelView?
		double v3 = v[3] - (modelView(0,3) * Np[0] + modelView(1,3) * Np[1] + modelView(2,3) * Np[2]);

		clipPlane[0] = Np[0];
		clipPlane[1] = Np[1];
		clipPlane[2] = Np[2];
		clipPlane[3] = v3;
	}
}

void rhiRenderer::enableClipPlane(unsigned int n)
{
	if (n == 0)
	{
		m_clipEnabled = true;
	}
}

void rhiRenderer::disableClipPlane(unsigned int n)
{
	if (n == 0)
	{
		m_clipEnabled = false;
	}
}

void rhiRenderer::useOverlayImage(bool b)
{
	m_useOverlay = b;
}

void rhiRenderer::setOverlayImage(const QImage& img)
{
	if (m_rhi->isYUpInNDC())
		m_overlayPass->setImage(img.mirrored());
	else
		m_overlayPass->setImage(img);
}

void rhiRenderer::setTriadInfo(const QMatrix4x4& m, QRhiViewport vp)
{
	m_overlayPass->m_overlayVP = vp;
	m_overlayPass->m_overlayVM = m;
}

void rhiRenderer::start()
{
	ResetStats();

	// start by setting all meshes as inactive
	m_solidPass->reset();
	m_linePass->reset();
	m_pointPass->reset();

	// reset matrices
	m_viewMatrix.setToIdentity();
	m_modelMatrix.setToIdentity();
	m_projMatrix.setToIdentity();

	// default viewport is entire view
	QSize size = m_sc->currentPixelSize();
	m_viewport = { 0, 0, float(size.width()), float(size.height()) };
}

void rhiRenderer::TimingInfo::update()
{
	m_toc = high_resolution_clock::now();
	time_point<high_resolution_clock> toc = high_resolution_clock::now();
	double sec = duration_cast<dseconds>(m_toc - m_tic).count();
	if (sec > 0)
	{
		m_fps = 1.0 / sec;
		if ((m_frame % 50) == 0)
		{
			m_fpsMin = m_fpsMax = m_fps;
		}
		else
		{
			if (m_fps < m_fpsMin) m_fpsMin = m_fps;
			if (m_fps > m_fpsMax) m_fpsMax = m_fps;
		}
	}
	m_tic = m_toc;
	m_frame++;
}

void rhiRenderer::finish()
{
	timing.update();

	QRhiResourceUpdateBatch* resourceUpdates = m_rhi->nextResourceUpdateBatch();

	if (m_initialUpdates) {
		resourceUpdates->merge(m_initialUpdates);
		m_initialUpdates->release();
		m_initialUpdates = nullptr;
	}

	QRhiCommandBuffer* cb = m_sc->currentFrameCommandBuffer();

	// set global properties
	m_global.setLightPosition(m_light);
	m_global.setSpecularColor(m_lightSpecular);
	m_global.setClipPlane(clipPlane);
	m_global.update(resourceUpdates);

	if (m_texture.needsUpload)
	{
		m_texture.upload(resourceUpdates);
		m_texture.needsUpload = false;
	}

	// update solid mesh data
	m_solidPass->m_proj = m_projMatrix;
	m_solidPass->m_view = m_viewMatrix;
	m_solidPass->update(resourceUpdates);

	// update line mesh data
	m_linePass->m_proj = m_projMatrix;
	m_linePass->m_view = m_viewMatrix;
	m_linePass->update(resourceUpdates);

	// update point mesh data
	m_pointPass->m_proj = m_projMatrix;
	m_pointPass->m_view = m_viewMatrix;
	m_pointPass->update(resourceUpdates);

	// update fps indicator
	m_canvasPass->setFPS(timing.m_fps, timing.m_fpsMin, timing.m_fpsMax);
	m_canvasPass->update(resourceUpdates);

	// overlay stuff
	if (m_useOverlay)
	{
		m_overlayPass->update(resourceUpdates);

		// render into overlay
		cb->beginPass(m_overlayPass->renderTarget(), Qt::red, {1.0f, 0}, resourceUpdates);
		{
			// Draw the triad
			m_overlayPass->drawTriad(cb);
		}
		cb->endPass();

		// resourceUpdates is automatically released by beginPass, so allocate a new one
		resourceUpdates = m_rhi->nextResourceUpdateBatch();
	}

	// start the rendering pass
	cb->beginPass(m_sc->currentFrameRenderTarget(), m_bgColor, { 1.0f, 0 }, resourceUpdates);
	{
		// set the viewport 
		cb->setViewport({ m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3] });

		// render solid meshes
		m_solidPass->draw(cb);

		// render line meshes
		m_linePass->draw(cb);

		// render point meshes
		m_pointPass->draw(cb);

		// render fps indicator
		m_canvasPass->draw(cb);

		// render overlay
		if (m_useOverlay)
		{
			m_overlayPass->draw(cb);
		}
	}
	cb->endPass();
}

void rhiRenderer::beginShape()
{
	mb.beginShape();
}

void rhiRenderer::endShape()
{
	mb.endShape();
	GLMesh* pm = mb.takeMesh();
	if (pm)
	{
		if ((pm->Faces() == 0) && (pm->Edges() > 0))
		{
			renderGMeshEdges(*pm, false);
		}
		delete pm;
	}
}

void rhiRenderer::begin(PrimitiveType prim)
{
	mb.begin(prim);
}

void rhiRenderer::end()
{
	mb.end();
}

void rhiRenderer::vertex(const vec3d& r)
{
	mb.vertex(r);
}

void rhiRenderer::normal(const vec3d& n)
{
	mb.normal(n);
}

void rhiRenderer::texCoord1d(double t)
{
	mb.texCoord1d(t);
}

void rhiRenderer::texCoord2d(double r, double s)
{
	mb.texCoord2d(r, s);
}
