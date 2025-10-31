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

void FrontFaceRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, rhi::SharedResources* sr)
{
	SolidShader shader(m_rhi);

	m_sr.reset(shader.createShaderResource(m_rhi, sr));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(true);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(shader.begin(), shader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);

	m_pl->setCullMode(QRhiGraphicsPipeline::Back);
	m_pl->setFrontFace(QRhiGraphicsPipeline::CCW);

	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void BackFaceRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, rhi::SharedResources* sr)
{
	SolidShader shader(m_rhi);

	m_sr.reset(shader.createShaderResource(m_rhi, sr));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(rp);
	m_pl->setSampleCount(sampleCount);

	m_pl->setDepthTest(true);
	m_pl->setDepthWrite(true);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(shader.begin(), shader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);

	m_pl->setCullMode(QRhiGraphicsPipeline::Front);
	m_pl->setFrontFace(QRhiGraphicsPipeline::CCW);

	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void SolidRenderPass::create(QRhiSwapChain* sc, rhi::SharedResources* sr)
{
	sharedResource = sr;

	m_frontPass.reset(new FrontFaceRenderPass(m_rhi));
	m_frontPass->create(sc->renderPassDescriptor(), sc->sampleCount(), sr);

	m_backPass.reset(new BackFaceRenderPass(m_rhi));
	m_backPass->create(sc->renderPassDescriptor(), sc->sampleCount(), sr);
}

rhi::Mesh* SolidRenderPass::addGLMesh(const GLMesh& mesh, bool cacheMesh)
{
	auto it = m_meshList.find(&mesh);
	if (it != m_meshList.end())
	{
		if (cacheMesh == false)
		{
			it->second->CreateFromGLMesh(&mesh);
		}
		return it->second;
	}
	else
	{
		rhi::MeshShaderResource* sr = SolidShader::createShaderResource(m_rhi, sharedResource);
		rhi::TriMesh<SolidShader::Vertex>* rm = new rhi::TriMesh<SolidShader::Vertex>(m_rhi, sr);
		rm->CreateFromGLMesh(&mesh);
		m_meshList[&mesh] = rm;
		return rm;
	}
}

void SolidRenderPass::reset()
{
	for (auto& it : m_meshList) it.second->setActive(false);
}

void SolidRenderPass::update(QRhiResourceUpdateBatch* u)
{
	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.second;
		if (m.isActive())
			m.Update(u, m_proj);
	}
}

void SolidRenderPass::draw(QRhiCommandBuffer* cb)
{
	cb->setGraphicsPipeline(m_backPass->pipeline());
	cb->setShaderResources();

	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.second;
		if (m.isActive())
			m.Draw(cb);
	}

	// render front faces next
	cb->setGraphicsPipeline(m_frontPass->pipeline());
	cb->setShaderResources();

	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.second;
		if (m.isActive())
			m.Draw(cb);
	}
}

void SolidRenderPass::clearCache()
{
	for (auto& it : m_meshList) delete it.second;
	m_meshList.clear();
}

void SolidRenderPass::clearUnusedCache()
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
}
