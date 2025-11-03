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

void FaceRenderPass::create(QRhiRenderPassDescriptor* rp, int sampleCount, rhi::SharedResources* sr)
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

	m_pl->setCullMode(cullMode);
	m_pl->setFrontFace(QRhiGraphicsPipeline::CCW);

	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	m_pl->create();
}

void TwoPassSolidRenderPass::create(QRhiSwapChain* sc, rhi::SharedResources* sr)
{
	sharedResource = sr;

	m_frontPass.reset(new FaceRenderPass(m_rhi));
	m_frontPass->setCullMode(QRhiGraphicsPipeline::Back);
	m_frontPass->create(sc->renderPassDescriptor(), sc->sampleCount(), sr);

	m_backPass.reset(new FaceRenderPass(m_rhi));
	m_backPass->setCullMode(QRhiGraphicsPipeline::Front);
	m_backPass->create(sc->renderPassDescriptor(), sc->sampleCount(), sr);
}

rhi::Mesh* TwoPassSolidRenderPass::addGLMesh(const GLMesh& mesh, bool cacheMesh)
{
	if (mesh.Faces() == 0) return nullptr;

	auto it = m_meshList.end();
	if (cacheMesh)
	{
		auto it = m_meshList.find(&mesh);
		if (it != m_meshList.end())
			return it->second;
	}

	rhi::MeshShaderResource* sr = SolidShader::createShaderResource(m_rhi, sharedResource);
	rhi::TriMesh<SolidShader::Vertex>* rm = new rhi::TriMesh<SolidShader::Vertex>(m_rhi, sr);
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

void TwoPassSolidRenderPass::update(QRhiResourceUpdateBatch* u)
{
	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.second;
		if (m.isActive())
			m.Update(u);
	}
}

void TwoPassSolidRenderPass::draw(QRhiCommandBuffer* cb)
{
	cb->setGraphicsPipeline(m_backPass->pipeline());
	cb->setShaderResources();

	for (auto& it : renderBatch)
	{
		it.mesh->Draw(cb, it.vertexOffset, it.vertexCount);
	}

	// render front faces next
	cb->setGraphicsPipeline(m_frontPass->pipeline());
	cb->setShaderResources();

	for (auto& it : renderBatch)
	{
		it.mesh->Draw(cb, it.vertexOffset, it.vertexCount);
	}
}

void SolidRenderPass::create(QRhiSwapChain* sc, rhi::SharedResources* sr)
{
	SolidShader shader(m_rhi);

	QRhiRenderPassDescriptor* rp = sc->renderPassDescriptor();
	int sampleCount = sc->sampleCount();

	sharedResource = sr;
	m_sr.reset(shader.createShaderResource(m_rhi, sr));

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

void SolidRenderPass::draw(QRhiCommandBuffer* cb)
{
	cb->setGraphicsPipeline(pipeline());
	cb->setShaderResources();

	for (auto& it : renderBatch)
	{
		it.mesh->Draw(cb, it.vertexOffset, it.vertexCount);
	}
}

rhi::Mesh* SolidRenderPass::addGLMesh(const GLMesh& mesh, bool cacheMesh)
{
	if (mesh.Faces() == 0) return nullptr;

	auto it = m_meshList.end();
	if (cacheMesh)
	{
		auto it = m_meshList.find(&mesh);
		if (it != m_meshList.end())
			return it->second;
	}

	rhi::MeshShaderResource* sr = SolidShader::createShaderResource(m_rhi, sharedResource);
	rhi::TriMesh<SolidShader::Vertex>* rm = new rhi::TriMesh<SolidShader::Vertex>(m_rhi, sr);
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

void SolidRenderPass::update(QRhiResourceUpdateBatch* u)
{
	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.second;
		if (m.isActive())
			m.Update(u);
	}
}
