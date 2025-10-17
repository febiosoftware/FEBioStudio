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
#include <GLLib/GLContext.h>

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

rhiSceneView::rhiSceneView(CMainWindow* wnd, QRhi::Implementation graphicsApi)
	: RhiWindow(graphicsApi)
{
	m_wnd = wnd;
}

rhiSceneView::~rhiSceneView()
{
	delete m_rhiRender;
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

	m_rhiRender = new rhiRenderer(m_rhi.get(), m_sc.get(), m_rp.get());
	m_rhiRender->init();
}

void rhiSceneView::customRender()
{
	rhiDocument* doc = dynamic_cast<rhiDocument*>(m_wnd->GetDocument());

	// TODO: This doesn't always work! It is possible for a new document to have the same pointer
	// than the old one!
	if (doc != m_doc)
	{
		m_rhiRender->clearCache();
		m_doc = doc;
	}

	rhiScene* scene = m_doc->GetRhiScene();
	GLCamera& cam = scene->GetCamera();

	m_rhiRender->setViewProjection(m_proj);

	GLContext rc;
	rc.m_x = rc.m_y = 0;
	rc.m_w = width();
	rc.m_h = height();
	rc.m_cam = &cam;
	scene->Render(*m_rhiRender, rc);

	m_rhiRender->finish();
}

void rhiSceneView::mousePressEvent(QMouseEvent* ev)
{
	m_prevPos = ev->pos();

	setRenderMode(RenderMode::DYNAMIC);
}

void rhiSceneView::mouseMoveEvent(QMouseEvent* ev)
{
	if (m_doc == nullptr) return;
	GLCamera& cam = m_doc->GetScene()->GetCamera();

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
