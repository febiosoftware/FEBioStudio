#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "Document.h"
#include "PostDocument.h"
#include <PostGL/GLModel.h>

void CMainWindow::on_actionUndoViewChange_triggered()
{
	ui->glview->UndoViewChange();
}

void CMainWindow::on_actionRedoViewChange_triggered()
{
	ui->glview->RedoViewChange();
}

void CMainWindow::on_actionZoomSelect_triggered()
{
	ui->glview->ZoomSelection();
}

void CMainWindow::on_actionZoomExtents_triggered()
{
	ui->glview->ZoomExtents();
}

void CMainWindow::on_actionViewCapture_toggled(bool bchecked)
{
	ui->glview->showSafeFrame(bchecked);
	RedrawGL();
}

void CMainWindow::on_actionOrtho_toggled(bool b)
{
	ui->glview->TogglePerspective(b);
}

void CMainWindow::on_actionShowGrid_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_bgrid = b;
	RedrawGL();
}

void CMainWindow::on_actionShowMeshLines_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_bmesh = b;
	Update(this);
}

void CMainWindow::on_actionShowEdgeLines_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_bfeat = b;
	Update(this);
}

void CMainWindow::on_actionBackfaceCulling_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
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

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_bnorm = b;
	RedrawGL();
}

void CMainWindow::on_actionShowFibers_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_bfiber = b;
	RedrawGL();
}

void CMainWindow::on_actionShowMatAxes_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_blma = b;
	RedrawGL();
}

void CMainWindow::on_actionShowDiscrete_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_showDiscrete = b;
	RedrawGL();
}

void CMainWindow::on_actionToggleLight_triggered()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_bLighting = !view.m_bLighting;
	RedrawGL();
}

void CMainWindow::on_actionFront_triggered()
{
	ui->glview->changeViewMode(VIEW_FRONT);
}

void CMainWindow::on_actionBack_triggered()
{
	ui->glview->changeViewMode(VIEW_BACK);
}

void CMainWindow::on_actionLeft_triggered()
{
	ui->glview->changeViewMode(VIEW_LEFT);
}

void CMainWindow::on_actionRight_triggered()
{
	ui->glview->changeViewMode(VIEW_RIGHT);
}

void CMainWindow::on_actionTop_triggered()
{
	ui->glview->changeViewMode(VIEW_TOP);
}

void CMainWindow::on_actionBottom_triggered()
{
	ui->glview->changeViewMode(VIEW_BOTTOM);
}

void CMainWindow::on_actionWireframe_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	VIEW_SETTINGS& view = ui->glview->GetViewSettings();
	view.m_nrender = (b ? RENDER_WIREFRAME : RENDER_SOLID);
	RedrawGL();
}

void CMainWindow::on_actionSnap3D_triggered()
{
	vec3d r = GetGLView()->GetPivotPosition();
	ui->glview->Set3DCursor(r);
	RedrawGL();
}

void CMainWindow::on_actionTrack_toggled(bool b)
{
	ui->glview->TrackSelection(b);
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
	sprintf(szname, "key%02d", n);
	t.SetName(szname);
	view.AddCameraKey(t);
	ui->postPanel->Update();
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
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	CGView& view = *doc->GetView();
	CGLCamera& cam = view.GetCamera();
	GLCameraTransform transform;
	cam.GetTransform(transform);
	int views = ui->tab->views();
	for (int i = 1; i < views; ++i)
	{
		CDocument* doci = ui->tab->getDocument(i);
		if (doci != doc)
		{
			CGLCamera& cami = doci->GetView()->GetCamera();

			// copy the transforms
			cami.SetTransform(transform);
			cami.Update(true);
		}
	}
}

void CMainWindow::on_actionToggleConnected_triggered()
{
	ui->glc->toggleSelectConnected();
}
