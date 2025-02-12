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
#include "MainWindow.h"
#include "DlgStartThread.h"
#include "ui_mainwindow.h"
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLMirrorPlane.h>
#include <PostGL/GLVectorPlot.h>
#include <PostGL/GLTensorPlot.h>
#include <PostGL/GLStreamLinePlot.h>
#include <PostGL/GLParticleFlowPlot.h>
#include <PostGL/GLSlicePLot.h>
#include <PostGL/GLIsoSurfacePlot.h>
#include <ImageLib/ImageModel.h>
#include <PostLib/ImageSlicer.h>
#include <PostLib/VolumeRenderer.h>
#include <PostLib/MarchingCubes.h>
#include <PostGL/GLVolumeFlowPlot.h>
#include <PostGL/GLModel.h>
#include <PostGL/GLPointProbe.h>
#include <PostGL/GLCurveProbe.h>
#include <PostGL/GLRuler.h>
#include <PostGL/GLMusclePath.h>
#include <PostGL/GLLinePlot.h>
#include <PostGL/GLPlotGroup.h>
#include <PostLib/FEPostModel.h>
#include <QMessageBox>
#include <QTimer>
#include "GraphWindow.h"
#include "SummaryWindow.h"
#include "StatsWindow.h"
#include "IntegrateWindow.h"
#include "ScatterWindow.h"
#include "DlgImportLines.h"
#include "DlgTimeSettings.h"
#include "PostDocument.h"
#include "ModelDocument.h"
#include "DlgWarpImage.h"
#include <ImageLib/ImageFilter.h>

QString warningNoActiveModel = "Please select the view tab to which you want to add this plot.";

Post::CGLModel* CMainWindow::GetCurrentModel()
{
	CGLModelDocument* doc = dynamic_cast<CGLModelDocument*>(GetDocument());
	if (doc== nullptr) return nullptr;
	return doc->GetGLModel();
}

void CMainWindow::on_actionPlaneCut_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLPlaneCutPlot* pp = new Post::CGLPlaneCutPlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionMirrorPlane_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLMirrorPlane* pp = new Post::CGLMirrorPlane();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionVectorPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLVectorPlot* pp = new Post::CGLVectorPlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionTensorPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLTensorPlot* pp = new Post::GLTensorPlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionStreamLinePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLStreamLinePlot* pp = new Post::CGLStreamLinePlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionParticleFlowPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::CGLParticleFlowPlot* pp = new Post::CGLParticleFlowPlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionVolumeFlowPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLVolumeFlowPlot* pp = new Post::GLVolumeFlowPlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}


void CMainWindow::on_actionImageSlicer_triggered()
{
	// get the document
	CDocument* modelDoc = GetModelDocument();
	CDocument* postDoc = GetPostDocument();

	CImageModel* img = nullptr;
	if (modelDoc)
	{
		img = dynamic_cast<CImageModel*>(ui->modelViewer->GetCurrentObject());
	}
	else if (postDoc)
	{
		img = dynamic_cast<CImageModel*>(ui->postPanel->GetSelectedObject());
	}

	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CImageSlicer* slicer = new Post::CImageSlicer(img);
	slicer->Create();
	img->AddImageRenderer(slicer);

	if (modelDoc)
	{
		ui->modelViewer->Update();
		ui->modelViewer->Select(slicer);
	}
	else if (postDoc)
	{
		ui->postPanel->Update();
		ui->postPanel->SelectObject(slicer);
	}
	RedrawGL();
}

void CMainWindow::on_actionVolumeRender_triggered()
{
	// get the document
	CDocument* modelDoc = GetModelDocument();
	CDocument* postDoc = GetPostDocument();

	CImageModel* img = nullptr;
	if (modelDoc)
	{
		img = dynamic_cast<CImageModel*>(ui->modelViewer->GetCurrentObject());
	}
	else if (postDoc)
	{
		img = dynamic_cast<CImageModel*>(ui->postPanel->GetSelectedObject());
	}

	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CVolumeRenderer* vr = new Post::CVolumeRenderer(img);
	vr->Create();
	img->AddImageRenderer(vr);

	if (modelDoc)
	{
		ui->modelViewer->Update();
		ui->modelViewer->Select(vr);
	}
	else if (postDoc)
	{
		ui->postPanel->Update();
		ui->postPanel->SelectObject(vr);
	}
	RedrawGL();
}

