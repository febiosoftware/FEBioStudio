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

class CFEBioMonitorView::Ui 
{
public:
	CPlotWidget* plot;

public:
	void setup(CFEBioMonitorView* view)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);
		l->addWidget(plot = new CPlotWidget);
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
