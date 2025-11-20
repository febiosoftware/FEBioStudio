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
#include <GLLib/GLTexture1D.h>

class CRGBAImage;

struct SolidResources {
	QRhiBuffer* globalBuf = nullptr;
	rhi::Texture* tex1D = nullptr;
	rhi::Texture* envTex = nullptr;
};

class FaceRenderPass : public rhi::RenderPass
{
public:
	FaceRenderPass(QRhi* rhi) : rhi::RenderPass(rhi) {}

	void create(QRhiRenderPassDescriptor* rp, int sampleCount, SolidResources sr);

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
	TwoPassSolidRenderPass(QRhi* rhi) : rhi::MeshRenderPass(rhi), m_tex1D(rhi), m_envTex(rhi) {}

	void create(QRhiSwapChain* sc, QRhiBuffer* globalBuf);

	void draw(QRhiCommandBuffer* cb) override;

	void update(QRhiResourceUpdateBatch* u) override;

	rhi::Mesh* newMesh(const GLMesh* mesh) override;

	rhi::MeshShaderResource* createShaderResource() override;

public:
	void setTexture1D(GLTexture1D& tex1D);

	unsigned int setEnvironmentMap(const CRGBAImage& img);

private:
	std::unique_ptr<FaceRenderPass> m_backPass;
	std::unique_ptr<FaceRenderPass> m_frontPass;

	QRhiBuffer* m_globalBuf = nullptr;

	rhi::Texture m_tex1D;

	rhi::Texture m_envTex;
	const CRGBAImage* envImg = nullptr;
};

class SolidRenderPass : public rhi::MeshRenderPass
{
public:
	SolidRenderPass(QRhi* rhi) : rhi::MeshRenderPass(rhi) {}

	void create(QRhiSwapChain* sc, QRhiBuffer* globalBuf);

	rhi::Mesh* newMesh(const GLMesh* mesh) override;

	rhi::MeshShaderResource* createShaderResource() override;

private:
	std::unique_ptr<rhi::MeshShaderResource> m_sr;
	QRhiBuffer* m_globalBuf = nullptr;
};
