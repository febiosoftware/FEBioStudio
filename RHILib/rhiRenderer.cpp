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

rhiRenderer::rhiRenderer(QRhi* rhi, QRhiSwapChain* sc, QRhiRenderPassDescriptor* rp) : m_rhi(rhi), m_sc(sc), m_rp(rp)
{
}

rhiRenderer::~rhiRenderer()
{
	clearCache();
}

QRhiGraphicsPipeline* rhiRenderer::createPipeline(QVector<QRhiShaderStage>& shaders, QRhiGraphicsPipeline::CullMode cullMode)
{
	QRhiGraphicsPipeline* pl = m_rhi->newGraphicsPipeline();

	pl->setSampleCount(m_sc->sampleCount());
	pl->setDepthTest(true);
	pl->setDepthWrite(true);

	// Blend factors default to One, OneOneMinusSrcAlpha, which is convenient.
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = true;
	blendState.srcColor = QRhiGraphicsPipeline::SrcAlpha;
	blendState.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
	blendState.opColor = QRhiGraphicsPipeline::Add;

	blendState.srcAlpha = QRhiGraphicsPipeline::One;
	blendState.dstAlpha = QRhiGraphicsPipeline::OneMinusSrcAlpha;
	blendState.opAlpha = QRhiGraphicsPipeline::Add;


	pl->setTargetBlends({ blendState });
	pl->setShaderStages(shaders.begin(), shaders.end());
	pl->setTopology(QRhiGraphicsPipeline::Triangles);

	// set the layout of the vertex data buffer.
	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings({
		{ 9 * sizeof(float) }
		});
	inputLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 },
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) },
		{ 0, 2, QRhiVertexInputAttribute::Float3, 6 * sizeof(float) }
		});
	pl->setVertexInputLayout(inputLayout);
	pl->setShaderResourceBindings(m_colorSrb->get());
	pl->setRenderPassDescriptor(m_rp);

	pl->setCullMode(cullMode);
	pl->setFrontFace(QRhiGraphicsPipeline::CCW);
	pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);

	pl->create();

	return pl;
}

void rhiRenderer::init()
{
	m_initialUpdates = m_rhi->nextResourceUpdateBatch();

	globalBuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 32));
	globalBuf->create();

	m_colorSrb.reset(new rhi::ShaderResource());
	m_colorSrb->create(m_rhi, globalBuf.get());

	// Load shaders
	QVector<QRhiGraphicsShaderStage> shaders= {
		{ QRhiShaderStage::Vertex  , getShader(QLatin1String(":/RHILib/shaders/color.vert.qsb")) },
		{ QRhiShaderStage::Fragment, getShader(QLatin1String(":/RHILib/shaders/color.frag.qsb")) } };

	m_frontRender.reset(createPipeline(shaders, QRhiGraphicsPipeline::Back));
	m_backRender.reset(createPipeline(shaders, QRhiGraphicsPipeline::Front));
}

void rhiRenderer::clearCache()
{
	for (auto& it : m_meshList) delete it.second;
	m_meshList.clear();
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
	m_view.setToIdentity();

	// target in camera coordinates
	vec3d r = cam.Target();

	// position the target in camera coordinates
	m_view.translate(-r.x, -r.y, -r.z);

	// orient the camera
	quatd q = cam.m_rot.Value();
	m_view.rotate(QQuaternion(q.w, q.x, q.y, q.z));

	// translate to world coordinates
	vec3d p = cam.GetPosition();
	m_view.translate(-p.x, -p.y, -p.z);
}

void rhiRenderer::setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly)
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
	}
	else
	{
		rhi::ShaderResource* sr = new rhi::ShaderResource();
		sr->create(m_rhi, globalBuf.get());
		rhi::Mesh* rm = new rhi::Mesh(m_rhi, sr);
		rm->CreateFromGLMesh(&mesh);
		rm->SetVertexColor(col);
		rm->SetMaterial(m_currentMat);
		m_meshList[&mesh] = rm;
	}
}

void rhiRenderer::setViewProjection(const QMatrix4x4& proj)
{
	m_proj = proj;
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

	// set light properties
	float s[4] = { 0.f };
	m_lightSpecular.toFloat(s);

	float lp[8] = { 
		m_light.x, m_light.y, m_light.z, 0.f,
		s[0], s[1], s[2], 1.f
	};
	resourceUpdates->updateDynamicBuffer(globalBuf.get(), 0, 32, lp);

	// update mesh data
	for (auto& it : m_meshList)
		it.second->Update(resourceUpdates, m_proj, m_view);

	// start the rendering pass
	cb->beginPass(m_sc->currentFrameRenderTarget(), m_bgColor, { 1.0f, 0 }, resourceUpdates);

	// render back faces first
	cb->setGraphicsPipeline(m_backRender.get());
	cb->setViewport({ 0, 0, float(outputSizeInPixels.width()), float(outputSizeInPixels.height()) });
	cb->setShaderResources();

	for (auto& it : m_meshList)
		it.second->Draw(cb);

	// render front faces next
	cb->setGraphicsPipeline(m_frontRender.get());
	cb->setShaderResources();

	for (auto& it : m_meshList)
		it.second->Draw(cb);

	cb->endPass();
}
