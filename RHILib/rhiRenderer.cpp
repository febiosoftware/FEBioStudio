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
#include <GLLib/GLTexture3D.h>


void GlobalUniformBlock::create(QRhi* rhi)
{
	m_ub.create({
		{ rhi::UniformBlock::VEC4, "lightPos" },
		{ rhi::UniformBlock::VEC4, "ambient"},
		{ rhi::UniformBlock::VEC4, "specColor"},
		{ rhi::UniformBlock::VEC4, "clipPlane"}
	});

	m_ub.setVec4(LIGHTSPEC, GLColor::White());
	m_ub.setVec4(LIGHTAMB, GLColor(50, 50, 50));

	m_ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_ub.size()));
	m_ubuf->create();
}

void GlobalUniformBlock::setLightPosition(const vec3f& lp) { m_ub.setVec4(LIGHTPOS, lp); }

void GlobalUniformBlock::setAmbientColor(GLColor c)
{
	float f[4] = { 0.f };
	c.toFloat(f);
	m_ub.setVec4(LIGHTAMB, f[0], f[1], f[2], f[3]);
}

void GlobalUniformBlock::setSpecularColor(GLColor c) 
{ 
	float f[4] = { 0.f };
	c.toFloat(f);
	m_ub.setVec4(LIGHTSPEC, f[0], f[1], f[2], f[3]);
}
void GlobalUniformBlock::setClipPlane(const float f[4]) { m_ub.setVec4(CLIPPLANE, f[0], f[1], f[2], f[3]); }

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

rhiRenderer::rhiRenderer(QRhi* rhi, QRhiSwapChain* sc, QRhiRenderPassDescriptor* rp) : m_rhi(rhi), m_sc(sc), m_rp(rp), m_tex1D(rhi), m_envTex(rhi)
{
	m_lightSpecular = GLColor::White();
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
	m_tex1D.create(img);
	m_tex1D.upload(m_initialUpdates);

	// create with dummy image
	QImage envImg(QSize(100, 100), QImage::Format_RGB32);
	envImg.fill(Qt::black);
	m_envTex.create(envImg);

	// convenience class for passing around all the resources that are shared between shaders
	m_sharedResources = { m_global.get(), m_tex1D.texture.get(), m_tex1D.sampler.get(), m_envTex.texture.get(), m_envTex.sampler.get()};

	// create all render passes
	m_solidPass.reset(new TwoPassSolidRenderPass(m_rhi));
	m_solidPass->create(m_sc, &m_sharedResources);

	m_solidOverlayPass.reset(new SolidRenderPass(m_rhi));
	m_solidOverlayPass->setDepthTest(false);
	m_solidOverlayPass->create(m_sc, &m_sharedResources);

	m_volumeRenderPass.reset(new VolumeRenderPass(m_rhi));
	m_volumeRenderPass->create(m_sc);

	m_linePass.reset(new LineRenderPass(m_rhi));
	m_linePass->create(m_sc, &m_sharedResources);

	m_lineOverlayPass.reset(new LineRenderPass(m_rhi));
	m_lineOverlayPass->setDepthTest(false);
	m_lineOverlayPass->create(m_sc, &m_sharedResources);

	m_pointPass.reset(new PointRenderPass(m_rhi));
	m_pointPass->create(m_sc, &m_sharedResources);

	m_pointOverlayPass.reset(new PointRenderPass(m_rhi));
	m_pointOverlayPass->setDepthTest(false);
	m_pointOverlayPass->create(m_sc, &m_sharedResources);

	m_overlay2DPass.reset(new OverlayRenderPass(m_rhi));
	m_overlay2DPass->create(m_sc, &m_sharedResources);

	// create the canvas pass (for rendering fps)
	m_canvasPass.reset(new CanvasRenderPass(m_rhi));
	m_canvasPass->create(m_sc);

	// disable this pass by default
	m_canvasPass->enable(false);

	// background rendering pass
	m_gradientPass.reset(new GradientRenderPass(m_rhi));
	m_gradientPass->create(m_sc);

	// reset timing
	timing.m_tic = timing.m_toc = steady_clock::now();
}

