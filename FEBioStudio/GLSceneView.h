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
#pragma once
#include <QOpenGLWidget>
#include <GLLib/GLViewSettings.h>
#include <FSCore/math3d.h>
#include "OpenGLRenderer.h"

class CGLScene;
class CGView;
class CGLCamera;

//! This class is used for rendering CGLScenes
class CGLSceneView : public QOpenGLWidget
{
	Q_OBJECT

public:
	CGLSceneView(QWidget* parent = nullptr);

	GLViewSettings& GetViewSettings() { return m_view; }

	virtual CGLScene* GetActiveScene();
	virtual void RenderScene();
	virtual void RenderCanvas();
	virtual void RenderBackground();

	CGView* GetView();
	CGLCamera* GetCamera();

	//! Setup the projection matrix
	void SetupProjection();

	void GetViewport(int vp[4]) const;

	void ScreenToView(int x, int y, double& fx, double& fy);

protected:
	void initializeGL() override;
	void paintGL() override;

	void PrepScene();

private:
	void mousePressEvent(QMouseEvent* ev) override;
	void mouseMoveEvent(QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void wheelEvent(QWheelEvent* ev) override;

protected:
	GLViewSettings	m_view;
	int	m_viewport[4];		//!< store viewport coordinates
	double	m_ox;
	double	m_oy;

	QPoint m_prevPos;	//!< last mouse position

	OpenGLRenderer	m_ogl;
};

// This class manages its own scene
class CGLManagedSceneView : public CGLSceneView
{
public:
	CGLManagedSceneView(CGLScene* scene, QWidget* parent = nullptr);
	~CGLManagedSceneView();

	CGLScene* GetActiveScene() override { return m_scene; }

private:
	CGLScene* m_scene;
};
