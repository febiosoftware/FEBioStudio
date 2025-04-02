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
#include "ui_config.h"
#include "ui_mainwindow.h"
#include "TextDocument.h"
#include "../FEBioMonitor/FEBioReportDoc.h"

// configuration base class
void Ui::CUIConfig::Apply()
{
	// disable some File menu items
	ui->actionImportGeom->setEnabled(false);
	ui->actionExportGeom->setEnabled(false);
	ui->actionExportFE->setEnabled(false);
	ui->menuRecentGeomFiles->menuAction()->setEnabled(false);
	ui->menuImportImage->menuAction()->setEnabled(false);
	ui->actionSnapShot->setEnabled(false);
	ui->actionRayTrace->setEnabled(false);

	// disable some FEBio menu items
	ui->actionFEBioRun->setEnabled(false);
	ui->actionFEBioStop->setEnabled(false);
	ui->actionFEBioCheck->setEnabled(false);

	// disable some Tools menu items
	ui->actionCurveEditor->setEnabled(false);
	ui->actionMeshInspector->setEnabled(false);
	ui->actionMeshDiagnostic->setEnabled(false);
	ui->actionMaterialTest->setEnabled(false);

	// hide some toolbars
	ui->monitorToolBar->hide();
}

// Configure for no active document
void Ui::CEmptyConfig::Apply()
{
	Ui::CUIConfig::Apply();

	ui->menuEdit->menuAction()->setVisible(false);
	ui->menuEditPost->menuAction()->setVisible(false);
	ui->menuEditTxt->menuAction()->setVisible(false);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(false);
	ui->menuPost->menuAction()->setVisible(false);
	ui->menuRecord->menuAction()->setVisible(false);
	ui->menuView->menuAction()->setVisible(false);

	ui->buildToolBar->hide();
	ui->postToolBar->hide();
	ui->imageToolBar->hide();
	ui->pFontToolBar->hide();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->HideControlBar();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
	ui->febioMonitor->parentWidget()->hide();
	ui->febioMonitorView->parentWidget()->hide();
#ifdef HAS_PYTHON
    ui->pythonToolsPanel->parentWidget()->hide();
#endif

	ui->fileViewer->parentWidget()->raise();

	ui->setActiveCentralView(CMainCentralWidget::HTML_VIEWER);
	ui->ShowDefaultBackground();
}

// Configure for HTML document
void Ui::CHTMLConfig::Apply()
{
	Ui::CUIConfig::Apply();

	ui->setActiveCentralView(CMainCentralWidget::HTML_VIEWER);

	ui->menuEdit->menuAction()->setVisible(false);
	ui->menuEditPost->menuAction()->setVisible(false);
	ui->menuEditTxt->menuAction()->setVisible(false);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(false);
	ui->menuPost->menuAction()->setVisible(false);
	ui->menuRecord->menuAction()->setVisible(false);
	ui->menuView->menuAction()->setVisible(false);

	ui->buildToolBar->hide();
	ui->postToolBar->hide();
	ui->imageToolBar->hide();
	ui->pFontToolBar->hide();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->HideControlBar();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
	ui->febioMonitor->parentWidget()->hide();
	ui->febioMonitorView->parentWidget()->hide();
#ifdef HAS_PYTHON
    ui->pythonToolsPanel->parentWidget()->hide();
#endif

	ui->fileViewer->parentWidget()->raise();
}