void rhiRenderer::clearCache()
{
	if (m_solidPass) m_solidPass->clearCache();
	if (m_solidOverlayPass) m_solidOverlayPass->clearCache();
	if (m_volumeRenderPass) m_volumeRenderPass->clearCache();
	if (m_linePass ) m_linePass ->clearCache();
	if (m_lineOverlayPass) m_lineOverlayPass->clearCache();
	if (m_pointPass) m_pointPass->clearCache();
	if (m_pointOverlayPass) m_pointOverlayPass->clearCache();
}

QSize rhiRenderer::pixelSize() const
{
	return m_sc->surfacePixelSize();
}

int rhiRenderer::surfaceWidth() const { return pixelSize().width(); }
int rhiRenderer::surfaceHeight() const { return pixelSize().height(); }

GLRenderStats rhiRenderer::GetRenderStats() const
{
	// update cache count
	GLRenderStats stats = GLRenderEngine::GetRenderStats();

	stats.cachedObjects = 0;
	stats.cachedObjects += m_solidPass->cachedMeshes();
	stats.cachedObjects += m_solidOverlayPass->cachedMeshes();
	stats.cachedObjects += m_linePass->cachedMeshes();
	stats.cachedObjects += m_lineOverlayPass->cachedMeshes();
	stats.cachedObjects += m_pointPass->cachedMeshes();
	stats.cachedObjects += m_pointOverlayPass->cachedMeshes();
	stats.cachedObjects += m_volumeRenderPass->cachedMeshes();

	// note that at this point, nothing has actually been uploaded yet.
	// so this reflects the total of the last frame. 
	stats.dataUploadSize = rhi::Mesh::uploadedBytes;

	return stats;
}

void rhiRenderer::clearUnusedCache()
{
	m_solidPass->clearUnusedCache();
	m_solidOverlayPass->clearUnusedCache();
	m_volumeRenderPass->clearUnusedCache();
	m_linePass->clearUnusedCache();
	m_lineOverlayPass->clearUnusedCache();
	m_pointPass->clearUnusedCache();
	m_pointOverlayPass->clearUnusedCache();
}

void rhiRenderer::deleteCachedMesh(GLMesh* gm)
{
	m_solidPass->removeCachedMesh(gm);
	m_solidOverlayPass->removeCachedMesh(gm);
	m_volumeRenderPass->removeCachedMesh(gm);
	m_linePass->removeCachedMesh(gm);
	m_lineOverlayPass->removeCachedMesh(gm);
	m_pointPass->removeCachedMesh(gm);
	m_pointOverlayPass->removeCachedMesh(gm);
}

void rhiRenderer::setClearColor(const GLColor& c)
{
	m_clearColor = QColor(c.r, c.g, c.b, c.a);
}

void rhiRenderer::setBackgroundGradient(const GLColor& c1, const GLColor& c2, GradientType grad)
{
	if (m_rhi->isYUpInNDC() || (grad == GradientType::VERTICAL))
		m_gradientPass->setColorGradient(c1, c2, (int)grad);
	else
		m_gradientPass->setColorGradient(c2, c1, (int)grad);
}

void rhiRenderer::setLightPosition(unsigned int n, const vec3f& lp)
{
	m_light = lp;
}

