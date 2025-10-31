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

	GLScene* scene = GetActiveScene();

	// Render the 3D scene
	PrepScene();
	RenderBackground();
	RenderScene(*m_ogl);

	// finish up
	m_ogl->finish();

	// if the camera is animating, we need to redraw
	if (scene && scene->GetCamera().IsAnimating())
	{
		scene->GetCamera().Update();
		QTimer::singleShot(50, this, SLOT(repaint()));
	}
}

void CGLSceneView::RenderScene(GLRenderEngine& re)
{
	GLScene* scene = GetActiveScene();
	if (scene)
	{
		GLContext rc;
		rc.m_cam = &scene->GetCamera();
		rc.m_settings = GetViewSettings();
		scene->Render(re, rc);
	}
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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

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
