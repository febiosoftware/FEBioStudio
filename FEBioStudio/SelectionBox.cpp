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
#include "SelectionBox.h"
#include "Document.h"	// for CActiveSelection
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolButton>
#include <QGridLayout>
#include <QMessageBox>
#include <MeshLib/FEItemListBuilder.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include "MainWindow.h"
#include "ModelDocument.h"
#include "DlgPickNamedSelection.h"

class Ui::CSelectionBox
{
public:
	QLineEdit*		name;
	QLabel*			type;
	QListWidget*	list;
	QWidget*		nameType;
	QToolButton*	pb1;
	QToolButton*	pb2;
	QToolButton*	pb3;
	QToolButton*	pb4;

	QToolButton*	clr;
	QToolButton*	collapse;
	QToolButton*	pick;

	bool	m_collapsed;

public:
	void setupUi(QWidget* parent)
	{
		nameType = new QWidget;

		m_collapsed = false;

		QHBoxLayout* hb = new QHBoxLayout;
		hb->setContentsMargins(0, 0, 0, 0);
		hb->addWidget(name = new QLineEdit);
		hb->addWidget(pick = new QToolButton); 
		pick->setObjectName("pick");
		pick->setText("...");

		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", hb); name->setObjectName("name");

		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0,0,0,0);
		h->addWidget(type = new QLabel(""));
		h->addWidget(clr = new QToolButton); clr->setIcon(QIcon(":/icons/delete.png"));
		clr->setToolTip("Clear the selection.");
		clr->setObjectName("clearSelection");
		h->addWidget(collapse = new QToolButton); collapse->setIcon(QIcon(":/icons/list.png"));
		collapse->setToolTip("Toggle collapsed/expanded view");
		collapse->setObjectName("toggleCollapse");
		collapse->setCheckable(true);
		collapse->setChecked(!m_collapsed);
		form->addRow("Type:", h);
		
		nameType->setLayout(form);

		list = new QListWidget;
		list->setObjectName("list");
		list->setSelectionMode(QAbstractItemView::ExtendedSelection);
		//	list->setFixedHeight(200);

		list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

		const int W = 24;
		pb1 = new QToolButton; pb1->setObjectName("addButton"); pb1->setIcon(QIcon(":/icons/selectAdd.png")); 
		pb1->setAutoRaise(true); pb1->setFixedSize(W, W); pb1->setIconSize(pb1->size());
		pb2 = new QToolButton; pb2->setObjectName("subButton"); pb2->setIcon(QIcon(":/icons/selectSub.png"));
		pb2->setAutoRaise(true); pb2->setFixedSize(W, W); pb2->setIconSize(pb2->size());
		pb3 = new QToolButton; pb3->setObjectName("delButton"); pb3->setIcon(QIcon(":/icons/selectDel.png"));
		pb3->setAutoRaise(true); pb3->setFixedSize(W, W); pb3->setIconSize(pb3->size());
		pb4 = new QToolButton; pb4->setObjectName("selButton"); pb4->setIcon(QIcon(":/icons/select.png"   ));
		pb4->setAutoRaise(true); pb4->setFixedSize(W, W); pb4->setIconSize(pb4->size());

		QVBoxLayout* buttonLayout = new QVBoxLayout;
		buttonLayout->addWidget(pb1);
		buttonLayout->addWidget(pb2);
		buttonLayout->addWidget(pb3);
		buttonLayout->addWidget(pb4);
		buttonLayout->addStretch();
		buttonLayout->setContentsMargins(0,0,0,0);

		QGridLayout* mainLayout = new QGridLayout;
		mainLayout->addWidget(nameType, 0, 0);
		mainLayout->addWidget(list, 1, 0);
		mainLayout->addLayout(buttonLayout, 1, 1);
		mainLayout->setContentsMargins(0,0,0,0);

		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}
};

CSelectionBox::CSelectionBox(QWidget* parent) : QWidget(parent), ui(new Ui::CSelectionBox)
{
	ui->setupUi(this);
}

void CSelectionBox::showNameType(bool b)
{
	ui->nameType->setVisible(b);
}

void CSelectionBox::setName(const QString& name)
{
	ui->name->setText(name);
}

void CSelectionBox::setType(const QString& type)
{
	ui->type->setText(type);
}