void CMainWindow::on_actionMarchingCubes_triggered()
{
	// get the document
	CDocument* modelDoc = GetModelDocument();
	CDocument* postDoc = GetPostDocument();

	CImageModel* img = nullptr;
	if (modelDoc)
	{
		img = dynamic_cast<CImageModel*>(ui->modelViewer->GetCurrentObject());
	}
	else if (postDoc)
	{
		img = dynamic_cast<CImageModel*>(ui->postPanel->GetSelectedObject());
	}

	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CMarchingCubes* mc = new Post::CMarchingCubes(img);
	mc->Create();
	img->AddImageRenderer(mc);
	if (modelDoc)
	{
		ui->modelViewer->Update();
		ui->modelViewer->Select(mc);
	}
	else if (postDoc)
	{
		ui->postPanel->Update();
		ui->postPanel->SelectObject(mc);
	}
	RedrawGL();
}

void CMainWindow::on_actionImageWarp_triggered()
{
	// get the document
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	// get the selected image model
	if (postDoc->ImageModels() == 0)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must import at least one image first.");
        return;
    }

    CDlgWarpImage dlg(postDoc, this);

    dlg.exec();

	ui->postPanel->Update(true);

	RedrawGL();
}

void CMainWindow::on_actionAddProbe_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if ((doc == nullptr) || !doc->IsValid())
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	// get the selection center
	FESelection* sel = doc->GetCurrentSelection();
	if (sel && sel->Size())
	{
		BOX box = sel->GetBoundingBox();
		vec3d c = box.Center();

		// create the probe and set its initial position
		Post::GLPointProbe* probe = new Post::GLPointProbe();
		probe->SetInitialPosition(c);

		// if the current selection is a plot-group, we'll add it there
		Post::CGLModel* glm = doc->GetGLModel();
		Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(ui->postPanel->GetSelectedObject());
		if (pg) pg->AddPlot(probe);
		else glm->AddPlot(probe);

		if (GetPostDocument()) UpdatePostPanel(true, probe);
		else Update(nullptr, true);
		RedrawGL();
	}
}

void CMainWindow::on_actionAddCurveProbe_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLCurveProbe* probe = new Post::GLCurveProbe();
	glm->AddPlot(probe);

	if (GetPostDocument()) UpdatePostPanel(true, probe);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionAddRuler_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLRuler* ruler = new Post::GLRuler();
	glm->AddPlot(ruler);

	if (GetPostDocument()) UpdatePostPanel(true, ruler);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionMusclePath_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	// create new muscle path
	Post::GLMusclePath* musclePath = new Post::GLMusclePath();

	// if the current selection is a plot-group, we'll add it there
	Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(ui->postPanel->GetSelectedObject());
	if (pg)
	{
		// add it to the group
		pg->AddPlot(musclePath);
	}
	else
	{
		// just add it to the model
		glm->AddPlot(musclePath);
	}

	if (GetPostDocument()) UpdatePostPanel(true, musclePath);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionPlotGroup_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr)
	{
		QMessageBox::information(this, "FEBio Studio", warningNoActiveModel);
		return;
	}

	Post::GLPlotGroup* mpg = new Post::GLPlotGroup();
	glm->AddPlot(mpg);

	if (GetPostDocument()) UpdatePostPanel(true, mpg);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionIsosurfacePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLIsoSurfacePlot* pp = new Post::CGLIsoSurfacePlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);
	RedrawGL();
}

void CMainWindow::on_actionSlicePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLSlicePlot* pp = new Post::CGLSlicePlot();
	glm->AddPlot(pp);

	if (GetPostDocument()) UpdatePostPanel(true, pp);
	else Update(nullptr, true);

	RedrawGL();
}

