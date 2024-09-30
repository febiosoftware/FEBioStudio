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
#include "GLPlot.h"
#include "GLPlotGroup.h"
#include <GLWLib/GLWidgetManager.h>
#include <GLWLib/GLLegendBar.h>
using namespace Post;

CGLPlot::CGLPlot(CGLModel* po) : CGLVisual(po)
{
	m_renderOrder = 0;
	m_pgroup = nullptr;
}

CGLPlot::~CGLPlot()
{
}

void CGLPlot::Reload()
{

}

void CGLPlot::SetGroup(GLPlotGroup* pg) { m_pgroup = pg; }

GLPlotGroup* CGLPlot::GetGroup() { return m_pgroup; }

bool CGLPlot::Intersects(Ray& ray, Intersection& q) { return false; }

FESelection* CGLPlot::SelectComponent(int index) { return nullptr; }

void CGLPlot::ClearSelection() {}

void CGLPlot::UpdateTexture() 
{

}

void CGLPlot::SetRenderOrder(int renderOrder)
{
	m_renderOrder = renderOrder;
}

int CGLPlot::GetRenderOrder() const
{
	return m_renderOrder;
}

CGLLegendPlot::CGLLegendPlot()
{
	m_pbar = nullptr;
}

CGLLegendPlot::~CGLLegendPlot()
{
	SetLegendBar(nullptr);
}

void CGLLegendPlot::SetLegendBar(GLLegendBar* bar)
{
	if (m_pbar)
	{
		CGLWidgetManager::GetInstance()->RemoveWidget(m_pbar);
		delete m_pbar;
	}

	m_pbar = bar;
	if (m_pbar) CGLWidgetManager::GetInstance()->AddWidget(m_pbar);
}

GLLegendBar* CGLLegendPlot::GetLegendBar()
{
	return m_pbar;
}

void CGLLegendPlot::ChangeName(const std::string& name)
{
	CGLObject::SetName(name);
	if (m_pbar) m_pbar->copy_label(name.c_str());
}

bool CGLLegendPlot::ShowLegend() const
{ 
	return (m_pbar ? m_pbar->visible() : false);
}

void CGLLegendPlot::ShowLegend(bool b) 
{ 
	if (m_pbar)
	{
		if (b) m_pbar->show(); else m_pbar->hide();
	}
}

void CGLLegendPlot::Activate(bool b)
{
	CGLPlot::Activate(b);
	if (b) ShowLegend(true); else ShowLegend(false);
}
