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
#include "rhiRenderer.h"
#include <QFile>

static QShader getShader(const QString& name)
{
	QFile f(name);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());

	return QShader();
}

static QRhiGraphicsPipeline::TargetBlend defaultBlendState()
{
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = true;
	blendState.srcColor = QRhiGraphicsPipeline::SrcAlpha;
	blendState.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
	blendState.opColor = QRhiGraphicsPipeline::Add;
	blendState.srcAlpha = QRhiGraphicsPipeline::One;
	blendState.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
	blendState.opAlpha = QRhiGraphicsPipeline::Add;
	return blendState;
}

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

void rhi::PointRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr)
{
	// create point render pipeline
	QRhiVertexInputLayout pointMeshLayout;
	pointMeshLayout.setBindings({
		{ 3 * sizeof(float) }
		});
	pointMeshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 } // position
		});

	QVector<QRhiShaderStage> shaders = {
		{ QRhiShaderStage::Vertex  , getShader(QLatin1String(":/RHILib/shaders/point.vert.qsb")) },
		{ QRhiShaderStage::Fragment, getShader(QLatin1String(":/RHILib/shaders/point.frag.qsb")) } };

	m_sr.reset(new rhi::PointShaderResource());
	m_sr->create(m_rhi, sr);

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(false);
	m_pl->setTargetBlends({ defaultBlendState()});

	m_pl->setShaderStages(shaders.begin(), shaders.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Points);

	m_pl->setVertexInputLayout(pointMeshLayout);
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void rhi::LineRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr)
{
	// create point render pipeline
	QRhiVertexInputLayout lineMeshLayout;
	lineMeshLayout.setBindings({
		{ 3 * sizeof(float) }
		});
	lineMeshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 } // position
		});

	QVector<QRhiShaderStage> shaders = {
		{ QRhiShaderStage::Vertex  , getShader(QLatin1String(":/RHILib/shaders/lines.vert.qsb")) },
		{ QRhiShaderStage::Fragment, getShader(QLatin1String(":/RHILib/shaders/lines.frag.qsb")) } };

	m_sr.reset(new rhi::LineShaderResource());
	m_sr->create(m_rhi, sr);

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(false);
	m_pl->setTargetBlends({ defaultBlendState()});

	m_pl->setShaderStages(shaders.begin(), shaders.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Lines);

	m_pl->setVertexInputLayout(lineMeshLayout);
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void rhi::FrontFaceRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr)
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 12 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // normal 
		{ 0, 2, QRhiVertexInputAttribute::Float3, 6 * sizeof(float) }, // color
		{ 0, 3, QRhiVertexInputAttribute::Float3, 9 * sizeof(float) }, // texcoord
		});

	QVector<QRhiShaderStage> shaders = {
		{ QRhiShaderStage::Vertex  , getShader(QLatin1String(":/RHILib/shaders/color.vert.qsb")) },
		{ QRhiShaderStage::Fragment, getShader(QLatin1String(":/RHILib/shaders/color.frag.qsb")) } };

	m_sr.reset(new rhi::ColorShaderResource());
	m_sr->create(m_rhi, sr);

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(true);
	m_pl->setTargetBlends({ defaultBlendState() });

	m_pl->setShaderStages(shaders.begin(), shaders.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);

	m_pl->setCullMode(QRhiGraphicsPipeline::Back);
	m_pl->setFrontFace(QRhiGraphicsPipeline::CCW);

	m_pl->setVertexInputLayout(meshLayout);
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void rhi::BackFaceRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr)
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 12 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // normal 
		{ 0, 2, QRhiVertexInputAttribute::Float3, 6 * sizeof(float) }, // color
		{ 0, 3, QRhiVertexInputAttribute::Float3, 9 * sizeof(float) }, // texcoord
		});

	QVector<QRhiShaderStage> shaders = {
		{ QRhiShaderStage::Vertex  , getShader(QLatin1String(":/RHILib/shaders/color.vert.qsb")) },
		{ QRhiShaderStage::Fragment, getShader(QLatin1String(":/RHILib/shaders/color.frag.qsb")) } };

	m_sr.reset(new rhi::ColorShaderResource());
	m_sr->create(m_rhi, sr);

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(true);
	m_pl->setTargetBlends({ defaultBlendState() });

	m_pl->setShaderStages(shaders.begin(), shaders.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);

	m_pl->setCullMode(QRhiGraphicsPipeline::Front);
	m_pl->setFrontFace(QRhiGraphicsPipeline::CCW);

	m_pl->setVertexInputLayout(meshLayout);
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

rhiRenderer::rhiRenderer(QRhi* rhi, QRhiSwapChain* sc, QRhiRenderPassDescriptor* rp) : m_rhi(rhi), m_sc(sc), m_rp(rp)
{
}

rhiRenderer::~rhiRenderer()
{
	clearCache();
}