void CSelectionBox::enableAddButton(bool b)
{
	ui->pb1->setEnabled(b);
}

void CSelectionBox::enableRemoveButton(bool b)
{
	ui->pb2->setEnabled(b);
}

void CSelectionBox::enableDeleteButton(bool b)
{
	ui->pb3->setEnabled(b);
}

void CSelectionBox::enableSelectButton(bool b)
{
	ui->pb4->setEnabled(b);
}

void CSelectionBox::enableAllButtons(bool b)
{
	enableAddButton(b);
	enableRemoveButton(b);
	enableDeleteButton(b);
	enableSelectButton(b);
}

void CSelectionBox::setCollapsed(bool b)
{
	ui->collapse->setChecked(!b);
}

void CSelectionBox::on_addButton_clicked()
{
	emit addButtonClicked();
}

void CSelectionBox::on_subButton_clicked()
{
	emit subButtonClicked();
}

void CSelectionBox::on_delButton_clicked()
{
	emit delButtonClicked();
}

void CSelectionBox::on_selButton_clicked()
{
	emit selButtonClicked();
}

void CSelectionBox::on_clearSelection_clicked()
{
	emit clearButtonClicked();
}

void CSelectionBox::on_toggleCollapse_toggled(bool b)
{
	ui->m_collapsed = !b;
	ui->list->clear();
	if (ui->m_collapsed)
	{
		QListWidgetItem* it = new QListWidgetItem(ui->list);
		it->setText(QString("[items: %1]").arg(m_items.size()));
	}
	else
	{
		for (int i = 0; i < m_items.size(); ++i)
		{
			QListWidgetItem* it = new QListWidgetItem(ui->list);
			it->setText(m_items[i].m_label);
		}
	}
}

void CSelectionBox::on_pick_clicked(bool b)
{
	emit pickClicked();
}

void CSelectionBox::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

void CSelectionBox::on_list_itemDoubleClicked(QListWidgetItem *item)
{
	if (ui->m_collapsed) ui->collapse->setChecked(true);
	else
	{
		emit selButtonClicked();
	}
}

void CSelectionBox::clearData()
{
	ui->list->clear();
	m_items.clear();
}

void CSelectionBox::addData(const QString& item, int data, int fmt, bool checkForDuplicates)
{
	// make sure the data does not exist yet
	if (checkForDuplicates)
	{
		for (int i = 0; i < m_items.size(); ++i)
			if (m_items[i].m_data == data)
			{
				return;
			}
	}

	m_items.push_back(Item(item, data));

	if (ui->m_collapsed == false)
	{
		ui->list->addItem(item);

		if (fmt != 0)
		{
			QListWidgetItem* w = ui->list->item(ui->list->count() - 1);
			w->setForeground(Qt::red);
		}
	}
	else
	{
		if (ui->list->count() == 0) ui->list->addItem("");
		QListWidgetItem* w = ui->list->item(0); assert(w);
		w->setText(QString("[items: %1]").arg(m_items.size()));
	}
}

void CSelectionBox::addData(const vector<int>& data)
{
	for (int i=0; i<data.size(); ++i)
	{
		QString t = QString::number(data[i]);
		addData(t, data[i]);
	}
}

void CSelectionBox::removeData(int ndata)
{
	for (int j = 0; j < m_items.size(); ++j)
	{
		if (m_items[j].m_data == ndata)
		{
			m_items.erase(m_items.begin() + j);

			if (ui->m_collapsed)
			{
				QListWidgetItem* w = ui->list->item(0);
				w->setText(QString("[items: %1]").arg(m_items.size()));
			}
			else
			{
				delete ui->list->takeItem(j);
			}
			break;
		}
	}
}

void CSelectionBox::removeData(const vector<int>& data)
{
	for (int i = 0; i < data.size(); ++i)
	{
		int ndata = data[i];

		for (int j = 0; j < m_items.size(); ++j)
		{
			if (m_items[j].m_data == ndata)
			{
				m_items.erase(m_items.begin() + j);
				if (ui->m_collapsed == false) delete ui->list->takeItem(j);
				break;
			}
		}
	}

	if (ui->m_collapsed)
	{
		QListWidgetItem* w = ui->list->item(0);
		w->setText(QString("[items: %1]").arg(m_items.size()));
	}
}

