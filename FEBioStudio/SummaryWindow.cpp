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
#include "SummaryWindow.h"
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
#include <PostLib/FEPostMesh.h>
#include <PostGL/GLDataMap.h>
#include <PostLib/GLModel.h>
#include "PostDocument.h"

CSummaryWindow::CSummaryWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString wndTitle = windowTitle();
	wndTitle += ":Summary";
	if (postDoc) wndTitle += QString(" [%1]").arg(QString::fromStdString(postDoc->GetDocTitle()));
	setWindowTitle(wndTitle);

	QCheckBox* selectionOnly = new QCheckBox("Selection only");
	AddToolBarWidget(selectionOnly);
	QObject::connect(selectionOnly, SIGNAL(stateChanged(int)), this, SLOT(onSelectionOnlyChanged(int)));

	QCheckBox* volAvg = new QCheckBox("Volume average");
	volAvg->setChecked(true);
	AddToolBarWidget(volAvg);
	QObject::connect(volAvg, SIGNAL(stateChanged(int)), this, SLOT(onVolumeAverageChanged(int)));

	m_bselectionOnly = false;
	m_bvolumeAverage = true;
	m_ncurrentData = -1;
}

void CSummaryWindow::onSelectionOnlyChanged(int n)
{
	m_bselectionOnly = (n == Qt::Checked);
	Update(false, true);
}

void CSummaryWindow::onVolumeAverageChanged(int n)
{
	m_bvolumeAverage = (n == Qt::Checked);
	Update(false, true);
}

void CSummaryWindow::Update(bool breset, bool bfit)
{
	CPostDocument* doc = GetPostDoc();
	if (breset)
	{
		if (doc->IsValid())
		{
			SetYDataSelector(new CModelDataSelector(doc->GetFSModel(), Post::TENSOR_SCALAR));
		}
		else return;
	}

	Post::CGLModel* po = doc->GetGLModel();
	Post::FEPostModel* pfem = doc->GetFSModel();
	Post::FEPostMesh* pfe = po->GetActiveMesh();
	int nodes = pfe->Nodes();

	// get the selection mode
	int nmode = po->GetSelectionType();

	// get the current data field
	int ndata = GetCurrentYValue();
	if (ndata < 0) return;

	// store current data
	m_ncurrentData = ndata;

	// get the text and make it the plot title
	SetPlotTitle("Summary of " + GetCurrentYText());

	// see if selection only box is checked
	bool bsel = m_bselectionOnly;

	// see if volume average is checked
	bool bvol = m_bvolumeAverage;

	// decide if we want to find the node/face/elem stat
	int neval = -1;
	if (bsel)
	{
		if (nmode == SELECT_FE_NODES) neval = 0;
		if (nmode == SELECT_FE_EDGES) neval = 1;
		if (nmode == SELECT_FE_FACES) neval = 2;
		if (nmode == SELECT_FE_ELEMS) neval = 3;
	}
	else
	{
		if (IS_NODE_FIELD(m_ncurrentData)) neval = 0;
		if (IS_EDGE_FIELD(m_ncurrentData)) neval = 1;
		if (IS_FACE_FIELD(m_ncurrentData)) neval = 2;
		if (IS_ELEM_FIELD(m_ncurrentData)) neval = 3;
	}
	assert(neval >= 0);

	// clear the graph
	ClearPlots();

	// get the number of time steps
	int nsteps = doc->GetStates();

	// allocate data
	vector<double> x(nsteps);
	// add the data series
	for (int i=0; i<nsteps; i++) x[i] = pfem->GetState(i)->m_time;

	CPlotData* dataMax = new CPlotData;
	CPlotData* dataMin = new CPlotData;
	CPlotData* dataAvg = new CPlotData;
	dataMax->setLabel("Max");
	dataAvg->setLabel("Avg");
	dataMin->setLabel("Min");

	// get the displacemet map
	Post::CGLDisplacementMap* pdm = po->GetDisplacementMap();

	// loop over all time steps
	for (int i=0; i<nsteps; i++)
	{
		// get the state
		Post::FEState* ps = pfem->GetState(i);

		// we need to make sure that the displacements are updated
		// in case the user evaluates the strains
		if (pdm) pdm->UpdateState(i);

		// store the nodal values and field
		int noldField = ps->m_nField;

		// evaluate the mesh
		pfem->Evaluate(m_ncurrentData, i);
 
		// store min/max values
		RANGE rng;
		switch (neval)
		{
		case 0: rng = EvalNodeRange(*pfem, i, bsel); break;
		case 1: rng = EvalEdgeRange(*pfem, i, bsel); break;
		case 2: rng = EvalFaceRange(*pfem, i, bsel, bvol); break;
		case 3: rng = EvalElemRange(*pfem, i, bsel, bvol); break;
		}
		dataMax->addPoint(x[i], rng.fmax);
		dataMin->addPoint(x[i], rng.fmin);
		dataAvg->addPoint(x[i], rng.favg);
		
		// reset the field data
		if (noldField >= 0) pfem->Evaluate(noldField, i);
	}

	// add the data
	AddPlotData(dataMax);
	AddPlotData(dataAvg);
	AddPlotData(dataMin);

	// redraw
	FitPlotsToData();
	RedrawPlot();

	UpdatePlots();
}

