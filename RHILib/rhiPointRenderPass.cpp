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
#include "rhiPointRenderPass.h"
#include "rhiUtil.h"
#include "rhiShader.h"

void PointRenderPass::create(QRhiSwapChain* sc, rhi::SharedResources* sr)
{
	sharedResources = sr;

	QRhiRenderPassDescriptor* rp = sc->renderPassDescriptor();
	int sampleCount = sc->sampleCount();

	PointShader pointShader(m_rhi);

	m_sr.reset(pointShader.createShaderResource(m_rhi, sr));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(false);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(pointShader.begin(), pointShader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Points);

	m_pl->setVertexInputLayout(pointShader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

rhi::Mesh* PointRenderPass::addGLMesh(const GLMesh& mesh, bool cacheMesh)
{
	auto it = m_pointMeshList.find(&mesh);
	if (it != m_pointMeshList.end())
	{
		if (cacheMesh == false)
		{
			it->second->CreateFromGLMesh(&mesh);
		}
		return it->second;
	}
	else
	{
		rhi::MeshShaderResource* sr = PointShader::createShaderResource(m_rhi, sharedResources);
		rhi::PointMesh<PointShader::Vertex>* rm = new rhi::PointMesh<PointShader::Vertex>(m_rhi, sr);
		rm->CreateFromGLMesh(&mesh);
		m_pointMeshList[&mesh] = rm;
		return rm;
	}
}

void PointRenderPass::reset()
{
	for (auto& it : m_pointMeshList) it.second->setActive(false);
}

void PointRenderPass::update(QRhiResourceUpdateBatch* u)
{
	for (auto& it : m_pointMeshList)
	{
		rhi::Mesh& m = *it.second;
		if (m.isActive())
			m.Update(u, m_proj, m_view);
	}
}

void PointRenderPass::draw(QRhiCommandBuffer* cb)
{
	if (!m_pointMeshList.empty())
	{
		cb->setGraphicsPipeline(m_pl.get());
		cb->setShaderResources();
		for (auto& it : m_pointMeshList)
		{
			rhi::Mesh& m = *it.second;
			if (m.isActive())
				m.Draw(cb);
		}
	}
}

void PointRenderPass::clearCache()
{
	for (auto& it : m_pointMeshList) delete it.second;
	m_pointMeshList.clear();
}

void PointRenderPass::clearUnusedCache()
{
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
