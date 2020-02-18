#include "stdafx.h"
#include "BuildPanel.h"
#include "ui_buildpanel.h"
#include "MainWindow.h"
#include "Document.h"

CBuildPanel::CBuildPanel(::CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CBuildPanel)
{
	ui->setup(this, wnd);
}

CCreatePanel* CBuildPanel::CreatePanel()
{
	return ui->create;
}

void CBuildPanel::Update()
{
	CCommandPanel* p = ui->currentPanel();
	if (p) p->Update();
	ui->mainWindow->UpdateGLControlBar();
	ui->mainWindow->GetDocument()->UpdateSelection();
	ui->mainWindow->RedrawGL();
}

void CBuildPanel::on_buildTab_currentChanged(int index)
{
	Update();
}

bool CBuildPanel::IsEditPanelVisible()
{
	return (ui->tab->currentIndex() == Ui::CBuildPanel::EDIT_PANEL);
}

bool CBuildPanel::IsMeshPanelVisible()
{
	return (ui->tab->currentIndex() == Ui::CBuildPanel::MESH_PANEL);
}

bool CBuildPanel::OnEscapeEvent()
{
	CCommandPanel* p = ui->currentPanel();
	if (p) return p->OnEscapeEvent();
	else return false;
}

bool CBuildPanel::OnDeleteEvent()
{
	CCommandPanel* p = ui->currentPanel();
	if (p) return p->OnDeleteEvent();
	else return false;
}

void CBuildPanel::Apply()
{
	CCommandPanel* p = ui->currentPanel();
	if (p) p->Apply();
}
