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
#include "BuildPanel.h"
#include "ui_buildpanel.h"
#include "MainWindow.h"
#include "GLDocument.h"

CBuildPanel::CBuildPanel(::CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CBuildPanel)
{
	ui->setup(this, wnd);
}

CCreatePanel* CBuildPanel::CreatePanel()
{
	return ui->create;
}

void CBuildPanel::Update(bool breset)
{
	CWindowPanel* p = ui->currentPanel();
	if (p) p->Update(breset);
	CGLDocument* doc = ui->mainWindow->GetGLDocument();
	if (doc)
	{
		ui->mainWindow->UpdateGLControlBar();
		doc->UpdateSelection(false);
		ui->mainWindow->RedrawGL();
	}
}

void CBuildPanel::on_buildTab_currentChanged(int index)
{
	Update(true);
}

CWindowPanel* CBuildPanel::GetActivePanel()
{
	if (IsEditPanelVisible()) return ui->edit;
	if (IsMeshPanelVisible()) return ui->mesh;
	if (IsCreatePanelVisible()) return ui->mesh;
	if (IsToolsPanelVisible()) return ui->tools;
	return nullptr;
}

bool CBuildPanel::IsEditPanelVisible()
{
	return (ui->tab->currentIndex() == Ui::CBuildPanel::EDIT_PANEL);
}

bool CBuildPanel::IsMeshPanelVisible()
{
	return (ui->tab->currentIndex() == Ui::CBuildPanel::MESH_PANEL);
}

bool CBuildPanel::IsCreatePanelVisible()
{
	return (ui->tab->currentIndex() == Ui::CBuildPanel::CREATE_PANEL);
}

bool CBuildPanel::IsToolsPanelVisible()
{
	return (ui->tab->currentIndex() == Ui::CBuildPanel::TOOLS_PANEL);
}

bool CBuildPanel::OnEscapeEvent()
{
	CWindowPanel* p = ui->currentPanel();
	if (p) return p->OnEscapeEvent();
	else return false;
}

bool CBuildPanel::OnDeleteEvent()
{
	CWindowPanel* p = ui->currentPanel();
	if (p) return p->OnDeleteEvent();
	else return false;
}

void CBuildPanel::Apply()
{
	CWindowPanel* p = ui->currentPanel();
	if (p) p->Apply();
}
