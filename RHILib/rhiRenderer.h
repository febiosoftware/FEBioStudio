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

namespace rhi {
	struct Texture
	{
		std::unique_ptr<QRhiTexture> texture;
		std::unique_ptr<QRhiSampler> sampler;
		QImage image;
		bool needsUpload = false;

		void upload(QRhiResourceUpdateBatch* u)
		{
			u->uploadTexture(texture.get(), image);
		}
	};

	class RenderPass
	{
	public:
		RenderPass(QRhi* rhi) : m_rhi(rhi) {}

	protected:
		QRhi* m_rhi;
	};

	class PointRenderPass : public RenderPass
	{
	public:
		PointRenderPass(QRhi* rhi) : RenderPass(rhi) {}

		void create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr);

		QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

	private:
		std::unique_ptr<QRhiGraphicsPipeline> m_pl;
		std::unique_ptr<rhi::PointShaderResource> m_sr;
	};

	class LineRenderPass : public RenderPass
	{
	public:
		LineRenderPass(QRhi* rhi) : RenderPass(rhi) {}

		void create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr);

		QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

	private:
		std::unique_ptr<QRhiGraphicsPipeline> m_pl;
		std::unique_ptr<rhi::LineShaderResource> m_sr;
	};

	class FrontFaceRenderPass : public RenderPass
	{
	public:
		FrontFaceRenderPass(QRhi* rhi) : RenderPass(rhi) {}

		void create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr);

		QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

	private:
		std::unique_ptr<QRhiGraphicsPipeline> m_pl;
		std::unique_ptr<rhi::ColorShaderResource> m_sr;
	};

	class BackFaceRenderPass : public RenderPass
	{
	public:
		BackFaceRenderPass(QRhi* rhi) : RenderPass(rhi) {}

		void create(QRhiRenderPassDescriptor* rp, int sampleCount, SharedResources* sr);

		QRhiGraphicsPipeline* pipeline() { return m_pl.get(); }

	private:
		std::unique_ptr<QRhiGraphicsPipeline> m_pl;
		std::unique_ptr<rhi::ColorShaderResource> m_sr;
	};
}

class GlobalUniformBlock
{
public:
	GlobalUniformBlock() {}
	void create(QRhi* rhi);

	QRhiBuffer* get() { return m_ubuf.get(); }

	void update(QRhiResourceUpdateBatch* u);

public:
	void setLightPosition(const vec3f& lp);
	void setSpecularColor(GLColor c);
	void setClipPlane(const float f[4]);

private:
	rhi::UniformBlock m_ub;
	std::unique_ptr<QRhiBuffer> m_ubuf;
};

class rhiRenderer : public GLRenderEngine
{
public:
	rhiRenderer(QRhi* rhi, QRhiSwapChain* sc, QRhiRenderPassDescriptor* rp);
	~rhiRenderer();

	void init();

	void start() override;
	void finish() override;

	void setViewProjection(const QMatrix4x4& proj);

	// clear all cached resources
	void clearCache();

	// clear cached resources that were not used in the last frame
	void clearUnusedCache();

public:

	void positionCamera(const GLCamera& cam) override;

	void pushTransform() override;
	void popTransform() override;
	void translate(const vec3d& r) override;
	void rotate(const quatd& rot) override;
	void rotate(double deg, double x, double y, double z) override;
	void scale(double x, double, double z) override;

	void setLightPosition(unsigned int n, const vec3f& lp) override;
	void setLightSpecularColor(unsigned int lightIndex, const GLColor& col);

	void setBackgroundColor(const GLColor& c) override;

	void setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly) override;

	void setColor(GLColor c) override;
	void setMaterial(const GLMaterial& mat) override;

	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;

	void renderGMeshEdges(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMeshNodes(const GLMesh& mesh, bool cacheMesh = true) override;

	void setTexture(GLTexture1D& tex);

	void setClipPlane(unsigned int n, const double* v) override;
	void enableClipPlane(unsigned int n) override;
	void disableClipPlane(unsigned int n) override;

private:
	QRhi* m_rhi;
	QRhiSwapChain* m_sc;
	QRhiRenderPassDescriptor* m_rp;

	GlobalUniformBlock m_global;
	std::unique_ptr<rhi::BackFaceRenderPass> m_backPass;
	std::unique_ptr<rhi::FrontFaceRenderPass> m_frontPass;
	std::unique_ptr<rhi::LineRenderPass> m_linePass;
	std::unique_ptr<rhi::PointRenderPass> m_pointPass;

	rhi::Texture m_texture;

	rhi::SharedResources m_sharedResources;

	QRhiResourceUpdateBatch* m_initialUpdates = nullptr;

	QMatrix4x4 m_projMatrix;
	QMatrix4x4 m_viewMatrix;
	QMatrix4x4 m_modelMatrix;
	std::stack<QMatrix4x4> m_transformStack;
	std::map<const GLMesh*, rhi::TriMesh*> m_meshList;
	std::map<const GLMesh*, rhi::LineMesh*> m_lineMeshList;
	std::map<const GLMesh*, rhi::PointMesh*> m_pointMeshList;

	bool m_clipEnabled = false;
	float clipPlane[4];

private:
	vec3f m_light;
	GLColor m_lightSpecular;
	QColor m_bgColor;
	GLMaterial m_currentMat;
};
