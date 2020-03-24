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
#include <PostGL/GLDataMap.h>
#include <PostGL/GLModel.h>
#include "PostDoc.h"

CSummaryWindow::CSummaryWindow(CMainWindow* wnd, CPostDoc* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString title = "FEBio Studio: Summary";
	setWindowTitle(title);

	m_ncurrentData = -1;
}

void CSummaryWindow::Update(bool breset, bool bfit)
{
	CPostDoc* doc = GetPostDoc();
	if (breset)
	{
		if (doc->IsValid())
		{
			SetYDataSelector(new CModelDataSelector(doc->GetFEModel(), Post::DATA_SCALAR));
		}
		else return;
	}

	Post::CGLModel* po = doc->GetGLModel();
	Post::FEModel* pfem = doc->GetFEModel();
	Post::FEPostMesh* pfe = po->GetActiveMesh();
	int nodes = pfe->Nodes();

	// get the selection mode
	int nmode = po->GetSelectionMode();

	// get the current data field
	int ndata = GetCurrentYValue();
	if (ndata <= 0) return;

	// store current data
	m_ncurrentData = ndata;

	// get the text and make it the plot title
	SetPlotTitle("Summary of " + GetCurrentYText());

	// see if selection only box is checked
	bool bsel = false; // ui->selectionOnly->isChecked();

	// see if volume average is checked
	bool bvol = true; // ui->volumeAverage->isChecked();

	// decide if we want to find the node/face/elem stat
	int neval = -1;
	if ((bsel && (nmode == Post::SELECT_NODES)) || IS_NODE_FIELD(m_ncurrentData)) neval = 0;
	if ((bsel && (nmode == Post::SELECT_EDGES)) || IS_EDGE_FIELD(m_ncurrentData)) neval = 1;
	if ((bsel && (nmode == Post::SELECT_FACES)) || IS_FACE_FIELD(m_ncurrentData)) neval = 2;
	if ((bsel && (nmode == Post::SELECT_ELEMS)) || IS_ELEM_FIELD(m_ncurrentData)) neval = 3;
	assert(neval >= 0);

	// clear the graph
	ClearPlots();

	// get the number of time steps
	int nsteps = doc->GetStates();

	// allocate data
	vector<double> x(nsteps);
	// add the data series
	for (int i=0; i<nsteps; i++) x[i] = pfem->GetState(i)->m_time;

	CLineChartData* dataMax = new CLineChartData;
	CLineChartData* dataMin = new CLineChartData;
	CLineChartData* dataAvg = new CLineChartData;
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
CSummaryWindow::RANGE CSummaryWindow::EvalNodeRange(Post::FEModel& fem, int nstate, bool bsel)
{
	RANGE rng = {-1e20f, 1e20f, 0.f};

	Post::FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& mesh = *state.GetFEMesh();

	float sum = 0;
	
	int NN = mesh.Nodes();
	for (int i=0; i<NN; i++)
	{
		FENode& node = mesh.Node(i);
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
CSummaryWindow::RANGE CSummaryWindow::EvalEdgeRange(Post::FEModel& fem, int nstate, bool bsel)
{
	RANGE rng = {-1e20f, 1e20f, 0.f};

	Post::FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& mesh = *state.GetFEMesh();

	float sum = 0;
	
	int NE = mesh.Edges();
	for (int i=0; i<NE; i++)
	{
		FEEdge& edge = mesh.Edge(i);
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
CSummaryWindow::RANGE CSummaryWindow::EvalElemRange(Post::FEModel& fem, int nstate, bool bsel, bool bvol)
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
CSummaryWindow::RANGE CSummaryWindow::EvalFaceRange(Post::FEModel& fem, int nstate, bool bsel, bool bvol)
{
	RANGE rng = {-1e20f, 1e20f, 0.f};

	Post::FEState& state = *fem.GetState(nstate);
	Post::FEPostMesh& mesh = *state.GetFEMesh();

	float sum = 0.f;
	int NF = mesh.Faces();
	for (int i=0; i<NF; i++)
	{
		FEFace& f = mesh.Face(i);

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