void CSelectionBox::getSelectedItems(vector<int>& sel)
{
	if (ui->m_collapsed)
	{
		QListWidgetItem* w = ui->list->item(0);
		if (w && w->isSelected()) getAllItems(sel);
	}
	else
	{
		QModelIndexList selItems = ui->list->selectionModel()->selectedRows();
		if (selItems.empty() == false)
		{
			for (QModelIndexList::iterator it = selItems.begin(); it != selItems.end(); ++it)
			{
				int row = it->row();
				sel.push_back(m_items[row].m_data);
			}
		}
	}
}

void CSelectionBox::getAllItems(vector<int>& data)
{
	data.resize(m_items.size());
	for (int i=0; i< m_items.size(); ++i) data[i] = m_items[i].m_data;
}

void CSelectionBox::getAllNames(QStringList& names)
{
	names.clear();

	int N = ui->list->count();
	for (int i = 0; i<N; ++i)
	{
		QListWidgetItem* item = ui->list->item(i);
		QString t = item->text();
		names.push_back(t);
	}
}

void CSelectionBox::getSelectedItems(list<int>& sel)
{
	if (ui->m_collapsed)
	{
		QListWidgetItem* w = ui->list->item(0);
		if (w && w->isSelected())
		{
			for (int i = 0; i < m_items.size(); ++i) sel.push_back(m_items[i].m_data);
		}
	}
	else
	{
		QModelIndexList selItems = ui->list->selectionModel()->selectedRows();
		if (selItems.empty() == false)
		{
			for (QModelIndexList::iterator it = selItems.begin(); it != selItems.end(); ++it)
			{
				int row = it->row();
				sel.push_back(m_items[row].m_data);
			}
		}
	}
}

void CSelectionBox::removeSelectedItems()
{
	if (ui->m_collapsed == false)
	{
		int N = ui->list->count();
		for (int i = 0; i < N; ++i)
		{
			QListWidgetItem* item = ui->list->item(i);
			if (item->isSelected())
			{
				item = ui->list->takeItem(i);
				delete item;

				m_items.erase(m_items.begin() + i);
				N--;
				i--;
			}
		}
	}
}

//-----------------------------------------------------------------------------
CItemListSelectionBox::CItemListSelectionBox(QWidget* parent) : CSelectionBox(parent)
{

}

