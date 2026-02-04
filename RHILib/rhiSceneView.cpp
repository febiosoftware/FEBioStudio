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
#include "rhiSceneView.h"
#include <QPainter>
#include <QFile>
#include <QWidget>
#include <FEBioStudio/MainWindow.h>
#include <GLLib/GLContext.h>
#include <QMouseEvent>
#include <QTimer>

rhiSceneView::rhiSceneView()
{
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

void rhiSceneView::customInit()
{
	m_rhiRender = new rhiRenderer(m_rhi.get(), m_sc.get(), m_rp.get());
	m_rhiRender->setDPR(devicePixelRatio());
	m_rhiRender->init();
}

void rhiSceneView::customRender()
{
	m_rhiRender->start();
	m_rhiRender->useOverlayImage(false);

	RenderScene(*m_rhiRender);

	m_rhiRender->finish();
}

void rhiSceneView::onFrameFinished()
{
	// release any resources that are no longer needed
	m_rhiRender->clearUnusedCache();

	// if the camera is animating, we need to redraw
	GLScene* scene = GetActiveScene();
	if ((renderMode() == RenderMode::STATIC) && scene && scene->GetCamera().IsAnimating())
	{
		scene->GetCamera().Update();
		QTimer::singleShot(50, this, &rhiSceneView::requestUpdate);
	}
}

void rhiSceneView::RenderScene(GLRenderEngine& re)
{
	if (m_scene)
	{
		GLContext rc;
		rc.m_cam = &m_scene->GetCamera();
		m_scene->Render(*m_rhiRender, rc);
	}
}

void rhiSceneView::ShowFPS(bool b)
{
	if (m_rhiRender) m_rhiRender->showFPS(b);
}

void rhiSceneView::mousePressEvent(QMouseEvent* ev)
{
	m_prevPos = ev->pos();

	setRenderMode(RenderMode::DYNAMIC);

	ev->accept();
}

void rhiSceneView::mouseMoveEvent(QMouseEvent* ev)
{
	if (m_scene == nullptr) return;

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
	int x = (int)event->position().x();
	int y = (int)event->position().y();

	setRenderMode(RenderMode::STATIC);

	event->accept();
}