//-----------------------------------------------------------------------------
// Evaluate the range of unpacked nodal data values
CSummaryWindow::RANGE CSummaryWindow::EvalNodeRange(Post::FEPostModel& fem, int nstate, bool bsel)
{
	RANGE rng = {-1e20f, 1e20f, 0.f};

	Post::FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& mesh = *state.GetFEMesh();

	float sum = 0;
	
	int NN = mesh.Nodes();
	for (int i=0; i<NN; i++)
	{
		FSNode& node = mesh.Node(i);
		if ((bsel == false) || (node.IsSelected()))
		{
			float val = state.m_NODE[i].m_val;
			rng.favg += val;
			sum += 1.f;
			if (val > rng.fmax) rng.fmax = val;
			if (val < rng.fmin) rng.fmin = val;
		}
	}

	if (sum == 0.f) rng.fmin = rng.fmax = 0.f;
	else rng.favg /= sum;

	return rng;
}

//-----------------------------------------------------------------------------
// Evaluate the range of unpacked nodal data values
CSummaryWindow::RANGE CSummaryWindow::EvalEdgeRange(Post::FEPostModel& fem, int nstate, bool bsel)
{
	RANGE rng = {-1e20f, 1e20f, 0.f};

	Post::FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& mesh = *state.GetFEMesh();

	float sum = 0;
	
	int NE = mesh.Edges();
	for (int i=0; i<NE; i++)
	{
		FSEdge& edge = mesh.Edge(i);
		if ((bsel == false) || (edge.IsSelected()))
		{
			float val = state.m_EDGE[i].m_val;
			rng.favg += val;
			sum += 1.f;
			if (val > rng.fmax) rng.fmax = val;
			if (val < rng.fmin) rng.fmin = val;
		}
	}

	if (sum == 0.f) rng.fmin = rng.fmax = 0.f;
	else rng.favg /= sum;

	return rng;
}

//-----------------------------------------------------------------------------
// Evaluate the range of unpacked element data values
CSummaryWindow::RANGE CSummaryWindow::EvalElemRange(Post::FEPostModel& fem, int nstate, bool bsel, bool bvol)
{
	RANGE rng = {-1e20f, 1e20f, 0.f};

	Post::FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& mesh = *state.GetFEMesh();

	ValArray& elemData = state.m_ElemData;

	float sum = 0.f;
	int NE = mesh.Elements();
	for (int i=0; i<NE; i++)
	{
		FEElement_& e = mesh.ElementRef(i);
		int ne = e.Nodes();

		if ((bsel == false) || (e.IsSelected()))
		{
			float w = 1;
			if (bvol) w = mesh.ElementVolume(i);

			for (int j=0; j<ne; ++j)
			{
//				float val = state.m_ELEM[i].m_val;
				float val = elemData.value(i, j);

				rng.favg += val*w;
				sum += w;
				if (val > rng.fmax) rng.fmax = val;
				if (val < rng.fmin) rng.fmin = val;
			}
		}
	}

	if (sum == 0.f) rng.fmin = rng.fmax = 0.f;
	else rng.favg /= sum;

	return rng;
}

//-----------------------------------------------------------------------------
// Evaluate the range of unpacked face data values
CSummaryWindow::RANGE CSummaryWindow::EvalFaceRange(Post::FEPostModel& fem, int nstate, bool bsel, bool bvol)
{
	RANGE rng = {-1e20f, 1e20f, 0.f};

	Post::FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& mesh = *state.GetFEMesh();

	float sum = 0.f;
	int NF = mesh.Faces();
	for (int i=0; i<NF; i++)
	{
		FSFace& f = mesh.Face(i);

		if ((bsel == false) || (f.IsSelected()))
		{
			float val = state.m_FACE[i].m_val;
			float w = 1;
			if (bvol) w = mesh.FaceArea(f);

			rng.favg += val*w;
			sum += w;
			if (val > rng.fmax) rng.fmax = val;
			if (val < rng.fmin) rng.fmin = val;
		}
	}

	if (sum == 0.f) rng.fmin = rng.fmax = 0.f;
	else rng.favg /= sum;

	return rng;
}
