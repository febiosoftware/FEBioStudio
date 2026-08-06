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
	m_renderItems.clear();
	m_srSize = 0;

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
		if ((it->mesh->isActive() == false) || (it->gluid == 0))
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
	unsigned int uid = (mesh ? mesh->GetUID() : 0);
	for (auto it = m_meshList.begin(); it != m_meshList.end(); )
	{
		if (it->gluid == uid)
		{
			delete it->mesh;
			it = m_meshList.erase(it);
		}
		else
			++it;
	}
}

rhi::Mesh* rhi::MeshRenderPass::addGLMesh(const GLMesh& mesh, bool cacheMesh)
{
	if (mesh.Nodes() == 0) return nullptr;

	// see if we have already cached this mesh
	auto it = m_meshList.end();
	if (cacheMesh)
	{
		it = m_meshList.find(mesh.GetUID());
	}

	// if we didn't find it, create a new mesh
	if (it == m_meshList.end())
	{
		// create a new mesh
		rhi::Mesh* rm = newMesh(&mesh); assert(rm);
		if (rm == nullptr) return nullptr;
		m_meshList.push_back((cacheMesh ? mesh.GetUID() : 0), rm);
		it = m_meshList.back();
	}

	// since we need the mesh, mark it as active so we don't delete it during a clearUnusedCache call
	it->mesh->setActive(true);

	return it->mesh;
}

rhi::SubMesh* rhi::MeshRenderPass::getSubMesh(rhi::Mesh& mesh, int subMeshIndex)
{
	// get the partition's submesh
	rhi::SubMesh* subMesh = mesh.getSubMesh(subMeshIndex + 1); assert(subMesh);
	if (subMesh)
	{
		// mark the submesh as active
		subMesh->isActive = true;
	}

	return subMesh;
}

rhi::MeshRenderItem* rhi::MeshRenderPass::addRenderItem(rhi::SubMesh* subMesh, const GLMaterial& mat, bool doClipping, const QMatrix4x4& mvMatrix, bool invertFaces)
{
	MeshShaderResource* sr = nullptr;

	if (m_srSize == m_sr.size())
		m_sr.emplace_back(createShaderResource());

	sr = m_sr[m_srSize++].get();

	m_renderItems.push_back({ subMesh, mat, doClipping, mvMatrix, sr, invertFaces });
	return &m_renderItems.back();
}

void rhi::MeshRenderPass::update(QRhiResourceUpdateBatch* u)
{
	for (auto& it : m_meshList)
	{
		rhi::Mesh& m = *it.mesh;
		if (m.isActive())
			m.Update(u);
	}

	for (auto& it : m_renderItems)
	{
		if (it.subMesh && it.sr)
		{
			it.sr->setData(it);
			it.sr->update(u);
		}
	}
}

void rhi::MeshRenderPass::draw(QRhiCommandBuffer* cb)
{
	if (!m_renderItems.empty())
	{
		cb->setGraphicsPipeline(m_pl.get());
		cb->setShaderResources();

		drawMeshItems(cb);
	}
}

void rhi::MeshRenderPass::drawMeshItems(QRhiCommandBuffer* cb)
{
	Mesh* currentMesh = nullptr;
	for (auto& it : m_renderItems)
	{
		if (it.subMesh && it.sr)
		{
			rhi::SubMesh* sm = it.subMesh;
			if (currentMesh != it.subMesh->mesh)
			{
				currentMesh = it.subMesh->mesh;
				currentMesh->BindVertexBuffer(cb);
			}

			cb->setShaderResources(it.sr->get());
			cb->draw(sm->vertexCount, 1, sm->vertexStart);
		}
	}
}
