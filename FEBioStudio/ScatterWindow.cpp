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
#include "ScatterWindow.h"
#include "PostDocument.h"
#include "DataFieldSelector.h"
#include <PostLib/constants.h>

static inline double frand()
{
	return (double)rand() / (double)RAND_MAX;
}

CScatterWindow::CScatterWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	m_xprev = -1;
	m_yprev = -1;
	GetPlotWidget()->setChartStyle(ChartStyle::DENSITY_PLOT);
	GetPlotWidget()->showLegend(false);
	CPlotData* d = new CPlotData;
	AddPlotData(d);
}

void CScatterWindow::Update(bool breset, bool bfit)
{
	CPostDocument* doc = GetPostDoc();
	if (breset)
	{
		if (doc->IsValid())
		{
			SetXDataSelector(new CModelDataSelector(doc->GetFSModel(), Post::TENSOR_SCALAR));
			SetYDataSelector(new CModelDataSelector(doc->GetFSModel(), Post::TENSOR_SCALAR));
		}
		else return;
	}

	int xdata = GetCurrentXValue();
	int ydata = GetCurrentYValue();
	if ((xdata <= 0) || (ydata <= 0)) return;

	// get the title
	QString xtext = GetCurrentXText();
	QString ytext = GetCurrentYText();

	SetXAxisLabel(xtext);
	SetYAxisLabel(ytext);
	SetPlotTitle(QString("%1 --- %2").arg(xtext).arg(ytext));

	CPlotData* plt = GetPlotData(0);
	plt->clear();

	Post::FEPostModel& fem = *doc->GetFSModel();
	Post::FEState* state = fem.CurrentState();
	FSMesh* mesh = state->GetFEMesh();
	int ntime = fem.CurrentTimeIndex();
	if (IS_NODE_FIELD(xdata))
	{
		for (int i = 0; i < mesh->Nodes(); ++i)
		{
			Post::NODEDATA dx, dy;
			fem.EvaluateNode(i, ntime, xdata, dx);
			fem.EvaluateNode(i, ntime, ydata, dy);
			if (dx.m_ntag && dy.m_ntag)
				plt->addPoint(dx.m_val, dy.m_val);
		}
	}
	else if (IS_ELEM_FIELD(xdata))
	{
		float dummy[FSElement::MAX_NODES];
		for (int i = 0; i < mesh->Elements(); ++i)
		{
			float xval, yval;
			if (fem.EvaluateElement(i, ntime, xdata, dummy, xval) &&
				fem.EvaluateElement(i, ntime, ydata, dummy, yval))
			{
				plt->addPoint(xval, yval);
			}
		}
	}
	else if (IS_FACE_FIELD(xdata))
	{
		float dummy[FSElement::MAX_NODES];
		for (int i = 0; i < mesh->Faces(); ++i)
		{
			float xval, yval;
			if (fem.EvaluateFace(i, ntime, xdata, dummy, xval) &&
				fem.EvaluateFace(i, ntime, ydata, dummy, yval))
			{
				plt->addPoint(xval, yval);
			}
		}
	}
	if (breset || (m_xprev != xdata) || (m_yprev != ydata))
		FitPlotsToData();

	m_xprev = xdata;
	m_yprev = ydata;

	UpdatePlots();
}
