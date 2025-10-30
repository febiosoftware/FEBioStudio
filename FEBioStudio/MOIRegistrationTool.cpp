/*This file is part of the FEBio Studio source code and is licensed under the MIT license
 listed below.

 See Copyright-FEBio-Studio.txt for details.

 Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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
#include "MOIRegistrationTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QGroupBox>
#include <QMessageBox>
#include "ModelDocument.h"
#include "SelectionBox.h"
#include <GeomLib/GObject.h>
#include <MeshTools/MOIRegistration.h>
#include <GeomLib/GGroup.h>
#include "MainWindow.h"
#include "Commands.h"
using namespace std;

class MOIRegistrationToolUI : public QWidget
{
private:
    QCheckBox* m_area;
    CSelectionBox* m_src;
    CSelectionBox* m_trg;

private:
	FSItemListBuilder* m_srcList;
	FSItemListBuilder* m_trgList;

public:
    MOIRegistrationToolUI(CMOIRegistrationTool* w)
    {
        m_srcList = nullptr;
        m_trgList = nullptr;

        QFormLayout* f = new QFormLayout;
        f->addRow("Use area MOI:", m_area = new QCheckBox); m_area->checkState();
        QPushButton* apply = new QPushButton("Apply");

        f->setAlignment(Qt::AlignRight);

        QGroupBox* pg1 = new QGroupBox("Source");
        QVBoxLayout* l1 = new QVBoxLayout;
        m_src = new CSelectionBox;
        m_src->showNameType(false);
        l1->addWidget(m_src);
        pg1->setLayout(l1);

        QGroupBox* pg2 = new QGroupBox("Target");
        QVBoxLayout* l2 = new QVBoxLayout;
        m_trg = new CSelectionBox;
        m_trg->showNameType(false);
        l2->addWidget(m_trg);
        pg2->setLayout(l2);

        QVBoxLayout* l = new QVBoxLayout;

        l->addWidget(pg1);
        l->addWidget(pg2);
		l->addLayout(f);

        l->addWidget(apply);

        l->addStretch();
        setLayout(l);

        QObject::connect(apply, SIGNAL(clicked()), w, SLOT(OnApply()));

        QObject::connect(m_src, SIGNAL(addButtonClicked()), w, SLOT(on_src_addButtonClicked()));
        QObject::connect(m_src, SIGNAL(subButtonClicked()), w, SLOT(on_src_subButtonClicked()));
        QObject::connect(m_src, SIGNAL(delButtonClicked()), w, SLOT(on_src_delButtonClicked()));
        QObject::connect(m_src, SIGNAL(selButtonClicked()), w, SLOT(on_src_selButtonClicked()));
        QObject::connect(m_src, SIGNAL(clearButtonClicked()), w, SLOT(on_src_clearButtonClicked()));

        QObject::connect(m_trg, SIGNAL(addButtonClicked()), w, SLOT(on_trg_addButtonClicked()));
        QObject::connect(m_trg, SIGNAL(subButtonClicked()), w, SLOT(on_trg_subButtonClicked()));
        QObject::connect(m_trg, SIGNAL(delButtonClicked()), w, SLOT(on_trg_delButtonClicked()));
        QObject::connect(m_trg, SIGNAL(selButtonClicked()), w, SLOT(on_trg_selButtonClicked()));
        QObject::connect(m_trg, SIGNAL(clearButtonClicked()), w, SLOT(on_trg_clearButtonClicked()));
    }

    bool useArea() { return m_area->isChecked(); }

	bool UpdateSelectionList(FSItemListBuilder*& pl, FSItemListBuilder* items)
	{
		if (pl == nullptr) pl = items;
		else
		{
			if (pl && (items == nullptr)) {
				delete pl;
				pl = nullptr;
			}
			else
			{
				if (items->Type() != pl->Type()) return false;

				// make sure the same mesh is involved
				FSGroup* pm_prv = dynamic_cast<FSGroup*>(pl);
				FSGroup* pm_new = dynamic_cast<FSGroup*>(items);
				if (pm_prv && pm_new && (pm_prv->GetMesh() != pm_new->GetMesh())) return false;

				vector<int> itemlist = items->CopyItems();
				pl->Merge(itemlist);
				delete items;
			}
		}
		return true;
	}

	FSItemListBuilder* sourceList() { return m_srcList; }
	FSItemListBuilder* targetList() { return m_trgList; }

	bool SetSourceList(FSItemListBuilder* items)
	{
		if (UpdateSelectionList(m_srcList, items) == false) return false;
		SetSelection(m_src, m_srcList);
        return true;
    }

	bool SetTargetList(FSItemListBuilder* items)
	{
		if (UpdateSelectionList(m_trgList, items) == false) return false;
		SetSelection(m_trg, m_trgList);
		return true;
	}

	void SetSelection(CSelectionBox* sel, FSItemListBuilder* item)
	{
		if (item == 0)
		{
			sel->setName("");
			sel->setType("");
			sel->clearData();
			return;
		}

		// set the name
		QString name = QString::fromStdString(item->GetName());
//		sel->showNameType(true);
//		sel->setName(name);
		sel->enableAllButtons(true);
		sel->clearData();
		sel->setCollapsed(false);

		// set the type
		QString type("(unknown)");
		switch (item->Type())
		{
		case GO_PART:
		{
			sel->setType("Domains");
			GPartList& g = dynamic_cast<GPartList&>(*item);
			vector<GPart*> parts = g.GetPartList();
			FSItemListBuilder::Iterator it = item->begin();
			for (int i = 0; i < parts.size(); ++i, ++it)
			{
				GPart* pg = parts[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), *it, 1);
			}
		}
		break;
		case GO_FACE:
		{
			sel->setType("Surfaces");
			GFaceList& g = dynamic_cast<GFaceList&>(*item);
			vector<GFace*> surfs = g.GetFaceList();
			FSItemListBuilder::Iterator it = item->begin();
			for (int i = 0; i < surfs.size(); ++i, ++it)
			{
				GFace* pg = surfs[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), *it, 1);
			}
		}
		break;
		case GO_EDGE:
		{
			sel->setType("Curves");
			GEdgeList& g = dynamic_cast<GEdgeList&>(*item);
			vector<GEdge*> edges = g.GetEdgeList();
			FSItemListBuilder::Iterator it = item->begin();
			for (int i = 0; i < edges.size(); ++i, ++it)
			{
				GEdge* pg = edges[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), *it, 1);
			}
		}
		break;
		case GO_NODE:
		{
			sel->setType("Nodes");
			GNodeList& g = dynamic_cast<GNodeList&>(*item);
			vector<GNode*> nodes = g.GetNodeList();
			FSItemListBuilder::Iterator it = item->begin();
			for (int i = 0; i < nodes.size(); ++i, ++it)
			{
				GNode* pg = nodes[i];
				if (pg) sel->addData(QString::fromStdString(pg->GetName()), pg->GetID());
				else sel->addData(QString("[invalid reference]"), *it, 1);
			}
		}
		break;
		default:
			switch (item->Type())
			{
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

			sel->setType(type);

			// set the data
			vector<int> items;
			items.insert(items.end(), item->begin(), item->end());

			//		sort(items.begin(), items.end());
			//		unique(items.begin(), items.end());

			sel->setCollapsed(true);
			for (int i = 0; i < (int)items.size(); ++i) sel->addData(QString::number(items[i]), items[i], 0, false);
		}
	}

	void clearSelections()
	{
		SetSelection(m_src, nullptr);
		SetSelection(m_trg, nullptr);
		delete m_srcList; m_srcList = nullptr;
		delete m_trgList; m_trgList = nullptr;
	}
};

// constructor
CMOIRegistrationTool::CMOIRegistrationTool(CMainWindow* wnd) : CAbstractTool(wnd, "MOI Registration")
{
    ui = nullptr;
}

QWidget* CMOIRegistrationTool::createUi()
{
    if (ui == nullptr) ui = new MOIRegistrationToolUI(this);
    return ui;
}

void CMOIRegistrationTool::OnApply()
{
    CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
    if ((doc == nullptr) || (!doc->IsValid())) return;

	FSItemListBuilder* src = ui->sourceList();
	FSItemListBuilder* trg = ui->targetList();

	vector<vec3d> srcNodes;
	vector<vec3d> trgNodes;
	GObject* srcObj = GetSelectionNodes(src, srcNodes);
	GObject* trgObj = GetSelectionNodes(trg, trgNodes);

	if ((srcObj == nullptr) || (trgObj == nullptr)) 
	{
		QMessageBox::critical(GetMainWindow(), "MOI Registration", "Invalid selection.");
		return;
	}

	GMOIRegistration moi;
    moi.SetUseArea(ui->useArea());
	Transform Q = moi.Register(trgObj, srcObj);

	vec3d t = Q.GetPosition();
	quatd q = Q.GetRotation();

	Transform Qs = srcObj->GetTransform();
	Qs.Rotate(q, vec3d(0,0,0));
	Qs.Translate(t);
//	Qs.Scale(Q.GetScale());

	GetMainWindow()->AddLogEntry(QString("MOI Registration:\n"));
	GetMainWindow()->AddLogEntry(QString("  Translation   : %1, %2, %3\n").arg(t.x).arg(t.y).arg(t.z));
	GetMainWindow()->AddLogEntry(QString("  Rotation      : %1, %2, %3, %4\n").arg(q.x).arg(q.y).arg(q.z).arg(q.w));

	doc->DoCommand(new CCmdTransformObject(srcObj, Qs), srcObj->GetName());

	GetMainWindow()->RedrawGL();
}

void CMOIRegistrationTool::Activate()
{
	ui->clearSelections();
}

void CMOIRegistrationTool::Deactivate()
{
	ui->clearSelections();
}

FSItemListBuilder* CMOIRegistrationTool::getSelection()
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return nullptr;

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return nullptr;

	FSItemListBuilder* item = ps->CreateItemList();
	return item;
}

void CMOIRegistrationTool::on_src_addButtonClicked()
{
	FSItemListBuilder* item = getSelection();
    if (item) ui->SetSourceList(item);
}

void CMOIRegistrationTool::on_src_subButtonClicked() {}
void CMOIRegistrationTool::on_src_delButtonClicked() {}
void CMOIRegistrationTool::on_src_selButtonClicked() {}
void CMOIRegistrationTool::on_src_clearButtonClicked() {}

void CMOIRegistrationTool::on_trg_addButtonClicked() 
{
	FSItemListBuilder* item = getSelection();
	if (item) ui->SetTargetList(item);
}

void CMOIRegistrationTool::on_trg_subButtonClicked() {}
void CMOIRegistrationTool::on_trg_delButtonClicked() {}
void CMOIRegistrationTool::on_trg_selButtonClicked() {}
void CMOIRegistrationTool::on_trg_clearButtonClicked() {}
