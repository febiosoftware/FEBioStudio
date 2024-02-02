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
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include "ModelDocument.h"

class CDlgPartSelector::UI
{
public:
	CModelDocument* doc = nullptr;

	class Item
	{
	public:
		GPart* pg;
		QString name;
		QString type;
		QString matType;
	};

private:
	CMainWindow* wnd = nullptr;
	QTableWidget* list = nullptr;
	QLineEdit* flt = nullptr;
	GModel* gm = nullptr;
	std::vector<Item> partList;

public:
	void setup(CMainWindow* mainWnd, CDlgPartSelector* dlg)
	{
		wnd = mainWnd;
		list = new QTableWidget;
		list->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

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
		connect(list, &QTableWidget::itemClicked, dlg, &CDlgPartSelector::onItemClicked);
		connect(list, &QTableWidget::itemSelectionChanged, dlg, &CDlgPartSelector::onSelectionChanged);
		connect(flt, &QLineEdit::textChanged, dlg, &CDlgPartSelector::onFilterChanged);
		connect(showAll, &QPushButton::clicked, dlg, &CDlgPartSelector::onShowAll);
		connect(hideAll, &QPushButton::clicked, dlg, &CDlgPartSelector::onHideAll);
	}

	void addItem(Item& it, int index)
	{
		bool isVisible = it.pg->IsVisible();
		QTableWidgetItem* wi = new QTableWidgetItem(it.name);
		wi->setData(Qt::UserRole, index);
		wi->setCheckState(isVisible ? Qt::Checked : Qt::Unchecked);
		list->setItem(index, 0, wi);
		list->setItem(index, 1, new QTableWidgetItem(it.type));
		list->setItem(index, 2, new QTableWidgetItem(it.matType));
	}

	void buildList(GModel* geom)
	{
		gm = geom;
		updateList();
	}

	void updateList()
	{
		list->clear();
		list->setColumnCount(3);
		list->setHorizontalHeaderLabels(QStringList() << "name" << "type" << "material");
		list->horizontalHeader()->setStretchLastSection(true);
		int W = list->rect().width();
		list->setColumnWidth(0, W / 2);
		list->setColumnWidth(1, W / 4);
		partList.clear();

		if (gm == nullptr) return;

		FSModel& fem = *doc->GetFSModel();

		QString filter = flt->text();
		for (GPartIterator it(*gm); it.isValid(); ++it)
		{
			GPart* pg = it;
			QString name = QString::fromStdString(pg->GetName());

			QString type;
			if      (pg->IsSolid()) type = "solid";
			else if (pg->IsShell()) type = "shell";
			else if (pg->IsBeam ()) type = "beam";

			int matID = pg->GetMaterialID();
			GMaterial* mat = fem.GetMaterialFromID(matID);
			QString matType;
			if (mat)
			{
				FSMaterial* pm = mat->GetMaterialProperties();
				if (pm) matType = QString(pm->GetTypeString());
			}

			bool addPart = false;
			if (filter.isEmpty()) addPart = true;
			else if (name.contains(filter, Qt::CaseInsensitive)) addPart = true;
			else if (type.contains(filter, Qt::CaseInsensitive)) addPart = true;
			else if (matType.contains(filter, Qt::CaseInsensitive)) addPart = true;

			if (addPart)
			{
				partList.push_back({ pg, name, type, matType });
			}
		}

		int N = partList.size();
		list->setRowCount(N);
		for (size_t n = 0; n<N; ++n)
		{
			addItem(partList[n], n);
		}
	}

	GPart* getPart(QTableWidgetItem* it)
	{
		if (it == nullptr) return nullptr;
		if (it->column() != 0) return nullptr;

		int partIndex = it->data(Qt::UserRole).toInt();
		if ((partIndex >= 0) && (partIndex < partList.size())) return partList[partIndex].pg;
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

	vector<GPart*> getPartList()
	{
		vector<GPart*> parts(partList.size());
		for (size_t i = 0; i < partList.size(); ++i) parts[i] = partList[i].pg;
		return parts;
	}

	void showAllParts()
	{
		vector<GPart*> parts = getPartList();
		gm->ShowParts(parts, true);
		updateList();
		wnd->RedrawGL();
	}

	void hideAllParts()
	{
		vector<GPart*> parts = getPartList();
		gm->ShowParts(parts, false);
		updateList();
		wnd->RedrawGL();
	}

	void updateSelection()
	{
		for (GPartIterator it(*gm); it.isValid(); ++it) it->UnSelect();

		QList<QTableWidgetItem*> selectedItems = list->selectedItems();
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
	setMinimumSize(800, 600);
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

void CDlgPartSelector::onItemClicked(QTableWidgetItem* it)
{
	if (it == nullptr) return;
	if (it->column() != 0) return;

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
