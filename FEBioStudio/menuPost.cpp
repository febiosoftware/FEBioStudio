#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLMirrorPlane.h>
#include <PostGL/GLVectorPlot.h>
#include <PostGL/GLTensorPlot.h>
#include <PostGL/GLStreamLinePlot.h>
#include <PostGL/GLParticleFlowPlot.h>
#include <PostGL/GLSlicePLot.h>
#include <PostViewLib/ImageModel.h>
#include <PostViewLib/ImageSlicer.h>
#include <PostViewLib/VolRender.h>
#include <PostGL/GLModel.h>
#include <QMessageBox>
#include "PostDoc.h"

Post::CGLModel* CMainWindow::GetCurrentModel()
{
	CFEBioJob* job = dynamic_cast<CFEBioJob*>(ui->modelViewer->GetCurrentObject());
	if (job == nullptr) return nullptr;

	CPostDoc* pd = job->GetPostDoc();
	if (pd == nullptr) return nullptr;

	return pd->GetGLModel();
}

void CMainWindow::on_actionPlaneCut_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLPlaneCutPlot* pp = new Post::CGLPlaneCutPlot(glm);
	glm->AddPlot(pp);

/*	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();
*/	RedrawGL();
}

void CMainWindow::on_actionMirrorPlane_triggered()
{
/*	CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	CGLMirrorPlane* pp = new CGLMirrorPlane(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();
	RedrawGL();
*/
}

void CMainWindow::on_actionVectorPlot_triggered()
{
/*	CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	CGLVectorPlot* pp = new CGLVectorPlot(glm);
	glm->AddPlot(pp);
	doc->UpdateFEModel();

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
*/
}

void CMainWindow::on_actionTensorPlot_triggered()
{
/*	CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	GLTensorPlot* pp = new GLTensorPlot(glm);
	glm->AddPlot(pp);
	doc->UpdateFEModel();

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
*/
}

void CMainWindow::on_actionStreamLinePlot_triggered()
{
/*	CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	CGLStreamLinePlot* pp = new CGLStreamLinePlot(glm);
	glm->AddPlot(pp);
	doc->UpdateFEModel();

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
*/
}

void CMainWindow::on_actionParticleFlowPlot_triggered()
{
/*	CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	CGLParticleFlowPlot* pp = new CGLParticleFlowPlot(glm);
	glm->AddPlot(pp);
	doc->UpdateFEModel();

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
*/
}

void CMainWindow::on_actionImageSlicer_triggered()
{
/*	CGLObject* po = ui->modelViewer->GetCurrentObject();
	CImageModel* img = dynamic_cast<CImageModel*>(po);
	if (img == nullptr)
	{
		QMessageBox::critical(this, "PostView", "Please select an image data set first.");
	}
	else
	{
		CImageSlicer* slicer = new CImageSlicer(img);
		slicer->Create();
		img->AddImageRenderer(slicer);
		ui->modelViewer->Update(true);
		ui->modelViewer->selectObject(slicer);
		RedrawGL();
	}
*/
}

void CMainWindow::on_actionVolumeRender_triggered()
{
/*	CGLObject* po = ui->modelViewer->selectedObject();
	CImageModel* img = dynamic_cast<CImageModel*>(po);
	if (img == nullptr)
	{
		QMessageBox::critical(this, "PostView", "Please select an image data set first.");
	}
	else
	{
		CVolRender* vr = new CVolRender(img);
		vr->Create();
		img->AddImageRenderer(vr);
		ui->modelViewer->Update(true);
		ui->modelViewer->selectObject(vr);
		RedrawGL();
	}
*/
}

void CMainWindow::on_actionMarchingCubes_triggered()
{
/*	CGLObject* po = ui->modelViewer->selectedObject();
	CImageModel* img = dynamic_cast<CImageModel*>(po);
	if (img == nullptr)
	{
		QMessageBox::critical(this, "PostView", "Please select an image data set first.");
	}
	else
	{
		CMarchingCubes* mc = new CMarchingCubes(img);
		mc->Create();
		img->AddImageRenderer(mc);
		ui->modelViewer->Update(true);
		ui->modelViewer->selectObject(mc);
		RedrawGL();
	}
*/
}

void CMainWindow::on_actionIsosurfacePlot_triggered()
{
/*	CDocument* doc = GetActiveDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	CGLIsoSurfacePlot* pp = new CGLIsoSurfacePlot(doc->GetGLModel());
	doc->AddPlot(pp);
	doc->UpdateFEModel();

	ui->modelViewer->Update(true);
	ui->modelViewer->selectObject(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
*/
}

void CMainWindow::on_actionSlicePlot_triggered()
{
/*	CDocument* doc = GetDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	CGLSlicePlot* pp = new CGLSlicePlot(doc->GetGLModel());
	doc->AddPlot(pp);
	doc->UpdateFEModel();

	ui->modelViewer->Update(true);
	ui->modelViewer->selectObject(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
*/
}

void CMainWindow::on_actionDisplacementMap_triggered()
{
/*	CDocument* doc = GetDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	CGLModel* pm = doc->GetGLModel();
	if (pm->GetDisplacementMap() == 0)
	{
		if (pm->AddDisplacementMap() == false)
		{
			QMessageBox::warning(this, "PostView", "You need at least one vector field before you can define a displacement map.");
		}
		else
		{
			doc->UpdateFEModel(true);
			ui->modelViewer->Update(true);
		}
	}
	else
	{
		QMessageBox::information(this, "PostView", "This model already has a displacement map.");
	}
*/
}

void CMainWindow::on_actionGraph_triggered()
{
/*	CGraphWindow* pg = new CModelGraphWindow(this);
	AddGraph(pg);

	pg->show();
	pg->raise();
	pg->activateWindow();
	pg->Update();
*/
}

void CMainWindow::on_actionSummary_triggered()
{
/*	CSummaryWindow* summaryWindow = new CSummaryWindow(this);

	summaryWindow->Update(true);
	summaryWindow->show();
	summaryWindow->raise();
	summaryWindow->activateWindow();

	AddGraph(summaryWindow);
*/
}

void CMainWindow::on_actionStats_triggered()
{
/*	CStatsWindow* statsWindow = new CStatsWindow(this);
	statsWindow->Update(true);
	statsWindow->show();
	statsWindow->raise();
	statsWindow->activateWindow();

	AddGraph(statsWindow);
*/
}

void CMainWindow::on_actionIntegrate_triggered()
{
/*	CIntegrateWindow* integrateWindow = new CIntegrateWindow(this);
	integrateWindow->Update(true);
	integrateWindow->show();
	integrateWindow->raise();
	integrateWindow->activateWindow();

	AddGraph(integrateWindow);
*/
}
