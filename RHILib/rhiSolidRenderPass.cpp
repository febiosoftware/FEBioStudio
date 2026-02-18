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
#include "rhiSolidRenderPass.h"
#include "rhiUtil.h"
#include "rhiShader.h"
#include "rhiTriMesh.h"
#include <GLLib/GLTexture2D.h>

void FaceRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, SolidResources sr)
{
	SolidShader shader(m_rhi);

	m_sr.reset(shader.createShaderResource(m_rhi, sr.globalBuf, *sr.tex1D, *sr.envTex));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(true);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(shader.begin(), shader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);

	m_pl->setCullMode(cullMode);
	m_pl->setFrontFace(QRhiGraphicsPipeline::CCW);

	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void TwoPassSolidRenderPass::create(QRhiSwapChain* sc, QRhiBuffer* globalBuf, rhi::Texture* tex1D)
{
	m_globalBuf = globalBuf;
	m_tex1D = tex1D;

	SolidResources sr = { globalBuf, tex1D, &m_envTex };

	// create with dummy image
	QImage envImg(QSize(100, 100), QImage::Format_RGB32);
	envImg.fill(Qt::black);
	m_envTex.create(envImg);

	m_frontPass.reset(new FaceRenderPass(m_rhi));
	m_frontPass->setCullMode(QRhiGraphicsPipeline::Back);
	m_frontPass->create(sc->renderPassDescriptor(), sc->sampleCount(), sr);

	m_backPass.reset(new FaceRenderPass(m_rhi));
	m_backPass->setCullMode(QRhiGraphicsPipeline::Front);
	m_backPass->create(sc->renderPassDescriptor(), sc->sampleCount(), sr);
}

void TwoPassSolidRenderPass::update(QRhiResourceUpdateBatch* u)
{
	m_envTex.update(u);
	rhi::MeshRenderPass::update(u);
}

void TwoPassSolidRenderPass::draw(QRhiCommandBuffer* cb)
{
	if (m_meshList.empty()) return;

	cb->setGraphicsPipeline(m_backPass->pipeline());
	cb->setShaderResources();

	for (auto& it : m_meshList)
	{
		if (it.mesh->isActive())
			it.mesh->Draw(cb);
	}

	// render front faces next
	cb->setGraphicsPipeline(m_frontPass->pipeline());
	cb->setShaderResources();

	for (auto& it : m_meshList)
	{
		if (it.mesh->isActive())
			it.mesh->Draw(cb);
	}
}

rhi::Mesh* TwoPassSolidRenderPass::newMesh(const GLMesh* mesh)
{
	if (mesh == nullptr) return nullptr;
	rhi::Mesh* rm = new rhi::TriMesh<SolidShader::Vertex>(m_rhi);
	if (!rm->CreateFromGLMesh(mesh))
	{
		delete rm;
		rm = nullptr;
	}
	return rm;
}

rhi::MeshShaderResource* TwoPassSolidRenderPass::createShaderResource()
{
	return SolidShader::createShaderResource(m_rhi, m_globalBuf, *m_tex1D, m_envTex);
}

unsigned int TwoPassSolidRenderPass::setEnvironmentMap(const CRGBAImage& img)
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

void SolidRenderPass::create(QRhiSwapChain* sc, QRhiBuffer* globalBuf)
{
	m_globalBuf = globalBuf;

	DiffuseShader shader(m_rhi);

	QRhiRenderPassDescriptor* rp = sc->renderPassDescriptor();
	int sampleCount = sc->sampleCount();

	m_sr.reset(shader.createShaderResource(m_rhi, globalBuf));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(m_depthTest);
	m_pl->setDepthWrite(m_depthTest);
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

rhi::Mesh* SolidRenderPass::newMesh(const GLMesh* mesh)
{
	if (mesh == nullptr) return nullptr;
	rhi::Mesh* rm = new rhi::TriMesh<DiffuseShader::Vertex>(m_rhi);
	if (!rm->CreateFromGLMesh(mesh))
	{
		delete rm;
		rm = nullptr;
	}
	return rm;
}

rhi::MeshShaderResource* SolidRenderPass::createShaderResource()
{
	return DiffuseShader::createShaderResource(m_rhi, m_globalBuf);
}
