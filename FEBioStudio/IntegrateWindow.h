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
#include <QMainWindow>
#include <vector>
#include "Document.h"
#include "GraphWindow.h"

class CMainWindow;
class CPlotData;
class FSMesh;

namespace Post {
	class FEState;
	class CGLPlaneCutPlot;
}

class CIntegrateWindow : public CGraphWindow
{
	Q_OBJECT

public:
	CIntegrateWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void Update(bool breset = true, bool bfit = false) override;

private:
	void UpdateSourceOptions();

	void UpdateIntegral();
	void IntegrateSelection(CPlotData& data);
	void IntegratePlaneCut(Post::CGLPlaneCutPlot* pp, CPlotData& data);

	void setDataSource(int n) override { m_nsrc = n; Update(false, true); }

private slots:
	void OnConfigChanged(int i);

private:
	std::vector<Post::CGLPlaneCutPlot*>	m_src;
	int		m_nsrc;
	int		m_nconf;
	bool	m_updating;
};

class CIntegrateSurfaceWindow : public CGraphWindow
{
	Q_OBJECT

public:
	CIntegrateSurfaceWindow(CMainWindow* wnd, CPostDocument* postDoc);

	void Update(bool breset = true, bool bfit = false) override;

private:
	void UpdateIntegral();
	void IntegrateSelection(CPlotData& dataX, CPlotData& dataY, CPlotData& dataZ);

private:
	int		m_nsrc;
	bool	m_updating;
};
