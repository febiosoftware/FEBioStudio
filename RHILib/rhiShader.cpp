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
#include "rhiShader.h"
#include "rhiUtil.h"

void rhi::Shader::create(const QString& vertexShader, const QString& fragmentShader)
{
	shaders = {
		{ QRhiShaderStage::Vertex  , rhi::getShader(vertexShader) },
		{ QRhiShaderStage::Fragment, rhi::getShader(fragmentShader) }
	};
}

class PointShaderResource : public rhi::MeshShaderResource
{
public:
	enum { MVP, MV, COL, CLIP, VCOL };

public:
	PointShaderResource(QRhi* rhi, rhi::SharedResources* sharedResources) : MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mvp"},
			{rhi::UniformBlock::MAT4, "mv"},
			{rhi::UniformBlock::VEC4, "col"},
			{rhi::UniformBlock::INT , "useClipping"},
			{rhi::UniformBlock::INT , "useVertexColor"}
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		static const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, sharedResources->globalbuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get())
			});
		srb->create();
	}

	void setData(const rhi::Mesh& m) override
	{
		float diffuse[4] = { 0.f };
		m.mat.diffuse.toFloat(diffuse);

		QMatrix4x4 mvp = m.prMatrix * m.mvMatrix;

		m_data.setMat4(MVP, mvp);
		m_data.setMat4(MV, m.mvMatrix);
		m_data.setVec4(COL, diffuse);
		m_data.setInt (CLIP, (m.doClipping? 1 : 0));
		m_data.setInt (VCOL, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR ? 1 : 0));
	}
};

PointShader::PointShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create(
		QLatin1String(":/RHILib/shaders/point.vert.qsb"),
		QLatin1String(":/RHILib/shaders/point.frag.qsb")
	);
}

QRhiVertexInputLayout PointShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 6 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3*sizeof(float)} // color
		});
	return meshLayout;
}

rhi::MeshShaderResource* PointShader::createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource)
{
	return new PointShaderResource(rhi, sharedResource);
}


class LineShaderResource : public rhi::MeshShaderResource
{
public:
	enum { MVP, MV, COL, CLIP, VCOL };

public:
	LineShaderResource(QRhi* rhi, rhi::SharedResources* sharedResources) : rhi::MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mvp"},
			{rhi::UniformBlock::MAT4, "mv"},
			{rhi::UniformBlock::VEC4, "col"},
			{rhi::UniformBlock::INT , "useClipping" },
			{rhi::UniformBlock::INT , "useVertexColor" }
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		static const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, sharedResources->globalbuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get())
			});
		srb->create();
	}

	void setData(const rhi::Mesh& m) override
	{
		float diffuse[4] = { 0.f };
		m.mat.diffuse.toFloat(diffuse);

		QMatrix4x4 mvp = m.prMatrix * m.mvMatrix;

		m_data.setMat4(MVP, mvp);
		m_data.setMat4(MV, m.mvMatrix);
		m_data.setVec4(COL, diffuse);
		m_data.setInt (CLIP, (m.doClipping ? 1 : 0));
		m_data.setInt (VCOL, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR ? 1 : 0));
	}
};

LineShader::LineShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create(
		QLatin1String(":/RHILib/shaders/lines.vert.qsb"),
		QLatin1String(":/RHILib/shaders/lines.frag.qsb")
	);
}

QRhiVertexInputLayout LineShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 7 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float4, 3 * sizeof(float) } // color
		});
	return meshLayout;
}

rhi::MeshShaderResource* LineShader::createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource)
{
	return new LineShaderResource(rhi, sharedResource);
}

class ColorShaderResource : public rhi::MeshShaderResource
{
public:
	ColorShaderResource(QRhi* rhi, rhi::SharedResources* sharedResources) : MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4 , "mvp"},
			{rhi::UniformBlock::MAT4 , "mv"},
			{rhi::UniformBlock::VEC4 , "col"},
			{rhi::UniformBlock::FLOAT, "specExp"},
			{rhi::UniformBlock::FLOAT, "specStrength"},
			{rhi::UniformBlock::FLOAT, "opacity"},
			{rhi::UniformBlock::INT  , "useTexture"},
			{rhi::UniformBlock::INT  , "useStipple"},
			{rhi::UniformBlock::INT  , "useClipping"},
			{rhi::UniformBlock::INT  , "useVertexColor"},
			{rhi::UniformBlock::INT  , "useLighting"},
			{rhi::UniformBlock::INT  , "frontOnly"}
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, sharedResources->globalbuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
				QRhiShaderResourceBinding::sampledTexture(2, visibility, sharedResources->texture, sharedResources->sampler)
			});
		srb->create();
	}

	void setData(const rhi::Mesh& m) override
	{
		float diffuse[4] = { 0.f };
		m.mat.diffuse.toFloat(diffuse);

		QMatrix4x4 mvp = m.prMatrix * m.mvMatrix;

		m_data.setMat4 ( 0, mvp);
		m_data.setMat4 ( 1, m.mvMatrix);
		m_data.setVec4 ( 2, diffuse);
		m_data.setFloat( 3, m.mat.shininess);
		m_data.setFloat( 4, m.mat.reflectivity);
		m_data.setFloat( 5, m.mat.opacity);
		m_data.setInt  ( 6, (m.mat.diffuseMap == GLMaterial::TEXTURE_1D ? 1 : 0));
		m_data.setInt  ( 7, (m.mat.type == GLMaterial::HIGHLIGHT ? 1 : 0));
		m_data.setInt  ( 8, (m.doClipping ? 1 : 0));
		m_data.setInt  ( 9, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR ? 1 : 0));
		m_data.setInt  (10, (m.mat.type == GLMaterial::CONSTANT) || (m.mat.type == GLMaterial::OVERLAY) ? 0 : 1);
		m_data.setInt  (11, (m.mat.frontOnly ? 1 : 0));
	}
};

