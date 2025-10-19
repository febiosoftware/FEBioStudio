/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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
#include <rhi/qrhi.h>
#include <GLLib/GLMesh.h>
#include <GLLib/GLMaterial.h>

namespace rhi {

	struct SharedResources
	{
		QRhiBuffer* globalbuf = nullptr;
		QRhiTexture* texture = nullptr;
		QRhiSampler* sampler = nullptr;
	};

	struct ColorShaderResource
	{
		std::unique_ptr<QRhiBuffer> ubuf;
		std::unique_ptr<QRhiShaderResourceBindings> srb;

		void create(QRhi* rhi, SharedResources* sharedResources);

		QRhiShaderResourceBindings* get() { return srb.get(); }

		void update(QRhiResourceUpdateBatch* u, float* f);
	};

	struct LineShaderResource
	{
		std::unique_ptr<QRhiBuffer> ubuf;
		std::unique_ptr<QRhiShaderResourceBindings> srb;

		void create(QRhi* rhi);

		QRhiShaderResourceBindings* get() { return srb.get(); }

		void update(QRhiResourceUpdateBatch* u, float* f);
	};

	class Mesh
	{
		struct Vertex {
			vec3f r; // coordinate
			vec3f n; // normal
			vec3f c; // color
			vec3f t; // texture coordinate
		};

	public:
		Mesh(QRhi* rhi, rhi::ColorShaderResource* srb);

		bool CreateFromGLMesh(const GLMesh* gmsh);

		void SetVertexColor(const vec3f& c);

		void SetColor(const vec3f& c);
		void SetMaterial(GLMaterial mat);

		void Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view);

		void Draw(QRhiCommandBuffer* cb);

		void SetModelMatrix(const QMatrix4x4& Q) { modelMatrix = Q; }

		void setActive(bool b) { active = b; }
		bool isActive() const { return active; }

	private:
		std::vector<Vertex> vertexData;
		std::unique_ptr<QRhiBuffer> vbuf;
		unsigned int vertexCount = 0;

		std::unique_ptr<ColorShaderResource> sr;

		QRhi* m_rhi = nullptr;

		QMatrix4x4 modelMatrix;
		vec3f color = vec3f(0.7f, 0.7f, 0.7f);
		float shininess = 0.8f;
		float reflectivity = 0.8f;
		float opacity = 1.0f;
		bool useTexture = false;
		bool active = false;

	private:
		Mesh(const Mesh&) = delete;
		void operator = (const Mesh&) = delete;
	};

	class LineMesh
	{
		struct Vertex {
			vec3f r; // coordinate
		};

	public:
		LineMesh(QRhi* rhi, rhi::LineShaderResource* srb);

		bool CreateFromGLMesh(const GLMesh* gmsh);

		void SetColor(const vec3f& c);

		void Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view);

		void Draw(QRhiCommandBuffer* cb);

		void SetModelMatrix(const QMatrix4x4& Q) { modelMatrix = Q; }

		void setActive(bool b) { active = b; }
		bool isActive() const { return active; }

	private:
		std::vector<Vertex> vertexData;
		std::unique_ptr<QRhiBuffer> vbuf;
		unsigned int vertexCount = 0;

		std::unique_ptr<LineShaderResource> sr;

		QRhi* m_rhi = nullptr;

		bool active = false;
		QMatrix4x4 modelMatrix;
		vec3f color = vec3f(0.7f, 0.7f, 0.7f);

	private:
		LineMesh(const LineMesh&) = delete;
		void operator = (const LineMesh&) = delete;
	};

} // rhi
