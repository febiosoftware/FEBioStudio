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
#include "StatsWindow.h"
#include "MainWindow.h"
#include "Document.h"
#include "PlotWidget.h"
#include "DataFieldSelector.h"
#include <QComboBox>
#include <QToolBar>
#include <QLabel>
#include <QAction>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <PostLib/constants.h>
#include <PostGL/GLDataMap.h>
#include <PostGL/GLModel.h>
#include <PostGL/GLPlaneCutPlot.h>
#include "PostDocument.h"

CStatsWindow::CStatsWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString title = "FEBioStudio: Statistics";
	setWindowTitle(title);
	setMinimumWidth(500);
	resize(600, 500);
	GetPlotWidget()->setChartStyle(ChartStyle::BARCHART_PLOT);
}

void CStatsWindow::Update(bool breset, bool bfit)
{
	CPostDocument* doc = GetPostDoc();
	if (doc->IsValid() == false) return;

	Post::FEPostMesh* pm = doc->GetFSModel()->GetFEMesh(0);
	int N, i, n;

	bool belemfield = IS_ELEM_FIELD(doc->GetEvalField());

	N = 0;
	if (belemfield)
	{
		for (i=0; i<pm->Elements(); ++i) if (pm->ElementRef(i).IsEnabled()) ++N;
	}
	else
	{
		for (i=0; i<pm->Nodes(); ++i) if (pm->Node(i).IsEnabled()) ++N;
	}

	int nbins = (int) sqrt((double)N);

	std::vector<int> bin(nbins, 0);

	Post::FEState* ps = doc->GetGLModel()->GetActiveState();
	double v, minv = 0, maxv = 0;
	N = 0;
	if (belemfield)
	{
		for (i=0; i<pm->Elements(); ++i)
		{
			FEElement_& elem = pm->ElementRef(i);
			if (elem.IsEnabled())
			{
				v = ps->m_ELEM[i].m_val;
				if (N==0) minv = maxv = v;
				if (v < minv) minv = v;
				if (v > maxv) maxv = v;
				++N;
			}
		}
	}
	else
	{
		for (i=0; i<pm->Nodes(); ++i)
		{	
			FENode& node = pm->Node(i);
			if (node.IsEnabled())
			{
				v = ps->m_NODE[i].m_val;
				if (N==0) minv = maxv = v;
				if (v < minv) minv = v;
				if (v > maxv) maxv = v;
				++N;
			}
		}
	}

	if (minv == maxv) ++maxv;

	if (belemfield)
	{
		for (i=0; i<pm->Elements(); ++i)
		{
			FEElement_& elem = pm->ElementRef(i);
			if (elem.IsEnabled())
			{
				v = ps->m_ELEM[i].m_val;
				n = (int) ((nbins-1)*((v - minv)/(maxv - minv)));
				if ((n>=0) && (n<nbins)) bin[n]++;
			}
		}
	}
	else
	{
		for (i=0; i<pm->Nodes(); ++i)
		{
			FENode& node = pm->Node(i);
			if (node.IsEnabled())
			{
				v = ps->m_NODE[i].m_val;
				n = (int) ((nbins-1)*((v - minv)/(maxv - minv)));
				if ((n>=0) && (n<nbins)) bin[n]++;
			}
		}
	}

	ClearPlots();
	
	if (bin.empty() == false)
	{
		CPlotData* data = new CPlotData;
		data->setLabel("data");
		for (int i=0; i<(int)bin.size(); ++i)
		{
			double x = minv + i*(maxv - minv)/(bin.size() - 1);
			double y = bin[i];
			data->addPoint(x, y);
		}
		AddPlotData(data);

		UpdateSelection(true);
	}

	// redraw
	FitPlotsToData();
	RedrawPlot();

	UpdatePlots();
}

void CStatsWindow::UpdateSelection(bool breset)
{

}
