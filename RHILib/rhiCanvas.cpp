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
#include "rhiCanvas.h"
#include "rhiUtil.h"
#include "rhiMesh.h"
#include "rhiShader.h"
#include <QPainter>

void CanvasRenderPass::create(QRhiSwapChain* sc)
{
	m_sc = sc;

	// fps indicator texture
	QSize size(320, 40);
	m_fpsTex.create(QImage(size, QImage::Format_RGBA8888_Premultiplied));

	m_fpsub.create(m_rhi);

	CanvasShader shader(m_rhi);

	// create shader resources
	m_sr.reset(shader.createShaderResource(m_rhi, m_fpsTex, m_fpsub.get()));

	m_pl.reset(m_rhi->newGraphicsPipeline());
	m_pl->setRenderPassDescriptor(sc->renderPassDescriptor());
	m_pl->setSampleCount(sc->sampleCount());

	m_pl->setDepthTest(false);
	m_pl->setDepthWrite(false);
	m_pl->setTargetBlends({ rhi::defaultBlendState() });

	m_pl->setShaderStages(shader.begin(), shader.end());
	m_pl->setTopology(QRhiGraphicsPipeline::Triangles);
	m_pl->setVertexInputLayout(shader.meshLayout());
	m_pl->setShaderResourceBindings(m_sr->get());
	m_pl->create();

	// create the mesh
	rhi::MeshShaderResource* sr = shader.createShaderResource(m_rhi, m_fpsTex, m_fpsub.get());

	m_fpsMesh.reset(new FPSMesh(m_rhi));
	m_fpsMesh->create(size);
	m_fpsMesh->setShaderResource(sr);
	m_fpsMesh->getSubMesh(0)->SetActive(true);
}

void CanvasRenderPass::update(QRhiResourceUpdateBatch* u)
{
	// update viewport size for canvas uniform block
	QSize pixelSize = m_sc->surfacePixelSize();
	vec2f vp;
	vp.x = pixelSize.width();
	vp.y = pixelSize.height();
	m_fpsub.setViewPort(vp);
	m_fpsub.update(u);

	// update fps indicator
	const QImage& tex = m_fpsTex.getImage();
	QSize size = tex.size();
	QImage img(size, QImage::Format_RGBA8888_Premultiplied);
	double dpr = tex.devicePixelRatio();
	img.setDevicePixelRatio(dpr);
	img.fill(Qt::transparent);
	QPainter painter(&img);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	QFont font("Monospace", 20);
	painter.setFont(font);
	painter.setPen(Qt::red);
	QString txt = QString("fps: %1 [%2,%3]").arg(m_fps, 0, 'f', 1).arg(m_fpsMin, 0, 'f', 1).arg(m_fpsMax, 0, 'f', 1);
	painter.drawText(QRectF(10, 0, size.width() - 10, size.height()), txt);
	if (m_rhi->isYUpInNDC())
		m_fpsTex.setImage(img.mirrored());
	else
		m_fpsTex.setImage(img);
	m_fpsTex.update(u);

	m_fpsMesh->Update(u);
}

void CanvasRenderPass::draw(QRhiCommandBuffer* cb)
{
	cb->setGraphicsPipeline(m_pl.get());
	cb->setShaderResources();
	m_fpsMesh->Draw(cb);
}

FPSMesh::FPSMesh(QRhi* rhi) : rhi::Mesh(rhi)
{
}

void FPSMesh::create(QSize size)
{
	float W = (float)size.width();
	float H = (float)size.height();
	vec2f p[4] = { {0,0}, {W, 0}, {W, H}, {0,H} };
	vec2f t[4] = { {0,1}, {1, 1}, {1, 0}, {0,0} };

	int NV = 6;
	std::vector<Vertex> vertexData;
	vertexData.resize(NV);
	Vertex* v = vertexData.data();
	v[0].r = p[0]; v[0].t = t[0];
	v[1].r = p[1]; v[1].t = t[1];
	v[2].r = p[2]; v[2].t = t[2];
	v[3].r = p[2]; v[3].t = t[2];
	v[4].r = p[3]; v[4].t = t[3];
	v[5].r = p[0]; v[5].t = t[0];

	// create the vertex buffer
	rhi::Mesh::create(NV, sizeof(Vertex), vertexData.data());

	submeshes.emplace_back(std::make_unique<rhi::SubMesh>(this, 0, NV));
}
