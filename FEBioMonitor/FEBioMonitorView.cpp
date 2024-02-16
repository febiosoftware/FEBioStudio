/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "FEBioMonitorView.h"
#include <QBoxLayout>
#include "../FEBioStudio/PlotWidget.h"
#include "FEBioMonitorDoc.h"
#include <FECore/Callback.h>
#include <QSplitter>
#include <QTimer>
#ifdef WIN32
#include <windows.h>
#include <psapi.h>
#endif

size_t GetProcessMemory()
{
#ifdef WIN32
	PROCESS_MEMORY_COUNTERS memCounters;
	GetProcessMemoryInfo(GetCurrentProcess(), &memCounters, sizeof(memCounters));
	return (size_t)memCounters.WorkingSetSize;
#else
	return 0;
#endif
}

CMemoryWidget::CMemoryWidget(QWidget* parent) : CPlotWidget(parent)
{
	m_duration = 500;
	m_maxPoints = 120;

	CPlotData* pd = new CPlotData();
	pd->setMarkerType(MarkerType::NO_MARKER);
	addPlotData(pd);
	setBackgroundColor(QColor(64, 64, 64));
	setXAxisColor(QColor(255, 255, 255));
	setYAxisColor(QColor(255, 255, 255));
	setViewRect(QRectF(0, 0, m_maxPoints, 1));
	setViewLocked(true);
	setBoxColor(QColor(255, 255, 255));
	showLegend(false);

	QTimer::singleShot(m_duration, this, &CMemoryWidget::onTimer);
}

void CMemoryWidget::onTimer()
{
	static double n = 0;
	CPlotData& data = getPlotData(0);

	size_t memInMb = GetProcessMemory() / (1024*1024);
	if (data.size() < m_maxPoints)
	{
		data.addPoint(n++, memInMb);
	}
	else
	{
		for (int i = 0; i < m_maxPoints-1; ++i)
		{
			QPointF& p0 = data.Point(i);
			QPointF& p1 = data.Point(i + 1);
			p0 = p1;
		}
		data.Point(m_maxPoints - 1) = QPointF(n++, memInMb);
	}

	int N = data.size();
	double minx = data.Point(0).x();
	double miny = 0;
	double maxx = (N < m_maxPoints ? m_maxPoints - 1 : data.Point(m_maxPoints - 1).x());
	double maxy = 0;
	for (int i = 0; i < N; ++i)
		if (data.Point(i).y() > maxy) maxy = data.Point(i).y();
	if (maxy == 0) maxy = 1;

	setViewRect(QRectF(minx, miny, maxx - minx, maxy-miny));
	repaint();

	QTimer::singleShot(m_duration, this, &CMemoryWidget::onTimer);
}

class CFEBioMonitorView::Ui 
{
public:
	CPlotWidget* plot;
	CMemoryWidget* memview;

public:
	void setup(CFEBioMonitorView* view)
	{
		QVBoxLayout* l = new QVBoxLayout;

		QSplitter* splitter = new QSplitter(Qt::Horizontal);
		splitter->addWidget(plot = new CPlotWidget);
		splitter->addWidget(memview = new CMemoryWidget);
		splitter->setStretchFactor(0, 3);
		splitter->setStretchFactor(1, 1);
		l->setContentsMargins(0, 0, 0, 0);
		l->addWidget(splitter);
		view->setLayout(l);
	}
};

CFEBioMonitorView::CFEBioMonitorView(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new CFEBioMonitorView::Ui)
{
	ui->setup(this);
	CPlotData* rdata = new CPlotData; rdata->setLabel("R");
	CPlotData* edata = new CPlotData; edata->setLabel("E");
	CPlotData* udata = new CPlotData; udata->setLabel("D");
	ui->plot->addPlotData(rdata);
	ui->plot->addPlotData(edata);
	ui->plot->addPlotData(udata);
}

void CFEBioMonitorView::Update(bool reset)
{
	FEBioMonitorDoc* doc = dynamic_cast<FEBioMonitorDoc*>(GetDocument());
	if (doc == nullptr) return;
	if (doc->IsRunning() == false) return;

	if (reset) ui->plot->clearData();

	int currentEvent = doc->GetCurrentEvent();

	static int counter = 0;
	static bool addNorm0 = true;
	if (currentEvent == CB_INIT)
	{
		ui->plot->clearData();
		counter = 0; addNorm0 = true;
	}

	if (currentEvent == CB_MINOR_ITERS)
	{
		CPlotData& Rdata = ui->plot->getPlotData(0);
		CPlotData& Edata = ui->plot->getPlotData(1);
		CPlotData& Udata = ui->plot->getPlotData(2);

		FSConvergenceInfo info = doc->GetConvergenceInfo();

		if (addNorm0)
		{
			double R0 = log10(info.m_R0);
			Rdata.addPoint(counter, R0);

			double E0 = log10(info.m_E0);
			Edata.addPoint(counter, E0);

			double U0 = log10(info.m_U0);
			Udata.addPoint(counter, U0);

			addNorm0 = false;
			counter++;
		}

		double Rt = log10(info.m_Rt);
		Rdata.addPoint(counter, Rt);

		double Et = log10(info.m_Et);
		Edata.addPoint(counter, Et);

		double Ut = log10(info.m_Ut);
		Udata.addPoint(counter, Ut);

		ui->plot->OnZoomToFit();
		ui->plot->update();
		counter++;
	}
	else if ((currentEvent == CB_MAJOR_ITERS) || (currentEvent == CB_TIMESTEP_FAILED))
	{
		counter--;
		addNorm0 = true;
	}
}