QImage createTextureImage(QSize size)
{
	QImage img(size, QImage::Format_RGBA8888);
	img.fill(Qt::white);
	return img;
}

void rhiRenderer::init()
{
	m_initialUpdates = m_rhi->nextResourceUpdateBatch();

	m_global.create(m_rhi);

	// prep texture
	m_texture.sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
		QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
	m_texture.sampler->create();

	// For Vulkan it looks like we can't change the texture size after creation, so be careful!
	m_texture.texture.reset(m_rhi->newTexture(QRhiTexture::RGBA8, QSize(1024, 1)));
	m_texture.texture->create();
	m_texture.image = createTextureImage(m_texture.texture->pixelSize());
	m_texture.upload(m_initialUpdates);

	m_sharedResources = { m_global.get(), m_texture.texture.get(), m_texture.sampler.get()};

	m_backPass.reset(new rhi::BackFaceRenderPass(m_rhi));
	m_backPass->create(m_rp, m_sc->sampleCount(), &m_sharedResources);

	m_frontPass.reset(new rhi::FrontFaceRenderPass(m_rhi));
	m_frontPass->create(m_rp, m_sc->sampleCount(), &m_sharedResources);

	m_linePass.reset(new rhi::LineRenderPass(m_rhi));
	m_linePass->create(m_rp, m_sc->sampleCount(), &m_sharedResources);

	m_pointPass.reset(new rhi::PointRenderPass(m_rhi));
	m_pointPass->create(m_rp, m_sc->sampleCount(), &m_sharedResources);
}

void rhiRenderer::clearCache()
{
	for (auto& it : m_meshList) delete it.second;
	m_meshList.clear();

	for (auto& it : m_lineMeshList) delete it.second;
	m_lineMeshList.clear();

	for (auto& it : m_pointMeshList) delete it.second;
	m_pointMeshList.clear();
}

void rhiRenderer::clearUnusedCache()
{
	for (auto it = m_meshList.begin(); it != m_meshList.end(); ) {
		if (it->second->isActive() == false)
		{
			delete it->second;
			it = m_meshList.erase(it);
		}
		else
			++it;
	}

	for (auto it = m_lineMeshList.begin(); it != m_lineMeshList.end(); ) {
		if (it->second->isActive() == false)
		{
			delete it->second;
			it = m_lineMeshList.erase(it);
		}
		else
			++it;
	}

	for (auto it = m_pointMeshList.begin(); it != m_pointMeshList.end(); ) {
		if (it->second->isActive() == false)
		{
			delete it->second;
			it = m_pointMeshList.erase(it);
		}
		else
			++it;
	}
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
}

void rhiRenderer::setColor(GLColor c)
{
	m_currentMat.diffuse = m_currentMat.ambient = c;
}

void rhiRenderer::setMaterial(const GLMaterial& mat)
{
	m_currentMat = mat;
}

void rhiRenderer::renderGMesh(const GLMesh& mesh, bool cacheMesh)
{
	float f[4] = { 0.f };
	m_currentMat.diffuse.toFloat(f);
	vec3f col(f[0], f[1], f[2]);

	auto it = m_meshList.find(&mesh);
	if (it != m_meshList.end())
	{
		if (cacheMesh == false)
		{
			it->second->CreateFromGLMesh(&mesh);
			it->second->SetVertexColor(col);
		}
		it->second->SetMaterial(m_currentMat);
		it->second->SetModelMatrix(m_modelMatrix);
		it->second->doClipping = m_clipEnabled;
		it->second->setActive(true);
	}
	else
	{
		rhi::ColorShaderResource* sr = new rhi::ColorShaderResource();
		sr->create(m_rhi, &m_sharedResources);
		rhi::TriMesh* rm = new rhi::TriMesh(m_rhi, sr);
		rm->CreateFromGLMesh(&mesh);
		rm->SetVertexColor(col);
		rm->SetMaterial(m_currentMat);
		rm->SetModelMatrix(m_modelMatrix);
		rm->doClipping = m_clipEnabled;
		rm->setActive(true);
		m_meshList[&mesh] = rm;
	}
}

void rhiRenderer::renderGMeshEdges(const GLMesh& mesh, bool cacheMesh)
{
	float f[4] = { 0.f };
	m_currentMat.diffuse.toFloat(f);
	vec3f col(f[0], f[1], f[2]);

	auto it = m_lineMeshList.find(&mesh);
	if (it != m_lineMeshList.end())
	{
		if (cacheMesh == false)
		{
			it->second->CreateFromGLMesh(&mesh);
		}
		it->second->SetColor(col);
		it->second->SetModelMatrix(m_modelMatrix);
		it->second->doClipping = m_clipEnabled;
		it->second->setActive(true);
	}
	else
	{
		rhi::LineShaderResource* sr = new rhi::LineShaderResource();
		sr->create(m_rhi, &m_sharedResources);
		rhi::LineMesh* rm = new rhi::LineMesh(m_rhi, sr);
		rm->CreateFromGLMesh(&mesh);
		rm->SetColor(col);
		rm->SetModelMatrix(m_modelMatrix);
		rm->doClipping = m_clipEnabled;
		rm->setActive(true);
		m_lineMeshList[&mesh] = rm;
	}
}

