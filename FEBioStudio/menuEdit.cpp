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
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "DlgFind.h"
#include "DlgSelectRange.h"
#include "DlgTransform.h"
#include "ModelDocument.h"
#include "DlgCloneObject.h"
#include "DlgCloneGrid.h"
#include "DlgCloneRevolve.h"
#include "DlgMergeObjects.h"
#include "DlgDetachSelection.h"
#include "DlgPurge.h"
#include "PostDocument.h"
#include "XMLDocument.h"
#include "Commands.h"
#include <MeshTools/GModel.h>
#include <QMessageBox>
#include <QInputDialog>
#include <GeomLib/GPrimitive.h>
#include <PostGL/GLModel.h>
#include <MeshTools/FEMeshOverlap.h>
#include <sstream>

using std::stringstream;

void CMainWindow::on_actionUndo_triggered()
{
	CUndoDocument* doc = dynamic_cast<CUndoDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (doc->CanUndo())
	{
		doc->UndoCommand();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionRedo_triggered()
{
	CUndoDocument* doc = dynamic_cast<CUndoDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (doc->CanRedo())
	{
		doc->RedoCommand();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionInvertSelection_triggered()
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* ps = doc->GetCurrentSelection();
	if (ps)
	{
		doc->DoCommand(new CCmdInvertSelection(doc));
		Update();
	}
}

void CMainWindow::on_actionClearSelection_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* ps = doc->GetCurrentSelection();
	if (ps && ps->Size())
	{
		int item = doc->GetItemMode();
		int nsel = doc->GetSelectionMode();
		GModel* mdl = doc->GetGModel();
		GObject* po = doc->GetActiveObject();
		FSMesh* pm = (po ? po->GetFEMesh() : 0);
		FSMeshBase* pmb = (po ? po->GetEditableMesh() : 0);
		FSLineMesh* pml = (po ? po->GetEditableLineMesh() : 0);
		switch (item)
		{
		case ITEM_MESH:
		{
			switch (nsel)
			{
			case SELECT_OBJECT: doc->DoCommand(new CCmdSelectObject(mdl, 0, false)); break;
			case SELECT_PART: doc->DoCommand(new CCmdSelectPart(mdl, 0, 0, false)); break;
			case SELECT_FACE: doc->DoCommand(new CCmdSelectSurface(mdl, 0, 0, false)); break;
			case SELECT_EDGE: doc->DoCommand(new CCmdSelectEdge(mdl, 0, 0, false)); break;
			case SELECT_NODE: doc->DoCommand(new CCmdSelectNode(mdl, 0, 0, false)); break;
			case SELECT_DISCRETE: doc->DoCommand(new CCmdSelectDiscrete(mdl, 0, 0, false)); break;
			}
		}
		break;
		case ITEM_ELEM: doc->DoCommand(new CCmdSelectElements(pm, 0, 0, false)); break;
		case ITEM_FACE: doc->DoCommand(new CCmdSelectFaces(pmb, 0, 0, false)); break;
		case ITEM_EDGE: doc->DoCommand(new CCmdSelectFEEdges(pml, 0, 0, false)); break;
		case ITEM_NODE: doc->DoCommand(new CCmdSelectFENodes(pml, 0, 0, false)); break;
		}

		Update();
	}
}

void CMainWindow::on_actionDeleteSelection_triggered()
{
    CXMLDocument* xmlDoc  = dynamic_cast<CXMLDocument*>(GetDocument());
    if(xmlDoc)
    {
        // see if the focus is on the xml tree
        if(ui->xmlTree->hasFocus())
        {
            ui->xmlTree->on_removeSelectedRow_triggered();
            return;
        }
    }

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// give the build panel a chance to process this event first
	if (ui->buildPanel->OnDeleteEvent())
	{
		return;
	}

	// see if the focus is on the model viewer
	if (ui->modelViewer->IsFocus())
	{
		if (ui->modelViewer->OnDeleteEvent())
			return;
	}

	FESelection* psel = doc->GetCurrentSelection();
	if (psel == 0) return;

	if (dynamic_cast<GDiscreteSelection*>(psel))
	{
		GDiscreteSelection* pds = dynamic_cast<GDiscreteSelection*>(psel);
		CCmdGroup* pcmd = new CCmdGroup("Delete Discrete");
		FSModel* ps = doc->GetFSModel();
		GModel& model = ps->GetModel();
		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GDiscreteObject* po = model.DiscreteObject(i);
			if (po->IsSelected())
				pcmd->AddCommand(new CCmdDeleteDiscreteObject(&model, po));
		}
		doc->DoCommand(pcmd);

		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(model.DiscreteObject(i));
			if (pds)
			{
				for (int i = 0; i<pds->size();)
				{
					if (pds->element(i).IsSelected()) pds->RemoveElement(i);
					else i++;
				}
			}
		}
	}
	else if (dynamic_cast<GPartSelection*>(psel))
	{
		GModel& m = *doc->GetGModel();
		GPartSelection* sel = dynamic_cast<GPartSelection*>(psel);
		int n = sel->Count();
		if (n == 0) return;

		int nanswer = QMessageBox::question(this, "FEBio Studio", "Deleting parts cannot be undone.\nDo you wish to continue?");
		if (nanswer == QMessageBox::Yes)
		{
			GPartSelection::Iterator it(sel);
			vector<int> pid(n);
			for (int i = 0; i < n; ++i, ++it)
			{
				pid[i] = it->GetID();
			}

			for (int i = 0; i < n; ++i)
			{
				GPart* pg = m.FindPart(pid[i]); assert(pg);
				if (pg)
				{
					std::string partName = pg->GetName();
					if (m.DeletePart(pg) == false)
					{
						QString err; err = QString("Failed deleting Part \"%1\" (id = %2)").arg(QString::fromStdString(partName)).arg(pid[i]);
						QMessageBox::critical(this, "FEBio Studio", err);
						break;
					}
				}
				else
				{
					QString err; err = QString("Cannot find part with ID %1.").arg(pid[i]);
					QMessageBox::critical(this, "FEBio Studio", err);
					break;
				}
			}

			// TODO: This cannot be undone at the moment
			doc->ClearCommandStack();
		}
	}
	else
	{
		int item = doc->GetItemMode();
		if (item == ITEM_MESH)
		{
			int nsel = doc->GetSelectionMode();
			if (nsel == SELECT_OBJECT)
			{
				CCmdGroup* pcmd = new CCmdGroup("Delete");
				FSModel* ps = doc->GetFSModel();
				GModel& model = ps->GetModel();
				for (int i = 0; i<model.Objects(); ++i)
				{
					GObject* po = model.Object(i);
					if (po->IsSelected())
						pcmd->AddCommand(new CCmdDeleteGObject(&model, po));
				}
				doc->DoCommand(pcmd);
			}
			else
			{
				QMessageBox::information(this, "FEBio Studio", "Cannot delete this selection.");
			}
		}
		else
		{
			GObject* po = doc->GetActiveObject();
			if (po == 0) return;

			GMeshObject* pgo = dynamic_cast<GMeshObject*>(po);
			if (pgo && pgo->GetFEMesh()) doc->DoCommand(new CCmdDeleteFESelection(pgo, doc->GetItemMode()));

			GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
			if (pso && pso->GetSurfaceMesh()) doc->DoCommand(new CCmdDeleteFESurfaceSelection(pso, doc->GetItemMode()));

			GPrimitive* pp = dynamic_cast<GPrimitive*>(po);
			if (pp)
			{
				QMessageBox::information(this, "FEBio Studio", "Cannot delete mesh selections of a primitive object.");
			}
		}
	}
	Update(0, true);
	ClearStatusMessage();
}

