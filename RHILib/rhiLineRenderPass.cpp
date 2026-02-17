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
#include "rhiLineRenderPass.h"
#include "rhiUtil.h"
#include "rhiShader.h"
#include "rhiLineMesh.h"

void LineRenderPass::create(QRhiSwapChain* sc, QRhiBuffer* globalBuf)
{
	// prep 1D texture
	QImage img(QSize(1024, 1), QImage::Format_RGBA8888);
	img.fill(Qt::white);
	m_tex1D.create(img);

	m_globalBuf = globalBuf;

	QRhiRenderPassDescriptor* rp = sc->renderPassDescriptor();
	int sampleCount = sc->sampleCount();

	LineShader lineShader(m_rhi);

	m_sr.reset(lineShader.createShaderResource(m_rhi, m_globalBuf, m_tex1D));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(m_depthTest);
	m_pl->setDepthWrite(false);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(lineShader.begin(), lineShader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Lines);

	m_pl->setVertexInputLayout(lineShader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

rhi::Mesh* LineRenderPass::newMesh(const GLMesh* mesh)
{
	if (mesh == nullptr) return nullptr;
	rhi::Mesh* rm = new rhi::LineMesh<LineShader::Vertex>(m_rhi);
	if (!rm->CreateFromGLMesh(mesh))
	{
		delete rm;
		rm = nullptr;
	}
	return rm;
}

rhi::MeshShaderResource* LineRenderPass::createShaderResource()
{
	return LineShader::createShaderResource(m_rhi, m_globalBuf, m_tex1D);
}

void LineRenderPass::update(QRhiResourceUpdateBatch* u)
{
	if (m_tex1D.needsUpload)
	{
		m_tex1D.upload(u);
		m_tex1D.needsUpload = false;
	}

	rhi::MeshRenderPass::update(u);
}

void LineRenderPass::setTexture1D(GLTexture1D& tex)
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

