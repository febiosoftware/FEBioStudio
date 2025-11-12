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
#include "rhiOverlay.h"
#include "rhiUtil.h"
#include "rhiShader.h"
#include "rhiTriMesh.h"
#include <GLLib/GLMeshBuilder.h>
#include <GLLib/glx.h>

static GLMesh* buildTriadMesh()
{
	const double r0 = .05;
	const double r1 = .15;

	GLMeshBuilder mb;
	mb.start();

	mb.setMaterial(GLMaterial::PLASTIC, GLColor(255, 0, 0));
	mb.pushTransform();
	mb.rotate(90, 0, 1, 0);
	glx::drawCylinder(mb, r0, .9, 5);
	mb.translate(vec3d(0, 0, .8f));
	glx::drawCone(mb, r1, 0.2, 10);
	mb.popTransform();

	mb.setMaterial(GLMaterial::PLASTIC, GLColor(0, 255, 0));
	mb.pushTransform();
	mb.rotate(-90, 1, 0, 0);
	glx::drawCylinder(mb, r0, .9, 5);
	mb.translate(vec3d(0, 0, .8f));
	glx::drawCone(mb, r1, 0.2, 10);
	mb.popTransform();

	mb.setMaterial(GLMaterial::PLASTIC, GLColor(0, 0, 255));
	mb.pushTransform();
	glx::drawCylinder(mb, r0, .9, 5);
	mb.translate(vec3d(0, 0, .8f));
	glx::drawCone(mb, r1, 0.2, 10);
	mb.popTransform();

	mb.finish();

	return mb.takeMesh();
}

void TriadRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, rhi::SharedResources* sr)
{
	TriadShader shader(m_rhi);

	m_sr.reset(shader.createShaderResource(m_rhi, sr));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(true);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(shader.begin(), shader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);

	m_pl->setCullMode(QRhiGraphicsPipeline::None);
	m_pl->setFrontFace(QRhiGraphicsPipeline::CCW);

	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void OverlayShaderResource::create(QRhi* rhi, rhi::Texture& tex)
{
	srb.reset(rhi->newShaderResourceBindings());
	srb->setBindings({
			QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage,
									   tex.texture.get(), tex.sampler.get())
		});
	srb->create();
}

void OverlayRenderPass::create(QRhiSwapChain* sc, rhi::SharedResources* sharedResources)
{
	m_sc = sc;

	QRhiRenderPassDescriptor* rp = sc->renderPassDescriptor();
	int sampleCount = sc->sampleCount();

	// create texture sampler (need it when creating shader resource bindings)
	m_overlayTex.sampler.reset(m_rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
		QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge));
	m_overlayTex.sampler->create();

	// create overlay texture
	m_overlayTex.image = QImage(sc->surfacePixelSize(), QImage::Format_RGBA8888);
	m_overlayTex.image.fill(Qt::transparent);
	m_overlayTex.texture.reset(m_rhi->newTexture(QRhiTexture::RGBA8, sc->surfacePixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	m_overlayTex.texture->create();

	// load shaders
	OverlayShader shader(m_rhi);

	// shader resources
	m_sr.reset(new OverlayShaderResource());
	m_sr->create(m_rhi, m_overlayTex);

	// create pipeline
	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(false);
	m_pl->setDepthWrite(false);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(shader.begin(), shader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);
	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->create();

	// for rendering into the overlay texture, we also need the following
	// create a depth-stencil render buffer that matches overlay size & sample count
	m_overlayDepth.reset(m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, sc->surfacePixelSize(), 1));
	m_overlayDepth->create();

	QRhiColorAttachment colorAttachment(m_overlayTex.texture.get());
	QRhiTextureRenderTargetDescription rtDesc(colorAttachment, m_overlayDepth.get());

	m_overlayRT.reset(m_rhi->newTextureRenderTarget(rtDesc, QRhiTextureRenderTarget::PreserveColorContents));
	m_overlayPD.reset(m_overlayRT->newCompatibleRenderPassDescriptor());
	m_overlayRT->setRenderPassDescriptor(m_overlayPD.get());
	m_overlayRT->create();

	// create the triad stuff
	m_triadPass.reset(new TriadRenderPass(m_rhi));
	m_triadPass->create(renderPassDescriptor(), 1, sharedResources);

	// create the triad pass
	GLMesh* gltriad = buildTriadMesh();
	triadMesh.reset(new rhi::TriMesh<TriadShader::Vertex>(m_rhi));
	triadMesh->CreateFromGLMesh(gltriad);
	triadMesh->setShaderResource(TriadShader::createShaderResource(m_rhi, sharedResources));
	triadMesh->getSubMesh(0)->SetActive(true);

	GLMaterial mat;
	mat.diffuseMap = GLMaterial::VERTEX_COLOR;
	triadMesh->setMaterial(mat);
	delete gltriad;
}