void CMainWindow::on_actionHideSelection_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		switch (mdl.GetSelectionMode())
		{
		case Post::SELECT_NODES: mdl.HideSelectedNodes(); break;
		case Post::SELECT_EDGES: mdl.HideSelectedEdges(); break;
		case Post::SELECT_FACES: mdl.HideSelectedFaces(); break;
		case Post::SELECT_ELEMS: mdl.HideSelectedElements(); break;
		}
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		if (doc)
		{
			doc->HideCurrentSelection();
			UpdateModel();
			Update();
		}
	}
}

void CMainWindow::on_actionHideUnselected_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.HideUnselectedElements();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		doc->HideUnselected();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionUnhideAll_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.UnhideAll();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		doc->DoCommand(new CCmdUnhideAll(doc));
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionFind_triggered()
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	GObject* po = GetActiveObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	int nitem = doc->GetItemMode();
	int nsel = 0;
	if (nitem == ITEM_NODE) nsel = 0;
	if (nitem == ITEM_EDGE) nsel = 1;
	if (nitem == ITEM_FACE) nsel = 2;
	if (nitem == ITEM_ELEM) nsel = 3;

	CDlgFind dlg(this, nsel);

	if (dlg.exec())
	{
		if (dlg.m_bsel[0]) nitem = ITEM_NODE;
		if (dlg.m_bsel[1]) nitem = ITEM_EDGE;
		if (dlg.m_bsel[2]) nitem = ITEM_FACE;
		if (dlg.m_bsel[3]) nitem = ITEM_ELEM;

		SetItemSelectionMode(SELECT_OBJECT, nitem);

		vector<int> items = dlg.m_item;

		CGLControlBar* pb = ui->glw->glc;
		switch (nitem)
		{
		case ITEM_NODE: doc->DoCommand(new CCmdSelectFENodes(pm, items, !dlg.m_bclear)); break;
		case ITEM_EDGE: doc->DoCommand(new CCmdSelectFEEdges(pm, items, !dlg.m_bclear)); break;
		case ITEM_FACE: doc->DoCommand(new CCmdSelectFaces(pm, items, !dlg.m_bclear)); break;
		case ITEM_ELEM: doc->DoCommand(new CCmdSelectElements(pm, items, !dlg.m_bclear)); break;
		}

		CPostDocument* postDoc = dynamic_cast<CPostDocument*>(doc);
		if (postDoc) postDoc->GetGLModel()->UpdateSelectionLists();

		ReportSelection();
		RedrawGL();
	}
}