void CMainWindow::on_actionDisplacementMap_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	Post::CGLModel* pm = doc->GetGLModel();
	if (pm->GetDisplacementMap() == nullptr)
	{
		if (pm->AddDisplacementMap() == false)
		{
			QMessageBox::warning(this, "FEBio Studio", "You need at least one vector field before you can define a displacement map.");
		}
		else
		{
			doc->UpdateFEModel(true);
			ui->postPanel->Update(true);
		}
	}
	else
	{
		QMessageBox::information(this, "FEBio Studio", "This model already has a displacement map.");
	}
}

void CMainWindow::on_actionGraph_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CGraphWindow* pg = new CModelGraphWindow(this, postDoc);
	AddGraph(pg);

	pg->show();
	pg->raise();
	pg->activateWindow();
	pg->Update();
}

void CMainWindow::on_actionScatter_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CScatterWindow* scatterWindow = new CScatterWindow(this, postDoc);

	scatterWindow->Update(true);
	scatterWindow->show();
	scatterWindow->raise();
	scatterWindow->activateWindow();

	AddGraph(scatterWindow);
}

void CMainWindow::on_actionSummary_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CSummaryWindow* summaryWindow = new CSummaryWindow(this, postDoc);

	summaryWindow->Update(true);
	summaryWindow->show();
	summaryWindow->raise();
	summaryWindow->activateWindow();

	AddGraph(summaryWindow);
}

void CMainWindow::on_actionStats_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CStatsWindow* statsWindow = new CStatsWindow(this, postDoc);
	statsWindow->Update(true);
	statsWindow->show();
	statsWindow->raise();
	statsWindow->activateWindow();

	AddGraph(statsWindow);
}

void CMainWindow::on_actionIntegrate_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CIntegrateWindow* integrateWindow = new CIntegrateWindow(this, postDoc);
	integrateWindow->Update(true);
	integrateWindow->show();
	integrateWindow->raise();
	integrateWindow->activateWindow();

	AddGraph(integrateWindow);
}

void CMainWindow::on_actionIntegrateSurface_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;

	CIntegrateSurfaceWindow* integrateWindow = new CIntegrateSurfaceWindow(this, postDoc);
	integrateWindow->Update(true);
	integrateWindow->show();
	integrateWindow->raise();
	integrateWindow->activateWindow();

	AddGraph(integrateWindow);
}


void CMainWindow::on_actionImportPoints_triggered()
{
	CDlgImportPoints dlg(this);
	dlg.exec();
}

class processLinesThread : public CustomThread
{
public:
	processLinesThread(Post::LineDataModel& lines) : m_lines(lines)
	{
		m_stopRequest = false;
	}

	void run() Q_DECL_OVERRIDE
	{
		m_stopRequest = false;
		// process the line data
		int states = m_lines.States();
		m_completed = 0;
#pragma omp parallel for schedule(dynamic)
		for (int nstate = 0; nstate < states; ++nstate)
		{
			if (m_stopRequest == false)
			{
				Post::LineData& lineData = m_lines.GetLineData(nstate);
				lineData.processLines();
			}
#pragma omp atomic
			m_completed++;
		}
		m_completed = states;
		emit resultReady(m_stopRequest==false);
	}

public:
	bool hasProgress() override { return true; }

	double progress() override 
	{ 
		double pct = 100.0 * m_completed / m_lines.States();
		return pct; 
	}

	const char* currentTask() override { return "processing line data"; }

	void stop() override { m_stopRequest = true; }

private:
	bool	m_stopRequest;
	int		m_completed;
	Post::LineDataModel& m_lines;
};

