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
#include "PostDoc.h"

CIntegrateWindow::CIntegrateWindow(CMainWindow* wnd, CPostDoc* postDoc) : CGraphWindow(wnd, postDoc, 0)
{
	QString title = "PostView2: Integrate";
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
	CPostDoc* pdoc = GetPostDoc();

	Post::CGLModel* model = pdoc->GetGLModel();
	
	int nsrc = GetCurrentYValue();
	if (nsrc < 0) return;

	char sztitle[256] = {0};
	CLineChartData* data = new CLineChartData;
	Post::CGLPlaneCutPlot* pp = m_src[nsrc];
	if (pp == 0) 
	{
		// update based on current selection
		IntegrateSelection(*data);

		int nview = model->GetSelectionMode();
		sprintf(sztitle, "%s of %s", (nview == SELECT_NODES? "Sum" : "Integral"), pdoc->GetFieldString().c_str());
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
	CPostDoc* pdoc = GetPostDoc();

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
void CIntegrateWindow::IntegrateSelection(CLineChartData& data)
{
	// get the document
	CPostDoc* pdoc = GetPostDoc();
	Post::FEModel& fem = *pdoc->GetFEModel();
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
double CIntegrateWindow::IntegrateNodes(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	double res = 0.0;
	int N = mesh.Nodes();
	for (int i=0; i<N; ++i)
	{
		FENode& node = mesh.Node(i);
		if (node.IsSelected() && (ps->m_NODE[i].m_ntag > 0))
		{
			res += ps->m_NODE[i].m_val;
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
double CIntegrateWindow::IntegrateEdges(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	assert(false);
	return 0.0;
}

//-----------------------------------------------------------------------------
// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double CIntegrateWindow::IntegrateFaces(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	double res = 0.0;
	float v[FEFace::MAX_NODES];
	vec3f r[FEFace::MAX_NODES];
	for (int i=0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		if (f.IsSelected() && f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal values
			for (int j=0; j<nn; ++j) v[j] = ps->m_NODE[f.n[j]].m_val;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				v[3] = v[2];
				break;
			}

			// get the nodal coordinates
			for (int j=0; j<nn; ++j) r[j] = ps->m_NODE[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// add to integral
			res += IntegrateQuad(r, v);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double CIntegrateWindow::IntegrateElems(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	double res = 0.0;
	float v[FEElement::MAX_NODES];
	vec3f r[FEElement::MAX_NODES];
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement_& e = mesh.ElementRef(i);
		if (e.IsSelected() && (e.IsSolid()) && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and coordinates
			for (int j=0; j<nn; ++j) v[j] = ps->m_NODE[e.m_node[j]].m_val;
			for (int j=0; j<nn; ++j) r[j] = ps->m_NODE[e.m_node[j]].m_rt;
			switch (e.Type())
			{
			case FE_PENTA6:
				v[7] = v[5]; r[7] = r[5];
				v[6] = v[5]; r[6] = r[5];
				v[5] = v[4]; r[5] = r[4];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
				v[7] = v[3]; r[7] = r[3];
				v[6] = v[3]; r[6] = r[3];
				v[5] = v[3]; r[5] = r[3];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			}
			
			// add to integral
			res += IntegrateHex(r, v);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
void CIntegrateWindow::IntegratePlaneCut(Post::CGLPlaneCutPlot* pp, CLineChartData& data)
{
	// get the document
	CPostDoc* pdoc = GetPostDoc();
	Post::FEModel& fem = *pdoc->GetFEModel();
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
