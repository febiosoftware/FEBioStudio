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
#include "GLPlotGroup.h"
#include <MeshLib/Intersect.h>
#include <PostGL/GLModel.h>
#include <sstream>
using namespace std;
using namespace Post;

//=============================================================================
REGISTER_CLASS(GLPlotGroup, CLASS_PLOT, "plot-group", 0);

GLPlotGroup::GLPlotGroup()
{
	static int n = 1;
	SetTypeString("plot-group");
	std::stringstream ss;
	ss << "PlotGroup" << n++;
	SetName(ss.str());
}

void GLPlotGroup::Render(CGLContext& rc)
{
	for (int i = 0; i < m_plot.Size(); ++i)
	{
		if (m_plot[i]->IsActive()) m_plot[i]->Render(rc);
	}
}

void GLPlotGroup::Update()
{
	int n = (int)m_plot.Size();
	if (n == 0) return;

#pragma omp parallel for
	for (int i = 0; i < n; ++i)
	{
		m_plot[i]->Update();
	}
}

void GLPlotGroup::Update(int ntime, float dt, bool breset)
{
	int n = (int)m_plot.Size();
	if (n == 0) return;

#pragma omp parallel for
	for (int i = 0; i < n; ++i)
	{
		m_plot[i]->Update(ntime, dt, breset);
	}
}

bool GLPlotGroup::UpdateData(bool bsave)
{
	int n = (int)m_plot.Size();
	if (n == 0) return false;

#pragma omp parallel for
	for (int i = 0; i < n; ++i)
	{
		m_plot[i]->UpdateData(bsave);
	}

	return false;
}

void GLPlotGroup::AddPlot(CGLPlot* plt, bool update)
{
	plt->SetGroup(this);
	plt->SetModel(GetModel());
	if (update) plt->Update(GetModel()->CurrentTimeIndex(), 0.f, true);
	m_plot.Add(plt);
}

void GLPlotGroup::RemovePlot(CGLPlot* plt)
{
	assert(plt->GetGroup() == this);
	m_plot.Remove(plt);
	plt->SetGroup(nullptr);
}


void GLPlotGroup::MovePlotUp(Post::CGLPlot* plot)
{
	for (size_t i = 1; i < m_plot.Size(); ++i)
	{
		if (m_plot[i] == plot)
		{
			CGLPlot* prv = m_plot[i - 1];
			m_plot.Set(i - 1, plot);
			m_plot.Set(i, prv);
			return;
		}
	}
}

void GLPlotGroup::MovePlotDown(Post::CGLPlot* plot)
{
	for (size_t i = 0; i < m_plot.Size() - 1; ++i)
	{
		if (m_plot[i] == plot)
		{
			CGLPlot* nxt = m_plot[i + 1];
			m_plot.Set(i, nxt);
			m_plot.Set(i + 1, plot);
			return;
		}
	}
}

bool GLPlotGroup::Intersects(Ray& ray, Intersection& q)
{
	for (int i = 0; i < m_plot.Size(); ++i)
	{
		if (m_plot[i]->Intersects(ray, q))
		{
			q.m_index |= (i << 16);
			return true;
		}
	}
	return false;
}

FESelection* GLPlotGroup::SelectComponent(int index)
{
	int path = (index >> 16) & 0xFFFF;
	if ((path >= 0) && (path < m_plot.Size()))
	{
		int point = (index & 0xFFFF);
		return m_plot[path]->SelectComponent(point);
	}
	else return nullptr;
}

void GLPlotGroup::ClearSelection()
{
	for (int i = 0; i < m_plot.Size(); ++i) m_plot[i]->ClearSelection();
}