//------------------------------------------------------------------------------
void CMainWindow::on_actionImportLines_triggered()
{
	CDlgImportLines dlg(this);
	if (dlg.exec())
	{
		CPostDocument* doc = ui->m_wnd->GetPostDocument();
		if (doc == nullptr) return;

		string fileName = dlg.GetFileName().toStdString();
		string name = dlg.GetName().toStdString();

		Post::FEPostModel* fem = doc->GetFSModel();

		// allocate line model
		Post::LineDataModel* lineData = new Post::LineDataModel(fem);

		Post::Ang2LineDataSource* src = new Post::Ang2LineDataSource(lineData);

		bool bsuccess = src->Load(fileName.c_str());

		if (bsuccess)
		{
			// add a line plot for visualizing the line data
			Post::CGLLinePlot* pgl = new Post::CGLLinePlot();
			pgl->SetLineDataModel(lineData);
			pgl->SetName(name);
			ui->m_wnd->UpdatePostPanel(false, pgl);

			processLinesThread* thread = new processLinesThread(*lineData);
			CDlgStartThread dlg2(this, thread);
			
			bool bsuccess = false;
			if (dlg2.exec())
			{
				bool bsuccess = dlg2.GetReturnCode();
				if (bsuccess)
				{
					doc->GetGLModel()->AddPlot(pgl);
					ui->postPanel->Update();
					ui->postPanel->SelectObject(pgl);
				}
				else delete pgl;
			}
			else delete pgl;
		}
		else
		{
			delete lineData;
			QMessageBox::critical(0, "FEBio Studio", "Failed reading line data file.");
		}
	}
}

//-----------------------------------------------------------------------------
// set the current time value
void CMainWindow::SetCurrentTimeValue(float ftime)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	int n0 = doc->GetActiveState();
	doc->SetCurrentTimeValue(ftime);
	int n1 = doc->GetActiveState();

	if (n0 != n1)
	{
		ui->postToolBar->SetSpinValue(n1 + 1, true);
	}
	if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(false);

	// update the rest
	//	UpdateTools(false);
	UpdateGraphs(false);
	RedrawGL();
}

