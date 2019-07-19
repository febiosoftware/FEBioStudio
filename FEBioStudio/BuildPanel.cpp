#include "stdafx.h"
#include "BuildPanel.h"
#include "ui_buildpanel.h"

CBuildPanel::CBuildPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CBuildPanel)
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
}

void CBuildPanel::on_buildTab_currentChanged(int index)
{
	Update();
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
