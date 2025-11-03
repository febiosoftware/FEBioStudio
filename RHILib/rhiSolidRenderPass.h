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
#pragma once
#include "rhiMeshRenderPass.h"
#include "rhiMesh.h"

class FaceRenderPass : public rhi::RenderPass
{
public:
	FaceRenderPass(QRhi* rhi) : rhi::RenderPass(rhi) {}

	void create(QRhiRenderPassDescriptor* rp, int sampleCount, rhi::SharedResources* sr);

	QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

	void setCullMode(QRhiGraphicsPipeline::CullMode cm) { cullMode = cm; }

private:
	QRhiGraphicsPipeline::CullMode cullMode = QRhiGraphicsPipeline::None;

	std::unique_ptr<QRhiGraphicsPipeline> m_pl;
	std::unique_ptr<rhi::MeshShaderResource> m_sr;
};

class TwoPassSolidRenderPass : public rhi::MeshRenderPass
{
public:
	TwoPassSolidRenderPass(QRhi* rhi) : rhi::MeshRenderPass(rhi) {}

	void create(QRhiSwapChain* sc, rhi::SharedResources* sr);

	rhi::Mesh* addGLMesh(const GLMesh& mesh, int partition, bool cacheMesh) override;

	void update(QRhiResourceUpdateBatch* u) override;

	void draw(QRhiCommandBuffer* cb) override;

private:
	std::unique_ptr<FaceRenderPass> m_backPass;
	std::unique_ptr<FaceRenderPass> m_frontPass;
	rhi::SharedResources* sharedResource = nullptr;
};

class SolidRenderPass : public rhi::MeshRenderPass
{
public:
	SolidRenderPass(QRhi* rhi) : rhi::MeshRenderPass(rhi) {}

	void create(QRhiSwapChain* sc, rhi::SharedResources* sr);

	QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

	void update(QRhiResourceUpdateBatch* u) override;

	rhi::Mesh* addGLMesh(const GLMesh& mesh, int partition, bool cacheMesh);

	void draw(QRhiCommandBuffer* cb) override;

private:
	std::unique_ptr<QRhiGraphicsPipeline> m_pl;
	std::unique_ptr<rhi::MeshShaderResource> m_sr;
	rhi::SharedResources* sharedResource = nullptr;
};