//-----------------------------------------------------------------------------
void CMainWindow::onTimer()
{
	if (ui->m_isAnimating == false) return;

	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	TIMESETTINGS& time = doc->GetTimeSettings();

	int N = doc->GetFSModel()->GetStates();
	int N0 = time.m_start;
	int N1 = time.m_end;

	int nstep = doc->GetActiveState();

	if (time.m_bfix)
	{
		float f0 = doc->GetTimeValue(N0);
		float f1 = doc->GetTimeValue(N1);

		float ftime = doc->GetTimeValue();

		if (time.m_mode == MODE_FORWARD)
		{
			ftime += time.m_dt;
			if (ftime > f1)
			{
				if (time.m_bloop) ftime = f0;
				else { ftime = f1; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_REVERSE)
		{
			ftime -= time.m_dt;
			if (ftime < f0)
			{
				if (time.m_bloop) ftime = f1;
				else { ftime = f0; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_CYLCE)
		{
			ftime += time.m_dt*time.m_inc;
			if (ftime > f1)
			{
				time.m_inc = -1;
				ftime = f1;
				if (time.m_bloop == false) StopAnimation();
			}
			else if (ftime < f0)
			{
				time.m_inc = 1;
				ftime = f0;
				if (time.m_bloop == false) StopAnimation();
			}
		}

		SetCurrentTimeValue(ftime);
	}
	else
	{
		if (time.m_mode == MODE_FORWARD)
		{
			nstep++;
			if (nstep > N1)
			{
				if (time.m_bloop) nstep = N0;
				else { nstep = N1; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_REVERSE)
		{
			nstep--;
			if (nstep < N0)
			{
				if (time.m_bloop) nstep = N1;
				else { nstep = N0; StopAnimation(); }
			}
		}
		else if (time.m_mode == MODE_CYLCE)
		{
			nstep += time.m_inc;
			if (nstep > N1)
			{
				time.m_inc = -1;
				nstep = N1;
				if (time.m_bloop == false) StopAnimation();
			}
			else if (nstep < N0)
			{
				time.m_inc = 1;
				nstep = N0;
				if (time.m_bloop == false) StopAnimation();
			}
		}
		ui->postToolBar->SetSpinValue(nstep + 1);
	}

	// TODO: Should I start the event before or after the view is redrawn?
	if (ui->m_isAnimating)
	{
		if (doc == nullptr) return;
		if (doc->IsValid())
		{
			TIMESETTINGS& time = doc->GetTimeSettings();
			double fps = time.m_fps;
			if (fps < 1.0) fps = 1.0;
			double msec_per_frame = 1000.0 / fps;
			QTimer::singleShot(msec_per_frame, this, SLOT(onTimer()));
		}
	}
}

void CMainWindow::on_selectData_currentValueChanged(int index)
{
	//	if (index == -1)
	//		ui->actionColorMap->setDisabled(true);
	//	else
	{
		//		if (ui->actionColorMap->isEnabled() == false)
		//			ui->actionColorMap->setEnabled(true);

		int nfield = ui->postToolBar->GetDataField();
		CPostDocument* doc = GetPostDocument();
		if (doc == nullptr) return;
		doc->SetDataField(nfield);

		// turn on the colormap
		if (ui->postToolBar->IsColorMapActive() == false)
		{
			ui->postToolBar->ToggleColorMap();
		}

		ui->postPanel->SelectObject(doc->GetGLModel()->GetColorMap());

		GetGLView()->UpdateWidgets();
		RedrawGL();
	}

	UpdateGraphs(false);

	//	if (ui->modelViewer->isVisible()) ui->modelViewer->Update(false);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionPlay_toggled(bool bchecked)
{
	CPostDocument* doc = GetPostDocument();
	if (doc && doc->IsValid())
	{
		if (bchecked)
		{
			TIMESETTINGS& time = doc->GetTimeSettings();
			double fps = time.m_fps;
			if (fps < 1.0) fps = 1.0;
			double msec_per_frame = 1000.0 / fps;

			ui->m_isAnimating = true;
			QTimer::singleShot(msec_per_frame, this, SLOT(onTimer()));
		}
		else ui->m_isAnimating = false;
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionRefresh_triggered()
{
	ui->postToolBar->setDisabled(true);

	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc && doc->GetFileReader() && (doc->GetDocFilePath().empty() == false))
	{
		OpenFile(QString::fromStdString(doc->GetDocFilePath()), false);
	}
	else
	{
		QMessageBox::critical(this, "ERROR", "Can't refresh.");
	}
}

//-----------------------------------------------------------------------------
// set the current time of the current post doc
void CMainWindow::SetCurrentTime(int n)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	ui->postToolBar->SetSpinValue(n + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionFirst_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	ui->postToolBar->SetSpinValue(time.m_start + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionPrev_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	int nstep = doc->GetActiveState();
	nstep--;
	if (nstep < time.m_start) nstep = time.m_start;
	ui->postToolBar->SetSpinValue(nstep + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionNext_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	int nstep = doc->GetActiveState();
	nstep++;
	if (nstep > time.m_end) nstep = time.m_end;
	ui->postToolBar->SetSpinValue(nstep + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionLast_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	TIMESETTINGS& time = doc->GetTimeSettings();
	ui->postToolBar->SetSpinValue(time.m_end + 1);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionTimeSettings_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	CDlgTimeSettings dlg(doc, this);
	if (dlg.exec())
	{
		TIMESETTINGS& time = doc->GetTimeSettings();
		//		ui->timePanel->SetRange(time.m_start, time.m_end);

		int ntime = doc->GetActiveState();
		if ((ntime < time.m_start) || (ntime > time.m_end))
		{
			if (ntime < time.m_start) ntime = time.m_start;
			if (ntime > time.m_end) ntime = time.m_end;
		}

		ui->postToolBar->SetSpinValue(ntime + 1);
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_selectTime_valueChanged(int n)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	doc->SetActiveState(n - 1);
	RedrawGL();

	if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(false);

	if (ui->measureTool && ui->measureTool->isVisible()) ui->measureTool->Update();

	UpdateGraphs(false);
}

//-----------------------------------------------------------------------------
void CMainWindow::SetCurrentState(int n)
{
	CPostDocument* doc = GetPostDocument();
	if (doc)
		ui->postToolBar->SetSpinValue(n + 1);

	FEBioMonitorDoc* mdoc = dynamic_cast<FEBioMonitorDoc*>(GetDocument());
	if (mdoc && (mdoc->IsPaused() || !mdoc->IsRunning()))
	{
		mdoc->SetCurrentState(n);
		RedrawGL();
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionColorMap_toggled(bool bchecked)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	doc->ActivateColormap(bchecked);
	ui->postPanel->SelectObject(doc->GetGLModel()->GetColorMap());
	RedrawGL();
}
