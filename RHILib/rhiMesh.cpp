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

void rhi::ColorShaderResource::create(QRhi* rhi, SharedResources* sr)
{
	m_data.create({
		{UniformBlock::MAT4, "mvp"},
		{UniformBlock::MAT4, "mv"},
		{UniformBlock::VEC4, "col"},
		{UniformBlock::FLOAT, "specExp"},
		{UniformBlock::FLOAT, "specStrength"},
		{UniformBlock::FLOAT, "opacity"},
		{UniformBlock::FLOAT, "useTexture"},
		{UniformBlock::FLOAT, "useStipple"}
	});

	// create the buffer
	ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
	ubuf->create();

	// create resource binding
	static const QRhiShaderResourceBinding::StageFlags visibility =
		QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

	srb.reset(rhi->newShaderResourceBindings());
	srb->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, visibility, sr->globalbuf),
			QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
			QRhiShaderResourceBinding::sampledTexture(2, visibility, sr->texture, sr->sampler)
		});
	srb->create();
}

void rhi::ColorShaderResource::setData(const QMatrix4x4& mvp, const QMatrix4x4& mv, const rhi::TriMesh& m)
{
	m_data.setMat4(0, mvp);
	m_data.setMat4(1, mv);
	m_data.setVec4(2, m.color);
	m_data.setFloat(3, m.shininess);
	m_data.setFloat(4, m.reflectivity);
	m_data.setFloat(5, m.opacity);
	m_data.setFloat(6, (m.useTexture ? 1.f : 0.f));
	m_data.setFloat(7, (m.useStipple ? 1.f : 0.f));
}

void rhi::ColorShaderResource::update(QRhiResourceUpdateBatch* u)
{
	u->updateDynamicBuffer(ubuf.get(), 0, m_data.size(), m_data.data());
}

void rhi::LineShaderResource::create(QRhi* rhi)
{
	m_data.create({
		{UniformBlock::MAT4, "mvp"},
		{UniformBlock::MAT4, "mv"},
		{UniformBlock::VEC4, "col"}
	});

	// create the buffer
	ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
	ubuf->create();

	// create resource binding
	static const QRhiShaderResourceBinding::StageFlags visibility =
		QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

	srb.reset(rhi->newShaderResourceBindings());
	srb->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, visibility, ubuf.get())
		});
	srb->create();
}

void rhi::LineShaderResource::setData(const QMatrix4x4& mvp, const QMatrix4x4& mv, const vec3f& col)
{
	m_data.setMat4(MVP, mvp);
	m_data.setMat4(MV , mv);
	m_data.setVec4(COL, col);
}

void rhi::LineShaderResource::update(QRhiResourceUpdateBatch* u)
{
	u->updateDynamicBuffer(ubuf.get(), 0, m_data.size(), m_data.data());
}

void rhi::PointShaderResource::create(QRhi* rhi)
{
	m_data.create({
		{UniformBlock::MAT4, "mvp"},
		{UniformBlock::MAT4, "mv"},
		{UniformBlock::VEC4, "col"}
		});

	// create the buffer
	ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
	ubuf->create();

	// create resource binding
	static const QRhiShaderResourceBinding::StageFlags visibility =
		QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

	srb.reset(rhi->newShaderResourceBindings());
	srb->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, visibility, ubuf.get())
		});
	srb->create();
}

void rhi::PointShaderResource::setData(const QMatrix4x4& mvp, const QMatrix4x4& mv, const vec3f& col)
{
	m_data.setMat4(MVP, mvp);
	m_data.setMat4(MV , mv);
	m_data.setVec4(COL, col);
}

void rhi::PointShaderResource::update(QRhiResourceUpdateBatch* u)
{
	u->updateDynamicBuffer(ubuf.get(), 0, m_data.size(), m_data.data());
}

rhi::TriMesh::TriMesh(QRhi* rhi, rhi::ColorShaderResource* srb) : rhi::Mesh(rhi)
{
	sr.reset(srb);
}

bool rhi::TriMesh::CreateFromGLMesh(const GLMesh* gmsh)
{
	if (gmsh == nullptr) return false;

	int NF = gmsh->Faces();
	vertexCount = 3 * NF;
	vertexData.resize(vertexCount);
	Vertex* v = vertexData.data();
	BOX box = gmsh->GetBoundingBox();

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

			v->t = f.t[j];
		}
	}

	// create the vertex buffer
	vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(Vertex)*vertexCount));
	vbuf->create();

	return true;
}

void rhi::TriMesh::SetVertexColor(const vec3f& c)
{
	if (!vertexData.empty())
	{
		Vertex* v = vertexData.data();
		for (unsigned int i = 0; i < vertexCount; ++i, v++)
		{
			v->c = c;
		}
	}
}

