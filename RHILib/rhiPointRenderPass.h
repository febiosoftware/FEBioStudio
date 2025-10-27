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
#include "rhiPointMesh.h"

class PointRenderPass : public rhi::RenderPass
{
public:
	PointRenderPass(QRhi* rhi) : rhi::RenderPass(rhi) {}

	void create(QRhiSwapChain* sc, rhi::SharedResources* sr);

	rhi::Mesh* addGLMesh(const GLMesh& mesh, bool cacheMesh);

	void reset();

	void update(QRhiResourceUpdateBatch* u) override;

	void draw(QRhiCommandBuffer* cb) override;

	void clearCache();

	void clearUnusedCache();

public:
	QMatrix4x4 m_proj;
	QMatrix4x4 m_view;

private:
	std::unique_ptr<QRhiGraphicsPipeline> m_pl;
	std::unique_ptr<rhi::MeshShaderResource> m_sr;
	rhi::SharedResources* sharedResources = nullptr;

	std::map<const GLMesh*, rhi::Mesh*> m_pointMeshList;
};
