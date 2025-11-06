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
#include "GLScene.h"
#include "GLCamera.h"

GLScene::GLScene() 
{
	m_envtex = 0;
}

GLScene::~GLScene() 
{
	clear();
}

void GLScene::Render(GLRenderEngine& engine, GLContext& rc)
{
	engine.pushState();

	PositionCameraInScene(engine);

	GLScene& scene = *this;
	for (GLSceneItem* item : scene)
	{
		assert(item);
		item->render(engine, rc);
	}

	engine.popState();
}

void GLScene::Update()
{

}

void GLScene::SetupProjection(GLRenderEngine& re)
{
	BOX box = GetBoundingBox();

	double R = box.Radius();

	GLCamera& cam = GetCamera();
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

	int W = re.surfaceWidth();
	int H = re.surfaceHeight();

	double ar = 1;
	if (H == 0) ar = 1; ar = (double)W / (double)H;

	// set up projection matrix
	if (cam.IsOrtho())
	{
		// orthographic projection
		double f = 0.35 * cam.GetTargetDistance();
		double ox = f * ar;
		double oy = f;
		re.setOrthoProjection(-ox, ox, -oy, oy, fnear, ffar);
	}
	else
	{
		re.setProjection(fov, fnear, ffar);
	}
}

void GLScene::PositionCameraInScene(GLRenderEngine& re)
{
	GLCamera& cam = m_cam;

	SetupProjection(re);

	re.resetTransform();

	// target in camera coordinates
	vec3d r = cam.Target();

	// position the target in camera coordinates
	re.translate(-r);

	// orient the camera
	re.rotate(cam.m_rot.Value());

	// translate to world coordinates
	re.translate(-cam.GetPosition());
}

void GLScene::ActivateEnvironmentMap(GLRenderEngine& re)
{
	if (m_envtex == 0) LoadEnvironmentMap(re);
	if (m_envtex == 0) return;
	re.ActivateEnvironmentMap(m_envtex);
}

void GLScene::DeactivateEnvironmentMap(GLRenderEngine& re)
{
	if (m_envtex == 0) return;
	re.DeactivateEnvironmentMap(m_envtex);
}

void GLScene::LoadEnvironmentMap(GLRenderEngine& re)
{
	if (m_envtex != 0) return;
	if (m_envMap.empty()) return;
	m_envtex = re.LoadEnvironmentMap(m_envMap);
}

void GLScene::clear()
{
	for (GLSceneItem* item : m_Items) delete item;
	m_Items.clear();
}

GLCompositeSceneItem::GLCompositeSceneItem() {}
GLCompositeSceneItem::~GLCompositeSceneItem()
{
	for (auto item : m_items)
	{
		delete item;
	}
}

void GLCompositeSceneItem::render(GLRenderEngine& re, GLContext& rc)
{
	for (auto item : m_items)
	{
		item->render(re, rc);
	}
}
