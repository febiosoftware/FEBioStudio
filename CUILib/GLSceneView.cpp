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
#include "stdafx.h"
#include "GLSceneView.h"
#include <GLLib/GLScene.h>
#include <GLLib/GLContext.h>
#include <QMouseEvent>
#include <QTimer>
#include <QPainter>

GLViewSettings	CGLSceneView::m_view;

CGLSceneView::CGLSceneView(QWidget* parent) : QOpenGLWidget(parent)
{
	QSurfaceFormat fmt = format();
	fmt.setSamples(16);
	setFormat(fmt);

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_AcceptTouchEvents, true);
	setMouseTracking(true);

	m_ox = m_oy = 1;
}

GLScene* CGLSceneView::GetActiveScene()
{
	return nullptr;
}

void CGLSceneView::initializeGL()
{
	if (m_ogl == nullptr) m_ogl = new OpenGLRenderer;
	m_ogl->init();
}

void CGLSceneView::RenderBackground()
{
	GLScene* scene = GetActiveScene();
	if (scene == nullptr)
	{
		glClearColor(.2f, .2f, .2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		return;
	}

	// set up the viewport
	m_ogl->pushProjection();
	m_ogl->setOrthoProjection(-1, 1, -1, 1, -1, 1);

	m_ogl->pushTransform();
	m_ogl->resetTransform();

	m_ogl->pushState();
	m_ogl->disable(GLRenderEngine::DEPTHTEST);
	m_ogl->disable(GLRenderEngine::LIGHTING);
	m_ogl->disable(GLRenderEngine::CULLFACE);

	GLViewSettings& view = GetViewSettings();

	GLColor c[4];

	switch (view.m_nbgstyle)
	{
	case BG_COLOR1:
		c[0] = c[1] = c[2] = c[3] = view.m_col1; break;
	case BG_COLOR2:
		c[0] = c[1] = c[2] = c[3] = view.m_col2; break;
	case BG_HORIZONTAL:
		c[0] = c[1] = view.m_col2;
		c[2] = c[3] = view.m_col1;
		break;
	case BG_VERTICAL:
		c[0] = c[3] = view.m_col1;
		c[1] = c[2] = view.m_col2;
		break;
	}

	m_ogl->begin(GLRenderEngine::QUADS);
	{
		m_ogl->setColor(c[0]); m_ogl->vertex(vec3d(-1, -1, 0));
		m_ogl->setColor(c[1]); m_ogl->vertex(vec3d( 1, -1, 0));
		m_ogl->setColor(c[2]); m_ogl->vertex(vec3d( 1,  1, 0));
		m_ogl->setColor(c[3]); m_ogl->vertex(vec3d(-1,  1, 0));
	}
	m_ogl->end();

	m_ogl->popState();
	m_ogl->popTransform();
	m_ogl->popProjection();
}

void CGLSceneView::paintGL()
{
	assert(m_ogl);
	if (m_ogl == nullptr) return;

	// start rendering
	m_ogl->start();

	// Render the 3D scene
	PrepScene();
	RenderBackground();
	RenderScene();
	RenderCanvas();

	// finish up
	m_ogl->finish();

	// if the camera is animating, we need to redraw
	GLScene* scene = GetActiveScene();
	if (scene && scene->GetCamera().IsAnimating())
	{
		scene->GetCamera().Update();
		QTimer::singleShot(50, this, SLOT(repaint()));
	}
}

void CGLSceneView::RenderScene()
{
	GLScene* scene = GetActiveScene();
	if (scene)
	{
		GLContext rc;
		rc.m_cam = &scene->GetCamera();
		rc.m_settings = GetViewSettings();
		scene->Render(*m_ogl, rc);
	}
}

void CGLSceneView::RenderCanvas()
{
	GLScene* scene = GetActiveScene();
	if (scene)
	{
		GLCamera& cam = scene->GetCamera();
		GLContext rc;
		rc.m_cam = &cam;
		rc.m_settings = GetViewSettings();

		// set the projection Matrix to ortho2d so we can draw some stuff on the screen
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width(), height(), 0, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// We must turn off culling before we use the QPainter, otherwise
		// drawing using QPainter doesn't work correctly.
		glDisable(GL_CULL_FACE);

		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
		scene->RenderCanvas(painter, rc);
		painter.end();
	}
}

// setup the projection matrix
void CGLSceneView::SetupProjection()
{
	// set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	BOX box = scene->GetBoundingBox();

	double R = box.Radius();
	GLViewSettings& vs = GetViewSettings();

	GLCamera& cam = scene->GetCamera();
	vec3d p = cam.GlobalPosition();
	vec3d c = box.Center();
	double L = (c - p).Length();

	double ffar = (L + R) * 2;
	double fnear = 0.01 * ffar;
	double fov = cam.GetFOV();

	double D = 0.5 * cam.GetFinalTargetDistance();
	if ((D > 0) && (D < fnear)) fnear = D;

	cam.SetNearPlane(fnear);
	cam.SetFarPlane(ffar);

	double ar = 1;
	if (height() == 0) ar = 1; ar = (GLfloat)width() / (GLfloat)height();

	// set up projection matrix
	if (cam.IsOrtho())
	{
		// orthographic projection
		GLdouble f = 0.35 * cam.GetTargetDistance();
		m_ox = f * ar;
		m_oy = f;
		glOrtho(-m_ox, m_ox, -m_oy, m_oy, fnear, ffar);
	}
	else
	{
		// perspective projection
		GLdouble f = 1.0 / tan(fov * M_PI / 360.0);
		glFrustum(-fnear * tan(fov * M_PI / 360.0) * ar,
			 fnear * tan(fov * M_PI / 360.0) * ar,
			-fnear * tan(fov * M_PI / 360.0),
			 fnear * tan(fov * M_PI / 360.0),
			 fnear, ffar);
	}
}

void CGLSceneView::GetViewport(int vp[4]) const
{
	vp[0] = m_viewport[0];
	vp[1] = m_viewport[1];
	vp[2] = m_viewport[2];
	vp[3] = m_viewport[3];
}

GLCamera* CGLSceneView::GetCamera()
{
	GLScene* scene = GetActiveScene();
	if (scene) return &(scene->GetCamera());
	else return nullptr;
}

void CGLSceneView::PrepScene()
{
	GLfloat specular[] = { 1.f, 1.f, 1.f, 1.f };

	// store the viewport dimensions
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// setup projection
	SetupProjection();

	// reset the modelview matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// clear the model
	glClearColor(.0f, .0f, .0f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// set material properties
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 32);

	GLViewSettings& view = GetViewSettings();

	// set the line width
	glLineWidth(view.m_line_size);

	// turn on/off lighting
	if (view.m_bLighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	GLfloat d = view.m_diffuse;
	GLfloat dv[4] = { d, d, d, 1.f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dv);

	// set the ambient lighting intensity
	GLfloat f = view.m_ambient;
	GLfloat av[4] = { f, f, f, 1.f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, av);

	// position the light
	vec3f lp = m_view.m_light; lp.Normalize();
	m_ogl->setLightPosition(0, lp);

	GLScene* scene = GetActiveScene();
	if (scene)
	{
		GLCamera& cam = scene->GetCamera();
		cam.MakeActive();
	}
}

void CGLSceneView::mousePressEvent(QMouseEvent* ev)
{
	GLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	m_prevPos = ev->pos();
}

void CGLSceneView::mouseMoveEvent(QMouseEvent* ev)
{
	GLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	GLCamera& cam = scene->GetCamera();

	bool bshift = (ev->modifiers() & Qt::ShiftModifier   ? true : false);
	bool bctrl  = (ev->modifiers() & Qt::ControlModifier ? true : false);
	bool balt   = (ev->modifiers() & Qt::AltModifier     ? true : false);

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
			if (y0 > y1) cam.Zoom(0.95f);
			if (y0 < y1) cam.Zoom(1.0f / 0.95f);
		}
	}
	else return;

	repaint();

	m_prevPos = ev->pos();

	cam.Update(true);

	ev->accept();
}

