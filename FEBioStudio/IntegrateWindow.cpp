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

CIntegrateWindow::CIntegrateWindow(CMainWindow* wnd, CPostDocument* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString title = "FEBio Studio: Integrate";
	setWindowTitle(title);
	m_nsrc = -1;
}

void CIntegrateWindow::Update(bool breset, bool bfit)
{
	CDocument* doc = GetDocument();
	if (doc->IsValid() == false) return;

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
		sprintf(sztitle, "%s of %s", (nview == Post::SELECT_NODES ? "Sum" : "Integral"), pdoc->GetFieldString().c_str());
	}
	else 
	{
		// update based on plane cut plot
		IntegratePlaneCut(pp, *data);
		sprintf(sztitle, "%s of %s", "Integral", pdoc->GetFieldString().c_str());
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
	Post::FEPostModel& fem = *pdoc->GetFEModel();
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
			if      (nview == Post::SELECT_NODES) res = IntegrateNodes(mesh, ps);
			else if (nview == Post::SELECT_EDGES) res = IntegrateEdges(mesh, ps);
			else if (nview == Post::SELECT_FACES) res = IntegrateFaces(mesh, ps);
			else if (nview == Post::SELECT_ELEMS) res = IntegrateElems(mesh, ps);
			else assert(false);

			data.addPoint(ps->m_time, res);
		}
	}
}

//-----------------------------------------------------------------------------
void CIntegrateWindow::IntegratePlaneCut(Post::CGLPlaneCutPlot* pp, CPlotData& data)
{
	// get the document
	CPostDocument* pdoc = GetPostDoc();
	Post::FEPostModel& fem = *pdoc->GetFEModel();
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