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
#include <FSCore/math3d.h>
#include <FSCore/color.h>

namespace rhi {
	QShader getShader(const QString& name);

	QRhiGraphicsPipeline::TargetBlend defaultBlendState();

	class UniformBlock
	{
	public:
		enum ItemType
		{
			INT,
			FLOAT,
			VEC2,
			VEC4,
			MAT4,
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
				case INT  : addInt  (it.second); break;
				case FLOAT: addFloat(it.second); break;
				case VEC2 : addVec2(it.second);  break;
				case VEC4 : addVec4(it.second);  break;
				case MAT4 : addMat4(it.second);  break;
				}
			}
			create();
		}

		void setInt(unsigned int index, int v)
		{
			auto& it = m_items[index];
			assert(it.type == INT);
			memcpy(&m_data[it.offset], &v, sizeof(int));
		}

		void setFloat(unsigned int index, float v)
		{
			auto& it = m_items[index];
			assert(it.type == FLOAT);
			memcpy(&m_data[it.offset], &v, sizeof(float));
		}

		void setVec2(unsigned int index, const vec2f& v)
		{
			auto& it = m_items[index];
			assert(it.type == VEC2);
			float tmp[2] = { v.x, v.y };
			memcpy(&m_data[it.offset], &tmp, sizeof(float) * 2);
		}

		void setVec4(unsigned int index, const vec3f& v)
		{
			auto& it = m_items[index];
			assert(it.type == VEC4);
			float tmp[4] = { v.x, v.y, v.z, 1.0f };
			memcpy(&m_data[it.offset], &tmp, sizeof(float) * 4);
		}

		void setVec4(unsigned int index, float x, float y, float z, float w)
		{
			auto& it = m_items[index];
			assert(it.type == VEC4);
			float tmp[4] = { x, y, z, w };
			memcpy(&m_data[it.offset], tmp, sizeof(float) * 4);
		}

		void setVec4(unsigned int index, float v[4])
		{
			auto& it = m_items[index];
			assert(it.type == VEC4);
			memcpy(&m_data[it.offset], v, sizeof(float) * 4);
		}

		void setVec4(unsigned int index, const GLColor& c)
		{
			auto& it = m_items[index];
			assert(it.type == VEC4);
			float f[4] = { 0.f };
			c.toFloat(f);
			memcpy(&m_data[it.offset], f, sizeof(float) * 4);
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
		void addInt  (const char* name = nullptr) { m_items.push_back({ INT  , sizeof(float)     , 0, name }); }
		void addFloat(const char* name = nullptr) { m_items.push_back({ FLOAT, sizeof(float)     , 0, name }); }
		void addVec2 (const char* name = nullptr) { m_items.push_back({ VEC2 , sizeof(float) *  2, 0, name }); }
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
}
