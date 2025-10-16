/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include "rhiSceneView.h"
#include <QPainter>
#include <QFile>
#include <QWidget>
#include <QMouseEvent>
#include <GLLib/GLMesh.h>
#include <rhi/qshader.h>
#include "rhiDocument.h"
#include <FEBioStudio/MainWindow.h>

QWidget* createRHIWidget(CMainWindow* wnd, QRhi::Implementation api)
{
#if QT_CONFIG(vulkan)
	static QVulkanInstance inst;
	if (api == QRhi::Vulkan) {
		// Request validation, if available. This is completely optional
		// and has a performance impact, and should be avoided in production use.
		inst.setLayers({ "VK_LAYER_KHRONOS_validation" });
		// Play nice with QRhi.
		inst.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
		if (!inst.create()) {
			qWarning("Failed to create Vulkan instance, switching to OpenGL");
			api = QRhi::OpenGLES2;
		}
	}
#endif

	// choose sample count for MSAA
	unsigned int sampleCount = 4;

	// For OpenGL, to ensure there is a depth/stencil buffer for the window.
	 // With other APIs this is under the application's control (QRhiRenderBuffer etc.)
	 // and so no special setup is needed for those.
	if (api == QRhi::OpenGLES2)
	{
		QSurfaceFormat fmt;
		fmt.setDepthBufferSize(24);
		fmt.setStencilBufferSize(8);
		fmt.setSamples(4);
		// Special case macOS to allow using OpenGL there.
		// (the default Metal is the recommended approach, though)
		// gl_VertexID is a GLSL 130 feature, and so the default OpenGL 2.1 context
		// we get on macOS is not sufficient.
#ifdef Q_OS_MACOS
		fmt.setVersion(4, 1);
		fmt.setProfile(QSurfaceFormat::CoreProfile);
#endif
		QSurfaceFormat::setDefaultFormat(fmt);
	}

	RhiWindow* rhiWnd = new rhiSceneView(wnd, api);
	rhiWnd->setSampleCount(sampleCount);

#if QT_CONFIG(vulkan)
	if (api == QRhi::Vulkan)
		rhiWnd->setVulkanInstance(&inst);
#endif

	return QWidget::createWindowContainer(rhiWnd);
}

static QShader getShader(const QString& name)
{
	QFile f(name);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());

	return QShader();
}

rhiSceneView::rhiSceneView(CMainWindow* wnd, QRhi::Implementation graphicsApi)
	: RhiWindow(graphicsApi)
{
	m_wnd = wnd;
}

rhiSceneView::~rhiSceneView()
{
}

void flipX(GLMesh* pm)
{
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		GLMesh::NODE& nd = pm->Node(i);
		vec3f r = nd.r;
		r.x = -r.x;
		nd.r = r;
	}

	for (int i = 0; i < pm->Faces(); ++i)
	{
		GLMesh::FACE& f = pm->Face(i);
		int n1 = f.n[1];
		int n2 = f.n[2];
		f.n[1] = n2;
		f.n[2] = n1;
	}
	pm->Update();
}

void rhiSceneView::customInit()
{
	QString msg;
	msg += QString("backend = %1\n").arg(m_rhi->backendName());
	msg += QString("driver  = %1\n").arg(QString(m_rhi->driverInfo().deviceName));
	msg += QString("sample count = %1\n").arg(m_sc->sampleCount());
	m_wnd->AddLogEntry(msg);

	m_initialUpdates = m_rhi->nextResourceUpdateBatch();

	static const quint32 UBUF_SIZE = 128; // PV, Q
	m_ubuf.reset(m_rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, UBUF_SIZE));
	m_ubuf->create();

	m_colorTriSrb.reset(m_rhi->newShaderResourceBindings());
	static const QRhiShaderResourceBinding::StageFlags visibility =
		QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
	m_colorTriSrb->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0, visibility, m_ubuf.get())
		});
	m_colorTriSrb->create();

	m_colorPipeline.reset(m_rhi->newGraphicsPipeline());
	m_colorPipeline->setSampleCount(m_sc->sampleCount());
	m_colorPipeline->setDepthTest(true);
	m_colorPipeline->setDepthWrite(true);
	// Blend factors default to One, OneOneMinusSrcAlpha, which is convenient.
	QRhiGraphicsPipeline::TargetBlend premulAlphaBlend;
	premulAlphaBlend.enable = true;
	m_colorPipeline->setTargetBlends({ premulAlphaBlend });
	m_colorPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, getShader(QLatin1String(":/RHILib/shaders/color.vert.qsb")) },
		{ QRhiShaderStage::Fragment, getShader(QLatin1String(":/RHILib/shaders/color.frag.qsb")) }
		});
	// Draw triangles
	m_colorPipeline->setTopology(QRhiGraphicsPipeline::Triangles);

	// set the layout of the vertex data buffer.
	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings({
		{ 9 * sizeof(float) }
		});
	inputLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float3, 0 },
		{ 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) },
		{ 0, 2, QRhiVertexInputAttribute::Float3, 6 * sizeof(float) }
		});
	m_colorPipeline->setVertexInputLayout(inputLayout);
	m_colorPipeline->setShaderResourceBindings(m_colorTriSrb.get());
	m_colorPipeline->setRenderPassDescriptor(m_rp.get());
	m_colorPipeline->create();
}

