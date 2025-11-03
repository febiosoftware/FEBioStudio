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
#include "rhiRenderPass.h"
#include "rhiTexture.h"
#include "rhiMesh.h"

class TriadRenderPass : public rhi::RenderPass
{
public:
	TriadRenderPass(QRhi* rhi) : rhi::RenderPass(rhi) {}

	void create(QRhiRenderPassDescriptor* rp, int sampleCount, rhi::SharedResources* sr);

	QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

private:
	std::unique_ptr<QRhiGraphicsPipeline> m_pl;
	std::unique_ptr<rhi::MeshShaderResource> m_sr;
};

struct OverlayShaderResource
{
	std::unique_ptr<QRhiShaderResourceBindings> srb;
	void create(QRhi* rhi, rhi::Texture& tex);
	QRhiShaderResourceBindings* get() { return srb.get(); }
};

class OverlayRenderPass : public rhi::RenderPass
{
public:
	OverlayRenderPass(QRhi* rhi) : rhi::RenderPass(rhi), m_overlayTex(rhi) {}
	void create(QRhiSwapChain* sc, rhi::SharedResources* sharedResources);
	QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

	void update(QRhiResourceUpdateBatch* u) override;

	void draw(QRhiCommandBuffer* cb) override;

	void setImage(const QImage& img);

	QRhiRenderPassDescriptor* renderPassDescriptor() { return m_overlayPD.get(); }

	QRhiRenderTarget* renderTarget() { return m_overlayRT.get(); }

	void drawTriad(QRhiCommandBuffer* cb);

public:
	QRhiViewport m_overlayVP = { 0,0,100,100 };
	QMatrix4x4 m_overlayVM;

private:
	std::unique_ptr<QRhiGraphicsPipeline> m_pl;
	std::unique_ptr<QRhiTextureRenderTarget> m_overlayRT;
	std::unique_ptr<QRhiRenderPassDescriptor> m_overlayPD;
	std::unique_ptr<QRhiRenderBuffer> m_overlayDepth;
	std::unique_ptr<OverlayShaderResource> m_sr;
	rhi::Texture m_overlayTex;
	QRhiSwapChain* m_sc = nullptr;

	// triad rendering
	std::unique_ptr<rhi::Mesh> triadMesh;
	std::unique_ptr<TriadRenderPass> m_triadPass;
};