void OverlayRenderPass::setImage(const QImage& img)
{
	if (m_rhi->isYUpInNDC())
		m_overlayTex.image = img.mirrored();
	else
		m_overlayTex.image = img;
}

void OverlayRenderPass::update(QRhiResourceUpdateBatch* u)
{
	QSize pixelSize = m_sc->surfacePixelSize();

	if (!m_overlayTex.texture || (m_overlayTex.texture->pixelSize() != pixelSize))
	{
		if (!m_overlayTex.texture)
		{
			m_overlayTex.texture.reset(m_rhi->newTexture(QRhiTexture::RGBA8, pixelSize, 1,
				QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
			m_overlayDepth.reset(m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, pixelSize, 1));
		}
		else
		{
			m_overlayTex.texture->setPixelSize(pixelSize);
			m_overlayDepth.reset(m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, pixelSize, 1));
		}
		m_overlayTex.texture->create();
		m_overlayDepth->create();

		QRhiColorAttachment colorAttachment(m_overlayTex.texture.get());
		QRhiTextureRenderTargetDescription rtDesc(colorAttachment, m_overlayDepth.get());

		m_overlayRT.reset(m_rhi->newTextureRenderTarget(rtDesc, QRhiTextureRenderTarget::PreserveColorContents));
		m_overlayPD.reset(m_overlayRT->newCompatibleRenderPassDescriptor());
		m_overlayRT->setRenderPassDescriptor(m_overlayPD.get());
		m_overlayRT->create();
	}

	// upload before render pass, otherwise our rendering will be overwritten by the texture
	m_overlayTex.upload(u);

	// update triad stuff
	QMatrix4x4 proj = m_rhi->clipSpaceCorrMatrix();
	auto vp = m_overlayVP.viewport();
	double w = vp[2];
	double h = vp[3];
	double ar = (h == 0 ? 1 : w / h);
	float d = 1.2f;
	float dx = d * ar;
	float dy = d;

	if (m_rhi->isYUpInNDC() != m_rhi->isYUpInFramebuffer())
	{
		dy = -dy;
	}

	proj.ortho(-dx, dx, -dy, dy, -1, 1);

	triadMesh->setMatrices(m_overlayVM, proj);
	triadMesh->Update(u);
}

void OverlayRenderPass::draw(QRhiCommandBuffer* cb)
{
	if (m_overlayTex.image.isNull()) return;

	cb->setGraphicsPipeline(m_pl.get());
	cb->setShaderResources();
	cb->draw(3);
}

void OverlayRenderPass::drawTriad(QRhiCommandBuffer* cb)
{
	auto vp = m_overlayVP.viewport();

	if (m_rhi->isYUpInNDC() == m_rhi->isYUpInFramebuffer())
	{
		// flip the viewport
		double H = (double)m_sc->currentPixelSize().height();
		vp[1] = H - vp[3] - vp[1];
	}

	cb->setViewport({ vp[0], vp[1], vp[2], vp[3] });
	cb->setGraphicsPipeline(m_triadPass->pipeline());
	cb->setShaderResources();
	triadMesh->Draw(cb);
}
