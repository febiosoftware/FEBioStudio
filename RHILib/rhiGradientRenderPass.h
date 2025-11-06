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
#include "rhiShader.h"

class GradientShaderResource : public rhi::MeshShaderResource
{
public:
	GradientShaderResource(QRhi* rhi) : MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::VEC4, "col1"},
			{rhi::UniformBlock::VEC4, "col2"},
			{rhi::UniformBlock::INT, "orient"},
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, ubuf.get()),
			});
		srb->create();
	}

	void setData(GLColor col1, GLColor col2, int orient)
	{
		m_data.setVec4(0, col1);
		m_data.setVec4(1, col2);
		m_data.setInt (2, orient);
	}
};

// This render pass is currently only used for rendering the fps indicator
class GradientRenderPass : public rhi::RenderPass
{
public:
	GradientRenderPass(QRhi* rhi) : rhi::RenderPass(rhi) {}

	void create(QRhiSwapChain* sc);

	void update(QRhiResourceUpdateBatch* u) override;

	void draw(QRhiCommandBuffer* cb) override;

	void setColorGradient(GLColor c1, GLColor c2, int orientation);

private:
	QRhiSwapChain* m_sc = nullptr;
	std::unique_ptr<QRhiGraphicsPipeline> m_pl;
	std::unique_ptr<GradientShaderResource> m_sr;

	GLColor m_col1;
	GLColor m_col2;
	int m_orient = 0;
};
