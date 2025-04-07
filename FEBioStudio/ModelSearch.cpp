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
#include "ModelSearch.h"
#include <QLineEdit>
#include <QTableWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QContextMenuEvent>
#include <QHeaderView>
#include "GLDocument.h"
#include <FEMLib/FSModel.h>
#include <FEMLib/FELoad.h>
#include "ModelViewer.h"
#include "ModelTree.h"
#include "IconProvider.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/FSGroup.h>
#include "FEBioJob.h"
#include <ImageLib/ImageModel.h>

CModelSearch::CModelSearch(CModelViewer* view, CModelTree* tree, QWidget* parent) : QWidget(parent), m_view(view), m_tree(tree)
{
	QLabel* label = new QLabel("Filter:");
	m_flt = new QLineEdit; label->setBuddy(m_flt);

	QHBoxLayout* h = new QHBoxLayout;
	h->addWidget(label);
	h->addWidget(m_flt);
	h->setContentsMargins(0,0,0,0);

	m_list = new QTableWidget;
	m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_list->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_list->horizontalHeader()->setStretchLastSection(true);

	QVBoxLayout* l = new QVBoxLayout;
	l->addLayout(h);
	l->addWidget(m_list);
	l->setContentsMargins(0,0,0,0);

	setLayout(l);

	QObject::connect(m_flt, SIGNAL(textEdited(const QString&)), this, SLOT(onFilterChanged(const QString&)));
	QObject::connect(m_list, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(onItemDoubleClicked(QTableWidgetItem*)));
	QObject::connect(m_list, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(onItemClicked(QTableWidgetItem*)));
}

void CModelSearch::Build(CDocument* doc)
{
	m_list->clear();
	UpdateList();
}

void CModelSearch::onFilterChanged(const QString& t)
{
	UpdateList();
}

void CModelSearch::onItemClicked(QTableWidgetItem* item)
{
	if (item)
	{
		int n = item->data(Qt::UserRole).toInt();
		m_view->SetCurrentItem(n);
	}
	else
		m_view->SetCurrentItem(-1);

	emit itemChanged();
}

void CModelSearch::onItemDoubleClicked(QTableWidgetItem* item)
{
	int n = item->data(Qt::UserRole).toInt();
	CModelTreeItem& treeItem = m_tree->GetItem(n);
	m_view->Select(treeItem.obj);
}

void CModelSearch::contextMenuEvent(QContextMenuEvent* ev)
{
	QPoint pt = ev->globalPos();

	// clear the selection
	m_view->ClearSelection();

	QTableWidgetItem* item = m_list->currentItem();
	if (item == 0) return;

	// get the selection
	QList<QTableWidgetItem*> sel = m_list->selectedItems();
	if (sel.isEmpty()) return;

	if (sel.size() == 1)
	{
		int index = item->data(Qt::UserRole).toInt();
		CModelTreeItem& data = m_tree->GetItem(index);

		m_view->SetSelection(data.obj);
		m_view->ShowContextMenu(&data, pt);
	}
	else
	{
		int index = item->data(Qt::UserRole).toInt();
		CModelTreeItem& data = m_tree->GetItem(index);

		int ntype = data.type;
		if (ntype == 0) return;

		// only show the context menu if all objects are the same
		std::vector<FSObject*> objList;
		QList<QTableWidgetItem*>::iterator it = sel.begin();
		while (it != sel.end())
		{
			index = (*it)->data(Qt::UserRole).toInt();
			CModelTreeItem& di = m_tree->GetItem(index);

			if (di.type != ntype) return;

			objList.push_back(di.obj);

			++it;
		}

		// okay, we should only get here if the type is the same for all types
		m_view->SetSelection(objList);
		m_view->ShowContextMenu(&data, pt);
	}
}

void CModelSearch::GetSelection(std::vector<FSObject*>& objList)
{
	// get the selection
	QList<QTableWidgetItem*> sel = m_list->selectedItems();
	if (sel.isEmpty()) return;

	QList<QTableWidgetItem*>::iterator it = sel.begin();
	while (it != sel.end())
	{
		int index = (*it)->data(Qt::UserRole).toInt();
		CModelTreeItem& di = m_tree->GetItem(index);

		if (di.obj && 
			((di.flag & CModelTree::OBJECT_NOT_EDITABLE) == 0) &&
			((di.flag & CModelTree::DUPLICATE_ITEM)== 0))
				objList.push_back(di.obj);

		++it;
	}
}

