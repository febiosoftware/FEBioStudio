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
#include "rhiGradientRenderPass.h"

void GradientRenderPass::create(QRhiSwapChain* sc)
{
	m_sc = sc;
	GradientShader shader(m_rhi);

	// create shader resources
	m_sr.reset(new GradientShaderResource(m_rhi));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(sc->renderPassDescriptor());
	m_pl->setSampleCount(sc->sampleCount());

	m_pl->setDepthTest(false);
	m_pl->setDepthWrite(false);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(shader.begin(), shader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);
	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->create();
}

void GradientRenderPass::update(QRhiResourceUpdateBatch* u)
{
	m_sr->setData(m_col1, m_col2, m_orient);
	m_sr->update(u);
}

void GradientRenderPass::draw(QRhiCommandBuffer* cb)
{
	cb->setGraphicsPipeline(m_pl.get());
	cb->setShaderResources();
	cb->draw(3);
}

void GradientRenderPass::setColorGradient(GLColor c1, GLColor c2, int orientation)
{
	m_col1 = c1;
	m_col2 = c2;
	m_orient = orientation;
}