void CMainWindow::on_actionSelectRange_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;
	if (!postDoc->IsValid()) return;

	Post::CGLModel* model = postDoc->GetGLModel(); assert(model);
	if (model == 0) return;

	Post::CGLColorMap* pcol = postDoc->GetGLModel()->GetColorMap();
	if (pcol == 0) return;

	float d[2];
	pcol->GetRange(d);

	CDlgSelectRange dlg(this);
	dlg.m_min = d[0];
	dlg.m_max = d[1];

	if (dlg.exec())
	{
		CGLDocument* doc = GetGLDocument();
		switch (model->GetSelectionMode())
		{
		case Post::SELECT_NODES: doc->SetItemMode(ITEM_NODE); model->SelectNodesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_EDGES: doc->SetItemMode(ITEM_EDGE); model->SelectEdgesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_FACES: doc->SetItemMode(ITEM_FACE); model->SelectFacesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_ELEMS: doc->SetItemMode(ITEM_ELEM); model->SelectElemsInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		}

		model->UpdateSelectionLists();
		postDoc->UpdateFEModel();
		ReportSelection();
		UpdateGLControlBar();
		RedrawGL();
	}
}

void CMainWindow::on_actionToggleVisible_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.ToggleVisibleElements();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

		int nsel = doc->GetSelectionMode();
		int nitem = doc->GetItemMode();

		GModel* mdl = doc->GetGModel();

		CCommand* cmd = 0;
		if (nitem == ITEM_MESH)
		{
			switch (nsel)
			{
			case SELECT_OBJECT: cmd = new CCmdToggleObjectVisibility(mdl); break;
			case SELECT_PART: cmd = new CCmdTogglePartVisibility(mdl); break;
			case SELECT_DISCRETE: cmd = new CCmdToggleDiscreteVisibility(mdl); break;
			}
		}
		else
		{
			GObject* po = doc->GetActiveObject();
			if (po == 0) return;

			FSMesh* pm = po->GetFEMesh();
			FSMeshBase* pmb = po->GetEditableMesh();

			switch (nitem)
			{
			case ITEM_ELEM: if (pm) cmd = new CCmdToggleElementVisibility(pm); break;
			case ITEM_FACE: if (pmb) cmd = new CCmdToggleFEFaceVisibility(pm); break;
			}
		}

		if (cmd)
		{
			doc->DoCommand(cmd);
			UpdateModel();
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionNameSelection_triggered()
{
	static int nparts = 1;
	static int nsurfs = 1;
	static int nedges = 1;
	static int nnodes = 1;

	char szname[256] = { 0 };

	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;

	// we need a bit more for model docs
	FSModel* pfem = nullptr;
	GModel* mdl = nullptr;
	if (GetModelDocument())
	{
		CModelDocument* modelDoc = GetModelDocument();
		pfem = modelDoc->GetFSModel();
		mdl = modelDoc->GetGModel();
	}

	// make sure there is a selection
	FESelection* psel = doc->GetCurrentSelection();
	if ((psel == nullptr) || (psel->Size() == 0)) return;

	// set the name
	int item = doc->GetItemMode();
	switch (item)
	{
	case ITEM_ELEM: sprintf(szname, "Part%02d", nparts); break;
	case ITEM_FACE: sprintf(szname, "Surface%02d", nsurfs); break;
	case ITEM_EDGE: sprintf(szname, "EdgeSet%02d", nedges); break;
	case ITEM_NODE: sprintf(szname, "Nodeset%02d", nnodes); break;
	case ITEM_MESH:
	{
		int nsel = doc->GetSelectionMode();
		switch (nsel)
		{
		case SELECT_PART: sprintf(szname, "Part%02d", nparts); break;
		case SELECT_FACE: sprintf(szname, "Surface%02d", nsurfs); break;
		case SELECT_EDGE: sprintf(szname, "EdgeSet%02d", nedges); break;
		case SELECT_NODE: sprintf(szname, "Nodeset%02d", nnodes); break;
		default:
			return;
		}
	}
	break;
	}

	bool ok;
	QString text = QInputDialog::getText(this, "Name Selection", "Name:", QLineEdit::Normal, szname, &ok);

	if (ok && !text.isEmpty())
	{
		string sname = text.toStdString();
		const char* szname = sname.c_str();

		GObject* po = doc->GetActiveObject();

		// create a new group
		switch (item)
		{
		case ITEM_ELEM:
		{
			assert(po);
			FEElementSelection* pes = dynamic_cast<FEElementSelection*>(psel); assert(pes);
			FSPart* pg = dynamic_cast<FSPart*>(pes->CreateItemList());
			if (pg)
			{
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddPart(po, pg));
				++nparts;
				UpdateModel(pg);
			}
			else QMessageBox::critical(this, "Name Selection", "Failed to create named selection.");
		}
		break;
		case ITEM_FACE:
		{
			assert(po);
			FEFaceSelection* pfs = dynamic_cast<FEFaceSelection*>(psel);
			FSSurface* pg = dynamic_cast<FSSurface*>(pfs->CreateItemList());
			if (pg)
			{
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddSurface(po, pg));
				++nsurfs;
				UpdateModel(pg);
			}
			else QMessageBox::critical(this, "Name Selection", "Failed to create named selection.");
		}
		break;
		case ITEM_EDGE:
		{
			assert(po);
			FEEdgeSelection* pes = dynamic_cast<FEEdgeSelection*>(psel);
			FSEdgeSet* pg = dynamic_cast<FSEdgeSet*>(pes->CreateItemList());
			if (pg)
			{
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddFEEdgeSet(po, pg));
				++nsurfs;
				UpdateModel(pg);
			}
			else QMessageBox::critical(this, "Name Selection", "Failed to create named selection.");
		}
		break;
		case ITEM_NODE:
		{
			assert(po);
			FENodeSelection* pns = dynamic_cast<FENodeSelection*>(psel);
			FSNodeSet* pg = dynamic_cast<FSNodeSet*>(pns->CreateItemList());
			if (pg)
			{
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddNodeSet(po, pg));
				++nnodes;
				UpdateModel(pg);
			}
			else QMessageBox::critical(this, "Name Selection", "Failed to create named selection.");
		}
		break;
		case ITEM_MESH:
		{
			assert(pfem);
			assert(mdl);
			int nsel = doc->GetSelectionMode();
			switch (nsel)
			{
			case SELECT_PART:
			{
				GPartList* pg = new GPartList(pfem, dynamic_cast<GPartSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGPartGroup(mdl, pg));
				++nparts;
				UpdateModel(pg);
			}
			break;
			case SELECT_FACE:
			{
				GFaceList* pg = new GFaceList(pfem, dynamic_cast<GFaceSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGFaceGroup(mdl, pg));
				++nsurfs;
				UpdateModel(pg);
			}
			break;
			case SELECT_EDGE:
			{
				GEdgeList* pg = new GEdgeList(pfem, dynamic_cast<GEdgeSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGEdgeGroup(mdl, pg));
				++nedges;
				UpdateModel(pg);
			}
			break;
			case SELECT_NODE:
			{
				GNodeList* pg = new GNodeList(pfem, dynamic_cast<GNodeSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGNodeGroup(mdl, pg));
				++nnodes;
				UpdateModel(pg);
			}
			break;
			}
		}
		}
	}
}

