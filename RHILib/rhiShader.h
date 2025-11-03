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
#include "rhiMesh.h"

namespace rhi {

	// Helper class for managing all shader related resources
	class Shader
	{
	public:
		Shader(QRhi* rhi) : m_rhi(rhi) {}
		virtual ~Shader() {}

		// return shader stages
		QVector<QRhiShaderStage>::Iterator begin() { return shaders.begin(); }
		QVector<QRhiShaderStage>::Iterator end() { return shaders.end(); }

		// return vertex input layout for this shader
		virtual QRhiVertexInputLayout meshLayout() = 0;

	protected:
		void create(const QString& vertexShader, const QString& fragmentShader);

	protected:
		QRhi* m_rhi = nullptr;

	private:
		QVector<QRhiShaderStage> shaders;
	};
}

class PointShader : public rhi::Shader
{
public:
	struct Vertex {
		vec3f r; // coordinate
		vec3f c; // color
		void operator = (const GLMesh::NODE& nd) 
		{ 
			r = nd.r; 
			float f[4] = { 0 };
			nd.c.toFloat(f);
			c = vec3f(f[0], f[1], f[2]);
		}
	};

public:
	PointShader(QRhi* rhi);

	QRhiVertexInputLayout meshLayout() override;

	static rhi::MeshShaderResource* createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource);
};

class LineShader : public rhi::Shader
{
public:
	struct Vertex {
		vec3f r; // position
		float c[4] = { 0.f }; // color

		void operator = (const GLMesh::NODE& nd)
		{ 
			r = nd.r; 
			nd.c.toFloat(c);
		}
	};

public:
	LineShader(QRhi* rhi);

	QRhiVertexInputLayout meshLayout() override;

	static rhi::MeshShaderResource* createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource);
};

class SolidShader : public rhi::Shader
{
public:
	struct Vertex {
		vec3f r; // coordinate
		vec3f n; // normal
		vec3f t; // texture coordinate
		float c[4] = { 0.f }; // color

		void operator = (const GLMesh::NODE& nd)
		{
			r = nd.r;
			n = nd.n;
			t = nd.t;
			nd.c.toFloat(c);
		}
	};

public:
	SolidShader(QRhi* rhi);

	QRhiVertexInputLayout meshLayout() override;

	static rhi::MeshShaderResource* createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource);
};

class CanvasShader : public rhi::Shader 
{
public:
	CanvasShader(QRhi* rhi);

	QRhiVertexInputLayout meshLayout() override;

	static rhi::MeshShaderResource* createShaderResource(QRhi* rhi, rhi::Texture& tex, QRhiBuffer* ub);
};

class OverlayShader : public rhi::Shader
{
public:
	OverlayShader(QRhi* rhi);

	QRhiVertexInputLayout meshLayout() override;
};

class TriadShader : public rhi::Shader
{
public:
	struct Vertex {
		vec3f r; // coordinate
		vec3f n; // normal
		vec3f c; // color

		void operator = (const GLMesh::NODE& nd)
		{
			r = nd.r;
			n = nd.n;

			float col[4] = { 0.f };
			nd.c.toFloat(col);
			c = vec3f(col[0], col[1], col[2]);
		}
	};

public:
	TriadShader(QRhi* rhi);

	QRhiVertexInputLayout meshLayout() override;

	static rhi::MeshShaderResource* createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource);
};

class VolumeShader : public rhi::Shader
{
public:
	struct Vertex {
		vec3f r; // coordinate
		vec3f t; // texture coordinate

		void operator = (const GLMesh::NODE& nd)
		{
			r = nd.r;
			t = nd.t;
		}
	};

public:
	VolumeShader(QRhi* rhi);

	QRhiVertexInputLayout meshLayout() override;

	static rhi::MeshShaderResource* createShaderResource(QRhi* rhi, rhi::Texture3D& tex, QRhiBuffer* buf);
};
