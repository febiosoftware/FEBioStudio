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
#include "GLPlot.h"
#include <FSCore/FSObjectList.h>

namespace Post {

class GLPlotGroup : public CGLPlot
{
public:
	GLPlotGroup();

	void Render(GLRenderEngine& re, GLContext& rc) override;

	void Update() override;
	void Update(int ntime, float dt, bool breset) override;

	bool UpdateData(bool bsave = true) override;

public:
	size_t Plots() const { return m_plot.Size(); }
	CGLPlot* GetPlot(size_t i) { return m_plot[i]; }

	void AddPlot(CGLPlot* plt, bool update = true);
	void RemovePlot(CGLPlot* plt);

	void MovePlotUp(Post::CGLPlot* plot);
	void MovePlotDown(Post::CGLPlot* plot);

public:
	bool Intersects(Ray& ray, Intersection& q) override;
	FESelection* SelectComponent(int index) override;
	void ClearSelection() override;

private:
	FSObjectList<Post::CGLPlot>	m_plot;
};

}
