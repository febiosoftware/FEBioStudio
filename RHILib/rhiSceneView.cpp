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
#include "rhiScene.h"
#include "rhiDocument.h"
#include <FEBioStudio/MainWindow.h>
#include <GLLib/GLContext.h>
#include <QMimeData>
#include <QFileInfo>
#include <QPainter>
#include <GLWLib/GLLabel.h>
#include <GLWLib/GLCheckBox.h>
#include <GLWLib/GLComposite.h>
#include <GLWLib/GLLegendBar.h>
#include <GLWLib/GLTriad.h>

rhiSceneView::rhiSceneView(CMainWindow* wnd)
{
	GLLabel* label = new GLLabel(0, 50, 450, 100, "Welcome to RHI!");
	label->set_font_size(40);
	m_Widget.AddWidget(label);

	const int W = 200;
	const int H = 30;

	GLComposite* menu = new GLComposite(0, 0, W, H);
	menu->align(GLWAlign::GLW_ALIGN_RIGHT | GLWAlign::GLW_ALIGN_TOP);

	GLCheckBox* meshLines = new GLCheckBox(0, 0, W, H, "mesh lines");
	meshLines->add_event_handler([this](GLWidget* w, int nevent) {
		GLCheckBox* b = dynamic_cast<GLCheckBox*>(w);
		rhiScene* s = dynamic_cast<rhiScene*>(this->GetScene());
		if (s) s->renderMesh = b->m_checked;
		});
	menu->add_widget(meshLines);

	GLCheckBox* meshNodes = new GLCheckBox(0, 0, W, H, "mesh nodes");
	meshNodes->add_event_handler([this](GLWidget* w, int nevent) {
		GLCheckBox* b = dynamic_cast<GLCheckBox*>(w);
		rhiScene* s = dynamic_cast<rhiScene*>(this->GetScene());
		if (s) s->renderNodes = b->m_checked;
		});
	menu->add_widget(meshNodes);
	m_Widget.AddWidget(menu);

	GLLegendBar* legend = new GLLegendBar(0, 0, 100, 400);
	legend->align(GLWAlign::GLW_ALIGN_RIGHT | GLWAlign::GLW_ALIGN_VCENTER);
	m_Widget.AddWidget(legend);

	triad = new GLTriad(0, 0, 100, 100);
	triad->align(GLWAlign::GLW_ALIGN_BOTTOM | GLWAlign::GLW_ALIGN_LEFT);
	m_Widget.AddWidget(triad);

	m_wnd = wnd;
}

rhiSceneView::~rhiSceneView()
{
	delete m_rhiRender;
}

void rhiSceneView::SetScene(GLScene* scene)
{
	m_scene = scene;
	if (m_rhiRender) m_rhiRender->clearCache();
	requestUpdate();
}

bool rhiSceneView::event(QEvent* event)
{
	/*
	if (m_doc && (event->type()==QEvent::Drop))
	{
		QDropEvent* ev = dynamic_cast<QDropEvent*>(event);
		if (ev)
		{
			foreach(const QUrl& url, ev->mimeData()->urls()) {
				QString fileName = url.toLocalFile();

				FileReader* fileReader = nullptr;

				QFileInfo file(fileName);
				m_doc->ImportFile(fileName);
			}

			m_doc->GetScene()->ZoomExtents(false);
			requestUpdate();
		}
	}
	*/
	return RhiWindow::event(event);
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
	m_rhiRender->setDPR(devicePixelRatio());
	m_rhiRender->init();
}

void rhiSceneView::customRender()
{
	if (m_scene == nullptr) return;

	GLCamera& cam = m_scene->GetCamera();

	m_rhiRender->start();

	GLContext rc;
	rc.m_cam = &cam;
	m_scene->Render(*m_rhiRender, rc);

	triad->setOrientation(cam.GetOrientation());

	bool renderOverlay = false;
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(m_scene);
	if (RhiScene) renderOverlay = RhiScene->renderOverlay;

	m_rhiRender->useOverlayImage(renderOverlay);
	if (renderOverlay)
	{
		QImage img(m_rhiRender->pixelSize(), QImage::Format_RGBA8888_Premultiplied);
		img.fill(Qt::transparent);
		QPainter painter(&img);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

		GLPainter glpainter(&painter, nullptr);

		m_Widget.DrawWidgets(&glpainter);
		painter.end();

		float H = (float)img.height();
		float h = (float)triad->h();
		QRhiViewport vp = { (float)triad->x(), H - (float)triad->y() - h, (float)triad->w(), h };
		quatd q = cam.GetOrientation();
		QMatrix4x4 Q; Q.rotate(QQuaternion(q.w, q.x, q.y, q.z));
		m_rhiRender->setTriadInfo(Q, vp);

		m_rhiRender->setOverlayImage(img);
	}

	m_rhiRender->finish();
}

void rhiSceneView::mousePressEvent(QMouseEvent* ev)
{
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(m_scene);
	bool useOverlay = false;
	if (RhiScene) useOverlay = RhiScene->renderOverlay;

	m_prevPos = ev->pos();

	// let the widget manager handle it first
	int x = m_prevPos.x();
	int y = m_prevPos.y();
	GLWidget* pw = GLWidget::get_focus();
	if (useOverlay && m_Widget.handle(x, y, GLWEvent::GLW_PUSH) == 1)
	{
		requestUpdate();
		return;
	}

	setRenderMode(RenderMode::DYNAMIC);
}

void rhiSceneView::mouseMoveEvent(QMouseEvent* ev)
{
	if (m_scene == nullptr) return;
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(m_scene);
	bool useOverlay = false;
	if (RhiScene) useOverlay = RhiScene->renderOverlay;

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

	// let the widget manager handle it first
	if (but1 && useOverlay && (m_Widget.handle(x1, y1, GLWEvent::GLW_DRAG) == 1))
	{
		requestUpdate();
		return;
	}

	GLCamera& cam = m_scene->GetCamera();

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
	rhiScene* RhiScene = dynamic_cast<rhiScene*>(m_scene);
	bool useOverlay = false;
	if (RhiScene) useOverlay = RhiScene->renderOverlay;

	int x = (int)event->position().x();
	int y = (int)event->position().y();

	// let the widget manager handle it first
	if (useOverlay && m_Widget.handle(x, y, GLWEvent::GLW_RELEASE) == 1)
	{
		requestUpdate();
		return;
	}

	setRenderMode(RenderMode::STATIC);
}

void rhiSceneView::onFrameFinished()
{
	// release any resources that are no longer needed
	m_rhiRender->clearUnusedCache();
}
