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
#include <GeomLib/GObject.h>

CGLScene::CGLScene() 
{
	m_envtex = 0;
}

CGLScene::~CGLScene() 
{
	clear();
}

CGView& CGLScene::GetView() { return m_view; }

void CGLScene::Render(GLRenderEngine& engine, CGLContext& rc)
{
	engine.pushState();

	CGLScene& scene = *this;
	for (GLSceneItem* item : scene)
	{
		assert(item);
		item->render(engine, rc);
	}

	engine.popState();
}

void CGLScene::Update()
{

}

void CGLScene::ActivateEnvironmentMap(GLRenderEngine& re)
{
	if (m_envtex == 0) LoadEnvironmentMap(re);
	if (m_envtex == 0) return;
	re.ActivateEnvironmentMap(m_envtex);
}

void CGLScene::DeactivateEnvironmentMap(GLRenderEngine& re)
{
	if (m_envtex == 0) return;
	re.DeactivateEnvironmentMap(m_envtex);
}

void CGLScene::LoadEnvironmentMap(GLRenderEngine& re)
{
	if (m_envtex != 0) return;
	if (m_envMap.isEmpty()) return;
	m_envtex = re.LoadEnvironmentMap(m_envMap.toStdString());
}

// this function will only adjust the camera if the currently
// selected object is too close.
void CGLScene::ZoomSelection(bool forceZoom)
{
	// get the selection's bounding box
	BOX box = GetSelectionBox();
	if (box.IsValid())
	{
		double f = box.GetMaxExtent();
		if (f < 1.0e-8) f = 1.0;

		CGLCamera& cam = GetCamera();

		double g = cam.GetFinalTargetDistance();
		if ((forceZoom == true) || (g < 2.0 * f))
		{
			cam.SetTarget(box.Center());
			cam.SetTargetDistance(2.0 * f);
		}
	}
	else ZoomExtents();
}

void CGLScene::ZoomExtents(bool banimate)
{
	BOX box = GetBoundingBox();

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0 * f);

	if (banimate == false) cam.Update(true);
}

//! zoom in on a box
void CGLScene::ZoomTo(const BOX& box)
{
	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0 * f);
}

void CGLScene::ZoomToObject(GObject* po)
{
	BOX box = po->GetGlobalBox();

	double f = box.GetMaxExtent();
	if (f == 0) f = 1;

	CGLCamera& cam = GetCamera();

	cam.SetTarget(box.Center());
	cam.SetTargetDistance(2.0 * f);
	cam.SetOrientation(po->GetRenderTransform().GetRotationInverse());
}

void CGLScene::clear()
{
	for (GLSceneItem* item : m_Items) delete item;
	m_Items.clear();
}
