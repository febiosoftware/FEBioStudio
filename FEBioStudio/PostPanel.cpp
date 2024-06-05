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

#pragma once
#include "PostPanel.h"
#include "PostModelPanel.h"
#include "PostDataPanel.h"
#include "MaterialPanel.h"
#include "StatePanel.h"
#include <QTabWidget>
#include <QBoxLayout>

class Ui::CPostPanel
{
public:
	::CPostModelPanel*	mdl;
	::CPostDataPanel*	data;
	::CMaterialPanel*	mat;
	::CStatePanel*		state;

	QTabWidget*	tab;

public:
	void setup(QWidget* parent, CMainWindow* wnd)
	{
		tab = new QTabWidget; tab->setObjectName("postTab");

		tab->addTab(mdl = new ::CPostModelPanel(wnd, parent), "View");
		tab->addTab(mat = new ::CMaterialPanel(wnd, parent), "Material");
		tab->addTab(data = new ::CPostDataPanel(wnd, parent), "Data");
		tab->addTab(state = new ::CStatePanel(wnd, parent), "State");

		QHBoxLayout* l = new QHBoxLayout;
		l->setContentsMargins(0,0,0,0);
		l->addWidget(tab);

		parent->setLayout(l);

		QMetaObject::connectSlotsByName(parent);
	}

	CWindowPanel* currentPanel()
	{
		return dynamic_cast<CWindowPanel*>(tab->currentWidget());
	}
};

CPostPanel::CPostPanel(CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CPostPanel)
{
	ui->setup(this, wnd);
}

void CPostPanel::Reset()
{
	ui->mdl->Update(true);
	Update();
}

void CPostPanel::Update(bool breset)
{
	CWindowPanel* p = ui->currentPanel();
	if (p) p->Update(breset);
}

void CPostPanel::on_postTab_currentChanged(int index)
{
	Update();
}

void CPostPanel::Apply()
{
	CWindowPanel* p = ui->currentPanel();
	if (p) p->Apply();
}

void CPostPanel::SelectObject(FSObject* po)
{
	ui->mdl->selectObject(po);
}

FSObject* CPostPanel::GetSelectedObject()
{
	if (ui->mdl->isVisible())
	{
		return ui->mdl->selectedObject();
	}
	else return nullptr;
}

void CPostPanel::OnViewChanged()
{
	CPostModelPanel* mdl = ui->mdl;
	if (mdl && mdl->isVisible()) mdl->UpdateView();
}
