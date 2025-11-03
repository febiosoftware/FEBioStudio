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
#include "rhiMesh.h"

void rhi::MeshShaderResource::update(QRhiResourceUpdateBatch* u)
{
	if (ubuf)
		u->updateDynamicBuffer(ubuf.get(), 0, m_data.size(), m_data.data());
}

rhi::Mesh::~Mesh()
{
	if (vertexData)
	{
		delete[] vertexData;
		vertexData = nullptr;
	}
}

void rhi::Mesh::create(unsigned int vertices, unsigned int sizeOfVertex, const void* data)
{
	nvertexCount = vertices;
	vbufSize = vertices * sizeOfVertex;
	vertexData = new unsigned char[vbufSize];
	memcpy(vertexData, data, vbufSize);
	vbuf.reset(m_rhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::VertexBuffer, vbufSize));
	vbuf->create();
}

void rhi::Mesh::Update(QRhiResourceUpdateBatch* u)
{
	if (vertexData)
	{
		u->uploadStaticBuffer(vbuf.get(), vertexData);
		delete[] vertexData;
		vertexData = nullptr;
	}

	if (sr)
	{
		QMatrix4x4 mvp = prMatrix * mvMatrix;
		sr->setData(mvp, mvMatrix, *this);
		sr->update(u);
	}
}

void rhi::Mesh::Draw(QRhiCommandBuffer* cb, int startIndex, int vertices)
{
	if (vertices < 0) vertices = nvertexCount;
	if (startIndex + vertices > nvertexCount) vertices = nvertexCount - startIndex;
	if (vertices > 0)
	{
		if (sr) cb->setShaderResources(sr->get());
		const QRhiCommandBuffer::VertexInput vbufBinding(vbuf.get(), 0);
		cb->setVertexInput(0, 1, &vbufBinding);
		cb->draw(vertices, 1, startIndex);
	}
}