void rhiRenderer::setLightSpecularColor(unsigned int lightIndex, const GLColor& col)
{
	m_lightSpecular = col;
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

void rhiRenderer::resetTransform()
{
	m_modelViewMatrix.setToIdentity();
}

void rhiRenderer::pushTransform()
{
	m_transformStack.push(m_modelViewMatrix);
	if (buildingShape) mb.pushTransform();
}

void rhiRenderer::popTransform()
{
	if (buildingShape) mb.popTransform();
	assert(!m_transformStack.empty());
	if (m_transformStack.empty()) return;
	m_modelViewMatrix = m_transformStack.top();
	m_transformStack.pop();
}

void rhiRenderer::translate(const vec3d& r)
{
	m_modelViewMatrix.translate(QVector3D(r.x, r.y, r.z));
	mb.translate(r);
}

void rhiRenderer::rotate(const quatd& rot)
{
	m_modelViewMatrix.rotate(QQuaternion(rot.w, rot.x, rot.y, rot.z));
	mb.rotate(rot);
}

void rhiRenderer::rotate(double deg, double x, double y, double z)
{
	quatd q(deg * DEG2RAD, vec3d(x, y, z));
	rotate(q);
}

void rhiRenderer::scale(double x, double y, double z)
{
	m_modelViewMatrix.scale(x, y, z);
}

void rhiRenderer::setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
{
	m_currentMat.type = matType;
	m_currentMat.diffuseMap = map;
	m_currentMat.frontOnly = frontOnly;
	m_currentMat.diffuse = m_currentMat.ambient = c;
	m_currentMat.specular = GLColor(0, 0, 0);
	m_currentMat.reflection = 0;
	m_currentMat.shininess = 0;

	mb.setColor(c);
}

void rhiRenderer::setColor(GLColor c)
{
	m_currentMat.diffuse = c;
	mb.setColor(c);
}

void rhiRenderer::setMaterial(const GLMaterial& mat)
{
	m_currentMat = mat;
	mb.setColor(mat.diffuse);
}

void rhiRenderer::renderGMesh(const GLMesh& mesh, bool cacheMesh)
{
	rhi::Mesh* pm = nullptr;
	if (m_currentMat.diffuseMap == GLMaterial::TEXTURE_3D)
	{
		pm = m_volumeRenderPass->addGLMesh(mesh, -1, cacheMesh);
	}
	else if (m_currentMat.type == GLMaterial::OVERLAY)
	{
		pm = m_solidOverlayPass->addGLMesh(mesh, -1, cacheMesh);
	}
	else
	{
		pm = m_solidPass->addGLMesh(mesh, -1, cacheMesh);
	}

	if (pm)
	{
		pm->SetMaterial(m_currentMat);
		pm->SetMatrices(m_modelViewMatrix, m_projMatrix);
		pm->doClipping = m_clipEnabled;
		pm->setActive(true);

		m_stats.triangles += (pm->vertexCount() / 3); // 3 vertices per triangle
	}
}

void rhiRenderer::renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh)
{
	rhi::Mesh* pm = nullptr;
	if (m_currentMat.type == GLMaterial::OVERLAY)
	{
		// TODO: implement this
	}
	else
	{
		pm = m_solidPass->addGLMesh(mesh, surfId, cacheMesh);
	}

	if (pm)
	{
		pm->SetMaterial(m_currentMat);
		pm->SetMatrices(m_modelViewMatrix, m_projMatrix);
		pm->doClipping = m_clipEnabled;
		pm->setActive(true);

		m_stats.triangles += (pm->vertexCount() / 3); // 3 vertices per triangle
	}
}

void rhiRenderer::renderGMeshEdges(const GLMesh& mesh, bool cacheMesh)
{
	rhi::Mesh* lineMesh = nullptr;
	if (m_currentMat.type == GLMaterial::OVERLAY)
		lineMesh = m_lineOverlayPass->addGLMesh(mesh, -1, cacheMesh);
	else
		lineMesh = m_linePass->addGLMesh(mesh, -1, cacheMesh);

	if (lineMesh)
	{
		lineMesh->SetMaterial(m_currentMat);
		lineMesh->SetMatrices(m_modelViewMatrix, m_projMatrix);
		lineMesh->doClipping = m_clipEnabled;
		lineMesh->setActive(true);

		m_stats.lines += (lineMesh->vertexCount() / 2); // 2 vertices per edge
	}
}

void rhiRenderer::renderGMeshEdges(const GLMesh& mesh, int partition, bool cacheMesh)
{
	rhi::Mesh* lineMesh = nullptr;
	if (m_currentMat.type == GLMaterial::OVERLAY)
		lineMesh = m_lineOverlayPass->addGLMesh(mesh, partition, cacheMesh);
	else
		lineMesh = m_linePass->addGLMesh(mesh, partition, cacheMesh);

	if (lineMesh)
	{
		lineMesh->SetMaterial(m_currentMat);
		lineMesh->SetMatrices(m_modelViewMatrix, m_projMatrix);
		lineMesh->doClipping = m_clipEnabled;
		lineMesh->setActive(true);

		m_stats.lines += (lineMesh->vertexCount() / 2); // 2 vertices per edge
	}
}

void rhiRenderer::renderGMeshNodes(const GLMesh& mesh, bool cacheMesh)
{
	rhi::Mesh* pointMesh = nullptr;
	if (m_currentMat.type == GLMaterial::OVERLAY)
		pointMesh = m_pointOverlayPass->addGLMesh(mesh, -1, cacheMesh);
	else
		pointMesh = m_pointPass->addGLMesh(mesh, -1, cacheMesh);

	if (pointMesh)
	{
		pointMesh->SetMaterial(m_currentMat);
		pointMesh->SetMatrices(m_modelViewMatrix, m_projMatrix);
		pointMesh->doClipping = m_clipEnabled;
		pointMesh->setActive(true);

		m_stats.points += pointMesh->vertexCount();
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
		m_tex1D.image = img;
		m_tex1D.needsUpload = true;
		tex.Update(false);
	}
}

