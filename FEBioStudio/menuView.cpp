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
#include "ui_mainwindow.h"
#include "Document.h"
#include "PostDocument.h"
#include "DocManager.h"
#include <PostLib/GLModel.h>
#include "GLPostScene.h"

void CMainWindow::on_actionUndoViewChange_triggered()
{
	GetGLView()->UndoViewChange();
}

void CMainWindow::on_actionRedoViewChange_triggered()
{
	GetGLView()->RedoViewChange();
}

void CMainWindow::on_actionZoomSelect_triggered()
{
	GetGLView()->ZoomSelection();
}

void CMainWindow::on_actionZoomExtents_triggered()
{
	GetGLView()->ZoomExtents();
}

void CMainWindow::on_actionViewCapture_toggled(bool bchecked)
{
	GetGLView()->ShowSafeFrame(bchecked);
	RedrawGL();
}

void CMainWindow::on_actionOrtho_toggled(bool b)
{
	GetGLView()->TogglePerspective(b);
}

void CMainWindow::on_actionShowGrid_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_bgrid = b;
	RedrawGL();
}

void CMainWindow::on_actionShowMeshLines_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_bmesh = b;
	Update(this);
}

void CMainWindow::on_actionShowEdgeLines_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_bfeat = b;
	Update(this);
}

void CMainWindow::on_actionBackfaceCulling_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_bcull = b;
	Update(this);
}

void CMainWindow::on_actionViewSmooth_toggled(bool bchecked)
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	Post::CGLModel* po = doc->GetGLModel();
	if (po)
	{
		Post::CGLColorMap* pcm = po->GetColorMap();
		if (pcm)
		{
			pcm->SetColorSmooth(bchecked);
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionShowNormals_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_bnorm = b;
	RedrawGL();
}

void CMainWindow::on_actionShowFibers_triggered()
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return;

	if (ui->fiberViz == nullptr) ui->fiberViz = new CDlgFiberViz(this);

	if (ui->fiberViz->isVisible()) ui->fiberViz->close();
	else ui->fiberViz->show();

	RedrawGL();
}

void CMainWindow::on_actionShowMatAxes_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_blma = b;
	RedrawGL();
}

void CMainWindow::on_actionShowDiscrete_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_showDiscrete = b;
	RedrawGL();
}

void CMainWindow::on_actionShowRigidBodies_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_brigid = b;
	RedrawGL();
}

void CMainWindow::on_actionShowRigidJoints_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_bjoint = b;
	RedrawGL();
}

void CMainWindow::on_actionShowRigidLabels_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_showRigidLabels = b;
	RedrawGL();
}

void CMainWindow::on_actionToggleTagInfo_triggered()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	int n = view.m_ntagInfo;
	view.m_ntagInfo = (n + 1) % 3;
	RedrawGL();
}

void CMainWindow::on_actionToggleLight_triggered()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_bLighting = !view.m_bLighting;
	RedrawGL();
}

void CMainWindow::on_actionFront_triggered()
{
	GetGLView()->changeViewMode(VIEW_FRONT);
}

void CMainWindow::on_actionBack_triggered()
{
	GetGLView()->changeViewMode(VIEW_BACK);
}

void CMainWindow::on_actionLeft_triggered()
{
	GetGLView()->changeViewMode(VIEW_LEFT);
}

void CMainWindow::on_actionRight_triggered()
{
	GetGLView()->changeViewMode(VIEW_RIGHT);
}

void CMainWindow::on_actionTop_triggered()
{
	GetGLView()->changeViewMode(VIEW_TOP);
}

void CMainWindow::on_actionBottom_triggered()
{
	GetGLView()->changeViewMode(VIEW_BOTTOM);
}

void CMainWindow::on_actionIsometric_triggered()
{
    GetGLView()->changeViewMode(VIEW_ISOMETRIC);
}

void CMainWindow::on_actionRenderMode_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	GLViewSettings& view = GetGLView()->GetViewSettings();
	view.m_nrender = (b ? RENDER_WIREFRAME : RENDER_SOLID);
	RedrawGL();
}

void CMainWindow::on_actionSnap3D_triggered()
{
	vec3d r = GetGLView()->GetPivotPosition();
	GetGLView()->Set3DCursor(r);
	RedrawGL();
}

void CMainWindow::on_actionTrack_triggered()
{
	CPostDocument* pdoc = GetPostDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	CGLPostScene* scene = dynamic_cast<CGLPostScene*>(pdoc->GetScene());
	if (scene) scene->ToggleTrackSelection();

	RedrawGL();
}

void CMainWindow::on_actionViewVPSave_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	CGView& view = *doc->GetView();
	CGLCamera& cam = view.GetCamera();
	GLCameraTransform t;
	cam.GetTransform(t);

	static int n = 0; n++;
	char szname[64] = { 0 };
	snprintf(szname, sizeof szname, "ViewPoint%02d", n);
	t.SetName(szname);
	GLCameraTransform* vp = view.AddCameraKey(t);
	ui->postPanel->Update();
	ui->postPanel->SelectObject(vp);

}

void CMainWindow::on_actionViewVPPrev_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	CGView& view = *doc->GetView();
	if (view.CameraKeys() > 0)
	{
		view.PrevKey();
		view.GetCamera().SetTransform(view.GetCurrentKey());
		RedrawGL();
	}
}

void CMainWindow::on_actionViewVPNext_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;

	CGView& view = *doc->GetView();
	if (view.CameraKeys() > 0)
	{
		view.NextKey();
		view.GetCamera().SetTransform(view.GetCurrentKey());
		RedrawGL();
	}
}

// sync the views of all documents to the currently active one
void CMainWindow::on_actionSyncViews_triggered()
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	CGView& view = *doc->GetView();
	CGLCamera& cam = view.GetCamera();
	GLCameraTransform transform;
	cam.GetTransform(transform);
	CDocManager* DM = GetDocManager();
	int docs = DM->Documents();
	for (int i = 0; i < docs; ++i)
	{
		CGLDocument* doci = dynamic_cast<CGLDocument*>(DM->GetDocument(i));
		if (doci && (doci != doc))
		{
			CGView& viewi = *doci->GetView();

			// copy the transforms
			CGLCamera& cami = viewi.GetCamera();
			cami.SetTransform(transform);
			cami.Update(true);

			// copy view parameters
			viewi.m_bortho = view.m_bortho;
		}
	}
}

void CMainWindow::on_actionToggleConnected_triggered()
{
	ui->centralWidget->glw->ToggleSelectConnected();
}

void CMainWindow::on_actionToggleFPS_triggered()
{
	GetGLView()->ToggleFPS();
	RedrawGL();
}