void CMainWindow::on_actionTransform_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* ps = doc->GetCurrentSelection();

	if (ps && ps->Size())
	{
		vec3d pos = GetGLView()->GetPivotPosition();
		quatd rot = ps->GetOrientation();
		vec3d r = rot.GetVector()*(180 * rot.GetAngle() / PI);
		vec3d scl = ps->GetScale();

		CDlgTransform dlg(this);
		dlg.m_pos = pos;
		dlg.m_relPos = vec3d(0, 0, 0);

		dlg.m_rot = r;
		dlg.m_relRot = vec3d(0, 0, 0);

		dlg.m_scl = scl;
		dlg.m_relScl = vec3d(1, 1, 1);

		dlg.Init();

		if (dlg.exec())
		{
			CCmdGroup* pcmd = new CCmdGroup("Transform");

			// translation
			vec3d dr = dlg.m_pos - pos + dlg.m_relPos;
			pcmd->AddCommand(new CCmdTranslateSelection(doc, dr));

			// rotation
			vec3d r = dlg.m_rot;
			double w = PI*r.Length() / 180;
			r.Normalize();
			rot = quatd(w, r)*rot.Inverse();

			r = dlg.m_relRot;
			w = PI*r.Length() / 180;
			r.Normalize();
			rot = quatd(w, r)*rot;

			CGLView* glview = GetGLView();

			pos += dr;
			pcmd->AddCommand(new CCmdRotateSelection(doc, rot, pos));

			// scale
			vec3d s1(1, 0, 0);
			vec3d s2(0, 1, 0);
			vec3d s3(0, 0, 1);

			// NOTE: not sure why, but I need to rotate the s vectors
			rot = rot*ps->GetOrientation();
			rot.RotateVector(s1);
			rot.RotateVector(s2);
			rot.RotateVector(s3);

			pcmd->AddCommand(new CCmdScaleSelection(doc, dlg.m_scl.x*dlg.m_relScl.x / scl.x, s1, pos));
			pcmd->AddCommand(new CCmdScaleSelection(doc, dlg.m_scl.y*dlg.m_relScl.y / scl.y, s2, pos));
			pcmd->AddCommand(new CCmdScaleSelection(doc, dlg.m_scl.z*dlg.m_relScl.z / scl.z, s3, pos));

			doc->DoCommand(pcmd);
			doc->GetGModel()->UpdateBoundingBox();
			UpdateGLControlBar();
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionCollapseTransform_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	GObjectSelection* sel = dynamic_cast<GObjectSelection*>(doc->GetCurrentSelection());
	if ((sel == nullptr) || (sel->Size() == 0))
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an object");
		return;
	}

	for (int i = 0; i < sel->Size(); ++i)
	{
		GObject* po = sel->Object(i);
		po->CollapseTransform();
	}
	RedrawGL();
}

