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

QString rhi::Shader::resourcePath = QLatin1String(":/RHILib/shaders/");

void rhi::Shader::create(const QString& vertexShader, const QString& fragmentShader)
{
	shaders = {
		{ QRhiShaderStage::Vertex  , rhi::getShader(resourcePath + vertexShader) },
		{ QRhiShaderStage::Fragment, rhi::getShader(resourcePath + fragmentShader) }
	};
}

class PointShaderResource : public rhi::MeshShaderResource
{
public:
	// NOTE: the order of the members is chosen to satisfy std140 alignment rules,
	// since rhi::UniformBlock packs members tightly.
	enum { MV, COL, VIEWPORT, CLIP, VCOL, PSIZE, DBIAS };

public:
	PointShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, const float* viewportSize) : MeshShaderResource(rhi), m_vp(viewportSize)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mv"},
			{rhi::UniformBlock::VEC4, "col"},
			{rhi::UniformBlock::VEC2, "viewport"},
			{rhi::UniformBlock::INT , "useClipping"},
			{rhi::UniformBlock::INT , "useVertexColor"},
			{rhi::UniformBlock::FLOAT, "pointSize"},
			{rhi::UniformBlock::INT , "useDepthBias"}
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		static const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, globalBuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get())
			});
		srb->create();
	}

	void setData(const rhi::MeshRenderItem& m) override
	{
		float diffuse[4] = { 0.f };
		m.mat.diffuse.toFloat(diffuse);

		m_data.setMat4(MV, m.mvMatrix);
		m_data.setVec4(COL, diffuse);
		m_data.setInt (CLIP, (m.doClipping? 1 : 0));
		m_data.setInt (VCOL, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR ? 1 : 0));
		m_data.setFloat(PSIZE, (m.pointSize < 1.f ? 1.f : m.pointSize));

		vec2f vp(1.f, 1.f);
		if (m_vp)
		{
			vp.x = (m_vp[0] > 0.f ? m_vp[0] : 1.f);
			vp.y = (m_vp[1] > 0.f ? m_vp[1] : 1.f);
		}
		m_data.setVec2(VIEWPORT, vp);
		m_data.setInt(DBIAS, (m.depthBias ? 1 : 0));
	}

private:
	const float* m_vp = nullptr; // viewport size (owned by render pass)
};

PointShader::PointShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("point.vert.qsb", "point.frag.qsb");
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

rhi::MeshShaderResource* PointShader::createShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, const float* viewportSize)
{
	return new PointShaderResource(rhi, globalBuf, viewportSize);
}


class LineShaderResource : public rhi::MeshShaderResource
{
public:
	enum { MV, COL, CLIP, VCOL, TEX };

public:
	LineShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, rhi::Texture& tex1D) : rhi::MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mv"},
			{rhi::UniformBlock::VEC4, "col"},
			{rhi::UniformBlock::INT , "useClipping" },
			{rhi::UniformBlock::INT , "useVertexColor" },
			{rhi::UniformBlock::INT , "useTexture" }
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		static const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, globalBuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
				QRhiShaderResourceBinding::sampledTexture(2, visibility, tex1D.texture.get(), tex1D.sampler.get())
			});
		srb->create();
	}

	void setData(const rhi::MeshRenderItem& m) override
	{
		float diffuse[4] = { 0.f };
		m.mat.diffuse.toFloat(diffuse);

		m_data.setMat4(MV, m.mvMatrix);
		m_data.setVec4(COL, diffuse);
		m_data.setInt (CLIP, (m.doClipping ? 1 : 0));
		m_data.setInt (VCOL, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR ? 1 : 0));
		m_data.setInt (TEX , (m.mat.diffuseMap == GLMaterial::TEXTURE_1D ? 1 : 0));
	}
};

LineShader::LineShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("lines.vert.qsb", "lines.frag.qsb");
}

QRhiVertexInputLayout LineShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 10 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // texture coordinate
		{ 0, 2, QRhiVertexInputAttribute::Float4, 6 * sizeof(float) } // color
		});
	return meshLayout;
}