void rhiRenderer::setTexture(GLTexture3D& tex)
{
	m_volumeRenderPass->setTexture3D(tex);
}

void rhiRenderer::setClipPlane(unsigned int n, const double* v)
{
	if (n == 0)
	{
		// Transform to eye coordinates
		QVector4D N(v[0], v[1], v[2], 0);
		QMatrix4x4 modelView = m_modelViewMatrix;
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

unsigned int rhiRenderer::SetEnvironmentMap(const CRGBAImage& img)
{
	if (img.isNull()) return 0;

	if (envImg == nullptr)
	{
		QImage map(img.GetBytes(), img.Width(), img.Height(), QImage::Format::Format_RGBA8888_Premultiplied);
		if (!map.isNull())
		{
			m_envTex.setImage(map);
			envImg = &img;
			return 1;
		}
		else return 0;
	}
	else return 1;
}

void rhiRenderer::ActivateEnvironmentMap(unsigned int mapid)
{
}

void rhiRenderer::DeactivateEnvironmentMap(unsigned int mapid)
{
}

void rhiRenderer::useOverlayImage(bool b)
{
	m_useOverlay = b;
}

void rhiRenderer::showFPS(bool b)
{
	m_canvasPass->enable(b);
}

void rhiRenderer::setOverlayImage(const QImage& img)
{
	m_overlay2DPass->setImage(img);
}

void rhiRenderer::setTriadInfo(const QMatrix4x4& m, QRhiViewport vp)
{
	m_overlay2DPass->m_overlayVP = vp;
	m_overlay2DPass->m_overlayVM = m;
}

void rhiRenderer::start()
{
	GLRenderEngine::start();

	// start by setting all meshes as inactive
	m_solidPass->reset();
	m_solidOverlayPass->reset();
	m_volumeRenderPass->reset();
	m_linePass->reset();
	m_lineOverlayPass->reset();
	m_pointPass->reset();
	m_pointOverlayPass->reset();

	// reset matrices
	m_modelViewMatrix.setToIdentity();
	m_projMatrix.setToIdentity();

	// default viewport is entire view
	QSize size = m_sc->currentPixelSize();
	m_viewport = { 0, 0, float(size.width()), float(size.height()) };
}

void rhiRenderer::TimingInfo::update()
{
	m_toc = steady_clock::now();
	time_point<steady_clock> toc = steady_clock::now();
	double sec = duration_cast<dseconds>(m_toc - m_tic).count();
	totalSec += sec;
	if (totalSec > 0)
	{
		m_frame++;
		double fps = 1.0 / sec; // instanteneous FPS
		m_fps = m_frame / totalSec; // average FPS
		if (m_frame  >= 50)
		{
			m_fpsMin = m_fpsMax = fps;
			m_frame = 0;
			totalSec = 0;
		}
		else
		{
			if (fps < m_fpsMin) m_fpsMin = fps;
			if (fps > m_fpsMax) m_fpsMax = fps;
		}
	}
	m_tic = m_toc;
}

void rhiRenderer::finish()
{
	// reset the mesh data upload size
	rhi::Mesh::uploadedBytes = 0;

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

	if (m_tex1D.needsUpload)
	{
		m_tex1D.upload(resourceUpdates);
		m_tex1D.needsUpload = false;
	}

	if (m_envTex.needsUpload)
	{
		m_envTex.upload(resourceUpdates);
		m_envTex.needsUpload = false;
	}

	// update solid mesh data
	m_solidPass->update(resourceUpdates);
	m_solidOverlayPass->update(resourceUpdates);
	m_volumeRenderPass->update(resourceUpdates);

	// update line mesh data
	m_linePass->update(resourceUpdates);
	m_lineOverlayPass->update(resourceUpdates);

	// update point mesh data
	m_pointPass->update(resourceUpdates);

	// update overlay point mesh data
	m_pointOverlayPass->update(resourceUpdates);

	// update fps indicator
	if (m_canvasPass->isEnabled())
	{
		m_canvasPass->setFPS(timing.m_fps, timing.m_fpsMin, timing.m_fpsMax);
		m_canvasPass->update(resourceUpdates);
	}

	// update background pass
	m_gradientPass->update(resourceUpdates);

	// overlay stuff
	if (m_useOverlay)
	{
		m_overlay2DPass->update(resourceUpdates);

		// render into overlay
		cb->beginPass(m_overlay2DPass->renderTarget(), Qt::red, {1.0f, 0}, resourceUpdates);
		{
			// Draw the triad
			m_overlay2DPass->drawTriad(cb);
		}
		cb->endPass();

		// resourceUpdates is automatically released by beginPass, so allocate a new one
		resourceUpdates = m_rhi->nextResourceUpdateBatch();
	}

	// start the rendering pass
	cb->beginPass(m_sc->currentFrameRenderTarget(), m_clearColor, { 1.0f, 0 }, resourceUpdates);
	{
		// set the viewport 
		cb->setViewport({ m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3] });

		// draw the background first
		m_gradientPass->draw(cb);

		// render solid meshes
		m_solidPass->draw(cb);

		// render line meshes
		m_linePass->draw(cb);

		// render point meshes
		m_pointPass->draw(cb);

		// render overlay solid meshes
		m_solidOverlayPass->draw(cb);

		// render overlay line meshes
		m_lineOverlayPass->draw(cb);

		// render overlay point meshes
		m_pointOverlayPass->draw(cb);

		// render volume meshes
		m_volumeRenderPass->draw(cb);

		// render fps indicator
		if (m_canvasPass->isEnabled())
			m_canvasPass->draw(cb);

		// render overlay
		if (m_useOverlay)
		{
			m_overlay2DPass->draw(cb);
		}
	}
	cb->endPass();

	if (captureNextFrame)
	{
		captureNextFrame = false;
		framesRequested++;

		QRhiReadbackDescription rbDesc;

		static QRhiReadbackResult rbResult;  // must stay alive until callback fires
		rbResult.completed = [this]() {
			qDebug() << "Readback done! Size:" << rbResult.pixelSize
				<< "Bytes:" << rbResult.data.size();

			int W = rbResult.pixelSize.width();
			int H = rbResult.pixelSize.height();

			const uchar* pixels = reinterpret_cast<const uchar*>(rbResult.data.constData());
			QImage img;
			
			if (rbResult.format == QRhiTexture::RGBA8)
				img = QImage(pixels, W, H, QImage::Format_RGBA8888);
			else if (rbResult.format == QRhiTexture::BGRA8)
				img = QImage(pixels, W, H, QImage::Format_RGBA8888).rgbSwapped();

			framesRequested--;
			if (framesRequested < 0) framesRequested = 0;

			emit this->captureFrameReady(img);
			};

		// --- Enqueue the readback on a resource update batch ---
		QRhiResourceUpdateBatch* u = m_rhi->nextResourceUpdateBatch();
		u->readBackTexture(rbDesc, &rbResult);

		// Commit the batch. You must pass the batch to the command buffer so QRhi
		// will process it. There are two common ways:
		//  - pass it to beginPass/endPass (typical): pass as the resourceUpdates parameter
		//  - or call cb->resourceUpdate(u) when not inside a pass.
		//
		// In this example we already ended the pass, so submit the batch directly:
		cb->resourceUpdate(u);
	}
}

void rhiRenderer::beginShape()
{
	mb.beginShape();
	buildingShape = true;
}

void rhiRenderer::endShape()
{
	mb.endShape();
	GLMesh* pm = mb.takeMesh();
	if (pm)
	{
		m_currentMat.diffuseMap = GLMaterial::VERTEX_COLOR;
		if (pm->Faces() > 0)
		{
			renderGMesh(*pm, false);
		}

		if (pm->Edges() > 0)
		{
			renderGMeshEdges(*pm, false);
		}

		if ((pm->Nodes() > 0) && (pm->Edges() == 0) && (pm->Faces() == 0))
		{
			renderGMeshNodes(*pm, false);
		}

		delete pm;
	}
	buildingShape = false;
}

void rhiRenderer::begin(PrimitiveType prim)
{
	if (!buildingShape) mb.beginShape();
	mb.begin(prim);
}

void rhiRenderer::end()
{
	mb.end();
	if (!buildingShape) endShape();
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
