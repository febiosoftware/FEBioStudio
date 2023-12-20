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
#include "IntegrateWindow.h"
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
#include <QComboBox>
#include <QBoxLayout>

CIntegrateWindow::CIntegrateWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString wndTitle = windowTitle();
	wndTitle += ":Integrate";
	if (postDoc) wndTitle += QString(" [%1]").arg(QString::fromStdString(postDoc->GetDocTitle()));
	setWindowTitle(wndTitle);

	QWidget* d = new QWidget;
	QHBoxLayout* l = new QHBoxLayout;
	l->setContentsMargins(2, 0, 2, 0);
	l->addWidget(new QLabel("configuration"));
	QComboBox* config = new QComboBox();
	l->addWidget(config);
	d->setLayout(l);
	config->addItem("Current");
	config->addItem("Reference");
	AddToolBarWidget(d);

	QObject::connect(config, SIGNAL(currentIndexChanged(int)), this, SLOT(OnConfigChanged(int)));

	m_nsrc = -1;
	m_nconf = 0;
}

void CIntegrateWindow::OnConfigChanged(int i)
{
	if ((i < 0) || (i > 1)) return;
	m_nconf = i;
	Update(true, true);
}

void CIntegrateWindow::Update(bool breset, bool bfit)
{
	CDocument* doc = GetDocument();
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	// update the source options
	m_updating = true;
	if (breset || (m_nsrc == -1)) UpdateSourceOptions();

	// Update integral
	UpdateIntegral();

	m_updating = false;

	// redraw
	FitPlotsToData();
	RedrawPlot();
}

//-----------------------------------------------------------------------------
void CIntegrateWindow::UpdateIntegral()
{
	// clear the view
	ClearPlots();

	// get the source object
	CPostDocument* pdoc = GetPostDoc();

	Post::CGLModel* model = pdoc->GetGLModel();
	
	int nsrc = GetCurrentYValue();
	if (nsrc < 0) return;

	char sztitle[256] = {0};
	CPlotData* data = new CPlotData;
	Post::CGLPlaneCutPlot* pp = m_src[nsrc];
	if (pp == 0) 
	{
		// update based on current selection
		IntegrateSelection(*data);

		int nview = model->GetSelectionMode();
		snprintf(sztitle, 256, "%s of %s", (nview == Post::SELECT_NODES ? "Sum" : "Integral"), pdoc->GetFieldString().c_str());
	}
	else 
	{
		// update based on plane cut plot
		IntegratePlaneCut(pp, *data);
		snprintf(sztitle, 256, "%s of %s", "Integral", pdoc->GetFieldString().c_str());
	}
	data->setLabel("Value");
	SetPlotTitle(sztitle);
	AddPlotData(data);
	FitPlotsToData();

	UpdatePlots();
}

//-----------------------------------------------------------------------------
void CIntegrateWindow::UpdateSourceOptions()
{
	// Add the selection source
	CGenericDataSelector* sel = new CGenericDataSelector();
	sel->AddOption("current selection");
	m_src.push_back((Post::CGLPlaneCutPlot*) 0);

	// get the document
	CPostDocument* pdoc = GetPostDoc();

	// add all plane cuts to the source options
	Post::GPlotList& plt = pdoc->GetGLModel()->GetPlotList();
	for (int i =0; i < plt.Size(); ++i)
	{
		Post::CGLPlaneCutPlot* pp = dynamic_cast<Post::CGLPlaneCutPlot*>(plt[i]);
		if (pp) 
		{
			string name = pp->GetName();
			sel->AddOption(QString::fromStdString(name));
			m_src.push_back(pp);
		}
	}

	if ((m_nsrc < 0) || (m_nsrc >= m_src.size()-1)) m_nsrc = 0;
	SetYDataSelector(sel, m_nsrc);
}

//-----------------------------------------------------------------------------
void CIntegrateWindow::IntegrateSelection(CPlotData& data)
{
	// get the document
	CPostDocument* pdoc = GetPostDoc();
	Post::FEPostModel& fem = *pdoc->GetFSModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
	Post::CGLModel* po = pdoc->GetGLModel();

	data.clear();

	// get view mode
	int nview = po->GetSelectionMode();

	// make sure the color map is active
	if (po->GetColorMap()->IsActive())
	{
		// get the number of time steps
		int ntime = pdoc->GetStates();

		// make sure all states are up-to-date
		pdoc->UpdateAllStates();

		// loop over all steps
		for (int i=0; i<ntime; ++i)
		{
			Post::FEState* ps = fem.GetState(i);

			// evaluate sum/integration
			double res = 0.0;

			if (m_nconf == 0)
			{
				if      (nview == Post::SELECT_NODES) res = IntegrateNodes(mesh, ps);
				else if (nview == Post::SELECT_EDGES) res = IntegrateEdges(mesh, ps);
				else if (nview == Post::SELECT_FACES) res = IntegrateFaces(mesh, ps);
				else if (nview == Post::SELECT_ELEMS) res = IntegrateElems(mesh, ps);
				else assert(false);
			}
			else
			{
				if      (nview == Post::SELECT_NODES) res = IntegrateNodes(mesh, ps);
				else if (nview == Post::SELECT_EDGES) res = IntegrateEdges(mesh, ps);
				else if (nview == Post::SELECT_FACES) res = IntegrateReferenceFaces(mesh, ps);
				else if (nview == Post::SELECT_ELEMS) res = IntegrateReferenceElems(mesh, ps);
				else assert(false);
			}

			data.addPoint(ps->m_time, res);
		}
	}
}