rhi::MeshShaderResource* LineShader::createShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, rhi::Texture& tex1D)
{
	return new LineShaderResource(rhi, globalBuf, tex1D);
}

//=============================================================================
// Wide lines are rendered as screen-space quads, since most backends
// (e.g. Metal, D3D, and Vulkan without the wideLines feature) only
// support 1-pixel wide line primitives.
class WideLineShaderResource : public rhi::MeshShaderResource
{
public:
	// NOTE: the order of the members is chosen to satisfy std140 alignment rules,
	// since rhi::UniformBlock packs members tightly.
	enum { MV, COL, VIEWPORT, LWIDTH, CLIP, VCOL, TEX, DBIAS };

public:
	WideLineShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, rhi::Texture& tex1D, const float* viewportSize) : rhi::MeshShaderResource(rhi), m_vp(viewportSize)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4 , "mv"},
			{rhi::UniformBlock::VEC4 , "col"},
			{rhi::UniformBlock::VEC2 , "viewport"},
			{rhi::UniformBlock::FLOAT, "lineWidth"},
			{rhi::UniformBlock::INT  , "useClipping" },
			{rhi::UniformBlock::INT  , "useVertexColor" },
			{rhi::UniformBlock::INT  , "useTexture" },
			{rhi::UniformBlock::INT  , "useDepthBias" }
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		static const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, globalBuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
				QRhiShaderResourceBinding::sampledTexture(2, visibility, tex1D.texture.get(), tex1D.sampler.get())
			});
		srb->create();
	}

	void setData(const rhi::MeshRenderItem& m) override
	{
		float diffuse[4] = { 0.f };
		m.mat.diffuse.toFloat(diffuse);

		vec2f vp(1.f, 1.f);
		if (m_vp)
		{
			vp.x = (m_vp[0] > 0.f ? m_vp[0] : 1.f);
			vp.y = (m_vp[1] > 0.f ? m_vp[1] : 1.f);
		}

		m_data.setMat4 (MV, m.mvMatrix);
		m_data.setVec4 (COL, diffuse);
		m_data.setVec2 (VIEWPORT, vp);
		m_data.setFloat(LWIDTH, (m.lineWidth < 1.f ? 1.f : m.lineWidth));
		m_data.setInt  (CLIP, (m.doClipping ? 1 : 0));
		m_data.setInt  (VCOL, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR ? 1 : 0));
		m_data.setInt  (TEX , (m.mat.diffuseMap == GLMaterial::TEXTURE_1D ? 1 : 0));
		m_data.setInt  (DBIAS, (m.depthBias ? 1 : 0));
	}

private:
	const float* m_vp = nullptr; // viewport size (owned by render pass)
};

WideLineShader::WideLineShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("widelines.vert.qsb", "widelines.frag.qsb");
}

QRhiVertexInputLayout WideLineShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ sizeof(WideLineShader::Vertex) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // segment start point
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // segment end point
		{ 0, 2, QRhiVertexInputAttribute::Float3, 6 * sizeof(float) }, // texture coordinate
		{ 0, 3, QRhiVertexInputAttribute::Float4, 9 * sizeof(float) }, // color
		{ 0, 4, QRhiVertexInputAttribute::Float2, 13 * sizeof(float) } // (endpoint, side)
		});
	return meshLayout;
}

rhi::MeshShaderResource* WideLineShader::createShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, rhi::Texture& tex1D, const float* viewportSize)
{
	return new WideLineShaderResource(rhi, globalBuf, tex1D, viewportSize);
}