void CItemListSelectionBox::SetItemList(FEItemListBuilder* item)
{
	// make sure we have an item list
	if (item == 0)
	{
		setName("");
		setType("");
		clearData();
		return;
	}

	// set the name
	QString name = QString::fromStdString(item->GetName());
	setName(name);
	enableAllButtons(true);
	clearData();
	setCollapsed(false);

	// set the type
	QString type("(unknown)");
	switch (item->Type())
	{
	case GO_PART:
	{
		setType("Domains");
		GPartList& g = dynamic_cast<GPartList&>(*item);
		vector<GPart*> parts = g.GetPartList();
		FEItemListBuilder::Iterator it = item->begin();
		for (int i = 0; i < parts.size(); ++i, ++it)
		{
			GPart* pg = parts[i];
			if (pg) addData(QString::fromStdString(pg->GetName()), pg->GetID());
			else addData(QString("[invalid reference]"), *it, 1);
		}
	}
	break;
	case GO_FACE:
	{
		setType("Surfaces");
		GFaceList& g = dynamic_cast<GFaceList&>(*item);
		vector<GFace*> surfs = g.GetFaceList();
		FEItemListBuilder::Iterator it = item->begin();
		for (int i = 0; i < surfs.size(); ++i, ++it)
		{
			GFace* pg = surfs[i];
			if (pg) addData(QString::fromStdString(pg->GetName()), pg->GetID());
			else addData(QString("[invalid reference]"), *it, 1);
		}
	}
	break;
	case GO_EDGE:
	{
		setType("Curves");
		GEdgeList& g = dynamic_cast<GEdgeList&>(*item);
		vector<GEdge*> edges = g.GetEdgeList();
		FEItemListBuilder::Iterator it = item->begin();
		for (int i = 0; i < edges.size(); ++i, ++it)
		{
			GEdge* pg = edges[i];
			if (pg) addData(QString::fromStdString(pg->GetName()), pg->GetID());
			else addData(QString("[invalid reference]"), *it, 1);
		}
	}
	break;
	case GO_NODE:
	{
		setType("Nodes");
		GNodeList& g = dynamic_cast<GNodeList&>(*item);
		vector<GNode*> nodes = g.GetNodeList();
		FEItemListBuilder::Iterator it = item->begin();
		for (int i = 0; i < nodes.size(); ++i, ++it)
		{
			GNode* pg = nodes[i];
			if (pg) addData(QString::fromStdString(pg->GetName()), pg->GetID());
			else addData(QString("[invalid reference]"), *it, 1);
		}
	}
	break;
	default:
		switch (item->Type())
		{
		case FE_PARTSET: type = "Parts"; break;
		case FE_ELEMSET: type = "Elements"; break;
		case FE_SURFACE: type = "Facets"; break;
		case FE_EDGESET: type = "Edges"; break;
		case FE_NODESET: type = "Nodes"; break;
		default:
			assert(false);
		}

		FSGroup* pg = dynamic_cast<FSGroup*>(item);
		if (pg)
		{
			FSMesh* mesh = pg->GetMesh();
			if (mesh)
			{
				GObject* po = mesh->GetGObject();
				if (po)
				{
					type += QString(" [%1]").arg(QString::fromStdString(po->GetName()));
				}
			}
		}
		setType(type);

		// set the data
		if (item->Type() == FE_PARTSET)
		{
			setCollapsed(false);
			FSPartSet* pg = dynamic_cast<FSPartSet*>(item);
			for (int i = 0; i < pg->size(); ++i)
			{
				GPart* part = pg->GetPart(i); assert(part);
				if (part)
				{
					addData(QString::fromStdString(part->GetName()), part->GetLocalID(), 0, false);
				}
			}
		}
		else
		{
			setCollapsed(true);
			vector<int> items;
			items.insert(items.end(), item->begin(), item->end());
			for (int i = 0; i < (int)items.size(); ++i) addData(QString::number(items[i]), items[i], 0, false);
		}
	}
}

CMeshSelectionBox::CMeshSelectionBox(CMainWindow* wnd, QWidget* parent) : CItemListSelectionBox(parent)
{
	m_wnd = wnd;
	m_pms = nullptr;

	QObject::connect(this, SIGNAL(addButtonClicked()), this, SLOT(onAddButtonClicked()));
	QObject::connect(this, SIGNAL(subButtonClicked()), this, SLOT(onSubButtonClicked()));
	QObject::connect(this, SIGNAL(delButtonClicked()), this, SLOT(onDelButtonClicked()));
	QObject::connect(this, SIGNAL(selButtonClicked()), this, SLOT(onSelButtonClicked()));
	QObject::connect(this, SIGNAL(clearButtonClicked()), this, SLOT(onClearButtonClicked()));
	QObject::connect(this, SIGNAL(pickClicked()), this, SLOT(onPickButtonClicked()));
	QObject::connect(this, SIGNAL(nameChanged(const QString&)), this, SLOT(onNameChanged(const QString&)));
}

void CMeshSelectionBox::SetSelection(FSMeshSelection* pms)
{
	m_pms = pms;
	if (pms)
		SetItemList(pms->GetItemList());
	else
		SetItemList(nullptr);
}

void CMeshSelectionBox::onNameChanged(const QString& t)
{
	FEItemListBuilder* pi = (m_pms ? m_pms->GetItemList() : nullptr);
	if (pi) pi->SetName(t.toStdString());
}