// Configure for model document
void Ui::CModelConfig::Apply()
{
	Ui::CUIConfig::Apply();

	::CMainWindow* wnd = ui->m_wnd;
	CGLDocument* doc = wnd->GetGLDocument();
	if (doc)
	{
		switch (doc->GetUIViewMode())
		{
		case CGLDocument::MODEL_VIEW:
			ui->centralWidget->setActiveView(CMainCentralWidget::GL_VIEWER);
			ui->modelViewer->SetFilter(FILTER_NONE);
			break;
		case CGLDocument::SLICE_VIEW:
			ui->centralWidget->setActiveView(CMainCentralWidget::IMG_SLICE);
			ui->modelViewer->SetFilter(FILTER_IMAGES);
			break;
		case CGLDocument::TIME_VIEW_2D:
			ui->centralWidget->setActiveView(CMainCentralWidget::TIME_VIEW_2D);
			ui->modelViewer->SetFilter(FILTER_IMAGES);
			break;
		}
	}

	ui->actionFEBioMonitor->setEnabled(true);
	ui->actionImportGeom->setEnabled(true);
	ui->actionExportGeom->setEnabled(true);
	ui->actionExportFE->setEnabled(true);
	ui->actionSnapShot->setEnabled(true);
	ui->actionRayTrace->setEnabled(true);
	ui->actionFEBioRun->setEnabled(true);
	ui->actionFEBioStop->setEnabled(true);
	ui->actionFEBioCheck->setEnabled(true);

	ui->actionCurveEditor->setEnabled(true);
	ui->actionMeshInspector->setEnabled(true);
	ui->actionMeshDiagnostic->setEnabled(true);
	ui->actionMaterialTest->setEnabled(true);

	ui->menuFEBio->menuAction()->setVisible(true);
	ui->menuRecentGeomFiles->menuAction()->setEnabled(true);
	ui->menuImportImage->menuAction()->setEnabled(true);
	ui->menuEdit->menuAction()->setVisible(true);
	ui->menuEditPost->menuAction()->setVisible(false);
	ui->menuEditTxt->menuAction()->setVisible(false);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(true);
	ui->menuPost->menuAction()->setVisible(false);
	ui->menuRecord->menuAction()->setVisible(true);
	ui->menuView->menuAction()->setVisible(true);

	ui->buildToolBar->show();
	ui->postToolBar->hide();
	ui->imageToolBar->show();
	ui->pFontToolBar->show();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->ShowControlBar();

	ui->modelViewer->parentWidget()->show();
	ui->buildPanel->parentWidget()->show();
	ui->postPanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->show();
	ui->infoPanel->parentWidget()->show();
	ui->timePanel->parentWidget()->hide();
	ui->febioMonitor->parentWidget()->hide();
	ui->febioMonitorView->parentWidget()->hide();
#ifdef HAS_PYTHON
    ui->pythonToolsPanel->parentWidget()->show();
#endif

	wnd->UpdateUiView();
	ui->modelViewer->parentWidget()->raise();
}

// Configure for "post" document
void Ui::CPostConfig::Apply()
{
	Ui::CUIConfig::Apply();

    ::CMainWindow* wnd = ui->m_wnd;
    CGLDocument* doc = wnd->GetGLDocument();
	if (doc)
	{
		switch (doc->GetUIViewMode())
		{
		case CGLDocument::MODEL_VIEW:
			ui->centralWidget->setActiveView(CMainCentralWidget::GL_VIEWER);
			break;
		case CGLDocument::SLICE_VIEW:
			ui->centralWidget->setActiveView(CMainCentralWidget::IMG_SLICE);
			break;
		case CGLDocument::TIME_VIEW_2D:
			ui->centralWidget->setActiveView(CMainCentralWidget::TIME_VIEW_2D);
			break;
		}
	}

	ui->actionImportGeom->setEnabled(true);
	ui->actionExportGeom->setEnabled(true);
	ui->menuRecentGeomFiles->menuAction()->setEnabled(true);
	ui->menuImportImage->menuAction()->setEnabled(true);
	ui->actionSnapShot->setEnabled(true);
	ui->actionRayTrace->setEnabled(true);

	ui->actionMeshInspector->setEnabled(true);
	ui->actionMeshDiagnostic->setEnabled(true);

	ui->menuEdit->menuAction()->setVisible(false);
	ui->menuEditPost->menuAction()->setVisible(true);
	ui->menuEditTxt->menuAction()->setVisible(false);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(false);
	ui->menuPost->menuAction()->setVisible(true);
	ui->menuRecord->menuAction()->setVisible(true);
	ui->menuView->menuAction()->setVisible(true);

	ui->buildToolBar->hide();
	ui->postToolBar->show();
	ui->imageToolBar->show();
	ui->pFontToolBar->show();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->ShowControlBar();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->show();
	ui->timePanel->parentWidget()->show();
	ui->logPanel->parentWidget()->show();
	ui->infoPanel->parentWidget()->show();
	ui->febioMonitor->parentWidget()->hide();
	ui->febioMonitorView->parentWidget()->hide();
#ifdef HAS_PYTHON
    ui->pythonToolsPanel->parentWidget()->show();
#endif

	ui->showTimeline();

	wnd->UpdatePostPanel();
	CPostDocument* postDoc = wnd->GetPostDocument(); assert(postDoc);
	if (postDoc && postDoc->IsValid()) ui->postToolBar->Update();
	else ui->postToolBar->setDisabled(true);

	if (ui->timePanel && ui->timePanel->isVisible()) ui->timePanel->Update(true);

	ui->postPanel->parentWidget()->raise();

	wnd->RedrawGL();
}

