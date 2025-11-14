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
		QRhiTexture* tex1D = nullptr;
		QRhiSampler* sampler1D = nullptr;
		QRhiTexture* envTex = nullptr;
		QRhiSampler* envSmplr = nullptr;
	};

	class SubMesh;

	// base class for mesh shaders resources
	class MeshShaderResource
	{
	public:
		MeshShaderResource(QRhi* rhi) : m_rhi(rhi) {}
		virtual ~MeshShaderResource() {}

		virtual void setData(const SubMesh& m) {}

		QRhiShaderResourceBindings* get() { return srb.get(); }

		void update(QRhiResourceUpdateBatch* u);

	protected:
		QRhi* m_rhi = nullptr;
		UniformBlock	m_data;
		std::unique_ptr<QRhiBuffer> ubuf;
		std::unique_ptr<QRhiShaderResourceBindings> srb;
	};

	class Mesh;

	class SubMesh
	{
	public:
		SubMesh(Mesh* msh, unsigned int n0, unsigned int ncount, rhi::MeshShaderResource* shaderResource = nullptr) : sr(shaderResource)
		{
			mesh = msh;
			vertexStart = n0;
			vertexCount = ncount;
			isActive = false;
		}

		void SetMaterial(const GLMaterial& material) { mat = material; }

		void SetModelView(const QMatrix4x4& mv)
		{
			mvMatrix = mv;
		}

		void SetActive(bool b) { isActive = b; }

		void DoClipping(bool b) { doClipping = b; }

		void Update(QRhiResourceUpdateBatch* u)
		{
			if (sr) {
				sr->setData(*this);
				sr->update(u);
			}
		}

	public:
		rhi::Mesh* mesh = nullptr;
		bool isActive = false;
		unsigned int vertexStart = 0;
		unsigned int vertexCount = 0;

	public:
		GLMaterial mat;
		bool doClipping = false;
		QMatrix4x4 mvMatrix; // model view

	public:
		std::unique_ptr<MeshShaderResource> sr; // shader resources
	};

	// base class for meshes.
	// Don't use this class directly. Instead, use one of the derived classes
	class Mesh
	{
	public:
		Mesh(QRhi* rhi) : m_rhi(rhi) {}
		virtual ~Mesh();

		void setActive(bool b) { active = b; }
		bool isActive() const { return active; }

		virtual bool CreateFromGLMesh(const GLMesh* gmsh) { return false; }

		size_t subMeshCount() const { return submeshes.size(); }

		SubMesh* getSubMesh(int i)
		{
			if (i < 0 || i >= (int)submeshes.size()) return nullptr;
			return submeshes[i].get();
		}

		void Update(QRhiResourceUpdateBatch* u);

		void Draw(QRhiCommandBuffer* cb);

		void setShaderResource(rhi::MeshShaderResource* sr, int subMeshIndex = -1)
		{
			subMeshIndex += 1;
			if ((subMeshIndex >= 0) && (subMeshIndex <= (int)submeshes.size()))
			{
				submeshes[subMeshIndex]->sr.reset(sr);
			}
		}

		void setMaterial(const GLMaterial& mat, int subMeshIndex = -1)
		{
			subMeshIndex += 1;
			if ((subMeshIndex >= 0) && (subMeshIndex <= (int)submeshes.size()))
			{
				submeshes[subMeshIndex]->SetMaterial(mat);
			}
		}

		void setModelView(const QMatrix4x4& mv, int subMeshIndex = -1)
		{
			subMeshIndex += 1;
			if ((subMeshIndex >= 0) && (subMeshIndex <= (int)submeshes.size()))
			{
				if (subMeshIndex >= (int)submeshes.size()) return;
				submeshes[subMeshIndex]->SetModelView(mv);
			}
		}

	protected:
		void create(unsigned int vertices, unsigned int sizeOfVertex, const void* data);

	protected:
		QRhi* m_rhi = nullptr;
		bool active = false;
		std::vector<std::unique_ptr<SubMesh> > submeshes;

	private:
		std::unique_ptr<QRhiBuffer> vbuf; // vertex buffer

		size_t vbufSize = 0;
		unsigned char* vertexData = nullptr;

	public:
		static size_t uploadedBytes;
	};
} // rhi
