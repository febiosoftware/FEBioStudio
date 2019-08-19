#pragma once
#include "PostPanel.h"
#include "PostDataPanel.h"
#include "MaterialPanel.h"
#include <QTabWidget>
#include <QBoxLayout>

class Ui::CPostPanel
{
public:
	::CPostDataPanel*	data;
	::CMaterialPanel*	mat;

	QTabWidget*	tab;

public:
	void setup(QWidget* parent, CMainWindow* wnd)
	{
		tab = new QTabWidget; tab->setObjectName("postTab");

		tab->addTab(mat = new ::CMaterialPanel(wnd, parent), "Material");
		tab->addTab(data = new ::CPostDataPanel(wnd, parent), "Data");

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

void CPostPanel::Update()
{
	CCommandPanel* p = ui->currentPanel();
	if (p) p->Update();
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