class ColorShaderResource : public rhi::MeshShaderResource
{
public:
	ColorShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, rhi::Texture& tex1D, rhi::Texture& envTex) : MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4 , "mv"},
			{rhi::UniformBlock::VEC4 , "ambient"},
			{rhi::UniformBlock::VEC4 , "diffuse"},
			{rhi::UniformBlock::VEC4 , "specular"},
			{rhi::UniformBlock::FLOAT, "specExp"},
			{rhi::UniformBlock::FLOAT, "opacity"},
			{rhi::UniformBlock::FLOAT, "reflection"},
			{rhi::UniformBlock::INT  , "useTexture"},
			{rhi::UniformBlock::INT  , "useStipple"},
			{rhi::UniformBlock::INT  , "useClipping"},
			{rhi::UniformBlock::INT  , "useVertexColor"},
			{rhi::UniformBlock::INT  , "useLighting"},
			{rhi::UniformBlock::INT  , "frontOnly"},
			{rhi::UniformBlock::INT  , "useFrontLight"},
			{rhi::UniformBlock::INT  , "invertFaces"}
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, globalBuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
				QRhiShaderResourceBinding::sampledTexture(2, visibility, tex1D.texture.get(), tex1D.sampler.get()),
				QRhiShaderResourceBinding::sampledTexture(3, visibility, envTex.texture.get(), envTex.sampler.get())
			});
		srb->create();
	}

	void setData(const rhi::MeshRenderItem& m) override
	{
		float diffuse[4] = { 0.f }, spec[4] = { 0.f }, amb[4] = { 0.f };
		m.mat.ambient.toFloat(amb);
		m.mat.diffuse.toFloat(diffuse);
		m.mat.specular.toFloat(spec);

		m_data.setMat4 ( 0, m.mvMatrix);
		m_data.setVec4 ( 1, amb);
		m_data.setVec4 ( 2, diffuse);
		m_data.setVec4 ( 3, spec);
		m_data.setFloat( 4, m.mat.shininess);
		m_data.setFloat( 5, m.mat.opacity);
		m_data.setFloat( 6, m.mat.reflection);
		m_data.setInt  ( 7, (m.mat.diffuseMap == GLMaterial::TEXTURE_1D ? 1 : 0));
		m_data.setInt  ( 8, ((m.mat.type == GLMaterial::HIGHLIGHT) || (m.mat.type == GLMaterial::GLASS) ? 1 : 0));
		m_data.setInt  ( 9, (m.doClipping ? 1 : 0));
		m_data.setInt  (10, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR ? 1 : 0));
		m_data.setInt  (11, (m.mat.type == GLMaterial::CONSTANT) || (m.mat.type == GLMaterial::OVERLAY) || (m.mat.type == GLMaterial::HIGHLIGHT) ? 0 : 1);
		m_data.setInt  (12, (m.mat.frontOnly ? 1 : 0));
		m_data.setInt  (13, (m.mat.diffuseMap == GLMaterial::TEXTURE_1D ? 0 : 1));
		m_data.setInt  (14, (m.invertFaces ? 1 : 0));
	}
};

SolidShader::SolidShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("color.vert.qsb", "color.frag.qsb");
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

rhi::MeshShaderResource* SolidShader::createShaderResource(QRhi* rhi, QRhiBuffer* globalBuf, rhi::Texture& tex1D, rhi::Texture& envTex)
{
	return new ColorShaderResource(rhi, globalBuf, tex1D, envTex);
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
	rhi::Shader::create("canvas.vert.qsb", "canvas.frag.qsb");
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
	rhi::Shader::create("overlay.vert.qsb", "overlay.frag.qsb");
}

QRhiVertexInputLayout OverlayShader::meshLayout()
{
	// this shader doesn't take vertex inputs
	return QRhiVertexInputLayout();
}

class TriadShaderResource : public rhi::MeshShaderResource
{
public:
	TriadShaderResource(QRhi* rhi, QRhiBuffer* global) : rhi::MeshShaderResource(rhi)
	{
		m_data.create({
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
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, global),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
			});
		srb->create();
	}

	void setData(const rhi::MeshRenderItem& m) override
	{
		m_data.setMat4(0, m.mvMatrix);
	}
};

TriadShader::TriadShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("triad.vert.qsb", "triad.frag.qsb");
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

rhi::MeshShaderResource* TriadShader::createShaderResource(QRhi* rhi, QRhiBuffer* globalBuf)
{
	return new TriadShaderResource(rhi, globalBuf);
}