void CMainWindow::on_actionClone_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneObject dlg(this);
	if (dlg.exec())
	{
		// get the model
		GModel& m = *doc->GetGModel();

		// clone the object
		GObject* pco = m.CloneObject(po);
		if (pco == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Could not clone this object.");
			return;
		}

		// set the name
		QString name = dlg.GetNewObjectName();
		std::string sname = name.toStdString();
		pco->SetName(sname);

		// add and select the new object
		doc->DoCommand(new CCmdAddAndSelectObject(&m, pco));

		// update windows
		Update(0, true);
	}
}

static GObject* copyObject = nullptr;

void CMainWindow::on_actionCopyObject_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	// get the model
	GModel& m = *doc->GetGModel();

	// clone the object
	GObject* pco = m.CloneObject(po);
	if (pco == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Could not clone this object.");
		return;
	}

	// copy the name
	pco->SetName(po->GetName());

	// store this object
	copyObject = pco;
}

void CMainWindow::on_actionPasteObject_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (copyObject == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No object to paste.");
		return;
	}

	// get the model
	GModel& m = *doc->GetGModel();

	// we need to make sure that the object has a unique name.
	string nameBase = copyObject->GetName();
	string name = nameBase;
	GObject* po = nullptr;
	do
	{
		po = m.FindObject(name);
		int n = 1;
		if (po)
		{
			stringstream ss;
			ss << nameBase << "(" << n++ << ")";
			name = ss.str();
		}
	} while (po);
	copyObject->SetName(name);

	// since the copy object was created in another model,
	// it is possible that its items IDs are already used in this model. 
	// therefore, we reindex the object
	copyObject->Reindex();

	// add and select the new object
	doc->DoCommand(new CCmdAddAndSelectObject(&m, copyObject));
	GetGLView()->ZoomToObject(copyObject);
	copyObject = nullptr;

	// update windows
	Update(0, true);
}

