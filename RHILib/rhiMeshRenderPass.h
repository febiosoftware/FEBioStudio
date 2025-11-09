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
#include "rhiMesh.h"

namespace rhi {

	class MeshList
	{
		struct Entry
		{
			const GLMesh* glmesh = nullptr;
			rhi::Mesh* mesh = nullptr;
			int partition = -1;
		};

	public:
		using Container = std::list<Entry>;

	public:
		MeshList() {}

		size_t size() const { return meshList.size(); }

		Container::iterator begin() { return meshList.begin(); }
		Container::iterator end() { return meshList.end(); }

		Container::iterator find(const GLMesh* m, int partition)
		{
			return std::find_if(begin(), end(), [=](const auto& item) { return ((item.glmesh == m) && (item.partition == partition)); });
		}

		void push_back(const GLMesh* gm, rhi::Mesh* rm, int partition)
		{
			meshList.push_back({ gm, rm, partition });
		}

		bool empty() const { return meshList.empty(); }

		void clear() { meshList.clear(); }

		Container::iterator erase(Container::iterator it) { return meshList.erase(it); }

	private:
		Container meshList;
	};

	// specialized class for render passes that use meshes
	class MeshRenderPass : public RenderPass
	{
	public:
		MeshRenderPass(QRhi* rhi) : RenderPass(rhi) {}

		virtual rhi::Mesh* addGLMesh(const GLMesh& mesh, int partition, bool cacheMesh) = 0;

		void reset();

		void clearCache();

		void clearUnusedCache();

		void removeCachedMesh(const GLMesh* mesh);

		size_t cachedMeshes() const { return m_meshList.size(); }

	protected:
		rhi::MeshList m_meshList;
	};
}