SolidShader::SolidShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create(
		QLatin1String(":/RHILib/shaders/color.vert.qsb"),
		QLatin1String(":/RHILib/shaders/color.frag.qsb")
	);
}

QRhiVertexInputLayout SolidShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 13 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // normal 
		{ 0, 2, QRhiVertexInputAttribute::Float3, 6 * sizeof(float) }, // texcoord
		{ 0, 3, QRhiVertexInputAttribute::Float4, 9 * sizeof(float) }, // color
		});
	return meshLayout;
}

rhi::MeshShaderResource* SolidShader::createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource)
{
	return new ColorShaderResource(rhi, sharedResource);
}

class CanvasShaderResource : public rhi::MeshShaderResource
{
public:
	CanvasShaderResource(QRhi* rhi, rhi::Texture& tex, QRhiBuffer* ub) : rhi::MeshShaderResource(rhi)
	{
		static const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(m_rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, ub),
				QRhiShaderResourceBinding::sampledTexture(1, visibility, tex.texture.get(), tex.sampler.get())
			});
		srb->create();
	}
};

CanvasShader::CanvasShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create(
		QLatin1String(":/RHILib/shaders/canvas.vert.qsb"),
		QLatin1String(":/RHILib/shaders/canvas.frag.qsb"));
}

QRhiVertexInputLayout CanvasShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 4 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float2, 0 }, // 2D position
		{ 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) }, // 2D tex coord
		});
	return meshLayout;
}

rhi::MeshShaderResource* CanvasShader::createShaderResource(QRhi* rhi, rhi::Texture& tex, QRhiBuffer* ub)
{
	return new CanvasShaderResource(rhi, tex, ub);
}

OverlayShader::OverlayShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create(
		QLatin1String(":/RHILib/shaders/overlay.vert.qsb"),
		QLatin1String(":/RHILib/shaders/overlay.frag.qsb"));
}

QRhiVertexInputLayout OverlayShader::meshLayout()
{
	// this shader doesn't take vertex inputs
	return QRhiVertexInputLayout();
}

class TriadShaderResource : public rhi::MeshShaderResource
{
public:
	TriadShaderResource(QRhi* rhi, rhi::SharedResources* sharedResources) : rhi::MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mvp"},
			{rhi::UniformBlock::MAT4, "mv"}
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, sharedResources->globalbuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
			});
		srb->create();
	}

	void setData(const rhi::Mesh& m) override
	{
		QMatrix4x4 mvp = m.prMatrix * m.mvMatrix;
		m_data.setMat4(0, mvp);
		m_data.setMat4(1, m.mvMatrix);
	}
};

TriadShader::TriadShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create(
		QLatin1String(":/RHILib/shaders/triad.vert.qsb"),
		QLatin1String(":/RHILib/shaders/triad.frag.qsb")
	);
}

QRhiVertexInputLayout TriadShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 9 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // normal 
		{ 0, 2, QRhiVertexInputAttribute::Float3, 6 * sizeof(float) }, // color
		});
	return meshLayout;
}

rhi::MeshShaderResource* TriadShader::createShaderResource(QRhi* rhi, rhi::SharedResources* sharedResource)
{
	return new TriadShaderResource(rhi, sharedResource);
}

//=============================================================================
class VolumeShaderResource : public rhi::MeshShaderResource
{
public:
	VolumeShaderResource(QRhi* rhi, rhi::Texture3D& tex, QRhiBuffer* sharedBuf) : MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mvp"},
			{rhi::UniformBlock::VEC4, "col"},
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, sharedBuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
				QRhiShaderResourceBinding::sampledTexture(2, visibility, tex.texture.get(), tex.sampler.get())
			});
		srb->create();
	}

	void setData(const rhi::Mesh& m) override
	{
		QMatrix4x4 mvp = m.prMatrix * m.mvMatrix;
		m_data.setMat4(0, mvp);
		m_data.setVec4(1, m.mat.diffuse);
	}
};

VolumeShader::VolumeShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create(
		QLatin1String(":/RHILib/shaders/volume.vert.qsb"),
		QLatin1String(":/RHILib/shaders/volume.frag.qsb")
	);
}

QRhiVertexInputLayout VolumeShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 6 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // texCoord
		});
	return meshLayout;
}

rhi::MeshShaderResource* VolumeShader::createShaderResource(QRhi* rhi, rhi::Texture3D& tex, QRhiBuffer* buf)
{
	return new VolumeShaderResource(rhi, tex, buf);
}