void rhiSceneView::customRender()
{
	rhiDocument* doc = dynamic_cast<rhiDocument*>(m_wnd->GetDocument());
	if (doc != m_doc)
	{
		m_doc = doc;
		GLMesh* gmsh = doc->GetMesh();

		// see if we need to update meshes
		if (m_meshList.empty())
		{
			m_meshList.resize(1);
			m_meshList[0].reset(new rhi::Mesh(m_rhi.get()));
		}

		bool b = m_meshList[0]->CreateFromGLMesh(gmsh); assert(b);

		BOX box = gmsh->GetBoundingBox();

		m_cam.SetTargetDistance(2.0*box.Radius());
		m_cam.Update(true);
	}

	QRhiResourceUpdateBatch* resourceUpdates = m_rhi->nextResourceUpdateBatch();

	if (m_initialUpdates) {
		resourceUpdates->merge(m_initialUpdates);
		m_initialUpdates->release();
		m_initialUpdates = nullptr;
	}

	QMatrix4x4 Q;
	quatd q = m_cam.GetOrientation();
	QQuaternion qt(q.w, q.x, q.y, q.z);
	Q.rotate(qt);

	QMatrix4x4 view;
	vec3d t = m_cam.Target();
	vec3d p = m_cam.GetPosition();
	view.translate(-t.x, -t.y, -t.z);
	view *= Q;
	view.translate(-p.x, -p.y, -p.z);

	QRhiCommandBuffer* cb = m_sc->currentFrameCommandBuffer();
	const QSize outputSizeInPixels = m_sc->currentPixelSize();

	// update mesh data
	for (auto& mesh : m_meshList) 
		mesh->Update(resourceUpdates, m_proj, view);

	// start the rendering pass
	cb->beginPass(m_sc->currentFrameRenderTarget(), QColor::fromRgbF(0.8f, 0.8f, 1.f), { 1.0f, 0 }, resourceUpdates);

	cb->setGraphicsPipeline(m_colorPipeline.get());
	cb->setViewport({ 0, 0, float(outputSizeInPixels.width()), float(outputSizeInPixels.height()) });
	cb->setShaderResources();

	for (auto& mesh : m_meshList)
		mesh->Draw(cb);

	cb->endPass();
}

void rhiSceneView::mousePressEvent(QMouseEvent* ev)
{
	m_prevPos = ev->pos();

	setRenderMode(RenderMode::DYNAMIC);
}

void rhiSceneView::mouseMoveEvent(QMouseEvent* ev)
{
	GLCamera& cam = m_cam;

	bool bshift = (ev->modifiers() & Qt::ShiftModifier ? true : false);
	bool bctrl = (ev->modifiers() & Qt::ControlModifier ? true : false);
	bool balt = (ev->modifiers() & Qt::AltModifier ? true : false);

	bool but1 = (ev->buttons() & Qt::LeftButton);
	bool but2 = (ev->buttons() & Qt::MiddleButton);
	bool but3 = (ev->buttons() & Qt::RightButton);

	// get the mouse position
	int x1 = ev->pos().x();
	int y1 = ev->pos().y();
	int x0 = m_prevPos.x();
	int y0 = m_prevPos.y();

	if (but1)
	{
		if (balt)
		{
			quatd qz = quatd((y1 - y0) * 0.01f, vec3d(0, 0, 1));
			cam.Orbit(qz);
		}
		else
		{
			quatd qx = quatd((y1 - y0) * 0.01f, vec3d(1, 0, 0));
			quatd qy = quatd((x1 - x0) * 0.01f, vec3d(0, 1, 0));

			cam.Orbit(qx);
			cam.Orbit(qy);
		}
	}
	else if (but2 || (but3 && balt))
	{
		vec3d r = vec3d(-(double)(x1 - x0), (double)(y1 - y0), 0.f);
		cam.PanView(r);
	}
	else if (but3)
	{
		if (bshift)
		{
			double D = (double)y0 - y1;
			double s = cam.GetFinalTargetDistance() * 1e-2;
			if (D < 0) s = -s;
			cam.Dolly(s);
		}
		else if (bctrl)
		{
			quatd qx = quatd((y0 - y1) * 0.001f, vec3d(1, 0, 0));
			quatd qy = quatd((x0 - x1) * 0.001f, vec3d(0, 1, 0));
			quatd q = qy * qx;
			cam.Pan(q);
		}
		else
		{
			if (y0 > y1) cam.Zoom(0.98f);
			if (y0 < y1) cam.Zoom(1.0f / 0.98f);
		}
	}
	else return;

	m_prevPos = ev->pos();

	cam.Update(true);

	ev->accept();
}

void rhiSceneView::mouseReleaseEvent(QMouseEvent* event)
{
	setRenderMode(RenderMode::STATIC);
}
