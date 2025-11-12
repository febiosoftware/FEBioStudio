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
#include "rhiVolumeRenderPass.h"
#include "rhiShader.h"
#include "rhiTriMesh.h"

void VolumeRenderPass::create(QRhiSwapChain* sc)
{
	settings.create({
		{ rhi::UniformBlock::VEC4 , "col1" },
		{ rhi::UniformBlock::VEC4 , "col2" },
		{ rhi::UniformBlock::VEC4 , "col3" },
		{ rhi::UniformBlock::FLOAT, "Imin" },
		{ rhi::UniformBlock::FLOAT, "Imax" },
		{ rhi::UniformBlock::FLOAT, "Iscl" },
		{ rhi::UniformBlock::FLOAT, "IsclMin" },
		{ rhi::UniformBlock::FLOAT, "Amin" },
		{ rhi::UniformBlock::FLOAT, "Amax" },
		{ rhi::UniformBlock::FLOAT, "gamma" },
		{ rhi::UniformBlock::INT  , "cmap" }
		});

	// create the buffer
	ubuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, settings.size()));
	ubuf->create();

	VolumeShader shader(m_rhi);

	QRhiRenderPassDescriptor* rp = sc->renderPassDescriptor();
	int sampleCount = sc->sampleCount();

	m_sr.reset(shader.createShaderResource(m_rhi, m_tex, ubuf.get()));

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

void VolumeRenderPass::draw(QRhiCommandBuffer* cb)
{
	if (m_meshList.empty()) return;
	if (!m_tex.texture) return;

	cb->setGraphicsPipeline(pipeline());
	cb->setShaderResources();

	rhi::MeshRenderPass::draw(cb);
}

rhi::Mesh* VolumeRenderPass::newMesh(const GLMesh* mesh)
{
	rhi::TriMesh<VolumeShader::Vertex>* rm = new rhi::TriMesh<VolumeShader::Vertex>(m_rhi);
	if (!rm->CreateFromGLMesh(mesh))
	{
		delete rm;
		rm = nullptr;
	}
	return rm;
}

rhi::MeshShaderResource* VolumeRenderPass::createShaderResource()
{
	return VolumeShader::createShaderResource(m_rhi, m_tex, ubuf.get());
}

void VolumeRenderPass::update(QRhiResourceUpdateBatch* u)
{
	u->updateDynamicBuffer(ubuf.get(), 0, settings.size(), settings.data());

	if (m_tex.needsUpload)
	{
		m_tex.upload(u);
		m_tex.needsUpload = false;
	}

	rhi::MeshRenderPass::update(u);
}

void VolumeRenderPass::setTexture3D(GLTexture3D& tex)
{
	settings.setVec4 ( 0, tex.col1);
	settings.setVec4 ( 1, tex.col2);
	settings.setVec4 ( 2, tex.col3);
	settings.setFloat( 3, tex.Imin);
	settings.setFloat( 4, tex.Imax);
	settings.setFloat( 5, tex.Iscale);
	settings.setFloat( 6, tex.IscaleMin);
	settings.setFloat( 7, tex.Amin);
	settings.setFloat( 8, tex.Amax);
	settings.setFloat( 9, tex.gamma);
	settings.setInt  (10, tex.cmap);

	if (tex.IsModified())
	{
		C3DImage* im = tex.Get3DImage();
		m_tex.create(*im);
		m_tex.needsUpload = true;

		// recreate shader resource for pipeline
		m_sr.reset(VolumeShader::createShaderResource(m_rhi, m_tex, ubuf.get()));
		m_pl->setShaderResourceBindings(m_sr->get());

		// delete all cached meshes since the shader resoures are no longer valid
		clearCache();

		tex.SetModified(false);
	}
}