void CGLSceneView::mouseReleaseEvent(QMouseEvent* ev)
{
	GLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

}

void CGLSceneView::wheelEvent(QWheelEvent* ev)
{
	GLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	GLCamera& cam = scene->GetCamera();

	Qt::KeyboardModifiers key = ev->modifiers();
	bool balt = (key & Qt::AltModifier);
	Qt::MouseEventSource eventSource = ev->source();
	if (eventSource == Qt::MouseEventSource::MouseEventNotSynthesized)
	{
		int y = ev->angleDelta().y();
		if (y == 0) y = ev->angleDelta().x();
		if (balt && GetViewSettings().m_bselbrush)
		{
			float& R = GetViewSettings().m_brushSize;
			if (y < 0) R -= 2.f;
			if (y > 0) R += 2.f;
			if (R < 2.f) R = 1.f;
			if (R > 500.f) R = 500.f;
		}
		else
		{
			if (y > 0) cam.Zoom(0.95f);
			if (y < 0) cam.Zoom(1.0f / 0.95f);
		}
	}
	else
	{
		if (balt) 
		{
			int y = ev->angleDelta().y();
			if (y > 0) cam.Zoom(0.95f);
			if (y < 0) cam.Zoom(1.0f / 0.95f);
		}
		else 
		{
			int dx = ev->pixelDelta().x();
			int dy = ev->pixelDelta().y();
			vec3d r = vec3d(-dx, dy, 0.f);
			cam.PanView(r);
		}
	}

	cam.Update(true);
	ev->accept();
	repaint();
}

void CGLSceneView::ScreenToView(int x, int y, double& fx, double& fy)
{
	GLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	double W = (double)width();
	double H = (double)height();

	if (H == 0.f) H = 0.001f;

	GLCamera& cam = scene->GetCamera();

	double fnear = cam.GetNearPlane();
	double fov = cam.GetFOV();


	double ar = W / H;

	double fh = 2.f * fnear * (double)tan(0.5 * fov * PI / 180);
	double fw = fh * ar;

	fx = -fw / 2 + x * fw / W;
	fy = fh / 2 - y * fh / H;
}
