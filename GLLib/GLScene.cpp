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
GLScene::GLScene() 
{
	m_envtex = 0;
}

GLScene::~GLScene() 
{
	clear();
}

CGView& GLScene::GetView() { return m_view; }

void GLScene::Render(GLRenderEngine& engine, GLContext& rc)
{
	engine.pushState();

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

// this function will only adjust the camera if the currently
// selected object is too close.
void GLScene::ZoomSelection(bool forceZoom)
{
	// get the selection's bounding box
	BOX box = GetSelectionBox();
	if (box.IsValid())
	{
		double f = box.GetMaxExtent();
		if (f < 1.0e-8) f = 1.0;

		GLCamera& cam = GetCamera();

		double g = cam.GetFinalTargetDistance();
		if ((forceZoom == true) || (g < 2.0 * f))
		{
			cam.SetTarget(box.Center());
			cam.SetTargetDistance(2.0 * f);
		}
	}
	else ZoomExtents();
}

void GLScene::ZoomExtents(bool banimate)
{
	BOX box = GetBoundingBox();

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	GLCamera& cam = GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0 * f);

	if (banimate == false) cam.Update(true);
}

//! zoom in on a box
void GLScene::ZoomTo(const BOX& box)
{
	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	GLCamera& cam = GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0 * f);
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
