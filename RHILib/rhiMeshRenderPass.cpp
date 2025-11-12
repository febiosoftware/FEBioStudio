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
#include "rhiMeshRenderPass.h"

void rhi::MeshRenderPass::reset()
{
	for (auto& it : m_meshList)
	{
		it.mesh->setActive(false);
		for (size_t i = 0; i < it.mesh->subMeshCount(); ++i)
		{
			rhi::SubMesh* sm = it.mesh->getSubMesh((int)i);
			if (sm) sm->SetActive(false);
		}
	}
}

void rhi::MeshRenderPass::clearCache()
{
	for (auto& it : m_meshList) delete it.mesh;
	m_meshList.clear();
}

void rhi::MeshRenderPass::clearUnusedCache()
{
	for (auto it = m_meshList.begin(); it != m_meshList.end(); ) {
		if ((it->mesh->isActive() == false) || (it->glmesh == nullptr))
		{
			delete it->mesh;
			it = m_meshList.erase(it);
		}
		else
			++it;
	}
}

void rhi::MeshRenderPass::removeCachedMesh(const GLMesh* mesh)
{
	for (auto it = m_meshList.begin(); it != m_meshList.end(); )
	{
		if (it->glmesh == mesh)
		{
			delete it->mesh;
			it = m_meshList.erase(it);
		}
		else
			++it;
	}
}

rhi::SubMesh* rhi::MeshRenderPass::addGLMesh(const GLMesh& mesh, int partition, bool cacheMesh)
{
	if (mesh.Nodes() == 0) return nullptr;

	// see if we have already cached this mesh
	auto it = m_meshList.end();
	if (cacheMesh)
	{
		it = m_meshList.find(&mesh);
	}

	// if we didn't find it, create a new mesh
	if (it == m_meshList.end())
	{
		// create a new mesh
		rhi::Mesh* rm = newMesh(&mesh); assert(rm);
		if (rm == nullptr) return nullptr;
		m_meshList.push_back((cacheMesh ? &mesh : nullptr), rm);
		it = m_meshList.back();
	}

	// since we need the mesh, mark it as active so we don't delete it during a clearUnusedCache call
	it->mesh->setActive(true);

	// get the partition's submesh
	rhi::SubMesh* subMesh = it->mesh->getSubMesh(partition + 1); assert(subMesh);
	if (subMesh)
	{
		// make sure the submesh has a shader resource
		if (subMesh->sr == nullptr)
		{
			subMesh->sr.reset(createShaderResource());
		}

		// mark the submesh as active
		subMesh->isActive = true;
	}

	return subMesh;
}

void rhi::MeshRenderPass::update(QRhiResourceUpdateBatch* u)
{
	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.mesh;
		if (m.isActive())
			m.Update(u);
	}
}

void rhi::MeshRenderPass::draw(QRhiCommandBuffer* cb)
{
	if (!m_meshList.empty())
	{
		cb->setGraphicsPipeline(m_pl.get());
		cb->setShaderResources();
		for (auto& it : m_meshList)
		{
			rhi::Mesh& m = *it.mesh;
			if (m.isActive())
				m.Draw(cb);
		}
	}
}