void CMainWindow::on_actionCloneGrid_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneGrid dlg(this);
	if (dlg.exec())
	{
		GModel& m = *doc->GetGModel();

		// clone the object
		vector<GObject*> newObjects = m.CloneGrid(po, dlg.m_rangeX[0], dlg.m_rangeX[1], dlg.m_rangeY[0], dlg.m_rangeY[1], dlg.m_rangeZ[0], dlg.m_rangeZ[1], dlg.m_inc[0], dlg.m_inc[1], dlg.m_inc[2]);
		if (newObjects.empty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to grid clone this object");
			return;
		}

		// add all the objects
		CCmdGroup* cmd = new CCmdGroup("Clone grid");
		for (int i = 0; i<(int)newObjects.size(); ++i)
		{
			cmd->AddCommand(new CCmdAddObject(&m, newObjects[i]));
		}
		doc->DoCommand(cmd);

		// update UI
		Update(0, true);
	}
}

void CMainWindow::on_actionCloneRevolve_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneRevolve dlg(this);
	if (dlg.exec())
	{
		GModel& m = *doc->GetGModel();

		vector<GObject*> newObjects = m.CloneRevolve(po, dlg.m_count, dlg.m_range, dlg.m_spiral, dlg.m_center, dlg.m_axis, dlg.m_rotateClones);
		if (newObjects.empty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to revolve clone this object");
			return;
		}

		// add all the objects
		CCmdGroup* cmd = new CCmdGroup("Clone revolve");
		for (int i = 0; i<(int)newObjects.size(); ++i)
		{
			cmd->AddCommand(new CCmdAddObject(&m, newObjects[i]));
		}
		doc->DoCommand(cmd);

		// update UI
		Update(0, true);
	}
}

void CMainWindow::on_actionMerge_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// make sure we have an object selection
	FESelection* currentSelection = doc->GetCurrentSelection();
	if (currentSelection->Type() != SELECT_OBJECTS)
	{
		QMessageBox::critical(this, "Merge Objects", "Cannot merge objects");
		return;
	}
	GObjectSelection* sel = dynamic_cast<GObjectSelection*>(currentSelection);
	if ((sel == nullptr) || (sel->Count() < 2))
	{
		QMessageBox::critical(this, "Merge Objects", "You need to select at least two objects.");
		return;
	}

	CDlgMergeObjects dlg(this);
	if (dlg.exec() == QDialog::Rejected) return;

	// merge the objects
	GModel& m = *doc->GetGModel();
	GObject* newObject = m.MergeSelectedObjects(sel, dlg.m_name, dlg.m_weld, dlg.m_tol);
	if (newObject == 0)
	{
		QMessageBox::critical(this, "Merge Objects", "Cannot merge objects");
		return;
	}

	// we need to delete the selected objects and add the new object
	// create the command that will do the attaching
	CCmdGroup* pcmd = new CCmdGroup("Attach");
	for (int i = 0; i<sel->Count(); ++i)
	{
		// remove the old object
		GObject* po = sel->Object(i);
		pcmd->AddCommand(new CCmdDeleteGObject(&m, po));
	}
	// add the new object
	pcmd->AddCommand(new CCmdAddAndSelectObject(&m, newObject));

	// perform the operation
	doc->DoCommand(pcmd);

	// update UI
	Update(0, true);
}

void CMainWindow::on_actionDetach_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0) || (sel->Type() != SELECT_FE_ELEMENTS))
	{
		QMessageBox::warning(this, "Detach Selection", "Cannot detach this selection");
		return;
	}

	CDlgDetachSelection dlg(this);
	if (dlg.exec())
	{
		GMeshObject* po = dynamic_cast<GMeshObject*>(doc->GetActiveObject()); assert(po);
		if (po == 0) return;

		// create a new object for this mesh
		GMeshObject* newObject = po->DetachSelection();

		// give the object a new name
		string newName = dlg.getName().toStdString();
		newObject->SetName(newName);

		// add it to the pile
		doc->DoCommand(new CCmdAddObject(doc->GetGModel(), newObject));

		UpdateModel(newObject, true);
	}
}


void CMainWindow::on_actionExtract_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0) || (sel->Type() != SELECT_FE_FACES))
	{
		QMessageBox::warning(this, "Extract Selection", "Cannot extract this selection");
		return;
	}

	CDlgExtractSelection dlg(this);
	if (dlg.exec())
	{
		GObject* po = doc->GetActiveObject();
		if (po == 0) return;

		// create a new object for this mesh
		GMeshObject* newObject = ExtractSelection(po);

		// give the object a new name
		string newName = dlg.getName().toStdString();
		newObject->SetName(newName);

		// add it to the pile
		doc->DoCommand(new CCmdAddObject(doc->GetGModel(), newObject));

		UpdateModel(newObject, true);
	}
}