//=============================================================================
class VolumeShaderResource : public rhi::MeshShaderResource
{
public:
	VolumeShaderResource(QRhi* rhi, rhi::Texture3D& tex, QRhiBuffer* globalBuf, QRhiBuffer* vrSettings) : MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mv"},
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
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, globalBuf),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, vrSettings),
				QRhiShaderResourceBinding::uniformBuffer(2, visibility, ubuf.get()),
				QRhiShaderResourceBinding::sampledTexture(3, visibility, tex.texture.get(), tex.sampler.get())
			});
		srb->create();
	}

	void setData(const rhi::MeshRenderItem& m) override
	{
		m_data.setMat4(0, m.mvMatrix);
		m_data.setVec4(1, m.mat.diffuse);
	}
};

VolumeShader::VolumeShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("volume.vert.qsb", "volume.frag.qsb");
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

rhi::MeshShaderResource* VolumeShader::createShaderResource(QRhi* rhi, rhi::Texture3D& tex, QRhiBuffer* global, QRhiBuffer* vrSettings)
{
	return new VolumeShaderResource(rhi, tex, global, vrSettings);
}

//=============================================================================
GradientShader::GradientShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("grad.vert.qsb", "grad.frag.qsb");
}

QRhiVertexInputLayout GradientShader::meshLayout()
{
	return QRhiVertexInputLayout();
}

//=============================================================================
class DiffuseShaderResource : public rhi::MeshShaderResource
{
public:
	DiffuseShaderResource(QRhi* rhi, QRhiBuffer* global) : rhi::MeshShaderResource(rhi)
	{
		m_data.create({
			{rhi::UniformBlock::MAT4, "mv"},
			{rhi::UniformBlock::VEC4, "col"},
			{rhi::UniformBlock::INT, "useStipple"},
			{rhi::UniformBlock::INT, "useClipping"},
			{rhi::UniformBlock::INT, "useLighting"},
			{rhi::UniformBlock::INT, "useVertexColor"},
			});

		// create the buffer
		ubuf.reset(rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_data.size()));
		ubuf->create();

		// create resource binding
		const QRhiShaderResourceBinding::StageFlags visibility =
			QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;

		srb.reset(rhi->newShaderResourceBindings());
		srb->setBindings({
				QRhiShaderResourceBinding::uniformBuffer(0, visibility, global),
				QRhiShaderResourceBinding::uniformBuffer(1, visibility, ubuf.get()),
			});
		srb->create();
	}

	void setData(const rhi::MeshRenderItem& m) override
	{
		m_data.setMat4(0, m.mvMatrix);
		m_data.setVec4(1, m.mat.diffuse);
		m_data.setInt(2, (m.mat.type == GLMaterial::HIGHLIGHT) || (m.mat.type == GLMaterial::GLASS) ? 1 : 0);
		m_data.setInt(3, (m.doClipping ? 1 : 0));
		m_data.setInt(4, (m.mat.type == GLMaterial::HIGHLIGHT) || (m.mat.type == GLMaterial::CONSTANT) || (m.mat.type == GLMaterial::OVERLAY) ? 0 : 1);
		m_data.setInt(5, (m.mat.diffuseMap == GLMaterial::VERTEX_COLOR) ? 1 : 0);
	}
};

DiffuseShader::DiffuseShader(QRhi* rhi) : rhi::Shader(rhi)
{
	rhi::Shader::create("diffuse.vert.qsb", "diffuse.frag.qsb");
}

QRhiVertexInputLayout DiffuseShader::meshLayout()
{
	QRhiVertexInputLayout meshLayout;
	meshLayout.setBindings({
		{ 10 * sizeof(float) }
		});
	meshLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 }, // position
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }, // normal
		{ 0, 2, QRhiVertexInputAttribute::Float4, 6 * sizeof(float) }, // color
		});
	return meshLayout;
}

rhi::MeshShaderResource* DiffuseShader::createShaderResource(QRhi* rhi, QRhiBuffer* global)
{
	return new DiffuseShaderResource(rhi, global);
}
