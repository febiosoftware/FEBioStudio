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
#include <GL/glew.h>
#include "GLSceneView.h"
#include "GLScene.h"
#include <GLLib/GLContext.h>
#include <QMouseEvent>
#include <QTimer>

static bool initGlew = false;

CGLSceneView::CGLSceneView(QWidget* parent) : QOpenGLWidget(parent)
{
	QSurfaceFormat fmt = format();
	//	fmt.setSamples(4);
	//	setFormat(fmt);

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_AcceptTouchEvents, true);
	setMouseTracking(true);

	m_view.Defaults();

	m_ox = m_oy = 1;
	m_light = vec3f(0.5, 0.5, 1);
}

CGLScene* CGLSceneView::GetActiveScene()
{
	return nullptr;
}

void CGLSceneView::initializeGL()
{
	GLfloat amb1[] = { .09f, .09f, .09f, 1.f };
	GLfloat dif1[] = { .8f, .8f, .8f, 1.f };

	//	GLfloat amb2[] = {.0f, .0f, .0f, 1.f};
	//	GLfloat dif2[] = {.3f, .3f, .4f, 1.f};

	if (initGlew == false)
	{
		GLenum err = glewInit();
		if (err != GLEW_OK)
		{
			const char* szerr = (const char*)glewGetErrorString(err);
			assert(err == GLEW_OK);
		}
		initGlew = true;
	}

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);

	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	glShadeModel(GL_FLAT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.5f);

	// enable lighting and set default options
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif1);

	glEnable(GL_POLYGON_OFFSET_FILL);

	//	glEnable(GL_LIGHT1);
	//	glLightfv(GL_LIGHT1, GL_AMBIENT, amb2);
	//	glLightfv(GL_LIGHT1, GL_DIFFUSE, dif2);

	// enable color tracking for diffuse color of materials
//	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glPointSize(7.0f);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

void CGLSceneView::RenderBackground()
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr)
	{
		glClearColor(.2f, .2f, .2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		return;
	}

	// set up the viewport
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

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

	glBegin(GL_QUADS);
	{
		glColor3ub(c[0].r, c[0].g, c[0].b); glVertex2f(-1, -1);
		glColor3ub(c[1].r, c[1].g, c[1].b); glVertex2f(1, -1);
		glColor3ub(c[2].r, c[2].g, c[2].b); glVertex2f(1, 1);
		glColor3ub(c[3].r, c[3].g, c[3].b); glVertex2f(-1, 1);
	}
	glEnd();

	glPopAttrib();

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}

void CGLSceneView::paintGL()
{
	PrepScene();
	RenderBackground();
	RenderScene();

	// if the camera is animating, we need to redraw
	CGLScene* scene = GetActiveScene();
	if (scene && scene->GetCamera().IsAnimating())
	{
		scene->GetCamera().Update();
		QTimer::singleShot(50, this, SLOT(repaint()));
	}
}

void CGLSceneView::RenderScene()
{
	CGLScene* scene = GetActiveScene();
	if (scene)
	{
		CGLCamera& cam = scene->GetCamera();
		cam.PositionInScene();

		CGLContext rc;
		rc.m_cam = &cam;
		rc.m_settings = GetViewSettings();
		rc.m_view = this;
		scene->Render(rc);
	}
}

// setup the projection matrix
void CGLSceneView::SetupProjection()
{
	// set up the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	BOX box = scene->GetBoundingBox();

	CGView& view = scene->GetView();
	CGLCamera& cam = view.GetCamera();

	double R = box.Radius();
	GLViewSettings& vs = GetViewSettings();

	vec3d p = cam.GlobalPosition();
	vec3d c = box.Center();
	double L = (c - p).Length();

	view.m_ffar = (L + R) * 2;
	view.m_fnear = 0.01f * view.m_ffar;

	double D = 0.5 * cam.GetFinalTargetDistance();
	if ((D > 0) && (D < view.m_fnear)) view.m_fnear = D;

	if (height() == 0) view.m_ar = 1; view.m_ar = (GLfloat)width() / (GLfloat)height();

	// set up projection matrix
	if (view.m_bortho)
	{
		GLdouble f = 0.35 * cam.GetTargetDistance();
		m_ox = f * view.m_ar;
		m_oy = f;
		glOrtho(-m_ox, m_ox, -m_oy, m_oy, view.m_fnear, view.m_ffar);
	}
	else
	{
		gluPerspective(view.m_fov, view.m_ar, view.m_fnear, view.m_ffar);
	}
}

void CGLSceneView::GetViewport(int vp[4]) const
{
	vp[0] = m_viewport[0];
	vp[1] = m_viewport[1];
	vp[2] = m_viewport[2];
	vp[3] = m_viewport[3];
}

CGView* CGLSceneView::GetView()
{
	CGLScene* scene = GetActiveScene();
	if (scene) return &(scene->GetView());
	else return nullptr;
}

CGLCamera* CGLSceneView::GetCamera()
{
	CGLScene* scene = GetActiveScene();
	if (scene) return &(scene->GetView().GetCamera());
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
	vec3f lp = GetLightPosition(); lp.Normalize();
	GLfloat fv[4] = { 0 };
	fv[0] = lp.x; fv[1] = lp.y; fv[2] = lp.z;
	glLightfv(GL_LIGHT0, GL_POSITION, fv);
}

void CGLSceneView::mousePressEvent(QMouseEvent* ev)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	m_prevPos = ev->pos();
}

void CGLSceneView::mouseMoveEvent(QMouseEvent* ev)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	CGLCamera& cam = scene->GetCamera();

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
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

}

void CGLSceneView::wheelEvent(QWheelEvent* ev)
{
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	CGLCamera& cam = scene->GetView().GetCamera();

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
	CGLScene* scene = GetActiveScene();
	if (scene == nullptr) return;

	double W = (double)width();
	double H = (double)height();

	if (H == 0.f) H = 0.001f;

	CGView& view = scene->GetView();

	double ar = W / H;

	double fh = 2.f * view.m_fnear * (double)tan(0.5 * view.m_fov * PI / 180);
	double fw = fh * ar;

	fx = -fw / 2 + x * fw / W;
	fy = fh / 2 - y * fh / H;
}

CGLManagedSceneView::CGLManagedSceneView(CGLScene* scene, QWidget* parent) : CGLSceneView(parent), m_scene(scene) 
{
	if (scene)
	{
		BOX box = scene->GetBoundingBox();
		GetCamera()->SetTargetDistance(box.GetMaxExtent()*1.5);
	}
}

CGLManagedSceneView::~CGLManagedSceneView() { delete m_scene; }