// Configure for txt document
void Ui::CTextConfig::Apply()
{
	Ui::CUIConfig::Apply();

	ui->setActiveCentralView(CMainCentralWidget::TEXT_VIEWER);

	ui->menuEdit->menuAction()->setVisible(false);
	ui->menuEditPost->menuAction()->setVisible(false);
	ui->menuEditTxt->menuAction()->setVisible(true);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(false);
	ui->menuPost->menuAction()->setVisible(false);
	ui->menuRecord->menuAction()->setVisible(false);
	ui->menuView->menuAction()->setVisible(false);

	ui->actionFEBioRun->setEnabled(true);
	ui->actionFEBioStop->setEnabled(true);
	ui->actionFEBioCheck->setEnabled(true);

	ui->buildToolBar->hide();
	ui->postToolBar->hide();
	ui->imageToolBar->hide();
	ui->pFontToolBar->hide();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->HideControlBar();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
	ui->febioMonitor->parentWidget()->hide();
	ui->febioMonitorView->parentWidget()->hide();
#ifdef HAS_PYTHON
    ui->pythonToolsPanel->parentWidget()->hide();
#endif
}

// Configure for XML document
void Ui::CXMLConfig::Apply()
{
	Ui::CUIConfig::Apply();

	::CMainWindow* wnd = ui->m_wnd;
	CXMLDocument* xmlDoc = dynamic_cast<CXMLDocument*>(wnd->GetDocument());
	if (xmlDoc)
	{
		ui->centralWidget->xmlTree->setModel(xmlDoc->GetModel());

		ui->actionEditXmlAsText->blockSignals(true);
		ui->actionEditXmlAsText->setChecked(xmlDoc->EditingText());
		ui->actionEditXmlAsText->blockSignals(false);

		ui->actionFEBioRun->setEnabled(true);
		ui->actionFEBioStop->setEnabled(true);
		ui->actionFEBioCheck->setEnabled(true);

		if (xmlDoc->EditingText())
		{
			ui->setActiveCentralView(CMainCentralWidget::TEXT_VIEWER);

			ui->menuEdit->menuAction()->setVisible(false);
			ui->menuEditPost->menuAction()->setVisible(false);
			ui->menuEditTxt->menuAction()->setVisible(true);
			ui->menuEditXml->menuAction()->setVisible(false);
			ui->menuPhysics->menuAction()->setVisible(false);
			ui->menuPost->menuAction()->setVisible(false);
			ui->menuRecord->menuAction()->setVisible(false);
			ui->menuView->menuAction()->setVisible(false);

			ui->buildToolBar->hide();
			ui->postToolBar->hide();
			ui->pFontToolBar->hide();
			ui->xmlToolbar->show();
			ui->imageToolBar->hide();

			ui->centralWidget->glw->HideControlBar();

			ui->modelViewer->parentWidget()->hide();
			ui->buildPanel->parentWidget()->hide();
			ui->postPanel->parentWidget()->hide();
			ui->logPanel->parentWidget()->hide();
			ui->infoPanel->parentWidget()->hide();
			ui->timePanel->parentWidget()->hide();
			ui->imageSettingsPanel->parentWidget()->hide();
			ui->febioMonitor->parentWidget()->hide();
			ui->febioMonitorView->parentWidget()->hide();

			for (int index = 1; index < ui->xmlToolbar->actions().size(); index++)
			{
				ui->xmlToolbar->actions()[index]->setVisible(false);
			}
		}
		else
		{
			ui->setActiveCentralView(CMainCentralWidget::XML_VIEWER);

			ui->menuEdit->menuAction()->setVisible(false);
			ui->menuEditPost->menuAction()->setVisible(false);
			ui->menuEditTxt->menuAction()->setVisible(false);
			ui->menuEditXml->menuAction()->setVisible(true);
			ui->menuPhysics->menuAction()->setVisible(false);
			ui->menuPost->menuAction()->setVisible(false);
			ui->menuRecord->menuAction()->setVisible(false);

			ui->buildToolBar->hide();
			ui->postToolBar->hide();
			ui->pFontToolBar->hide();
			ui->xmlToolbar->show();
			ui->imageToolBar->hide();

			ui->centralWidget->glw->HideControlBar();

			ui->modelViewer->parentWidget()->hide();
			ui->buildPanel->parentWidget()->hide();
			ui->postPanel->parentWidget()->hide();
			ui->logPanel->parentWidget()->hide();
			ui->infoPanel->parentWidget()->hide();
			ui->timePanel->parentWidget()->hide();
			ui->imageSettingsPanel->parentWidget()->hide();
			ui->febioMonitor->parentWidget()->hide();
			ui->febioMonitorView->parentWidget()->hide();
#ifdef HAS_PYTHON
            ui->pythonToolsPanel->parentWidget()->hide();
#endif

			for (auto action : ui->xmlToolbar->actions())
			{
				action->setVisible(true);
			}
		}
	}
}

