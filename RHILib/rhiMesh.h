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
#include <rhi/qrhi.h>
#include <GLLib/GLMesh.h>
#include <GLLib/GLMaterial.h>
#include "rhiTexture.h"
#include "rhiUtil.h"

namespace rhi {

	struct SharedResources
	{
		QRhiBuffer* globalbuf = nullptr;
		QRhiTexture* texture = nullptr;
		QRhiSampler* sampler = nullptr;
	};

	class Mesh;

	// base class for mesh shaders resources
	class MeshShaderResource
	{
	public:
		MeshShaderResource(QRhi* rhi) : m_rhi(rhi) {}
		virtual ~MeshShaderResource() {}

		virtual void setData(const QMatrix4x4& mvp, const QMatrix4x4& mv, const Mesh& m) {}

		QRhiShaderResourceBindings* get() { return srb.get(); }

		void update(QRhiResourceUpdateBatch* u);

	protected:
		QRhi* m_rhi = nullptr;
		UniformBlock	m_data;
		std::unique_ptr<QRhiBuffer> ubuf;
		std::unique_ptr<QRhiShaderResourceBindings> srb;
	};

	// base class for meshes.
	// Don't use this class directly. Instead, use one of the derived classes
	class Mesh
	{
	public:
		Mesh(QRhi* rhi, rhi::MeshShaderResource* shaderResource = nullptr) : m_rhi(rhi), sr(shaderResource) {}
		virtual ~Mesh();

		void setActive(bool b) { active = b; }
		bool isActive() const { return active; }

		void SetMaterial(const GLMaterial& m) { mat = m; }

		void SetModelMatrix(const QMatrix4x4& Q) { modelMatrix = Q; }

		virtual bool CreateFromGLMesh(const GLMesh* gmsh) { return false; }

		virtual void Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view);

		virtual void Draw(QRhiCommandBuffer* cb);

	protected:
		void create(unsigned int vertices, unsigned int sizeOfVertex, const void* data);

		int vertexCount() const { return (int)nvertexCount; }

	public:
		GLMaterial mat;
		bool doClipping = false;

	protected:
		QRhi* m_rhi = nullptr;

		QMatrix4x4 modelMatrix;
		bool active = false;

	private:
		std::unique_ptr<QRhiBuffer> vbuf; // vertex buffer
		std::unique_ptr<MeshShaderResource> sr; // shader resources

		unsigned int nvertexCount = 0;
		unsigned int vbufSize = 0;
		unsigned char* vertexData = nullptr;
	};
} // rhi
