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
		l->setMargin(0);
		l->addWidget(tab);

		parent->setLayout(l);

		QMetaObject::connectSlotsByName(parent);
	}

	CCommandPanel* currentPanel()
	{
		return dynamic_cast<CCommandPanel*>(tab->currentWidget());
	}
};

CPostPanel::CPostPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CPostPanel)
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
	CCommandPanel* p = ui->currentPanel();
	if (p) p->Update(breset);
}

void CPostPanel::on_postTab_currentChanged(int index)
{
	Update();
}

void CPostPanel::Apply()
{
	CCommandPanel* p = ui->currentPanel();
	if (p) p->Apply();
}

void CPostPanel::SelectObject(FSObject* po)
{
	ui->mdl->selectObject(po);
}

void CPostPanel::OnViewChanged()
{
	CPostModelPanel* mdl = ui->mdl;
	if (mdl && mdl->isVisible()) mdl->UpdateView();
}