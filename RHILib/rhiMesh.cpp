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
#include "rhiMesh.h"

void rhi::ShaderResource::create(QRhi* rhi, QRhiBuffer* sharedBuffer)
{
	// create the buffer
	ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, 128));
	ubuf->create();

	// create resource binding
	static const QRhiShaderResourceBinding::StageFlags visibility =
		QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

	srb.reset(rhi->newShaderResourceBindings());
	srb->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, visibility, sharedBuffer),
			QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get())
		});
	srb->create();
}

void rhi::ShaderResource::update(QRhiResourceUpdateBatch* u, const QMatrix4x4& mvp, const QMatrix4x4& mv)
{
	u->updateDynamicBuffer(ubuf.get(),  0, 64, mvp.constData());
	u->updateDynamicBuffer(ubuf.get(), 64, 64, mv.constData());
}

rhi::Mesh::Mesh(QRhi* rhi, rhi::ShaderResource* srb) : m_rhi(rhi)
{
	sr.reset(srb);
}

bool rhi::Mesh::CreateFromGLMesh(const GLMesh* gmsh)
{
	if (gmsh == nullptr) return false;

	int NF = gmsh->Faces();
	vertexCount = 3 * NF;
	vertexData.resize(vertexCount);
	Vertex* v = vertexData.data();
	for (int i = 0; i < NF; ++i)
	{
		const GLMesh::FACE& f = gmsh->Face(i);
		for (int j = 0; j < 3; ++j, ++v)
		{
			v->r = f.vr[j];
			v->n = f.vn[j];

			float c[4] = { 0.f };
			f.c[j].toFloat(c);
			v->c = vec3f(c[0], c[1], c[2]);
		}
	}

	// create the vertex buffer
	vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(Vertex)*vertexCount));
	vbuf->create();

	return true;
}

void rhi::Mesh::SetVertexColor(const vec3f& c)
{
	if (!vertexData.empty())
	{
		Vertex* v = vertexData.data();
		for (int i = 0; i < vertexCount; ++i, v++)
		{
			v->c = c;
		}
	}
}

void rhi::Mesh::Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view)
{
	if (!vertexData.empty())
	{
		u->uploadStaticBuffer(vbuf.get(), vertexData.data());
		vertexData.clear();
	}

	QMatrix4x4 mv = view * modelMatrix;
	QMatrix4x4 mvp = proj*mv;

	sr->update(u, mvp, mv);
}

void rhi::Mesh::Draw(QRhiCommandBuffer* cb)
{
	if (vertexCount > 0)
	{
		cb->setShaderResources(sr->get());
		const QRhiCommandBuffer::VertexInput vbufBinding(vbuf.get(), 0);
		cb->setVertexInput(0, 1, &vbufBinding);
		cb->draw(vertexCount);
	}
}