void CMainWindow::on_actionPurge_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgPurge dlg(this);
	if (dlg.exec())
	{
		FSModel* ps = doc->GetFSModel();
		ps->Purge(dlg.getOption());
		doc->ClearCommandStack();
		doc->SetModifiedFlag(true);
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionFaceToElem_triggered()
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (doc->GetSelectionMode() != SELECT_OBJECT) return;
	if (doc->GetItemMode() != ITEM_FACE) return;

	GObject* po = doc->GetActiveObject();
	if (po == nullptr) return;

	FSMesh* mesh = po->GetFEMesh();
	if (mesh == nullptr) return;

	vector<int> selectedElems = mesh->GetElementsFromSelectedFaces();
	if (selectedElems.empty() == false)
	{
		SetItemSelectionMode(SELECT_OBJECT, ITEM_ELEM);
		CCmdSelectElements* cmd = new CCmdSelectElements(mesh, selectedElems, false);
		doc->DoCommand(cmd);
	}

	RedrawGL();
}

void CMainWindow::on_actionSurfaceToFaces_triggered()
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (doc->GetSelectionMode() != SELECT_FACE) return;

	GObject* po = doc->GetActiveObject();
	if (po == nullptr) return;

	FSMesh* mesh = po->GetFEMesh();
	if (mesh == nullptr) return;

	// now, select the faces
	vector<int> faceList;
	for (int i = 0; i < mesh->Faces(); ++i)
	{
		FSFace& face = mesh->Face(i);
		GFace* pf = po->Face(face.m_gid);
		if (pf->IsSelected()) faceList.push_back(i);
	}

	// select elements
	doc->SetSelectionMode(SELECT_OBJECT);
	doc->SetItemMode(ITEM_FACE);
	doc->DoCommand(new CCmdSelectFaces(mesh, faceList, false));

	UpdateGLControlBar();
	RedrawGL();
}

void CMainWindow::on_actionSelectOverlap_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	GObject* po = doc->GetActiveObject();
	if (po == nullptr) return;
	if (po->GetFEMesh() == nullptr)
	{
		QMessageBox::critical(this, "Select Overlap", "You need to select an object that has a mesh.");
		return;
	}

	GModel& mdl = *doc->GetGModel();
	QStringList objects;
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* poi = mdl.Object(i);
		if (poi->GetEditableMesh())
			if (poi != po) objects.push_back(QString::fromStdString(poi->GetName()));
	}
	if (objects.isEmpty())
	{
		QMessageBox::critical(this, "Select Overlap", "No suitable target objects available. Valid target objects must have a mesh.");
		return;
	}

	QString select = QInputDialog::getItem(this, "Select Overlap", "Pick object:", objects, 0, false);
	if (select.isEmpty() == false)
	{
		GObject* trg = mdl.FindObject(select.toStdString());
		FSMesh* mesh = po->GetFEMesh();
		std::vector<int> faceList = MeshTools::FindSurfaceOverlap(mesh, trg->GetEditableMesh());
		
		SetItemSelectionMode(SELECT_OBJECT, ITEM_FACE);
		doc->DoCommand(new CCmdSelectFaces(mesh, faceList, false));
		RedrawGL();
	}
}

void CMainWindow::on_actionSyncSelection_triggered()
{
	ui->modelViewer->on_syncButton_clicked();
}

void CMainWindow::on_actionSelectIsolatedVertices_triggered()
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;

	GObject* po = doc->GetActiveObject();
	if (po == nullptr) return;

	FSMesh* mesh = po->GetFEMesh();
	if (mesh == nullptr) return;
	
	mesh->TagAllNodes(0);
	int NE = mesh->Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh->Element(i);
		int ne = el.Nodes();
		for (int j = 0; j < ne; ++j)
		{
			mesh->Node(el.m_node[j]).m_ntag = 1;
		}
	}

	std::vector<int> isolatedVerts;
	for (int i = 0; i < mesh->Nodes(); ++i)
	{
		if (mesh->Node(i).m_ntag == 0)
		{
			isolatedVerts.push_back(i);
		}
	}

	AddLogEntry(QString("%1: %2 isolated vertices found\n").arg(QString::fromStdString(po->GetName())).arg(isolatedVerts.size()));

	if (isolatedVerts.empty() == false)
	{
		CCommand* cmd = new CCmdSelectFENodes(mesh, isolatedVerts, false);
		doc->DoCommand(cmd);
		RedrawGL();
	}
}