void rhiRenderer::renderGMeshNodes(const GLMesh& mesh, bool cacheMesh)
{
	float f[4] = { 0.f };
	m_currentMat.diffuse.toFloat(f);
	vec3f col(f[0], f[1], f[2]);

	auto it = m_pointMeshList.find(&mesh);
	if (it != m_pointMeshList.end())
	{
		if (cacheMesh == false)
		{
			it->second->CreateFromGLMesh(&mesh);
		}
		it->second->SetColor(col);
		it->second->SetModelMatrix(m_modelMatrix);
		it->second->doClipping = m_clipEnabled;
		it->second->setActive(true);
	}
	else
	{
		rhi::PointShaderResource* sr = new rhi::PointShaderResource();
		sr->create(m_rhi, &m_sharedResources);
		rhi::PointMesh* rm = new rhi::PointMesh(m_rhi, sr);
		rm->CreateFromGLMesh(&mesh);
		rm->SetColor(col);
		rm->SetModelMatrix(m_modelMatrix);
		rm->doClipping = m_clipEnabled;
		rm->setActive(true);
		m_pointMeshList[&mesh] = rm;
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

void rhiRenderer::setViewProjection(const QMatrix4x4& proj)
{
	m_projMatrix = proj;
}

void rhiRenderer::start()
{
	ResetStats();

	// start by setting all meshes as inactive
	for (auto& it : m_meshList) it.second->setActive(false);
	for (auto& it : m_lineMeshList) it.second->setActive(false);
	for (auto& it : m_pointMeshList) it.second->setActive(false);

	// reset model and view matrices
	m_viewMatrix.setToIdentity();
	m_modelMatrix.setToIdentity();
}

void rhiRenderer::finish()
{
	QRhiResourceUpdateBatch* resourceUpdates = m_rhi->nextResourceUpdateBatch();

	if (m_initialUpdates) {
		resourceUpdates->merge(m_initialUpdates);
		m_initialUpdates->release();
		m_initialUpdates = nullptr;
	}

	QRhiCommandBuffer* cb = m_sc->currentFrameCommandBuffer();
	const QSize outputSizeInPixels = m_sc->currentPixelSize();

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

	// update mesh data
	for (auto& it : m_meshList)
	{
		rhi::TriMesh& m = *it.second;
		if (m.isActive())
			m.Update(resourceUpdates, m_projMatrix, m_viewMatrix);
	}

	// update mesh data
	for (auto& it : m_lineMeshList)
	{
		rhi::LineMesh& m = *it.second;
		if (m.isActive())
			m.Update(resourceUpdates, m_projMatrix, m_viewMatrix);
	}

	// update point mesh data
	for (auto& it : m_pointMeshList)
	{
		rhi::PointMesh& m = *it.second;
		if (m.isActive())
			m.Update(resourceUpdates, m_projMatrix, m_viewMatrix);
	}

	// start the rendering pass
	cb->beginPass(m_sc->currentFrameRenderTarget(), m_bgColor, { 1.0f, 0 }, resourceUpdates);

	// render back faces first
	cb->setGraphicsPipeline(m_backPass->pipeline());
	cb->setViewport({ 0, 0, float(outputSizeInPixels.width()), float(outputSizeInPixels.height()) });
	cb->setShaderResources();

	for (auto& it : m_meshList)
	{
		rhi::TriMesh& m = *it.second;
		if (m.isActive())
			m.Draw(cb);
	}

	// render front faces next
	cb->setGraphicsPipeline(m_frontPass->pipeline());
	cb->setShaderResources();

	for (auto& it : m_meshList)
	{
		rhi::TriMesh& m = *it.second;
		if (m.isActive())
			m.Draw(cb);
	}

	// render line meshes
	if (!m_lineMeshList.empty())
	{
		cb->setGraphicsPipeline(m_linePass->pipeline());
		cb->setShaderResources();
		for (auto& it : m_lineMeshList)
		{
			rhi::LineMesh& m = *it.second;
			if (m.isActive())
				m.Draw(cb);
		}
	}

	// render point meshes
	if (!m_pointMeshList.empty())
	{
		cb->setGraphicsPipeline(m_pointPass->pipeline());
		cb->setShaderResources();
		for (auto& it : m_pointMeshList)
		{
			rhi::PointMesh& m = *it.second;
			if (m.isActive())
				m.Draw(cb);
		}
	}

	cb->endPass();
}