//-----------------------------------------------------------------------------
void CIntegrateWindow::IntegratePlaneCut(Post::CGLPlaneCutPlot* pp, CPlotData& data)
{
	// get the document
	CPostDocument* pdoc = GetPostDoc();
	Post::FEPostModel& fem = *pdoc->GetFSModel();
	Post::CGLModel* po = pdoc->GetGLModel();

	data.clear();

	// make sure the color map is active
	if (po->GetColorMap()->IsActive())
	{
		// get the number of time steps
		int ntime = pdoc->GetStates();

		// make sure all states are up-to-date
		pdoc->UpdateAllStates();

		// loop over all steps
		for (int i=0; i<ntime; ++i)
		{
			Post::FEState* ps = fem.GetState(i);
			data.addPoint(ps->m_time, pp->Integrate(ps));
		}
	}
}

//=============================================================================

CIntegrateSurfaceWindow::CIntegrateSurfaceWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString wndTitle = windowTitle();
	wndTitle += ":IntegrateSurface";
	if (postDoc) wndTitle += QString(" [%1]").arg(QString::fromStdString(postDoc->GetDocTitle()));
	setWindowTitle(wndTitle);

	m_nsrc = -1;
}

void CIntegrateSurfaceWindow::Update(bool breset, bool bfit)
{
	CDocument* doc = GetDocument();
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	// update the source options
	m_updating = true;

	// Update integral
	UpdateIntegral();

	m_updating = false;

	// redraw
	FitPlotsToData();
	RedrawPlot();
}

//-----------------------------------------------------------------------------
void CIntegrateSurfaceWindow::UpdateIntegral()
{
	// clear the view
	ClearPlots();

	// get the source object
	CPostDocument* pdoc = GetPostDoc();

	Post::CGLModel* model = pdoc->GetGLModel();

	char sztitle[256] = { 0 };
	CPlotData* dataX = new CPlotData;
	CPlotData* dataY = new CPlotData;
	CPlotData* dataZ = new CPlotData;

	IntegrateSelection(*dataX, *dataY, *dataZ);

	int nview = model->GetSelectionMode();
	snprintf(sztitle, 256, "%s of %s", (nview == Post::SELECT_NODES ? "Sum" : "Integral"), pdoc->GetFieldString().c_str());

	SetPlotTitle(sztitle);
	dataX->setLabel("X"); AddPlotData(dataX);
	dataY->setLabel("Y"); AddPlotData(dataY);
	dataZ->setLabel("Z"); AddPlotData(dataZ);
	
	dataX->setLineColor(Qt::red  ); dataX->setFillColor(Qt::red);
	dataY->setLineColor(Qt::green); dataY->setFillColor(Qt::green);
	dataZ->setLineColor(Qt::blue ); dataZ->setFillColor(Qt::blue);

	FitPlotsToData();

	UpdatePlots();
}

//-----------------------------------------------------------------------------
void CIntegrateSurfaceWindow::IntegrateSelection(CPlotData& dataX, CPlotData& dataY, CPlotData& dataZ)
{
	// get the document
	CPostDocument* pdoc = GetPostDoc();
	Post::FEPostModel& fem = *pdoc->GetFSModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);
	Post::CGLModel* po = pdoc->GetGLModel();

	dataX.clear();
	dataY.clear();
	dataZ.clear();

	// make sure the color map is active
	if (po->GetColorMap()->IsActive())
	{
		// get the number of time steps
		int ntime = pdoc->GetStates();

		// make sure all states are up-to-date
		pdoc->UpdateAllStates();

		// loop over all steps
		for (int i = 0; i < ntime; ++i)
		{
			Post::FEState* ps = fem.GetState(i);

			// evaluate integration
			vec3d v = IntegrateSurfaceNormal(mesh, ps);

			dataX.addPoint(ps->m_time, v.x);
			dataY.addPoint(ps->m_time, v.y);
			dataZ.addPoint(ps->m_time, v.z);
		}
	}
}
