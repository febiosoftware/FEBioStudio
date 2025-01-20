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
#include "ICPRegistrationTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QGroupBox>
#include "ModelDocument.h"
#include "SelectionBox.h"
#include <GeomLib/GObject.h>
#include <MeshTools/ICPRegistration.h>
#include <GeomLib/GGroup.h>
#include "MainWindow.h"
#include "Commands.h"
using namespace std;

class ICPRegistrationToolUI : public QWidget
{
private:
    QLineEdit* m_tol;
    QLineEdit* m_maxiter;
    CSelectionBox* m_src;
    CSelectionBox* m_trg;

private:
    FEItemListBuilder* m_srcList;
    FEItemListBuilder* m_trgList;

public:
    ICPRegistrationToolUI(CICPRegistrationTool* w)
    {
        m_srcList = nullptr;
        m_trgList = nullptr;

        QFormLayout* f = new QFormLayout;
        f->addRow("Tolerance:", m_tol = new QLineEdit); m_tol->setValidator(new QDoubleValidator());
        f->addRow("Max. iterations:", m_maxiter = new QLineEdit); m_maxiter->setValidator(new QIntValidator(1, 10000));
        QPushButton* apply = new QPushButton("Apply");

        f->setAlignment(Qt::AlignRight);

        m_tol->setText(QString::number(0.01));
        m_maxiter->setText(QString::number(100));

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

    double tolerance() { return m_tol->text().toDouble(); }
    int maxIterations() { return m_maxiter->text().toInt(); }

	bool UpdateSelectionList(FEItemListBuilder*& pl, FEItemListBuilder* items)
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

	FEItemListBuilder* sourceList() { return m_srcList; }
	FEItemListBuilder* targetList() { return m_trgList; }

	bool SetSourceList(FEItemListBuilder* items)
	{
		if (UpdateSelectionList(m_srcList, items) == false) return false;
		SetSelection(m_src, m_srcList);
        return true;
    }

	bool SetTargetList(FEItemListBuilder* items)
	{
		if (UpdateSelectionList(m_trgList, items) == false) return false;
		SetSelection(m_trg, m_trgList);
		return true;
	}

	void SetSelection(CSelectionBox* sel, FEItemListBuilder* item)
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
			FEItemListBuilder::Iterator it = item->begin();
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
			FEItemListBuilder::Iterator it = item->begin();
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
			FEItemListBuilder::Iterator it = item->begin();
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
			FEItemListBuilder::Iterator it = item->begin();
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
CICPRegistrationTool::CICPRegistrationTool(CMainWindow* wnd) : CAbstractTool(wnd, "ICP Registration")
{
    ui = nullptr;
}

QWidget* CICPRegistrationTool::createUi()
{
    if (ui == nullptr) ui = new ICPRegistrationToolUI(this);
    return ui;
}

vector<vec3d> extractSurfaceNodes(GObject* po, vector<GPart*> partList)
{
	vector<vec3d> points;
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return points;

	pm->TagAllNodes(0);

	for (GPart* pg : partList)
	{
		int pid = pg->GetLocalID();
		for (int i = 0; i < pm->Elements(); ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.m_gid == pid)
			{
				int nn = el.Nodes();
				for (int j = 0; j < nn; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
			}			
		}
	}

	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.IsExternal())
		{
			int nn = face.Nodes();
			for (int j = 0; j < nn; ++j)
			{
				FSNode& nj = pm->Node(face.n[j]);
				if (nj.m_ntag == 1) nj.m_ntag = 2;
			}
		}
	}

	const Transform& Q = po->GetTransform();
	points.reserve(pm->Nodes());
	for (int i = 0; i < pm->Nodes(); ++i)
	{
		FSNode& ni = pm->Node(i);
		if (ni.m_ntag == 2)
		{
			vec3d r = ni.pos();
			vec3d p = Q.LocalToGlobal(r);
			points.push_back(p);
		}
	}

	return points;
}