// Configure for app document
void Ui::CAPPConfig::Apply()
{
	CUIConfig::Apply();

	ui->menuEdit->menuAction()->setVisible(false);
	ui->menuEditPost->menuAction()->setVisible(false);
	ui->menuEditTxt->menuAction()->setVisible(false);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(false);
	ui->menuPost->menuAction()->setVisible(false);
	ui->menuRecord->menuAction()->setVisible(false);
	ui->menuView->menuAction()->setVisible(false);

	ui->buildToolBar->hide();
	ui->postToolBar->hide();
	ui->imageToolBar->hide();
	ui->pFontToolBar->hide();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->HideControlBar();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->febioMonitor->parentWidget()->hide();
	ui->febioMonitorView->parentWidget()->hide();

	ui->setActiveCentralView(CMainCentralWidget::APP_VIEWER);
	ui->fileViewer->parentWidget()->raise();
}

// Configure for app document
void Ui::CMonitorConfig::Apply()
{
	CUIConfig::Apply();

	ui->centralWidget->setActiveView(CMainCentralWidget::GL_VIEWER);

	ui->actionFEBioStop->setEnabled(true);
	ui->actionFEBioMonitor->setEnabled(false);

	ui->menuEdit->menuAction()->setVisible(false);
	ui->menuEditPost->menuAction()->setVisible(true);
	ui->menuEditTxt->menuAction()->setVisible(false);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(false);
	ui->menuPost->menuAction()->setVisible(true);
	ui->menuRecord->menuAction()->setVisible(true);
	ui->menuView->menuAction()->setVisible(true);

	ui->buildToolBar->hide();
	ui->postToolBar->hide();
	ui->imageToolBar->hide();
	ui->pFontToolBar->hide();
	ui->xmlToolbar->hide();
	ui->monitorToolBar->show();

	ui->centralWidget->glw->ShowControlBar();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();

	ui->febioMonitor->parentWidget()->show();
	ui->febioMonitor->parentWidget()->raise();

	ui->febioMonitorView->parentWidget()->show();
	ui->febioMonitorView->parentWidget()->raise();

	ui->m_wnd->ShowLogPanel();
	ui->logPanel->ShowLog(::CLogPanel::FEBIO_LOG);
}

// Configure for app document
void Ui::CFEBReportConfig::Apply()
{
	CUIConfig::Apply();

	ui->setActiveCentralView(CMainCentralWidget::FEBREPORT_VIEW);

	ui->menuEdit->menuAction()->setVisible(false);
	ui->menuEditPost->menuAction()->setVisible(false);
	ui->menuEditTxt->menuAction()->setVisible(false);
	ui->menuEditXml->menuAction()->setVisible(false);
	ui->menuPhysics->menuAction()->setVisible(false);
	ui->menuPost->menuAction()->setVisible(false);
	ui->menuRecord->menuAction()->setVisible(false);
	ui->menuView->menuAction()->setVisible(false);
	ui->menuFEBio->menuAction()->setVisible(false);

	ui->buildToolBar->hide();
	ui->postToolBar->hide();
	ui->imageToolBar->hide();
	ui->pFontToolBar->hide();
	ui->xmlToolbar->hide();
	ui->monitorToolBar->hide();

	ui->centralWidget->glw->ShowControlBar();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->febioMonitor->parentWidget()->hide();
	ui->febioMonitorView->parentWidget()->hide();
}