void CMainWindow::on_actionGrowSelection_triggered()
{
	GObject* po = GetActiveObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	FSMeshBase* pmb = pm;
	if (pm == nullptr)
	{
		GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
		if (pso) pmb = pso->GetSurfaceMesh();
		if (pmb == nullptr) return;
	}

	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetDocument());
	if (doc == nullptr) return;
	if (doc->GetSelectionMode() != SELECT_OBJECT) return;

	VIEW_SETTINGS& vs = GetGLView()->GetViewSettings();

	int itemMode = doc->GetItemMode();
	switch (itemMode)
	{
	case ITEM_ELEM: doc->GrowElementSelection(pm, vs.m_bpart); break;
	case ITEM_FACE: doc->GrowFaceSelection(pmb, vs.m_bpart); break;
	case ITEM_EDGE: doc->GrowEdgeSelection(pmb); break;
	case ITEM_NODE: doc->GrowNodeSelection(pmb); break;
	}

	RedrawGL();
}

void CMainWindow::on_actionShrinkSelection_triggered()
{
	GObject* po = GetActiveObject();
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	FSMeshBase* pmb = pm;
	if (pm == nullptr)
	{
		GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
		if (pso) pmb = pso->GetSurfaceMesh();
		if (pmb == nullptr) return;
	}

	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return;
	if (doc->GetSelectionMode() != SELECT_OBJECT) return;

	int itemMode = doc->GetItemMode();
	switch (itemMode)
	{
	case ITEM_ELEM: doc->ShrinkElementSelection(pm); break;
	case ITEM_FACE: doc->ShrinkFaceSelection(pmb); break;
	case ITEM_EDGE: doc->ShrinkEdgeSelection(pmb); break;
	case ITEM_NODE: doc->ShrinkNodeSelection(pmb); break;
	}

	RedrawGL();
}

void CMainWindow::on_actionSelect_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_NONE);
	RedrawGL();
}

void CMainWindow::on_actionTranslate_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_MOVE);
	RedrawGL();
}

void CMainWindow::on_actionRotate_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_ROTATE);
	RedrawGL();
}

void CMainWindow::on_actionScale_toggled(bool b)
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_SCALE);
	RedrawGL();
}

void CMainWindow::on_selectCoord_currentIndexChanged(int n)
{
	CGLView* glview = GetGLView();
	switch (n)
	{
	case 0: glview->SetCoordinateSystem(COORD_GLOBAL); break;
	case 1: glview->SetCoordinateSystem(COORD_LOCAL); break;
	case 2: glview->SetCoordinateSystem(COORD_SCREEN); break;
	}
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionMeasureTool_triggered()
{
	if (ui->measureTool == nullptr) ui->measureTool = new CDlgMeasure(this);
	ui->measureTool->show();
}

void CMainWindow::on_actionPlaneCutTool_triggered()
{
	if (ui->planeCutTool == nullptr) ui->planeCutTool = new CDlgPlaneCut(this);
	ui->planeCutTool->show();
}

void CMainWindow::on_actionFindTxt_triggered()
{
	CTextDocument* doc = dynamic_cast<CTextDocument*>(GetDocument());
	if (doc == nullptr) return;

	QString txt = QInputDialog::getText(this, "FEBio Studio", "Text:");
	if (txt.isEmpty() == false)
	{
		ui->m_lastFindText = txt;

		if (ui->xmlEdit->find(txt) == false)
		{
			QMessageBox::information(this, "FEBio Studio", QString("Cannot find: %1").arg(txt));
		}
		else ui->xmlEdit->centerCursor();
	}
}

void CMainWindow::on_actionFindAgain_triggered()
{
	CTextDocument* doc = dynamic_cast<CTextDocument*>(GetDocument());
	if (doc == nullptr) return;

	if (ui->m_lastFindText.isEmpty() == false)
	{
		if (ui->xmlEdit->find(ui->m_lastFindText) == false)
		{
			QMessageBox::information(this, "FEBio Studio", QString("Cannot find: %1").arg(ui->m_lastFindText));
		}
		else ui->xmlEdit->centerCursor();
	}
}

void CMainWindow::on_actionToggleComment_triggered()
{
	CTextDocument* doc = dynamic_cast<CTextDocument*>(GetDocument());
	if (doc == nullptr) return;
	ui->xmlEdit->toggleLineComment();
}

void CMainWindow::on_actionDuplicateLine_triggered()
{
	CTextDocument* doc = dynamic_cast<CTextDocument*>(GetDocument());
	if (doc == nullptr) return;
	ui->xmlEdit->duplicateLine();
}

void CMainWindow::on_actionDeleteLine_triggered()
{
	CTextDocument* doc = dynamic_cast<CTextDocument*>(GetDocument());
	if (doc == nullptr) return;
	ui->xmlEdit->deleteLine();
}
