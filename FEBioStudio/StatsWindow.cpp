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
#include <QGridLayout>
#include <QLineEdit>
#include <PostLib/constants.h>
#include <PostGL/GLDataMap.h>
#include <PostLib/GLModel.h>
#include <PostGL/GLPlaneCutPlot.h>
#include "PostDocument.h"

class CStatsWidget : public QWidget
{
	QLineEdit* m_min;
	QLineEdit* m_max;
	QLineEdit* m_avg;
	QLineEdit* m_std;
	QLineEdit* m_ste;

public:
	CStatsWidget(QWidget* parent) : QWidget(parent)
	{
		QGridLayout* l = new QGridLayout;
		l->addWidget(new QLabel("Min:"), 0, 0); l->addWidget(m_min = new QLineEdit, 0, 1);
		l->addWidget(new QLabel("Max:"), 0, 2); l->addWidget(m_max = new QLineEdit, 0, 3);

		l->addWidget(new QLabel("Mean:"), 1, 0); l->addWidget(m_avg = new QLineEdit, 1, 1);
		l->addWidget(new QLabel("std. dev.:"), 1, 2); l->addWidget(m_std = new QLineEdit, 1, 3);
		l->addWidget(new QLabel("std. err.:"), 1, 4); l->addWidget(m_ste = new QLineEdit, 1, 5);

		m_min->setReadOnly(true);
		m_max->setReadOnly(true);
		m_avg->setReadOnly(true);
		m_std->setReadOnly(true);
		m_ste->setReadOnly(true);

		setLayout(l);

		setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	}

	void SetMin(double v) { m_min->setText(QString::number(v)); }
	void SetMax(double v) { m_max->setText(QString::number(v)); }
	void SetAvg(double v) { m_avg->setText(QString::number(v)); }
	void SetStd(double v) { m_std->setText(QString::number(v)); }
	void SetSte(double v) { m_ste->setText(QString::number(v)); }
};

CStatsWindow::CStatsWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString wndTitle = windowTitle();
	wndTitle += ":Stats";
	if (postDoc) wndTitle += QString(" [%1]").arg(QString::fromStdString(postDoc->GetDocTitle()));
	setWindowTitle(wndTitle);

	AddPanel(w = new CStatsWidget(this));

	GetPlotWidget()->showLegend(false);

	setMinimumWidth(500);
	resize(600, 500);
	GetPlotWidget()->setChartStyle(ChartStyle::BARCHART_PLOT);
}

void CStatsWindow::Update(bool breset, bool bfit)
{
	CPostDocument* doc = GetPostDoc();
	if (doc->IsValid() == false) return;

	Post::FEState* ps = doc->GetGLModel()->GetActiveState();

	Post::FEPostMesh* pm = doc->GetFSModel()->GetFEMesh(0);

	bool belemfield = IS_ELEM_FIELD(doc->GetEvalField());

	// collect the data
	std::vector<double> data;

	if (belemfield)
	{
		data.reserve(pm->Elements());
		for (int i = 0; i < pm->Elements(); ++i)
		{
			if (pm->ElementRef(i).IsEnabled())
			{
				double v = ps->m_ELEM[i].m_val;
				data.push_back(v);
			}
		}
	}
	else
	{
		data.reserve(pm->Nodes());
		for (int i = 0; i < pm->Nodes(); ++i)
		{
			if (pm->Node(i).IsEnabled())
			{
				double v = ps->m_NODE[i].m_val;
				data.push_back(v);
			}
		}
	}
	size_t N = data.size();
	if (N == 0) return;

	// evaluate the statistics
	double minv = data[0], maxv = data[0], sum = 0, sum2 = 0;
	for (double v : data)
	{
		if (v < minv) minv = v;
		if (v > maxv) maxv = v;

		sum += v;
		sum2 += v*v;
	}

	double M = (double)N;
	double avg = sum / M;
	double std = sqrt((sum2 - sum * sum / M) / M);
	double ste = std / M;

	// bin the data
	int nbins = (int)sqrt((double)N);
	std::vector<int> bin(nbins, 0);
	double D = (minv != maxv ? 1.0 / (maxv - minv) : 1.0);
	for (double v : data)
	{
		int n = (int) ((nbins-1)*((v - minv)*D));
		if ((n>=0) && (n<nbins)) bin[n]++;
	}

	// output new stats
	w->SetMin(minv);
	w->SetMax(maxv);
	w->SetAvg(avg);
	w->SetStd(std);
	w->SetSte(ste);

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