void CModelSearch::UpdateObject(FSObject* po)
{
	for (int i = 0; i < m_list->rowCount(); ++i)
	{
		QTableWidgetItem* item = m_list->item(i, 0);
		int n = item->data(Qt::UserRole).toInt();

		if ((n >= 0) && (n < m_tree->Items()))
		{
			CModelTreeItem& treeItem = m_tree->GetItem(n);
			FSObject* o = treeItem.obj;
			if (o == po)
			{
				FSStepComponent* pc = dynamic_cast<FSStepComponent*>(o);
				if (pc)
				{
					QFont font = item->font();
					font.setItalic(pc->IsActive()==false);
					item->setFont(font);
				}
			}
		}
	}
}

QString GetIconName(FSObject* po)
{
	if (dynamic_cast<GObject*  >(po)) return "selectObject";
	if (dynamic_cast<GPart*    >(po)) return "selectPart";
	if (dynamic_cast<GFace*    >(po)) return "selectSurface";
	if (dynamic_cast<GEdge*    >(po)) return "selectCurves";
	if (dynamic_cast<GNode*    >(po)) return "selectNodes";
	if (dynamic_cast<FSElemSet*>(po)) return "selElem";
	if (dynamic_cast<FSPartSet*>(po)) return "selElem";
	if (dynamic_cast<FSSurface*>(po)) return "selFace";
	if (dynamic_cast<FSEdgeSet*>(po)) return "selEdge";
	if (dynamic_cast<FSNodeSet*>(po)) return "selNode";
	if (dynamic_cast<GNodeList*>(po)) return "selectNodes";
	if (dynamic_cast<GFaceList*>(po)) return "selectSurface";
	if (dynamic_cast<GEdgeList*>(po)) return "selectCurves";
	if (dynamic_cast<GPartList*>(po)) return "selectPart";
	if (dynamic_cast<GMaterial*>(po)) return "material";
	if (dynamic_cast<FSLoadController*>(po)) return "curve";
	if (dynamic_cast<GDiscreteObject* >(po)) return "spring";
	if (dynamic_cast<CImageModel*>(po)) return "image";
	if (dynamic_cast<FSBoundaryCondition*>(po)) return "bc";
	if (dynamic_cast<FSInitialCondition*>(po)) return "ic";
	if (dynamic_cast<FSLoad*>(po)) return "load";
	if (dynamic_cast<FSStep*>(po)) return "step";
	if (dynamic_cast<FSPairedInterface*>(po)) return "contact";
	if (dynamic_cast<CFEBioJob*>(po)) return "febiorun";

	return QString("component");
}

void CModelSearch::UpdateList()
{
	m_list->clear();

	QString flt = m_flt->text();

	bool nofilter = flt.isEmpty();

	// objects that match the filter
	QVector<int> objList;

	int N = m_tree->Items();
	for (int i = 0; i < N; ++i)
	{
		CModelTreeItem& item = m_tree->GetItem(i);
		FSObject* o = item.obj;
		if (item.obj &&
			((item.flag & CModelTree::OBJECT_NOT_EDITABLE) == 0) &&
			((item.flag & CModelTree::DUPLICATE_ITEM) == 0))
		{
			QString s = QString::fromStdString(o->GetName());
			if (s.isEmpty() == false)
			{
				QString typeStr = QString::fromStdString(CGLDocument::GetTypeString(o));
				if (nofilter || s.contains(flt, Qt::CaseInsensitive) || typeStr.contains(flt, Qt::CaseInsensitive))
				{
					objList.push_back(i);
				}
			}
		}
	}
	if (objList.isEmpty()) return;

	m_list->setRowCount(objList.size());
	m_list->setColumnCount(2);
	int W = m_list->rect().width();
	m_list->setColumnWidth(0, W/2);
	m_list->setHorizontalHeaderLabels(QStringList() << "Name" << "Type");
	m_list->verticalHeader()->hide();

	for (int n = 0; n < objList.size(); ++n)
	{
		int i = objList[n];
		CModelTreeItem& item = m_tree->GetItem(i);
		FSObject* o = item.obj;

		QString s = QString::fromStdString(o->GetName());

		// column 0
		QTableWidgetItem* it = new QTableWidgetItem;
		it->setText(s);
		it->setData(Qt::UserRole, i);
		it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);

		QString iconName = GetIconName(o);
		if (!iconName.isEmpty()) it->setIcon(CIconProvider::GetIcon(iconName));

		FSStepComponent* pc = dynamic_cast<FSStepComponent*>(o);
		if (pc)
		{
			QFont font = it->font();
			font.setItalic(pc->IsActive()==false);
			it->setFont(font);
		}
		m_list->setItem(n, 0, it);

		// column 1
		it = new QTableWidgetItem;
		std::string typeStr = CGLDocument::GetTypeString(o);
		s = "";
		if (!typeStr.empty())
		{
			s = QString::fromStdString(typeStr);
		}
		it->setText(s);
		it->setData(Qt::UserRole, i);
		it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
		m_list->setItem(n, 1, it);
	}

	m_list->sortItems(0);

	m_view->SetCurrentItem(-1);
}
