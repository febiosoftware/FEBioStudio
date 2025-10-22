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

	class UniformBlock
	{
	public:
		enum ItemType
		{
			FLOAT,
			VEC4,
			MAT4
		};

		struct Item
		{
			ItemType type = FLOAT;
			unsigned int size = 0;
			unsigned int offset = 0;
			const char* szname = nullptr;
		};

	public:
		UniformBlock() {}

		void create(std::vector<std::pair<ItemType, const char*> > items)
		{
			for (auto& it : items)
			{
				switch (it.first)
				{
				case FLOAT: addFloat(it.second); break;
				case VEC4 : addVec4 (it.second);  break;
				case MAT4 : addMat4 (it.second);  break;
				}
			}
			create();
		}

		void setFloat(unsigned int index, float v)
		{
			auto& it = m_items[index];
			assert(it.type == FLOAT);
			memcpy(&m_data[it.offset], &v, sizeof(float));
		}

		void setVec4(unsigned int index, const vec3f& v)
		{
			auto& it = m_items[index];
			assert(it.type == VEC4);
			float tmp[4] = { v.x, v.y, v.z, 1.0f };
			memcpy(&m_data[it.offset], &tmp, sizeof(float) * 4);
		}

		void setMat4(unsigned int index, const QMatrix4x4& m)
		{
			auto& it = m_items[index];
			assert(it.type == MAT4);
			memcpy(&m_data[it.offset], m.constData(), sizeof(float) * 16);
		}

		size_t size() const { return m_data.size(); }

		const void* data() const { return m_data.data(); }

	private:
		void addFloat(const char* name = nullptr) { m_items.push_back({ FLOAT, sizeof(float)     , 0, name }); }
		void addVec4 (const char* name = nullptr) { m_items.push_back({ VEC4 , sizeof(float) *  4, 0, name }); }
		void addMat4 (const char* name = nullptr) { m_items.push_back({ MAT4 , sizeof(float) * 16, 0, name }); }

		void create()
		{
			unsigned int size = 0;
			unsigned int offset = 0;
			for (auto& it : m_items)
			{
				it.offset = offset;
				offset += it.size;
				size += it.size;
			}

			m_data.resize(size);
		}

	private:
		std::vector<Item>	m_items;
		std::vector<unsigned char>	m_data;
	};

	class TriMesh;

	struct ColorShaderResource
	{
		std::unique_ptr<QRhiBuffer> ubuf;
		std::unique_ptr<QRhiShaderResourceBindings> srb;

		void create(QRhi* rhi, SharedResources* sharedResources);

		QRhiShaderResourceBindings* get() { return srb.get(); }

		void setData(const QMatrix4x4& mvp, const QMatrix4x4& mv, const TriMesh& m);

		void update(QRhiResourceUpdateBatch* u);

	private:
		UniformBlock	m_data;
	};

	struct LineShaderResource
	{
	public:
		enum { MVP, MV, COL };

	public:

		std::unique_ptr<QRhiBuffer> ubuf;
		std::unique_ptr<QRhiShaderResourceBindings> srb;

		void create(QRhi* rhi);

		QRhiShaderResourceBindings* get() { return srb.get(); }

		void setData(const QMatrix4x4& mvp, const QMatrix4x4& mv, const vec3f& col);

		void update(QRhiResourceUpdateBatch* u);

	private:
		UniformBlock m_data;
	};

	struct PointShaderResource
	{
	public:
		enum { MVP, MV, COL };

	public:
		std::unique_ptr<QRhiBuffer> ubuf;
		std::unique_ptr<QRhiShaderResourceBindings> srb;

		void create(QRhi* rhi);

		QRhiShaderResourceBindings* get() { return srb.get(); }

		void update(QRhiResourceUpdateBatch* u);

		void setData(const QMatrix4x4& mvp, const QMatrix4x4& mv, const vec3f& col);

	private:
		UniformBlock m_data;
	};

	class Mesh
	{
	public:
		Mesh(QRhi* rhi) : m_rhi(rhi) {}

		void setActive(bool b) { active = b; }
		bool isActive() const { return active; }

		void SetColor(const vec3f& c) { color = c; }

		void SetModelMatrix(const QMatrix4x4& Q) { modelMatrix = Q; }

	public:
		vec3f color = vec3f(0.7f, 0.7f, 0.7f);

	protected:
		QRhi* m_rhi = nullptr;

		QMatrix4x4 modelMatrix;
		bool active = false;
	};

	class TriMesh : public Mesh
	{
		struct Vertex {
			vec3f r; // coordinate
			vec3f n; // normal
			vec3f c; // color
			vec3f t; // texture coordinate
		};

	public:
		TriMesh(QRhi* rhi, rhi::ColorShaderResource* srb);

		bool CreateFromGLMesh(const GLMesh* gmsh);

		void SetVertexColor(const vec3f& c);

		void SetMaterial(GLMaterial mat);

		void Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view);

		void Draw(QRhiCommandBuffer* cb);

	public:
		float shininess = 0.8f;
		float reflectivity = 0.8f;
		float opacity = 1.0f;
		bool useTexture = false;

	private:
		std::vector<Vertex> vertexData;
		std::unique_ptr<QRhiBuffer> vbuf;
		unsigned int vertexCount = 0;

		std::unique_ptr<ColorShaderResource> sr;

	private:
		TriMesh(const TriMesh&) = delete;
		void operator = (const TriMesh&) = delete;
	};

	class LineMesh : public Mesh
	{
		struct Vertex {
			vec3f r; // coordinate
		};

	public:
		LineMesh(QRhi* rhi, rhi::LineShaderResource* srb);

		bool CreateFromGLMesh(const GLMesh* gmsh);

		void Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view);

		void Draw(QRhiCommandBuffer* cb);

	private:
		std::vector<Vertex> vertexData;
		std::unique_ptr<QRhiBuffer> vbuf;
		unsigned int vertexCount = 0;

		std::unique_ptr<LineShaderResource> sr;

	private:
		LineMesh(const LineMesh&) = delete;
		void operator = (const LineMesh&) = delete;
	};

	class PointMesh : public Mesh
	{
		struct Vertex {
			vec3f r; // coordinate
		};

	public:
		PointMesh(QRhi* rhi, rhi::PointShaderResource* srb);

		bool CreateFromGLMesh(const GLMesh* gmsh);

		void Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view);

		void Draw(QRhiCommandBuffer* cb);

	private:
		std::vector<Vertex> vertexData;
		std::unique_ptr<QRhiBuffer> vbuf;
		unsigned int vertexCount = 0;

		std::unique_ptr<PointShaderResource> sr;

	private:
		PointMesh(const PointMesh&) = delete;
		void operator = (const PointMesh&) = delete;
	};

} // rhi