void rhi::TriMesh::SetMaterial(GLMaterial mat)
{
	float f[4] = { 0.f };
	mat.diffuse.toFloat(f);
	color = vec3f(f[0], f[1], f[2]);
	shininess = mat.shininess;
	reflectivity = mat.reflectivity;
	opacity = mat.opacity;
	useTexture = (mat.diffuseMap == GLMaterial::TEXTURE_1D);
	useStipple = (mat.type == GLMaterial::HIGHLIGHT);
}

void rhi::TriMesh::Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view)
{
	if (!vertexData.empty())
	{
		u->uploadStaticBuffer(vbuf.get(), vertexData.data());
		vertexData.clear();
	}

	QMatrix4x4 mv = view * modelMatrix;
	QMatrix4x4 mvp = proj*mv;

	sr->setData(mvp, mv, *this);
	sr->update(u);
}

void rhi::TriMesh::Draw(QRhiCommandBuffer* cb)
{
	if (vertexCount > 0)
	{
		cb->setShaderResources(sr->get());
		const QRhiCommandBuffer::VertexInput vbufBinding(vbuf.get(), 0);
		cb->setVertexInput(0, 1, &vbufBinding);
		cb->draw(vertexCount);
	}
}

rhi::LineMesh::LineMesh(QRhi* rhi, rhi::LineShaderResource* srb) : rhi::Mesh(rhi)
{
	sr.reset(srb);
}

bool rhi::LineMesh::CreateFromGLMesh(const GLMesh* gmsh)
{
	if (gmsh == nullptr) return false;

	int NE = gmsh->Edges();
	vertexCount = 2 * NE;
	vertexData.resize(vertexCount);
	Vertex* v = vertexData.data();
	BOX box = gmsh->GetBoundingBox();

	for (int i = 0; i < NE; ++i)
	{
		const GLMesh::EDGE& e = gmsh->Edge(i);
		for (int j = 0; j < 2; ++j, ++v)
		{
			v->r = gmsh->Node(e.n[j]).r;// e.vr[j];

			float c[4] = { 0.f };
			e.c[j].toFloat(c);
		}
	}

	// create the vertex buffer
	vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(Vertex) * vertexCount));
	vbuf->create();

	return true;
}

void rhi::LineMesh::Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view)
{
	if (!vertexData.empty())
	{
		u->uploadStaticBuffer(vbuf.get(), vertexData.data());
		vertexData.clear();
	}

	QMatrix4x4 mv = view * modelMatrix;
	QMatrix4x4 mvp = proj * mv;

	sr->setData(mvp, mv, color);
	sr->update(u);
}

void rhi::LineMesh::Draw(QRhiCommandBuffer* cb)
{
	if (vertexCount > 0)
	{
		cb->setShaderResources(sr->get());
		const QRhiCommandBuffer::VertexInput vbufBinding(vbuf.get(), 0);
		cb->setVertexInput(0, 1, &vbufBinding);
		cb->draw(vertexCount);
	}
}

rhi::PointMesh::PointMesh(QRhi* rhi, rhi::PointShaderResource* srb) : rhi::Mesh(rhi)
{
	sr.reset(srb);
}

bool rhi::PointMesh::CreateFromGLMesh(const GLMesh* gmsh)
{
	if (gmsh == nullptr) return false;

	int NN = gmsh->Nodes();
	vertexCount = NN;
	vertexData.resize(vertexCount);
	Vertex* v = vertexData.data();
	BOX box = gmsh->GetBoundingBox();

	for (int i = 0; i < NN; ++i, v++)
	{
		const GLMesh::NODE& nd = gmsh->Node(i);
		v->r = nd.r;
	}

	// create the vertex buffer
	vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(Vertex) * vertexCount));
	vbuf->create();

	return true;
}

void rhi::PointMesh::Update(QRhiResourceUpdateBatch* u, const QMatrix4x4& proj, const QMatrix4x4& view)
{
	if (!vertexData.empty())
	{
		u->uploadStaticBuffer(vbuf.get(), vertexData.data());
		vertexData.clear();
	}

	QMatrix4x4 mv = view * modelMatrix;
	QMatrix4x4 mvp = proj * mv;

	sr->setData(mvp, mv, color);
	sr->update(u);
}

void rhi::PointMesh::Draw(QRhiCommandBuffer* cb)
{
	if (vertexCount > 0)
	{
		cb->setShaderResources(sr->get());
		const QRhiCommandBuffer::VertexInput vbufBinding(vbuf.get(), 0);
		cb->setVertexInput(0, 1, &vbufBinding);
		cb->draw(vertexCount);
	}
}