void CMeshSelectionBox::onAddButtonClicked()
{
	if (m_pms == nullptr) return;

	FSModel* fem = m_pms->GetFSModel(); assert(fem);
	if (fem == nullptr) return;

	GModel& gm = fem->GetModel();

	// get the current selection
	FESelection* ps = CActiveSelection::GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	// create the item list from the selection
	FEItemListBuilder* pg = ps->CreateItemList();
	if (pg == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "You cannot assign an empty selection.");
		return;
	}

	// get the current item list
	FEItemListBuilder* pl = m_pms->GetItemList();

	// see whether the current list exists or not
	if (pl == nullptr)
	{
		// see if we can assign it
		int itemType = m_pms->GetMeshItemType();
		if (pg->Supports(itemType) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "You cannot apply the current selection to this model component.");
			delete pg;
			return;
		}

		m_pms->SetItemList(pg);
		SetItemList(pg);
	}
	else
	{
		// merge with the current list
		if (pg->Type() != pl->Type())
		{
			QMessageBox::critical(this, "FEBio Studio", "The selection is not of the correct type.");
		}
		else
		{
			// for groups, make sure that they are on the same mesh
			FSGroup* pg_prv = dynamic_cast<FSGroup*>(pl);
			FSGroup* pg_new = dynamic_cast<FSGroup*>(pg);
			if (pg_prv && pg_new && (pg_prv->GetMesh() != pg_new->GetMesh()))
			{
				QMessageBox::critical(this, "FEBio Studio", "You cannot assign the current selection.\nThe model component was already assigned to a different mesh.");
			}
			else
			{
				vector<int> l = pg->CopyItems();
				pl->Merge(l);
			}
			
			SetItemList(pl);
			delete pg;
		}
		emit selectionChanged();
		return;
	}
}

void CMeshSelectionBox::onSubButtonClicked()
{
	if (m_pms == nullptr) return;

	// get the current selection
	FESelection* ps = CActiveSelection::GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	// get the current item list
	FEItemListBuilder* pl = m_pms->GetItemList();
	if (pl == nullptr) return;

	// create the item list builder
	FEItemListBuilder* pg = ps->CreateItemList();

	// subtract from the current list
	if (pg->Type() == pl->Type())
	{
		vector<int> l = pg->CopyItems();
		pl->Subtract(l);
	}

	SetItemList(pl);
	delete pg;
	emit selectionChanged();
}

void CMeshSelectionBox::onDelButtonClicked()
{
	if (m_pms == nullptr) return;
	FEItemListBuilder* pl = m_pms->GetItemList();
	if (pl == nullptr) return;

	vector<int> items;
	getSelectedItems(items);

	pl->Subtract(items);

	SetItemList(pl);
	emit selectionChanged();
}

void CMeshSelectionBox::onSelButtonClicked()
{
	if (m_pms == nullptr) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

	// get the selection list
	vector<int> l;
	getSelectedItems(l);
	if (l.empty())
	{
		QMessageBox::information(this, "FEBio Studio", "Nothing to select");
		return;
	}

	pdoc->SelectItems(m_pms, l, 0);
	m_wnd->UpdateToolbar();
	m_wnd->Update();
}

void CMeshSelectionBox::onClearButtonClicked()
{
	if (m_pms == nullptr) return;

	FEItemListBuilder* pl = m_pms->GetItemList();
	if (pl)
	{
		m_pms->SetItemList(nullptr);
		delete pl;
		SetItemList(nullptr);
		emit selectionChanged();
	}
}

void CMeshSelectionBox::onPickButtonClicked()
{
	CModelDocument* pdoc = m_wnd->GetModelDocument();
	if (pdoc == nullptr) return;
	if (m_pms == nullptr) return;

	// find the required mesh type
	int meshType = m_pms->GetMeshItemType();

	GModel& gm = *pdoc->GetGModel();

	// build the candidate list
	QStringList names;
	if (meshType & FE_NODE_FLAG)
	{
		auto l = gm.AllNamedSelections(GO_NODE);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));

		l = gm.AllNamedSelections(FE_NODESET);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));
	}
	if ((meshType & FE_FACE_FLAG) || (meshType & FE_NODE_FLAG))
	{
		auto l = gm.AllNamedSelections(GO_FACE);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));

		l = gm.AllNamedSelections(FE_SURFACE);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));
	}

	// get the current selection
	FEItemListBuilder* pl = m_pms->GetItemList(0);

	CDlgPickNamedSelection dlg(this);
	dlg.setNameList(names);
	if (pl) dlg.setSelection(QString::fromStdString(pl->GetName()));
	if (dlg.exec())
	{
		QString qs = dlg.getSelection();
		if (qs.isEmpty() == false)
		{
			std::string s = qs.toStdString();
			if ((pl == nullptr) || (s != pl->GetName()))
			{
				pl = gm.FindNamedSelection(s);
				m_pms->SetItemList(pl);
				SetSelection(m_pms);
			}
		}
	}
}
