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

	// disable some FEBio menu items
	ui->actionFEBioRun->setEnabled(false);
	ui->actionFEBioStop->setEnabled(false);

	// disable some Tools menu items
	ui->actionCurveEditor->setEnabled(false);
	ui->actionMeshInspector->setEnabled(false);
	ui->actionMeshDiagnostic->setEnabled(false);
	ui->actionMaterialTest->setEnabled(false);
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

	ui->centralWidget->glw->glc->hide();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
#ifdef HAS_PYTHON
    ui->pythonToolsPanel->parentWidget()->hide();
#endif

	ui->fileViewer->parentWidget()->raise();

	ui->centralWidget->setActiveView(CMainCentralWidget::HTML_VIEWER);
	ui->ShowDefaultBackground();
}

// Configure for HTML document
void Ui::CHTMLConfig::Apply()
{
	Ui::CUIConfig::Apply();

	::CMainWindow* wnd = ui->m_wnd;
	CHTMLDocument* htmlDoc = dynamic_cast<CHTMLDocument*>(wnd->GetDocument()); assert(htmlDoc);

	if (htmlDoc) ui->centralWidget->htmlViewer->setDocument(htmlDoc->GetText());
	ui->centralWidget->setActiveView(CMainCentralWidget::HTML_VIEWER);

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

	ui->centralWidget->glw->glc->hide();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
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

	ui->actionImportGeom->setEnabled(true);
	ui->actionExportGeom->setEnabled(true);
	ui->actionExportFE->setEnabled(true);
	ui->menuRecentGeomFiles->menuAction()->setEnabled(true);
	ui->menuImportImage->menuAction()->setEnabled(true);
	ui->actionSnapShot->setEnabled(true);

	ui->actionFEBioRun->setEnabled(true);
	ui->actionFEBioStop->setEnabled(true);

	ui->actionCurveEditor->setEnabled(true);
	ui->actionMeshInspector->setEnabled(true);
	ui->actionMeshDiagnostic->setEnabled(true);
	ui->actionMaterialTest->setEnabled(true);

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

	ui->centralWidget->glw->glc->show();

	ui->modelViewer->parentWidget()->show();
	ui->buildPanel->parentWidget()->show();
	ui->postPanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->show();
	ui->infoPanel->parentWidget()->show();
	ui->timePanel->parentWidget()->hide();
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

	ui->centralWidget->setActiveView(CMainCentralWidget::GL_VIEWER);

	ui->actionImportGeom->setEnabled(true);
	ui->actionExportGeom->setEnabled(true);
	ui->menuRecentGeomFiles->menuAction()->setEnabled(true);
	ui->menuImportImage->menuAction()->setEnabled(true);
	ui->actionSnapShot->setEnabled(true);

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
	ui->imageToolBar->hide();
	ui->pFontToolBar->show();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->glc->show();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->show();
	ui->timePanel->parentWidget()->show();
	ui->logPanel->parentWidget()->show();
	ui->infoPanel->parentWidget()->show();
	ui->imageSettingsPanel->parentWidget()->hide();
#ifdef HAS_PYTHON
    ui->pythonToolsPanel->parentWidget()->show();
#endif

	ui->showTimeline();

	::CMainWindow* wnd = ui->m_wnd;
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

	::CMainWindow* wnd = ui->m_wnd;
	CTextDocument* txtDoc = dynamic_cast<CTextDocument*>(wnd->GetDocument()); assert(txtDoc);

	if (txtDoc)
	{
		ui->centralWidget->xmlEdit->blockSignals(true);
		ui->centralWidget->xmlEdit->SetDocument(txtDoc->GetText());
		ui->centralWidget->xmlEdit->blockSignals(false);
	}

	ui->centralWidget->setActiveView(CMainCentralWidget::TEXT_VIEWER);

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

	ui->buildToolBar->hide();
	ui->postToolBar->hide();
	ui->imageToolBar->hide();
	ui->pFontToolBar->hide();
	ui->xmlToolbar->hide();

	ui->centralWidget->glw->glc->hide();

	ui->modelViewer->parentWidget()->hide();
	ui->buildPanel->parentWidget()->hide();
	ui->postPanel->parentWidget()->hide();
	ui->logPanel->parentWidget()->hide();
	ui->infoPanel->parentWidget()->hide();
	ui->timePanel->parentWidget()->hide();
	ui->imageSettingsPanel->parentWidget()->hide();
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

		if (xmlDoc->EditingText())
		{
			ui->centralWidget->setActiveView(CMainCentralWidget::TEXT_VIEWER);
			ui->centralWidget->xmlEdit->SetDocument(xmlDoc->GetTextDocument());

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

			ui->centralWidget->glw->glc->hide();

			ui->modelViewer->parentWidget()->hide();
			ui->buildPanel->parentWidget()->hide();
			ui->postPanel->parentWidget()->hide();
			ui->logPanel->parentWidget()->hide();
			ui->infoPanel->parentWidget()->hide();
			ui->timePanel->parentWidget()->hide();
			ui->imageSettingsPanel->parentWidget()->hide();

			for (int index = 1; index < ui->xmlToolbar->actions().size(); index++)
			{
				ui->xmlToolbar->actions()[index]->setVisible(false);
			}
		}
		else
		{
			ui->centralWidget->setActiveView(CMainCentralWidget::XML_VIEWER);

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

			ui->centralWidget->glw->glc->hide();

			ui->modelViewer->parentWidget()->hide();
			ui->buildPanel->parentWidget()->hide();
			ui->postPanel->parentWidget()->hide();
			ui->logPanel->parentWidget()->hide();
			ui->infoPanel->parentWidget()->hide();
			ui->timePanel->parentWidget()->hide();
			ui->imageSettingsPanel->parentWidget()->hide();
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
