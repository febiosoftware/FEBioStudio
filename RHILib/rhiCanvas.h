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
#include "rhiUtil.h"
#include "rhiMesh.h"

class CanvasUniformBlock
{
public:
	CanvasUniformBlock() {}
	void create(QRhi* rhi);

	QRhiBuffer* get() { return m_ubuf.get(); }

	void update(QRhiResourceUpdateBatch* u);

public:
	void setViewPort(const vec2f& vp);

private:
	rhi::UniformBlock m_ub;
	std::unique_ptr<QRhiBuffer> m_ubuf;
};

class FPSMesh : public rhi::Mesh
{
	struct Vertex {
		vec2f r; // coordinate
		vec2f t; // texture coordinate
	};

public:
	FPSMesh(QRhi* rhi);

	void create(QSize size);

private:
	FPSMesh(const FPSMesh&) = delete;
	void operator = (const FPSMesh&) = delete;
};

// This render pass is currently only used for rendering the fps indicator
class CanvasRenderPass : public rhi::RenderPass
{
public:
	CanvasRenderPass(QRhi* rhi) : rhi::RenderPass(rhi), m_fpsTex(rhi) {}

	void create(QRhiSwapChain* sc);

	void update(QRhiResourceUpdateBatch* u) override;

	void draw(QRhiCommandBuffer* cb) override;

	void setFPS(double fps, double fpsMin, double fpsMax) { m_fps = fps; m_fpsMin = fpsMin; m_fpsMax = fpsMax; }

private:
	QRhiSwapChain* m_sc = nullptr;
	std::unique_ptr<QRhiGraphicsPipeline> m_pl;
	std::unique_ptr<rhi::MeshShaderResource> m_sr;

	// fps indicator
	CanvasUniformBlock m_fpsub;
	std::unique_ptr<FPSMesh> m_fpsMesh;
	rhi::Texture m_fpsTex;
	double m_fps = 1;
	double m_fpsMin = 1;
	double m_fpsMax = 1;
};
