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

void LineRenderPass::create(QRhiSwapChain* sc, rhi::SharedResources* sr)
{
	m_sharedResource = sr;

	QRhiRenderPassDescriptor* rp = sc->renderPassDescriptor();
	int sampleCount = sc->sampleCount();

	LineShader lineShader(m_rhi);

	m_sr.reset(lineShader.createShaderResource(m_rhi, sr));

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

rhi::Mesh* LineRenderPass::addGLMesh(const GLMesh& mesh, bool cacheMesh)
{
	if (mesh.Edges() == 0) return nullptr;

	auto it = m_meshList.end();
	if (cacheMesh)
	{
		auto it = m_meshList.find(&mesh);
		if (it != m_meshList.end())
			return it->second;
	}

	rhi::MeshShaderResource* sr = LineShader::createShaderResource(m_rhi, m_sharedResource);
	rhi::LineMesh<LineShader::Vertex>* rm = new rhi::LineMesh<LineShader::Vertex>(m_rhi, sr);
	rm->CreateFromGLMesh(&mesh);

	if (cacheMesh)
	{
		m_meshList.push_back(&mesh, rm);
	}
	else
	{
		m_meshList.push_back(nullptr, rm);
	}
	return rm;
}

void LineRenderPass::update(QRhiResourceUpdateBatch* u)
{
	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.second;
		if (m.isActive())
			m.Update(u, m_proj);
	}
}

void LineRenderPass::draw(QRhiCommandBuffer* cb)
{
	if (!m_meshList.empty())
	{
		cb->setGraphicsPipeline(m_pl.get());
		cb->setShaderResources();
		for (auto& it : m_meshList)
		{
			rhi::Mesh& m = *it.second;
			if (m.isActive())
				m.Draw(cb);
		}
	}
}