void CICPRegistrationTool::OnApply()
{
    CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
    if ((doc == nullptr) || (!doc->IsValid())) return;

	FEItemListBuilder* src = ui->sourceList();
	FEItemListBuilder* trg = ui->targetList();

	// get the part list
	GPartList* psrc = dynamic_cast<GPartList*>(src);
	GPartList* ptrg = dynamic_cast<GPartList*>(trg);
	if ((psrc == nullptr) || (ptrg == nullptr)) return;

	// get the object
	vector<GPart*> srcParts = psrc->GetPartList();
	vector<GPart*> trgParts = ptrg->GetPartList();
	if ((srcParts.size() < 1) || (trgParts.size() < 1)) return;

	// make sure all parts belong to the same object
	GObject* srcObj = dynamic_cast<GObject*>(srcParts[0]->Object());
	for (int i = 1; i < srcParts.size(); ++i)
	{
		GObject* poi = dynamic_cast<GObject*>(srcParts[i]->Object());
		if (poi != srcObj) return;
	}

	GObject* trgObj = dynamic_cast<GObject*>(trgParts[0]->Object());
	for (int i = 1; i < trgParts.size(); ++i)
	{
		GObject* poi = dynamic_cast<GObject*>(trgParts[i]->Object());
		if (poi != trgObj) return;
	}

	// extract all the surface nodes from the parts 
	vector<vec3d> srcNodes = extractSurfaceNodes(srcObj, srcParts);
	vector<vec3d> trgNodes = extractSurfaceNodes(trgObj, trgParts);

	GICPRegistration icp;
	icp.SetTolerance(ui->tolerance());
	icp.SetMaxIterations(ui->maxIterations());
	Transform Q = icp.Register(trgNodes, srcNodes);

	vec3d t = Q.GetPosition();
	quatd q = Q.GetRotation();

	Transform Qs = srcObj->GetTransform();
	Qs.Rotate(q, vec3d(0,0,0));
	Qs.Translate(t);
//	Qs.Scale(Q.GetScale());

	GetMainWindow()->AddLogEntry(QString("ICP Registration:\n"));
	GetMainWindow()->AddLogEntry(QString("  Iterations    : %1\n").arg(icp.Iterations()));
	GetMainWindow()->AddLogEntry(QString("  Relative error: %1\n").arg(icp.RelativeError()));
	GetMainWindow()->AddLogEntry(QString("  Translation   : %1, %2, %3\n").arg(t.x).arg(t.y).arg(t.z));
	GetMainWindow()->AddLogEntry(QString("  Rotation      : %1, %2, %3, %4\n").arg(q.x).arg(q.y).arg(q.z).arg(q.w));

	doc->DoCommand(new CCmdTransformObject(srcObj, Qs), srcObj->GetName());

	GetMainWindow()->RedrawGL();
}

void CICPRegistrationTool::Activate()
{
	ui->clearSelections();
}

void CICPRegistrationTool::Deactivate()
{
	ui->clearSelections();
}

FEItemListBuilder* CICPRegistrationTool::getSelection()
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return nullptr;

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return nullptr;

	FEItemListBuilder* item = ps->CreateItemList();
	return item;
}

void CICPRegistrationTool::on_src_addButtonClicked()
{
	FEItemListBuilder* item = getSelection();
    if (item) ui->SetSourceList(item);
}

void CICPRegistrationTool::on_src_subButtonClicked() {}
void CICPRegistrationTool::on_src_delButtonClicked() {}
void CICPRegistrationTool::on_src_selButtonClicked() {}
void CICPRegistrationTool::on_src_clearButtonClicked() {}

void CICPRegistrationTool::on_trg_addButtonClicked() 
{
	FEItemListBuilder* item = getSelection();
	if (item) ui->SetTargetList(item);
}

void CICPRegistrationTool::on_trg_subButtonClicked() {}
void CICPRegistrationTool::on_trg_delButtonClicked() {}
void CICPRegistrationTool::on_trg_selButtonClicked() {}
void CICPRegistrationTool::on_trg_clearButtonClicked() {}
