/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "DlgPartSelector.h"
#include "MainWindow.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "ModelDocument.h"

class CDlgPartSelector::UI
{
public:
	CModelDocument* doc = nullptr;

private:
	CMainWindow* wnd = nullptr;
	QListWidget* list = nullptr;
	QLineEdit* flt = nullptr;
	GModel* gm = nullptr;
	std::vector<GPart*> partList;

public:
	void setup(CMainWindow* mainWnd, CDlgPartSelector* dlg)
	{
		wnd = mainWnd;
		list = new QListWidget;

		flt = new QLineEdit;
		flt->setPlaceholderText("(filter parts)");

		QHBoxLayout* hflt = new QHBoxLayout;
		hflt->addWidget(new QLabel("Filter:"));
		hflt->addWidget(flt);

		QPushButton* showAll = new QPushButton("Show All");
		QPushButton* hideAll = new QPushButton("Hide All");
		QHBoxLayout* hbtn = new QHBoxLayout;
		hbtn->addWidget(showAll);
		hbtn->addWidget(hideAll);
		hbtn->addStretch();

		QVBoxLayout* l = new QVBoxLayout;
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);

		l->addLayout(hflt);
		l->addLayout(hbtn);
		l->addWidget(list);
		l->addWidget(bb);
		dlg->setLayout(l);

		connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
		connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		connect(list, &QListWidget::itemClicked, dlg, &CDlgPartSelector::onItemClicked);
		connect(list, &QListWidget::itemSelectionChanged, dlg, &CDlgPartSelector::onSelectionChanged);
		connect(flt, &QLineEdit::textChanged, dlg, &CDlgPartSelector::onFilterChanged);
		connect(showAll, &QPushButton::clicked, dlg, &CDlgPartSelector::onShowAll);
		connect(hideAll, &QPushButton::clicked, dlg, &CDlgPartSelector::onHideAll);
	}

	void addItem(QString name, int partIndex, bool isVisible)
	{
		QListWidgetItem* wi = new QListWidgetItem(name);
		wi->setData(Qt::UserRole, partIndex);
		wi->setCheckState(isVisible ? Qt::Checked : Qt::Unchecked);
		list->addItem(wi);
	}

	void buildList(GModel* geom)
	{
		gm = geom;
		updateList();
	}

	void updateList()
	{
		list->clear();
		if (gm == nullptr) return;
		partList.clear();

		QString filter = flt->text();
		for (GPartIterator it(*gm); it.isValid(); ++it)
		{
			GPart* pg = it;
			QString name = QString::fromStdString(pg->GetName());
			if (filter.isEmpty() || name.contains(filter, Qt::CaseInsensitive))
			{
				addItem(name, partList.size(), pg->IsVisible());
				partList.push_back(pg);
			}
		}
	}

	GPart* getPart(QListWidgetItem* it)
	{
		if (it == nullptr) return nullptr;
		int partIndex = it->data(Qt::UserRole).toInt();
		if ((partIndex >= 0) && (partIndex < partList.size())) return partList[partIndex];
		return nullptr;
	}

	void updatePart(GPart* pg, bool isChecked)
	{
		if (pg->IsVisible() && !isChecked)
		{
			gm->ShowPart(pg, false);
		}
		else if (!pg->IsVisible() && isChecked)
		{
			gm->ShowPart(pg, true);
		}
		else return;

		wnd->RedrawGL();
	}

	void showAllParts()
	{
		gm->ShowParts(partList, true);
		updateList();
		wnd->RedrawGL();
	}

	void hideAllParts()
	{
		gm->ShowParts(partList, false);
		updateList();
		wnd->RedrawGL();
	}

	void updateSelection()
	{
		QList<QListWidgetItem*> selectedItems = list->selectedItems();
		for (GPartIterator it(*gm); it.isValid(); ++it) it->UnSelect();

		for (auto it : selectedItems)
		{
			GPart* pg = getPart(it);
			if (pg) pg->Select();
		}

		doc->UpdateSelection(false);
		wnd->RedrawGL();
	}
};

CDlgPartSelector::CDlgPartSelector(CModelDocument* doc, CMainWindow* wnd) : QDialog(wnd), ui(new CDlgPartSelector::UI)
{
	setWindowTitle("Part Selector");
	ui->setup(wnd, this);
	ui->doc = doc;

	if (doc->GetSelectionMode() != SELECT_PART)
		wnd->on_actionSelectParts_toggled(true);

	if (doc)
	{
		GModel* gm = doc->GetGModel();
		ui->buildList(gm);
	}
}

void CDlgPartSelector::onItemClicked(QListWidgetItem* it)
{
	if (it == nullptr) return;
	bool isChecked = (it->checkState() == Qt::Checked);
	GPart* pg = ui->getPart(it);
	if (pg) ui->updatePart(pg, isChecked);
}

void CDlgPartSelector::onFilterChanged()
{
	ui->updateList();
}

void CDlgPartSelector::onShowAll()
{
	ui->showAllParts();
}

void CDlgPartSelector::onHideAll()
{
	ui->hideAllParts();
}

void CDlgPartSelector::onSelectionChanged()
{
	ui->updateSelection();
}
