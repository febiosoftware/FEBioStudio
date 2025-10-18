/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include <GLLib/GLRenderEngine.h>
#include <rhi/qrhi.h>
#include "rhiMesh.h"

class rhiRenderer : public GLRenderEngine
{
public:
	rhiRenderer(QRhi* rhi, QRhiSwapChain* sc, QRhiRenderPassDescriptor* rp);
	~rhiRenderer();

	void init();

	void finish() override;

	void setViewProjection(const QMatrix4x4& proj);

	void clearCache();

public:

	void positionCamera(const GLCamera& cam) override;

	void setLightPosition(unsigned int n, const vec3f& lp) override;
	void setLightSpecularColor(unsigned int lightIndex, const GLColor& col);

	void setBackgroundColor(const GLColor& c) override;

	void setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly) override;

	void setMaterial(const GLMaterial& mat) override;

	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;

private:
	QRhiGraphicsPipeline* createPipeline(QVector<QRhiShaderStage>& shaders, QRhiGraphicsPipeline::CullMode cullMode);

private:
	QRhi* m_rhi;
	QRhiSwapChain* m_sc;
	QRhiRenderPassDescriptor* m_rp;

	QRhiVertexInputLayout m_inputLayout;

	std::unique_ptr<QRhiBuffer> globalBuf;
	std::unique_ptr<QRhiGraphicsPipeline> m_backRender;
	std::unique_ptr<QRhiGraphicsPipeline> m_frontRender;
	std::unique_ptr<rhi::ShaderResource> m_colorSrb;
	std::unique_ptr<QRhiSampler> m_sampler;
	std::unique_ptr<QRhiTexture> m_texture;

	rhi::SharedResources m_sharedResources;

	QRhiResourceUpdateBatch* m_initialUpdates = nullptr;

	QMatrix4x4 m_proj;
	QMatrix4x4 m_view;
	std::map<const GLMesh*, rhi::Mesh*> m_meshList;

private:
	vec3f m_light;
	GLColor m_lightSpecular;
	QColor m_bgColor;
	GLMaterial m_currentMat;
};
