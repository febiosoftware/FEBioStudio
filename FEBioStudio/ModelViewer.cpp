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
#include "ModelViewer.h"
#include "ui_modelviewer.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEBodyLoad.h>
#include <QMessageBox>
#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>
#include "DlgEditOutput.h"
#include "DlgAddMeshData.h"
#include "MaterialEditor.h"
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <GeomLib/GObject.h>
#include <GeomLib/MeshLayer.h>
#include <GeomLib/GModel.h>
#include "Commands.h"
#include "PropertyList.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageFilter.h>
#include <ImageLib/ImageSource.h>
#include "DocManager.h"
#include "DlgAddPhysicsItem.h"
#include <FEBioLink/FEBioInterface.h>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <MeshIO/STLExport.h>
#include "PropertyList.h"

class CDlgWarnings : public QDialog
{
public:
	CDlgWarnings(QWidget* parent) : QDialog(parent)
	{
		setMinimumSize(800, 600);
		setWindowTitle("Model Warnings");

		m_out = new QPlainTextEdit;
		m_out->setReadOnly(true);
		m_out->setFont(QFont("Courier", 11));
		m_out->setWordWrapMode(QTextOption::NoWrap);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(m_out);
		l->addWidget(bb);

		setLayout(l);

		QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
	}

	void SetWarnings(QStringList errs)
	{
		QString txt;
		for (int i = 0; i < errs.size(); ++i)
		{
			txt += errs[i];
			txt += QString("\n");
		}

		txt += "\n====================================\n";
		txt += QString("Summary : %1 warnings").arg(errs.size());

		m_out->clear();
		m_out->setPlainText(txt);
	}

private:
	QPlainTextEdit* m_out = nullptr;
};

CModelViewer::CModelViewer(CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CModelViewer)
{
	ui->setupUi(wnd, this);
	m_currentObject = 0;
}

void CModelViewer::blockUpdate(bool block)
{
	ui->m_blockUpdate = block;
}

// clear the model viewer
void CModelViewer::Clear()
{
	m_currentObject = nullptr;
	ui->tree->clear();
	ui->tree->ClearData();
}

void CModelViewer::Update(bool breset)
{
	if (ui->m_blockUpdate) return;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

//	FSObject* po = m_currentObject;

	// NOTE: Not sure if this is the best place to do this
	// update the model
	FSModel* fem = doc->GetFSModel();
	fem->UpdateLoadControllerReferenceCounts();

	// rebuild the model tree
	ui->setWarningCount(0);
	ui->tree->Build(doc);
	if (ui->m_search->isVisible()) ui->m_search->Build(doc);

	// update the props panel
	ui->props->Update();

	if (doc)
	{
		FSObject* po = doc->GetActiveItem();
		if (po) Select(po);
	}
}

// get the currently selected object
FSObject* CModelViewer::GetCurrentObject()
{
	return m_currentObject;
}

void CModelViewer::UpdateObject(FSObject* po)
{
	if (ui->tree->isVisible())
	{
		ui->tree->UpdateObject(po);

		if (po && (po == m_currentObject))
		{
			QTreeWidgetItem* current = ui->tree->currentItem();
			if (current)
			{
				int n = current->data(0, Qt::UserRole).toInt();
				assert(ui->tree->m_data[n].obj == m_currentObject);
				SetCurrentItem(n);
			}
		}
	}
}

void CModelViewer::Select(FSObject* po)
{
	if (po == nullptr) m_currentObject = nullptr;
	ui->unCheckSearch();
	ui->tree->Select(po);
}

// select a list of objects
void CModelViewer::SelectObjects(const std::vector<FSObject*>& objList)
{
	ui->unCheckSearch();
	ui->tree->Select(objList);
}

void CModelViewer::on_modelTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
	if (current)
	{
		QVariant v = current->data(0, Qt::UserRole);
		SetCurrentItem(v.toInt());
	}
	else 
	{
		m_currentObject = nullptr;
		ui->props->SetObjectProps(0, 0, 0);
	}

	FSObject* po = GetCurrentObject();
	if (current)
	{
		ui->tree->UpdateItem(current);
	}
	emit currentObjectChanged(po);
}

void CModelViewer::on_modelSearch_itemChanged()
{
	FSObject* po = GetCurrentObject();
	emit currentObjectChanged(po);
}

void CModelViewer::on_modelTree_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
	OnOpenJob();
}

void CModelViewer::SetCurrentItem(int item)
{
	if (item >= 0)
	{
		CModelTreeItem& it = ui->tree->m_data[item];
		CPropertyList* props = it.props;
		FSObject* po = it.obj;
		if (it.flag & CModelTree::OBJECT_NOT_EDITABLE)
			ui->props->SetObjectProps(0, 0, 0);
		else
			ui->props->SetObjectProps(po, props, it.flag);
		m_currentObject = po;
	}
	else
	{
		ui->props->SetObjectProps(0, 0, 0);
		m_currentObject = 0;
	}

	CModelDocument* doc = GetMainWindow()->GetModelDocument();
	if (doc) doc->SetActiveItem(m_currentObject);
}

void CModelViewer::SetCurrentItem(CModelTreeItem& it)
{
	CPropertyList* props = it.props;
	FSObject* po = it.obj;
	if (it.flag & CModelTree::OBJECT_NOT_EDITABLE)
		ui->props->SetObjectProps(0, 0, 0);
	else
		ui->props->SetObjectProps(po, props, it.flag);
	m_currentObject = po;
}

void CModelViewer::on_searchButton_toggled(bool b)
{
	if (b) 
	{
		ui->m_search->Build(GetDocument());
		ui->props->SetObjectProps(0, 0, 0);
	}
	ui->showSearchPanel(b);
}

void CModelViewer::on_syncButton_clicked()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	if (pdoc == nullptr) return;

	GModel& mdl = *pdoc->GetGModel();
	FESelection* sel = pdoc->GetCurrentSelection();
	if (sel) 
	{
        int N = sel->Size();
            
        vector<FSObject*> objList;
        GObjectSelection* os = dynamic_cast<GObjectSelection*>(sel);
        if (os)
        {
            for (int i=0; i<N; ++i)
            {
                objList.push_back(os->Object(i));
            }
        }
            
        GPartSelection* gs = dynamic_cast<GPartSelection*>(sel);
        if (gs)
        {
            GPartSelection::Iterator it(gs);
            for (int i=0; i<N; ++i, ++it)
            {
                GPart* pg = it;
                if (pg)
                {
                    objList.push_back(pg);
                }
            }
        }
            
        GFaceSelection* ss = dynamic_cast<GFaceSelection*>(sel);
        if (ss)
        {
            GFaceSelection::Iterator it(ss);
            for (int i=0; i<N; ++i, ++it)
            {
                GFace* pg = it;
                if (pg)
                {
                    objList.push_back(pg);
                }
            }
        }
            
            
        GEdgeSelection* es = dynamic_cast<GEdgeSelection*>(sel);
        if (es)
        {
            GEdgeSelection::Iterator it(es);
            for (int i=0; i<N; ++i, ++it)
            {
                GEdge* pg = it;
                if (pg)
                {
                    objList.push_back(pg);
                }
            }
        }
            
        GNodeSelection* ns = dynamic_cast<GNodeSelection*>(sel);
        if (ns)
        {
            GNodeSelection::Iterator it(ns);
            for (int i=0; i<N; ++i, ++it)
            {
                GNode* pg = it;
                if (pg)
                {
                    objList.push_back(pg);
                }
            }
        }

		GDiscreteSelection* ds = dynamic_cast<GDiscreteSelection*>(sel);
		if (ds)
		{
			int N = mdl.DiscreteObjects();
			for (int i=0; i<N; ++i)
			{
				GDiscreteObject* po = mdl.DiscreteObject(i);
				if (dynamic_cast<GDiscreteElementSet*>(po))
				{
					GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(po);
					int NE = pds->size();
					for (int j=0; j<NE; ++j)
					{
						GDiscreteElement& de = pds->element(j);
						if (de.IsSelected())
						{
							// we can't actually show the individual springs,
							// so we just select the parent.
							objList.push_back(po);
							break;
						}
					}
				}
			} 
		}
            
        if (objList.size() == 1)
        {
            Select(objList[0]);
        }
        else
        {
            SelectObjects(objList);
        }
    }
}

void CModelViewer::on_refreshButton_clicked()
{
	Update(false);
}

bool CModelViewer::IsHighlightSelectionEnabled() const
{
	return ui->highlightButton->isChecked();
}

void CModelViewer::on_highlightButton_toggled(bool b)
{
	emit currentObjectChanged(GetCurrentObject());
}

void CModelViewer::on_selectButton_clicked()
{
	// make sure we have an object
	if (m_currentObject == 0) return;
	FSObject* po = m_currentObject;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());

	CCommand* pcmd = 0;
	if (dynamic_cast<GObject*>(po))
	{
		GObject* pm = dynamic_cast<GObject*>(po);
		if (pm->IsVisible() && !pm->IsSelected()) pcmd = new CCmdSelectObject(pdoc->GetGModel(), pm, false);
	}
	else if (dynamic_cast<FSPairedInterface*>(po))
	{
		FSPairedInterface* pci = dynamic_cast<FSPairedInterface*>(po);
		FEItemListBuilder* ps1 = pci->GetPrimarySurface();
		FEItemListBuilder* ps2 = pci->GetSecondarySurface();
		if (ps1) SelectItemList(ps1);
		if (ps2) SelectItemList(ps2);
	}
	else if (dynamic_cast<IHasItemLists*>(po))
	{
		IHasItemLists* pil = dynamic_cast<IHasItemLists*>(po);
		FEItemListBuilder* pitem = pil->GetItemList(0);
		if (pitem) SelectItemList(pitem);
	}
	else if (dynamic_cast<FEItemListBuilder*>(po))
	{
		FEItemListBuilder* pi = dynamic_cast<FEItemListBuilder*>(po);
		SelectItemList(pi);
	}
	else if (dynamic_cast<GPart*>(po))
	{
		OnSelectPart();
	}
	else if (dynamic_cast<GFace*>(po))
	{
		OnSelectSurface();
	}
	else if (dynamic_cast<GEdge*>(po))
	{
		OnSelectCurve();
	}
	else if (dynamic_cast<GNode*>(po))
	{
		OnSelectNode();
	}
	else if (dynamic_cast<GDiscreteElement*>(po))
	{
		GDiscreteElement* ps = dynamic_cast<GDiscreteElement*>(po);
		ps->Select();
	}
	else if (dynamic_cast<GDiscreteObject*>(po))
	{
		GDiscreteObject* ps = dynamic_cast<GDiscreteObject*>(po);
		GModel& fem = pdoc->GetFSModel()->GetModel();
		int n = fem.FindDiscreteObjectIndex(ps);
		pcmd = new CCmdSelectDiscrete(&fem, &n, 1, false);
	}
	else if (dynamic_cast<GMaterial*>(po))
	{
		GMaterial* mat = dynamic_cast<GMaterial*>(po);
		GModel* mdl = pdoc->GetGModel();
		list<GPart*> partList = mdl->FindPartsFromMaterial(mat->GetID());

		vector<int> partIdList;
		for (GPart* pg : partList) partIdList.push_back(pg->GetID());
		pdoc->SetSelectionMode(SELECT_PART);
		pcmd = new CCmdSelectPart(mdl, partIdList, false);
	}

	if (pcmd) pdoc->DoCommand(pcmd);
	GetMainWindow()->UpdateToolbar();
	GetMainWindow()->Update(this);
}

void CModelViewer::SelectItemList(FEItemListBuilder *pitem, bool badd)
{
	CCommand* pcmd = 0;

	int n = pitem->size();
	if (n == 0) return;

	int* pi = new int[n];
	FEItemListBuilder::Iterator it = pitem->begin();
	for (int i = 0; i<n; ++i, ++it) pi[i] = *it;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* ps = pdoc->GetFSModel();
	GModel* mdl = pdoc->GetGModel();

	switch (pitem->Type())
	{
	case GO_PART: pdoc->SetSelectionMode(SELECT_PART); pcmd = new CCmdSelectPart(mdl, pi, n, badd); break;
	case GO_FACE: pdoc->SetSelectionMode(SELECT_FACE); pcmd = new CCmdSelectSurface(mdl, pi, n, badd); break;
	case GO_EDGE: pdoc->SetSelectionMode(SELECT_EDGE); pcmd = new CCmdSelectEdge(mdl, pi, n, badd); break;
	case GO_NODE: pdoc->SetSelectionMode(SELECT_NODE); pcmd = new CCmdSelectNode(mdl, pi, n, badd); break;
	case FE_ELEMSET:
		{
			pdoc->SetSelectionMode(SELECT_OBJECT);
			pdoc->SetItemMode(ITEM_ELEM);

			FSGroup* pg = dynamic_cast<FSGroup*>(pitem);
			CCmdGroup* pcg = new CCmdGroup("Select Elements"); pcmd = pcg;
			FSMesh* pm = dynamic_cast<FSMesh*>(pg->GetMesh());
			pcg->AddCommand(new CCmdSelectObject(mdl, pg->GetGObject(), badd));
			pcg->AddCommand(new CCmdSelectElements(pm, pi, n, badd));
		}
		break;
	case FE_SURFACE:
		{
			pdoc->SetSelectionMode(SELECT_OBJECT);
			pdoc->SetItemMode(ITEM_FACE);

			FSGroup* pg = dynamic_cast<FSGroup*>(pitem);
			CCmdGroup* pcg = new CCmdGroup("Select Faces"); pcmd = pcg;
			FSMesh* pm = dynamic_cast<FSMesh*>(pg->GetMesh());
			pcg->AddCommand(new CCmdSelectObject(mdl, pg->GetGObject(), badd));
			pcg->AddCommand(new CCmdSelectFaces(pm, pi, n, badd));
		}
		break;
	case FE_NODESET:
		{
			pdoc->SetSelectionMode(SELECT_OBJECT);
			pdoc->SetItemMode(ITEM_NODE);

			FSGroup* pg = dynamic_cast<FSGroup*>(pitem);
			CCmdGroup* pcg = new CCmdGroup("Select Nodes"); pcmd = pcg;
			FSMesh* pm = dynamic_cast<FSMesh*>(pg->GetMesh());
			pcg->AddCommand(new CCmdSelectObject(mdl, pg->GetGObject(), badd));
			pcg->AddCommand(new CCmdSelectFENodes(pm, pi, n, badd));
		}
		break;
	}

	if (pcmd)
	{
		pdoc->DoCommand(pcmd);
		GetMainWindow()->UpdateToolbar();
		GetMainWindow()->RedrawGL();
	}

	delete[] pi;
}

void CModelViewer::AssignCurrentSelection(int ntarget)
{
	ui->props->AssignCurrentSelection(ntarget);
}

void CModelViewer::UpdateSelection()
{
	if (ui->m_search->isVisible())
		ui->m_search->GetSelection(m_selection);
	else
		ui->tree->GetSelection(m_selection);
}

void CModelViewer::Show()
{
	parentWidget()->raise();
}

bool CModelViewer::IsFocus()
{
	return ui->tree->hasFocus();
}

bool CModelViewer::OnDeleteEvent()
{
	on_deleteButton_clicked();
	return true;
}

void CModelViewer::on_deleteButton_clicked()
{
	OnDeleteItem();
}

void CModelViewer::on_warnings_clicked()
{
	QStringList warnings = ui->tree->GetAllWarnings();
	CDlgWarnings dlg(GetMainWindow());
	dlg.SetWarnings(warnings);
	dlg.exec();
}

void CModelViewer::on_props_paramChanged(FSCoreBase* pc, Param* p)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;
	if (p == nullptr) return;

	FSObject* po = pc;
	if (dynamic_cast<FSMaterial*>(po))
	{
		FSMaterial* mat = dynamic_cast<FSMaterial*>(po);
		GMaterial* gm = mat->GetOwner();
		po = gm;
	}

	QString sp;
	if (pc) sp = QString("\"%1.%2\"").arg(QString::fromStdString(po->GetName())).arg(p->GetLongName());
	else sp = QString("\"%1\"").arg(p->GetLongName());

	QString sv;
	switch (p->GetParamType())
	{
	case Param_INT   : sv = QString::number(p->GetIntValue()); break;
	case Param_FLOAT : sv = QString::number(p->GetFloatValue()); break;
	case Param_BOOL  : sv = (p->GetBoolValue()?"Yes":"No"); break;
	case Param_VEC3D : sv = QString::number(p->GetIntValue()); break;
	case Param_STRING: sv = QString("\"%1\"").arg(Vec3dToString(p->GetVec3dValue())); break;
	case Param_MATH  : sv = QString("\"%1\"").arg(QString::fromStdString(p->GetMathString())); break;
	case Param_COLOR : break;
	case Param_MAT3D : sv = Mat3dToString(p->GetMat3dValue()); break;
	case Param_MAT3DS: sv = Mat3dsToString(p->GetMat3dsValue()); break;
	case Param_VEC2I : sv = Vec2iToString(p->GetVec2iValue()); break;
	case Param_STD_VECTOR_INT   : sv = VectorIntToString(p->GetVectorIntValue());  break;
	case Param_STD_VECTOR_DOUBLE: sv = VectorDoubleToString(p->GetVectorDoubleValue());  break;
	case Param_STD_VECTOR_VEC2D: break;
	case Param_ARRAY_INT: break;			// fixed size array of int
	case Param_ARRAY_DOUBLE: break;			// fixed size array of double
	default:
		break;
	}

	QString msg;
	if (sv.isEmpty()) msg = QString("parameter %1 changed.").arg(sp);
	else msg = QString("parameter %1 changed to %2").arg(sp).arg(sv);
	doc->AppendChangeLog(msg);
}

void CModelViewer::on_props_nameChanged(const QString& txt)
{
	QTreeWidgetItem* item = ui->tree->currentItem();
	assert(item);
	if (item) item->setText(0, txt);
}

void CModelViewer::on_props_selectionChanged()
{
	ui->tree->UpdateObject(ui->props->GetCurrentObject());
	GetMainWindow()->RedrawGL();
}

void CModelViewer::on_props_dataChanged(bool b)
{
	if (b)
	{
		Update();
	}
	else
	{
		ui->tree->UpdateObject(ui->props->GetCurrentObject());
		ui->m_search->UpdateObject(ui->props->GetCurrentObject());

		CMainWindow* wnd = GetMainWindow();
		wnd->RedrawGL();
	}
}

void CModelViewer::on_props_modelChanged()
{
	Update();
}

void CModelViewer::on_filter_currentIndexChanged(int n)
{
	FSObject* po = GetCurrentObject();
	ui->tree->SetFilter(n);
	Update(true);
	Select(po);
}

void CModelViewer::OnDeleteItem()
{
	UpdateSelection();

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	doc->DeleteObjects(m_selection);
	Select(nullptr);
	Update();
	GetMainWindow()->RedrawGL();
}

void CModelViewer::OnUnhideAllObjects()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel* m = doc->GetGModel();
	m->ShowAllObjects();
	Update();
	GetMainWindow()->RedrawGL();
}

void CModelViewer::OnCreateNewMeshLayer()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel* gm = doc->GetGModel();
	int layers = gm->MeshLayers();
	QString s = QString("Layer") + QString::number(layers + 1);
	QString newLayer = QInputDialog::getText(this, "New Layer", "Layer name:", QLineEdit::Normal, s);
	if (newLayer.isEmpty() == false)
	{
		string layerName = newLayer.toStdString();
		int n = gm->FindMeshLayer(layerName);
		if (n >= 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed creating layer. Layer name already taken.");
		}
		else
		{
			CCmdGroup* cmd = new CCmdGroup(string("Add mesh layer: ") + layerName);
			cmd->AddCommand(new CCmdAddMeshLayer(gm, layerName));
			cmd->AddCommand(new CCmdSetActiveMeshLayer(gm, layers));
			doc->DoCommand(cmd);
			Update();
			GetMainWindow()->RedrawGL();
		}
	}
}

void CModelViewer::OnDeleteMeshLayer()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel* gm = doc->GetGModel();
	int layers = gm->MeshLayers();
	int activeLayer = gm->GetActiveMeshLayer();
	if ((activeLayer == 0) || (layers == 1))
	{
		QMessageBox::warning(this, "FEBio Studio", "You cannot delete the Default mesh layer.");
		return;
	}
	else
	{
		if (QMessageBox::question(this, "FEBio Studio", "Are you sure you want to delete the current mesh layer?"))
		{
			// to delete the active mesh layer, we must first select a different layer as the active layer.
			// We'll choose the default layer
			string s = gm->GetMeshLayerName(activeLayer);
			CCmdGroup* cmd = new CCmdGroup(string("Delete mesh layer: " + s));
			cmd->AddCommand(new CCmdSetActiveMeshLayer(gm, 0));
			cmd->AddCommand(new CCmdDeleteMeshLayer(gm, activeLayer));
			doc->DoCommand(cmd);
		}
	}
}

void CModelViewer::OnUnhideAllParts()
{
	GObject* po = dynamic_cast<GObject*>(m_currentObject);
	if (po)
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		GModel* m = doc->GetGModel();
		m->ShowAllParts(po);
		Update();
		GetMainWindow()->RedrawGL();
	}
}

void CModelViewer::OnHideInactiveParts()
{
	GObject* po = dynamic_cast<GObject*>(m_currentObject);
	if (po)
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		GModel* m = doc->GetGModel();
		list<GPart*> partList;
		for (int i = 0; i < po->Parts(); ++i)
		{
			GPart* pg = po->Part(i);
			if (!pg->IsActive()) partList.push_back(pg);
		}
		if (!partList.empty())
		{
			m->ShowParts(partList, false);
			Update();
			GetMainWindow()->RedrawGL();
		}
	}
}

void CModelViewer::OnDeleteNamedSelection()
{
	OnDeleteItem();
}

void CModelViewer::OnExportFESurface()
{
	FSSurface* surf = dynamic_cast<FSSurface*>(m_currentObject);
	if (surf == nullptr) return;

	QStringList filters;
	filters << "STL file (*.stl)";

	QString fileName = QFileDialog::getSaveFileName(this, "Save", QString(), QString("STL ascii (*.stl)"));
	{
		std::string filename = fileName.toStdString();
		FSProject dummy;
		STLExport writer(dummy);
		if (!writer.Write(filename.c_str(), surf))
			QMessageBox::critical(this, "FEBio Studio", QString("Couldn't export to STL file:\n%1").arg(QString::fromStdString(writer.GetErrorMessage())));

	}
}

void CModelViewer::OnHideObject()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	for (int i=0; i<m_selection.size(); ++i)
	{
		GObject* po = dynamic_cast<GObject*>(m_selection[i]); assert(po);
		if (po) 
		{
			m.ShowObject(po, false);

			QTreeWidgetItem* item = ui->tree->FindItem(po);
			if (item) item->setForeground(0, Qt::gray);
		}
	}

	CMainWindow* wnd = GetMainWindow();
	wnd->RedrawGL();
}

void CModelViewer::OnShowObject()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	for (int i=0; i<(int)m_selection.size(); ++i)
	{
		GObject* po = dynamic_cast<GObject*>(m_selection[i]); assert(po);
		if (po)
		{
			m.ShowObject(po, true);

			QTreeWidgetItem* item = ui->tree->FindItem(po);
			if (item) item->setForeground(0, Qt::black);
		}
	}
	CMainWindow* wnd = GetMainWindow();
	wnd->RedrawGL();
}

void CModelViewer::OnSelectObject()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	CMainWindow* wnd = GetMainWindow();
	wnd->SetSelectionMode(SELECT_OBJECT);

	vector<GObject*> sel; 
	for (int i = 0; i<m_selection.size(); ++i)
	{
		GObject* po = dynamic_cast<GObject*>(m_selection[i]); assert(po);
		if (po && po->IsVisible()) sel.push_back(po);
	}

	if (sel.empty() == false)
	{
		doc->DoCommand(new CCmdSelectObject(&m, sel, true));
		wnd->RedrawGL();
	}
}

void CModelViewer::OnDeleteAllDiscete()
{
	QString q("Are you sure you want to delete all discrete objects? This cannot be undone.");
	if (QMessageBox::question(this, "Delete All", q) == QMessageBox::Yes)
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument()); assert(doc);
		if (doc == nullptr) return;
		GModel& m = doc->GetFSModel()->GetModel();
		m.ClearDiscrete();

		Select(nullptr);
		Update();
		GetMainWindow()->RedrawGL();
	}
}

void CModelViewer::OnShowAllDiscrete()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument()); assert(doc);
	if (doc == nullptr) return;
	GModel& m = doc->GetFSModel()->GetModel();
	
	for (int i = 0; i < m.DiscreteObjects(); ++i)
	{
		GDiscreteObject* pd = m.DiscreteObject(i);
		if (pd->IsVisible() == false) pd->Show();
	}
	GetMainWindow()->RedrawGL();
}

void CModelViewer::OnHideAllDiscrete()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument()); assert(doc);
	if (doc == nullptr) return;
	GModel& m = doc->GetFSModel()->GetModel();

	for (int i = 0; i < m.DiscreteObjects(); ++i)
	{
		GDiscreteObject* pd = m.DiscreteObject(i);
		if (pd->IsVisible()) pd->Hide();
	}
	GetMainWindow()->RedrawGL();
}

void CModelViewer::OnSelectDiscreteObject()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	CMainWindow* wnd = GetMainWindow();
	wnd->SetSelectionMode(SELECT_DISCRETE);

	vector<GDiscreteObject*> sel;
	for (int i=0; i<(int)m_selection.size(); ++i)
	{
		GDiscreteObject* po = dynamic_cast<GDiscreteObject*>(m_selection[i]); assert(po);
		if (po) sel.push_back(po);
	}

	if (sel.empty() == false)
	{
		doc->DoCommand(new CCmdSelectDiscrete(&m, sel, true));
		wnd->RedrawGL();
	}
}

void CModelViewer::OnDetachDiscreteObject()
{
	GDiscreteElementSet* set = dynamic_cast<GDiscreteElementSet*>(m_currentObject); assert(set);
	if (set == 0) return;

	CMainWindow* wnd = GetMainWindow();
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	GObject* po = m.DetachDiscreteSet(set);
	if (po)
	{
		const std::string& name = "Detached_" + set->GetName();
		po->SetName(name);
		doc->DoCommand(new CCmdAddAndSelectObject(&m, po));
		Update();
		Select(po);
		wnd->RedrawGL();
	}
}

void CModelViewer::OnHideDiscreteObject()
{
	vector<GDiscreteObject*> sel;
	for (int i = 0; i < (int)m_selection.size(); ++i)
	{
		GDiscreteObject* po = dynamic_cast<GDiscreteObject*>(m_selection[i]); assert(po);
		if (po && po->IsVisible()) po->Hide();
	}
	CMainWindow* wnd = GetMainWindow();
	wnd->RedrawGL();
}

void CModelViewer::OnShowDiscreteObject()
{
	vector<GDiscreteObject*> sel;
	for (int i = 0; i < (int)m_selection.size(); ++i)
	{
		GDiscreteObject* po = dynamic_cast<GDiscreteObject*>(m_selection[i]); assert(po);
		if (po && (po->IsVisible() == false)) po->Show();
	}
	CMainWindow* wnd = GetMainWindow();
	wnd->RedrawGL();
}

void CModelViewer::OnChangeDiscreteType()
{
	CMainWindow* wnd = GetMainWindow();
	CModelDocument* doc = wnd->GetModelDocument();
	FSModel* fem = doc->GetFSModel();

	GDiscreteSpringSet* set = dynamic_cast<GDiscreteSpringSet*>(m_currentObject); assert(set);
	if (set == 0) return;

	QStringList items; items << "Linear" << "Nonlinear" << "Hill";
	QString item = QInputDialog::getItem(this, "Discrete Set Type", "Type:", items, 0, false);
	if (item.isEmpty() == false)
	{
		FSDiscreteMaterial* mat = nullptr;
		if (item == "Linear"   ) mat = FEBio::CreateDiscreteMaterial("linear spring", fem);
		if (item == "Nonlinear") mat = FEBio::CreateDiscreteMaterial("nonlinear spring", fem);
		if (item == "Hill"     ) mat = FEBio::CreateDiscreteMaterial("Hill", fem);
		if (mat)
		{
			delete set->GetMaterial();
			set->SetMaterial(mat);

			Update();
			Select(set);
		}
		else
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to assign new material.");
		}
	}
}

void CModelViewer::OnHidePart()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	for (int i=0; i<(int)m_selection.size(); ++i)
	{
		GPart* pg = dynamic_cast<GPart*>(m_selection[i]); assert(pg);
		if (pg) 
		{
			m.ShowPart(pg, false);

			QTreeWidgetItem* item = ui->tree->FindItem(pg);
			if (item) item->setForeground(0, Qt::gray);
		}
	}

	CMainWindow* wnd = GetMainWindow();
	wnd->RedrawGL();
}

void CModelViewer::OnSelectPartElements()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	if (m_selection.size() != 1) return;
	GPart* pg = dynamic_cast<GPart*>(m_selection[0]); assert(pg);

	GObject* po = dynamic_cast<GObject*>(pg->Object());
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	// set the correct selection mode
	doc->SetSelectionMode(SELECT_OBJECT);
	doc->SetItemMode(ITEM_ELEM);

	// make sure this object is selected first
	doc->DoCommand(new CCmdSelectObject(&m, po, false), po->GetName());

	// now, select the elements
	int lid = pg->GetLocalID();
	vector<int> elemList;
	for (int i = 0; i < pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.m_gid == lid) elemList.push_back(i);
	}

	// select elements
	doc->DoCommand(new CCmdSelectElements(pm, elemList, false));

	CMainWindow* wnd = GetMainWindow();
	wnd->UpdateGLControlBar();
	wnd->RedrawGL();
}

void CModelViewer::OnSelectSurfaceFaces()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	if (m_selection.size() != 1) return;
	GFace* pf = dynamic_cast<GFace*>(m_selection[0]); assert(pf);

	GObject* po = dynamic_cast<GObject*>(pf->Object());
	if (po == nullptr) return;

	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr) return;

	// set the correct selection mode
	doc->SetSelectionMode(SELECT_OBJECT);
	doc->SetItemMode(ITEM_FACE);

	// make sure this object is selected first
	doc->DoCommand(new CCmdSelectObject(&m, po, false), po->GetName());

	// now, select the faces
	int lid = pf->GetLocalID();
	vector<int> faceList;
	for (int i = 0; i < pm->Faces(); ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.m_gid == lid) faceList.push_back(i);
	}

	// select elements
	doc->DoCommand(new CCmdSelectFaces(pm, faceList, false));

	CMainWindow* wnd = GetMainWindow();
	wnd->UpdateGLControlBar();
	wnd->RedrawGL();
}

void CModelViewer::OnShowPart()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& m = doc->GetFSModel()->GetModel();

	for (int i = 0; i<(int)m_selection.size(); ++i)
	{
		GPart* pg = dynamic_cast<GPart*>(m_selection[i]); assert(pg);
		if (pg) 
		{
			m.ShowPart(pg);
		}
	}
	Update();
	CMainWindow* wnd = GetMainWindow();
	wnd->RedrawGL();
}

void CModelViewer::OnSelectPart()
{
	CMainWindow* wnd = GetMainWindow();
	wnd->SetSelectionMode(SELECT_PART);

	UpdateSelection();

	vector<int> part;
	for (int i = 0; i<(int)m_selection.size(); ++i)
	{
		GPart* pg = dynamic_cast<GPart*>(m_selection[i]); assert(pg);
		if (pg && pg->IsVisible()) part.push_back(pg->GetID());
	}
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	CCmdSelectPart* cmd = new CCmdSelectPart(doc->GetGModel(), part, false);
	doc->DoCommand(cmd);
	wnd->RedrawGL();
}

void CModelViewer::OnSelectSurface()
{
	CMainWindow* wnd = GetMainWindow();
	wnd->SetSelectionMode(SELECT_FACE);

	UpdateSelection();

	vector<int> surf;
	for (int i = 0; i<(int)m_selection.size(); ++i)
	{
		GFace* pg = dynamic_cast<GFace*>(m_selection[i]); assert(pg);
		if (pg && pg->IsVisible()) surf.push_back(pg->GetID());
	}
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	CCmdSelectSurface* cmd = new CCmdSelectSurface(doc->GetGModel(), surf, false);
	doc->DoCommand(cmd);
	wnd->RedrawGL();
}

void CModelViewer::OnSelectCurve()
{
	CMainWindow* wnd = GetMainWindow();
	wnd->SetSelectionMode(SELECT_EDGE);

	UpdateSelection();

	vector<int> edge;
	for (int i = 0; i<(int)m_selection.size(); ++i)
	{
		GEdge* pg = dynamic_cast<GEdge*>(m_selection[i]); assert(pg);
		if (pg && pg->IsVisible()) edge.push_back(pg->GetID());
	}

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	CCmdSelectEdge* cmd = new CCmdSelectEdge(doc->GetGModel(), edge, false);
	doc->DoCommand(cmd);
	wnd->RedrawGL();
}

void CModelViewer::OnSelectNode()
{
	CMainWindow* wnd = GetMainWindow();
	wnd->SetSelectionMode(SELECT_NODE);

	UpdateSelection();

	vector<int> node;
	for (int i = 0; i<(int)m_selection.size(); ++i)
	{
		GNode* pg = dynamic_cast<GNode*>(m_selection[i]); assert(pg);
		if (pg && pg->IsVisible()) node.push_back(pg->GetID());
	}

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	CCmdSelectNode* cmd = new CCmdSelectNode(doc->GetGModel(), node, false);
	doc->DoCommand(cmd);
	wnd->RedrawGL();
}

void CModelViewer::OnCopyMaterial()
{
	GMaterial* pmat = dynamic_cast<GMaterial*>(m_currentObject); assert(pmat);
	if (pmat == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// create a copy of the material
	FEBioMaterial* pm = dynamic_cast<FEBioMaterial*>(FEBio::CloneModelComponent(pmat->GetMaterialProperties(), fem));
	GMaterial* pmat2 = new GMaterial(pm);

	// add the material to the material deck
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	doc->DoCommand(new CCmdAddMaterial(doc->GetFSModel(), pmat2), pmat2->GetNameAndType());

	// update the model viewer
	Update();
	Select(pmat2);
}

void CModelViewer::OnChangeMaterial()
{
	GMaterial* gmat = dynamic_cast<GMaterial*>(m_currentObject); assert(gmat);
	if (gmat == 0) return;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FSProject& prj = doc->GetProject();
	FSModel& fem = *doc->GetFSModel();

	CDlgAddPhysicsItem dlg("Add Material", FEMATERIAL_ID, -1, &fem, false, false, this);
	if (dlg.exec())
	{
        int id = dlg.GetClassID();
        if(id == -1) return;

		FSMaterial* pmat = FEBio::CreateFEBioClass<FSMaterial>(id, &fem);
		if (pmat)
		{
			FSMaterial* oldMat = gmat->TakeMaterialProperties();
			if (oldMat)
			{
				FSProperty* prop = pmat->FindProperty("elastic");
				if (prop) prop->SetComponent(oldMat);
				else delete oldMat;
			}
			gmat->SetMaterialProperties(pmat);
			Update();
			Select(gmat);
		}
	}
}

void CModelViewer::OnMaterialHideParts()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();
	GModel& mdl = fem->GetModel();
	list<GPart*> partList;
	for (int i = 0; i < m_selection.size(); ++i)
	{
		GMaterial* mat = dynamic_cast<GMaterial*>(m_selection[i]); assert(mat);
		if (mat)
		{
			list<GPart*> partList_i = mdl.FindPartsFromMaterial(mat->GetID());
			if (partList_i.empty() == false)
			{
				partList.insert(partList.end(), partList_i.begin(), partList_i.end());
			}

		}
	}
	if (partList.empty() == false)
	{
		pdoc->DoCommand(new CCmdHideParts(&mdl, partList));
		GetMainWindow()->RedrawGL();
	}
}

void CModelViewer::OnMaterialShowParts()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();
	GModel& mdl = fem->GetModel();
	list<GPart*> partList;
	for (int i = 0; i < m_selection.size(); ++i)
	{
		GMaterial* mat = dynamic_cast<GMaterial*>(m_selection[i]); assert(mat);
		if (mat)
		{
			list<GPart*> partList_i = mdl.FindPartsFromMaterial(mat->GetID());
			if (partList_i.empty() == false)
			{
				partList.insert(partList.end(), partList_i.begin(), partList_i.end());
			}

		}
	}
	if (partList.empty() == false)
	{
		pdoc->DoCommand(new CCmdShowParts(&mdl, partList));
		GetMainWindow()->RedrawGL();
	}
}


void CModelViewer::OnMaterialHideOtherParts()
{
	GMaterial* mat = dynamic_cast<GMaterial*>(m_currentObject); assert(mat);
	if (mat == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();
	GModel& mdl = fem->GetModel();
	list<GPart*> partList = mdl.FindPartsFromMaterial(mat->GetID(), false);

	pdoc->DoCommand(new CCmdHideParts(&mdl, partList));
	GetMainWindow()->RedrawGL();
}

void CModelViewer::OnCopyInterface()
{
	FSInterface* pic = dynamic_cast<FSInterface*>(m_currentObject); assert(pic);
	if (pic == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the interface
	FSInterface* piCopy = dynamic_cast<FSInterface*>(FEBio::CloneModelComponent(pic, fem));
	assert(piCopy);

	// create a name
	string name = defaultInterfaceName(fem, pic);
	piCopy->SetName(name);

	// add the interface to the doc
	FSStep* step = fem->GetStep(pic->GetStep());
	pdoc->DoCommand(new CCmdAddInterface(step, piCopy), piCopy->GetNameAndType());

	// update the model viewer
	Update();
	Select(piCopy);
}

void CModelViewer::OnCopyBC()
{
	FSBoundaryCondition* pbc = dynamic_cast<FSBoundaryCondition*>(m_currentObject); assert(pbc);
	if (pbc == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the bc
	FSBoundaryCondition* pbcCopy = dynamic_cast<FSBoundaryCondition*>(FEBio::CloneModelComponent(pbc, fem));
	assert(pbcCopy);

	// create a name
	string name = defaultBCName(fem, pbc);
	pbcCopy->SetName(name);

	// add the bc to the doc
	FSStep* step = fem->GetStep(pbc->GetStep());
	pdoc->DoCommand(new CCmdAddBC(step, pbcCopy), pbcCopy->GetNameAndType());

	// update the model viewer
	Update();
	Select(pbcCopy);
}

void CModelViewer::OnCopyIC()
{
	FSInitialCondition* pic = dynamic_cast<FSInitialCondition*>(m_currentObject); assert(pic);
	if (pic == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the ic
	FSInitialCondition* picCopy = dynamic_cast<FSInitialCondition*>(FEBio::CloneModelComponent(pic, fem));
	assert(picCopy);

	// create a name
	string name = defaultICName(fem, pic);
	picCopy->SetName(name);

	// add the ic to the doc
	FSStep* step = fem->GetStep(pic->GetStep());
	pdoc->DoCommand(new CCmdAddIC(step, picCopy), picCopy->GetNameAndType());

	// update the model viewer
	Update();
	Select(picCopy);
}

void CModelViewer::OnCopyRigidConnector()
{
	FSRigidConnector* pc = dynamic_cast<FSRigidConnector*>(m_currentObject); assert(pc);
	if (pc == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the load
	FSRigidConnector* pcCopy =  dynamic_cast<FSRigidConnector*>(FEBio::CloneModelComponent(pc, fem));
	assert(pcCopy);

	// create a name
	string name = defaultRigidConnectorName(fem, pc);
	pcCopy->SetName(name);

	// add the load to the doc
	FSStep* step = fem->GetStep(pc->GetStep());
	pdoc->DoCommand(new CCmdAddRigidConnector(step, pcCopy), pcCopy->GetNameAndType());

	// update the model viewer
	Update();
	Select(pcCopy);
}

void CModelViewer::OnCopyConstraint()
{
	FSModelConstraint* pc = dynamic_cast<FSModelConstraint*>(m_currentObject); assert(pc);

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the load

	FSModelConstraint* pcCopy = dynamic_cast<FSModelConstraint*>(FEBio::CloneModelComponent(pc, fem));
	assert(pcCopy);

	// create a name
	string name = defaultConstraintName(fem, pc);
	pcCopy->SetName(name);

	// add the constraint to the doc
	FSStep* step = fem->GetStep(pc->GetStep());
	pdoc->DoCommand(new CCmdAddConstraint(step, pcCopy), pcCopy->GetNameAndType());

	// update the model viewer
	Update();
	Select(pcCopy);
}

void CModelViewer::OnCopyLoad()
{
	FSLoad* pl = dynamic_cast<FSLoad*>(m_currentObject); assert(pl);
	if (pl == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the load
	FSLoad* plCopy = dynamic_cast<FSLoad*>(FEBio::CloneModelComponent(pl, fem));
	assert(plCopy);

	// create a name
	string name = defaultLoadName(fem, pl);
	plCopy->SetName(name);

	// add the load to the doc
	FSStep* step = fem->GetStep(pl->GetStep());
	pdoc->DoCommand(new CCmdAddLoad(step, plCopy), plCopy->GetNameAndType());

	// update the model viewer
	Update();
	Select(plCopy);
}

void CModelViewer::OnCopyRigidBC()
{
	FSRigidBC* pc = dynamic_cast<FSRigidBC*>(m_currentObject); assert(pc);
	if (pc == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the load
	FSRigidBC* pcCopy = dynamic_cast<FSRigidBC*>(FEBio::CloneModelComponent(pc, fem));
	assert(pcCopy);

	// create a name
	string name = defaultRigidBCName(fem, pc);
	pcCopy->SetName(name);

	// add the load to the doc
	FSStep* step = fem->GetStep(pc->GetStep());
//	pdoc->DoCommand(new CCmdAddRC(step, pcCopy));
	step->AddRigidBC(pcCopy);

	// update the model viewer
	Update();
	Select(pcCopy);
}

void CModelViewer::OnCopyRigidIC()
{
	FSRigidIC* pc = dynamic_cast<FSRigidIC*>(m_currentObject); assert(pc);
	if (pc == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the load
	FSRigidIC* pcCopy = dynamic_cast<FSRigidIC*>(FEBio::CloneModelComponent(pc, fem));
	assert(pcCopy);

	// create a name
	string name = defaultRigidICName(fem, pc);
	pcCopy->SetName(name);

	// add the load to the doc
	FSStep* step = fem->GetStep(pc->GetStep());
	//	pdoc->DoCommand(new CCmdAddRC(step, pcCopy));
	step->AddRigidIC(pcCopy);

	// update the model viewer
	Update();
	Select(pcCopy);
}

void CModelViewer::OnCopyRigidLoad()
{
	FSRigidLoad* pc = dynamic_cast<FSRigidLoad*>(m_currentObject); assert(pc);
	if (pc == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	// copy the load
	FSRigidLoad* pcCopy = dynamic_cast<FSRigidLoad*>(FEBio::CloneModelComponent(pc, fem));
	assert(pcCopy);

	// create a name
	string name = defaultRigidLoadName(fem, pc);
	pcCopy->SetName(name);

	// add the load to the doc
	FSStep* step = fem->GetStep(pc->GetStep());
	step->AddRigidLoad(pcCopy);

	// update the model viewer
	Update();
	Select(pcCopy);
}

void CModelViewer::OnCopyStep()
{
	FSStep* ps = dynamic_cast<FSStep*>(m_currentObject); assert(ps);
	if (ps == 0) return;

	if (dynamic_cast<FSInitialStep*>(ps)) return;

	// copy the step
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	FEBioAnalysisStep* psCopy = dynamic_cast<FEBioAnalysisStep*>(FEBio::CloneModelComponent(ps, fem)); assert(psCopy);
	if (psCopy == nullptr)
	{
		QMessageBox::critical(this, "Copy Step", "Failed to copy step.");
		return;
	}

	// create a name
	string name = defaultStepName(fem, ps);
	psCopy->SetName(name);

	// add the step to the doc
	pdoc->DoCommand(new CCmdAddStep(fem, psCopy));

	// update the model viewer
	Update();
	Select(psCopy);
}

void CModelViewer::OnStepMoveUp()
{
	FSAnalysisStep* ps = dynamic_cast<FSAnalysisStep*>(m_currentObject); assert(ps);
	if (ps == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	int n = fem->GetStepIndex(ps); assert(n >= 1);
	if (n > 1)
	{
		FSStep* step0 = ps;
		FSStep* step1 = fem->GetStep(n - 1);

		string msg = step0->GetName() + string(" <--> ") + step1->GetName();
		pdoc->DoCommand(new CCmdSwapSteps(fem, step0, step1), msg);
		Update();
		Select(ps);
	}
}

void CModelViewer::OnStepMoveDown()
{
	FSAnalysisStep* ps = dynamic_cast<FSAnalysisStep*>(m_currentObject); assert(ps);
	if (ps == 0) return;

	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel* fem = pdoc->GetFSModel();

	int n = fem->GetStepIndex(ps); assert(n >= 1);
	if (n < fem->Steps() - 1)
	{
		FSStep* step0 = ps;
		FSStep* step1 = fem->GetStep(n + 1);

		string msg = step0->GetName() + string(" <--> ") + step1->GetName();
		pdoc->DoCommand(new CCmdSwapSteps(fem, step0, step1), msg);
		Update();
		Select(ps);
	}
}

void CModelViewer::OnRerunJob()
{
	CFEBioJob* job = dynamic_cast<CFEBioJob*>(m_currentObject); assert(job);
	if (job == 0) return;

	CMainWindow* wnd = GetMainWindow();
	wnd->RunFEBioJob(job);
}

void CModelViewer::OnOpenJob()
{
	FSObject* po = GetCurrentObject();
	if (po == nullptr) return;

	CFEBioJob* job = dynamic_cast<CFEBioJob*>(po);
	if (job == nullptr) return;

	CDocument* doc = job->GetDocument();
	assert(doc);
	QString plotFile = doc->ToAbsolutePath(job->GetPlotFileName());

	GetMainWindow()->OpenPostFile(plotFile, dynamic_cast<CModelDocument*>(doc), false);
}

void CModelViewer::OnEditOutput()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSProject& prj = pdoc->GetProject();

	CDlgEditOutput dlg(prj, this);
	dlg.exec();	
	UpdateCurrentItem();
}

void CModelViewer::OnEditOutputLog()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSProject& prj = pdoc->GetProject();

	CDlgEditOutput dlg(prj, this, 1);
	dlg.exec();
	UpdateCurrentItem();
}

void CModelViewer::UpdateCurrentItem()
{
	CModelTreeItem* item = ui->tree->GetCurrentData();
	if (item)
	{
		CPropertyList* prop = item->props;
		if (prop) prop->Update();
		SetCurrentItem(*item);
	}
}

void CModelViewer::SetFilter(int index)
{
    ui->m_filter->setCurrentIndex(index);
}

void CModelViewer::IncWarningCount()
{
	ui->m_errs->increase();
}

void CModelViewer::OnRemoveEmptySelections()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& mdl = pdoc->GetFSModel()->GetModel();
	mdl.RemoveEmptySelections();
	Update();
}

void CModelViewer::OnRemoveUnusedSelections()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& mdl = pdoc->GetFSModel()->GetModel();
	mdl.RemoveUnusedSelections();
	Update();
}

void CModelViewer::OnRemoveUnusedLoadControllers()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	FSModel& fem = *pdoc->GetFSModel();
	fem.RemoveUnusedLoadControllers();
	Update();
}

void CModelViewer::OnRemoveAllSelections()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel& mdl = pdoc->GetFSModel()->GetModel();
	mdl.RemoveNamedSelections();
	Update();
}

void CModelViewer::OnDeleteAllMeshAdaptors()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc)
	{
		FSModel& fem = *doc->GetFSModel();
		fem.DeleteAllMeshAdaptors();
		Update();
	}
}

// clear current FSObject selection
void CModelViewer::ClearSelection()
{
	m_selection.clear();
}

// set the current FSObject selection
void CModelViewer::SetSelection(std::vector<FSObject*>& sel)
{
	m_selection = sel;
}

void CModelViewer::SetSelection(FSObject* sel)
{
	m_selection.clear();
	m_selection.push_back(sel);
}

// show the context menu
void CModelViewer::ShowContextMenu(CModelTreeItem* data, QPoint pt)
{
	if (data == 0) return;

	QMenu menu(this);

	// add delete action
	bool del = false;

	CMainWindow* wnd = GetMainWindow();

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel* gm = doc->GetGModel();

	switch (data->type)
	{
	case MT_OBJECT_LIST:
	{
		menu.addAction("Show All Objects", this, SLOT(OnUnhideAllObjects()));
		menu.addAction("Part Viewer ...", GetMainWindow(), SLOT(onShowPartViewer()));
		menu.addSeparator();

		QMenu* sub = new QMenu("Set Active Mesh Layer");
		int layers = gm->MeshLayers();
		int activeLayer = gm->GetActiveMeshLayer();
		for (int i = 0; i < layers; ++i)
		{
			string s = gm->GetMeshLayerName(i);
			QAction* a = sub->addAction(QString::fromStdString(s));
			a->setCheckable(true);
			if (i == activeLayer) a->setChecked(true);
		}

		QObject::connect(sub, SIGNAL(triggered(QAction*)), GetMainWindow(), SLOT(OnSelectMeshLayer(QAction*)));

		menu.addAction(sub->menuAction());
		menu.addAction("New Mesh Layer ...", this, SLOT(OnCreateNewMeshLayer()));

		if (layers > 1)
		{
			menu.addAction("Delete Active Mesh Layer", this, SLOT(OnDeleteMeshLayer()));
		}
	}
	break;
	case MT_PART_LIST:
		menu.addAction("Show All Parts", this, SLOT(OnUnhideAllParts()));
		menu.addAction("Hide Inactive", this, SLOT(OnHideInactiveParts()));
		break;
	case MT_PART_GROUP:
	case MT_FACE_GROUP:
	case MT_EDGE_GROUP:
	case MT_NODE_GROUP:
		menu.addAction("Delete", this, SLOT(OnDeleteNamedSelection()));
		break;
	case MT_FEPART_GROUP:
	case MT_FEELEM_GROUP:
	case MT_FEEDGE_GROUP:
	case MT_FENODE_GROUP:
		menu.addAction("Delete", this, SLOT(OnDeleteNamedSelection()));
		break;
	case MT_FEFACE_GROUP:
		menu.addAction("Export ...", this, SLOT(OnExportFESurface()));
		menu.addAction("Delete", this, SLOT(OnDeleteNamedSelection()));
		break;
	case MT_MATERIAL_LIST:
	{
		menu.addAction("Add Material ...", wnd, SLOT(on_actionAddMaterial_triggered()));
//		menu.addAction("Export Materials ...", this, SLOT(OnExportAllMaterials()));

		QMenu* sub = new QMenu("Import Materials");
		QAction* ac = sub->addAction("From FEBio file ...");
		ac->setData(-1);

		CDocManager* docMng = wnd->GetDocManager();
		for (int i = 0; i < docMng->Documents(); ++i)
		{
			CModelDocument* doci = dynamic_cast<CModelDocument*>(docMng->GetDocument(i));
			if (doci && (doc != doci))
			{
				QAction* ac = sub->addAction(QString::fromStdString(doci->GetDocTitle()));
				ac->setData(i);
			}
		}
		QObject::connect(sub, SIGNAL(triggered(QAction*)), this, SLOT(OnImportMaterials(QAction*)));

		menu.addAction(sub->menuAction());
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllMaterials()));
	}
	break;
	case MT_BC_LIST:
		menu.addAction("Add Nodal BC ...", wnd, SLOT(on_actionAddNodalBC_triggered()));
		menu.addAction("Add Surface BC ...", wnd, SLOT(on_actionAddSurfaceBC_triggered()));
		menu.addAction("Add Linear Constraint ...", wnd, SLOT(on_actionAddGeneralBC_triggered()));
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllBC()));
		break;
	case MT_LOAD_LIST:
		menu.addAction("Add Nodal Load ..."  , wnd, SLOT(on_actionAddNodalLoad_triggered()));
		menu.addAction("Add Surface Load ...", wnd, SLOT(on_actionAddSurfLoad_triggered()));
		menu.addAction("Add Body Load ..."   , wnd, SLOT(on_actionAddBodyLoad_triggered()));
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllLoads()));
		break;
	case MT_IC_LIST:
		menu.addAction("Add Initial Condition ...", wnd, SLOT(on_actionAddIC_triggered()));
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllIC()));
		break;
	case MT_CONTACT_LIST:
		menu.addAction("Add Contact Interface ...", wnd, SLOT(on_actionAddContact_triggered()));
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllContact()));
		break;
	case MT_CONSTRAINT_LIST:
		menu.addAction("Add Surface Constraint ...", wnd, SLOT(on_actionAddSurfaceNLC_triggered()));
		menu.addAction("Add Body Constraint ..."   , wnd, SLOT(on_actionAddBodyNLC_triggered()));
		menu.addAction("Add General Constraint ...", wnd, SLOT(on_actionAddGenericNLC_triggered()));
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllConstraints()));
		break;
	case MT_RIGID_COMPONENT_LIST:
		menu.addAction("Add Rigid Constraint ...", wnd, SLOT(on_actionAddRigidBC_triggered()));
		menu.addAction("Add Rigid Initial Condition ...", wnd, SLOT(on_actionAddRigidIC_triggered()));
		menu.addAction("Add Rigid Connector ...", wnd, SLOT(on_actionAddRigidConnector_triggered()));
		menu.addAction("Add Rigid Load ...", wnd, SLOT(on_actionAddRigidLoad_triggered()));
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllRigidComponents()));
		break;
	case MT_STEP_LIST:
		menu.addAction("Add Analysis Step ...", wnd, SLOT(on_actionAddStep_triggered()));
		menu.addAction("Step Viewer ...", wnd, SLOT(on_actionStepViewer_triggered()));
		menu.addSeparator();
		menu.addAction("Delete All", this, SLOT(OnDeleteAllSteps()));
		break;
	case MT_PROJECT_OUTPUT:
	case MT_PROJECT_OUTPUT_PLT:
		menu.addAction("Edit output...", this, SLOT(OnEditOutput()));
		break;
	case MT_PROJECT_OUTPUT_LOG:
		menu.addAction("Edit output...", this, SLOT(OnEditOutputLog()));
		break;
	case MT_NAMED_SELECTION:
		menu.addAction("Remove empty", this, SLOT(OnRemoveEmptySelections()));
		menu.addAction("Remove unused", this, SLOT(OnRemoveUnusedSelections()));
		menu.addAction("Remove all", this, SLOT(OnRemoveAllSelections()));
		break;
	case MT_MESH_ADAPTOR_LIST:
		menu.addAction("Add Mesh Adaptor ...", wnd, SLOT(on_actionAddMeshAdaptor_triggered()));
		menu.addAction("Delete All", this, SLOT(OnDeleteAllMeshAdaptors()));
		break;
	case MT_OBJECT:
	{
		GObject* po = dynamic_cast<GObject*>(data->obj);
		if (po)
		{
			if (po->IsVisible())
			{
				menu.addAction("Select", this, SLOT(OnSelectObject()));
				menu.addAction("Hide", this, SLOT(OnHideObject()));
			}
			else
				menu.addAction("Show", this, SLOT(OnShowObject()));

			del = true;
		}
	}
	break;
	case MT_PART:
	{
		GPart* pg = dynamic_cast<GPart*>(data->obj);
		if (pg)
		{
			if (pg->IsVisible())
			{
				menu.addAction("Select", this, SLOT(OnSelectPart()));
				menu.addAction("Hide", this, SLOT(OnHidePart()));
				menu.addAction("Select elements", this, SLOT(OnSelectPartElements()));
			}
			else
				menu.addAction("Show", this, SLOT(OnShowPart()));

			// only parts of a GMeshObject can be deleted
			if (dynamic_cast<GMeshObject*>(pg->Object()))
				del = true;
		}
	}
	break;
	case MT_SURFACE:
		menu.addAction("Select", this, SLOT(OnSelectSurface()));
		menu.addAction("Select Faces", this, SLOT(OnSelectSurfaceFaces()));
		break;
	case MT_EDGE:
		menu.addAction("Select", this, SLOT(OnSelectCurve()));
		break;
	case MT_NODE:
		menu.addAction("Select", this, SLOT(OnSelectNode()));
		break;
	case MT_MATERIAL:
		menu.addAction("Copy", this, SLOT(OnCopyMaterial()));
		menu.addAction("Change...", this, SLOT(OnChangeMaterial()));
		menu.addAction("Hide parts", this, SLOT(OnMaterialHideParts()));
		menu.addAction("Show parts", this, SLOT(OnMaterialShowParts()));
		menu.addAction("Hide other parts", this, SLOT(OnMaterialHideOtherParts()));
		menu.addAction("Export Material(s) ...", this, SLOT(OnExportMaterials()));
		del = true;
		break;
	case MT_DISCRETE_LIST:
		menu.addAction("Delete all", this, SLOT(OnDeleteAllDiscete()));
		menu.addAction("Hide all", this, SLOT(OnHideAllDiscrete()));
		menu.addAction("Show all", this, SLOT(OnShowAllDiscrete()));
		break;
	case MT_DISCRETE_SET:
		menu.addAction("Select", this, SLOT(OnSelectDiscreteObject()));
		menu.addAction("Hide"  , this, SLOT(OnHideDiscreteObject()));
		menu.addAction("Show"  , this, SLOT(OnShowDiscreteObject()));
		menu.addAction("Detach", this, SLOT(OnDetachDiscreteObject()));
		menu.addAction("Change Type ...", this, SLOT(OnChangeDiscreteType()));
		del = true;
		break;
	case MT_DISCRETE:
		menu.addAction("Select", this, SLOT(OnSelectDiscreteObject()));
		del = true;
		break;
	case MT_CONTACT:
	{
		menu.addAction("Copy", this, SLOT(OnCopyInterface()));
		FSPairedInterface* pci = dynamic_cast<FSPairedInterface*>(data->obj);
		if (pci)
		{
			menu.addAction("Replace ...", this, SLOT(OnReplaceContactInterface()));
			menu.addAction("Swap Primary/Secondary", this, SLOT(OnSwapContactSurfaces()));
		}

		del = true;
	}
	break;
	case MT_BC:
		menu.addAction("Copy", this, SLOT(OnCopyBC()));
		del = true;
		break;
	case MT_RIGID_CONNECTOR:
		menu.addAction("Copy", this, SLOT(OnCopyRigidConnector()));
		del = true;
		break;
	case MT_IC:
		menu.addAction("Copy", this, SLOT(OnCopyIC()));
		del = true;
		break;
	case MT_LOAD:
		menu.addAction("Copy", this, SLOT(OnCopyLoad()));
		del = true;
		break;
	case MT_RIGID_BC:
		menu.addAction("Copy", this, SLOT(OnCopyRigidBC()));
		del = true;
		break;
	case MT_RIGID_IC:
		menu.addAction("Copy", this, SLOT(OnCopyRigidIC()));
		del = true;
		break;
	case MT_RIGID_LOAD:
		menu.addAction("Copy", this, SLOT(OnCopyRigidLoad()));
		del = true;
		break;
	case MT_CONSTRAINT:
		menu.addAction("Copy", this, SLOT(OnCopyConstraint()));
		del = true;
		break;
	case MT_STEP:
		{
			menu.addAction("Copy", this, SLOT(OnCopyStep()));
			FSAnalysisStep* step = dynamic_cast<FSAnalysisStep*>(data->obj);
			if (step)
			{
				menu.addAction("Move Up", this, SLOT(OnStepMoveUp()));
				menu.addAction("Move Down", this, SLOT(OnStepMoveDown()));
			}
			del = true;
		}
		break;
	case MT_LOAD_CONTROLLERS:
		menu.addAction("Add Load Controller ...", wnd, SLOT(on_actionAddLoadController_triggered()));
		menu.addAction("Remove unused", this, SLOT(OnRemoveUnusedLoadControllers()));
		menu.addAction("Delete All", wnd, SLOT(OnDeleteAllLoadControllers()));
		break;
	case MT_LOAD_CONTROLLER:
		del = true;
		break;
	case MT_MESH_DATA_LIST:
		menu.addAction("Add mesh data map ..."   , wnd, SLOT(on_actionAddMeshDataMap_triggered()));
		menu.addAction("Add mesh data generator ..."   , wnd, SLOT(on_actionAddMeshDataGenerator_triggered()));
		menu.addAction("Delete All", wnd, SLOT(OnDeleteAllMeshData()));
		break;
	case MT_MESH_DATA:
		menu.addAction("Edit ...", this, SLOT(OnEditMeshData()));
		del = true;
		break;
	case MT_JOBLIST:
		{
			menu.addAction("Delete All", this, SLOT(OnDeleteAllJobs()));
		}
		break;
	case MT_JOB:
		menu.addAction("Open", this, SLOT(OnOpenJob()));
		menu.addAction("Rerun job", this, SLOT(OnRerunJob()));
		del = true;
		break;
	case MT_3DIMAGE:
        {
			CImageModel* img = dynamic_cast<CImageModel*>(data->obj);
			if (img && (img->Get3DImage() == nullptr))
			{
				menu.addAction("Find image ...", this, &CModelViewer::OnFindImage);
			}
            QMenu* exportImage = menu.addMenu("Export Image");
            exportImage->addAction("Raw", this, &CModelViewer::OnExportRawImage);
#ifdef HAS_ITK
            exportImage->addAction("TIFF", this, &CModelViewer::OnExportTIFF);
            exportImage->addAction("NRRD", this, &CModelViewer::OnExportNRRD);
#endif
            del = true;
        }
		break;
	default:
		return;
	}

	if (del) 
	{
		menu.addSeparator();
		menu.addAction("Delete", this, SLOT(OnDeleteItem()));
	}

	menu.exec(pt);
}

void CModelViewer::OnExportMaterials()
{
	vector<GMaterial*> matList;

	if (m_selection.size() == 0)
	{
		GMaterial* m = dynamic_cast<GMaterial*>(m_currentObject);
		if (m) matList.push_back(m);
	}
	else
	{
		for (int i=0; i<(int)m_selection.size(); ++i)
		{
			FSObject* po = m_selection[i];
			GMaterial* m = dynamic_cast<GMaterial*>(po);
			if (m) matList.push_back(m);
		}
	}

	if (matList.size() > 0)
		GetMainWindow()->onExportMaterials(matList);
}

void CModelViewer::OnExportAllMaterials()
{
	GetMainWindow()->onExportAllMaterials();
}

void CModelViewer::OnImportMaterials(QAction* action)
{
	CMainWindow* wnd = GetMainWindow();
	int n = action->data().toInt();
	if (n == -1)
		wnd->onImportMaterials();
	else
	{
		CDocManager* docMng = wnd->GetDocManager();
		CModelDocument* doc = dynamic_cast<CModelDocument*>(docMng->GetDocument(n)); assert(doc);
		if (doc) wnd->onImportMaterialsFromModel(doc);
	}
}

void CModelViewer::OnDeleteAllMaterials()
{
	GetMainWindow()->DeleteAllMaterials();
}

void CModelViewer::OnSwapContactSurfaces()
{
	FSPairedInterface* pci = dynamic_cast<FSPairedInterface*>(m_currentObject);
	if (pci)
	{
		pci->SwapPrimarySecondary();
		UpdateObject(m_currentObject);
	}
}

void CModelViewer::OnReplaceContactInterface()
{
	FSPairedInterface* pci = dynamic_cast<FSPairedInterface*>(m_currentObject);
	if (pci)
	{
		GetMainWindow()->OnReplaceContactInterface(pci);
	}
}


void CModelViewer::OnDeleteAllBC()
{
	GetMainWindow()->DeleteAllBC();
}

void CModelViewer::OnDeleteAllLoads()
{
	GetMainWindow()->DeleteAllLoads();
}

void CModelViewer::OnDeleteAllIC()
{
	GetMainWindow()->DeleteAllIC();
}

void CModelViewer::OnDeleteAllContact()
{
	GetMainWindow()->DeleteAllContact();
}

void CModelViewer::OnDeleteAllConstraints()
{
	GetMainWindow()->DeleteAllConstraints();
}

void CModelViewer::OnDeleteAllRigidComponents()
{
	GetMainWindow()->DeleteAllRigidBCs();
	GetMainWindow()->DeleteAllRigidICs();
	GetMainWindow()->DeleteAllRigidLoads();
	GetMainWindow()->DeleteAllRigidConnectors();
}

void CModelViewer::OnDeleteAllSteps()
{
	GetMainWindow()->DeleteAllSteps();
}

void CModelViewer::OnDeleteAllJobs()
{
	GetMainWindow()->DeleteAllJobs();
}

void CModelViewer::OnEditMeshData()
{
	FEMeshData* data = dynamic_cast<FEMeshData*>(m_currentObject);
	if (data == nullptr) return;

	CDlgEditMeshData dlg(data, this);
	dlg.exec();
}

void CModelViewer::OnFindImage()
{
	CImageModel* img = dynamic_cast<CImageModel*>(m_currentObject);
	if (img == nullptr) return;
	if (img->Get3DImage()) return;

	CRawImageSource* src = dynamic_cast<CRawImageSource*>(img->GetImageSource());
	if (src)
	{
		QString filename = QFileDialog::getOpenFileName(GetMainWindow(), "Load Image", "", "Raw image (*.raw)");
		if (filename.isEmpty() == false)
		{
			src->SetFileName(filename.toStdString());
			img->Reload();
			Update();
		}
	}
	else
	{
		QMessageBox::information(this, "Find image", "Finding image is currently only supported for RAW images.");
	}
}

void CModelViewer::OnExportRawImage()
{
    CImageModel* img = dynamic_cast<CImageModel*>(m_currentObject);
	if (img == nullptr) return;

	QString filename = QFileDialog::getSaveFileName(GetMainWindow(), "Export Raw Image", "", "Raw (*.raw)");
	if (filename.isEmpty() == false)
	{
        if (img->ExportRAWImage(filename.toStdString()))
        {
            QString msg = QString("Image exported successfully to file\n%1").arg(filename);
            QMessageBox::information(GetMainWindow(), "Export image", msg);
        }
        else
        {
            QString msg = QString("Failed exporting image to file\n%1").arg(filename);
            QMessageBox::critical(GetMainWindow(), "Export image", msg);
        }
	}	
}

void CModelViewer::OnExportTIFF()
{
    CImageModel* img = dynamic_cast<CImageModel*>(m_currentObject);
	if (img == nullptr) return;

	QString filename = QFileDialog::getSaveFileName(GetMainWindow(), "Export TIFF", "", "TIFF (*.tiff)");
	if (filename.isEmpty() == false)
	{
        // QFileDialog does not enforce extensions on Linux, and so this check is necessary.
        QFileInfo info(filename);
        QString suffix = info.suffix();
        if(suffix != "tiff")
        {
            if(suffix.isEmpty())
            {
                filename.append(".tiff");
            }
            else
            {
                filename.replace(suffix, "tiff");
            }
        }

        if(QFileInfo::exists(filename))
        {
            auto ans = QMessageBox::question(GetMainWindow(), "File Exists", "%1 already exists.\n\nWould you like to overwrite it?");

            if(ans != QMessageBox::Yes) return;
        }

        if (img->ExportSITKImage(filename.toStdString()))
        {
            QString msg = QString("Image exported successfully to file\n%1").arg(filename);
            QMessageBox::information(GetMainWindow(), "Export image", msg);
        }
        else
        {
            QString msg = QString("Failed exporting image to file\n%1").arg(filename);
            QMessageBox::critical(GetMainWindow(), "Export image", msg);
        }
    }
}

void CModelViewer::OnExportNRRD()
{
    CImageModel* img = dynamic_cast<CImageModel*>(m_currentObject);
	if (img == nullptr) return;

	QString filename = QFileDialog::getSaveFileName(GetMainWindow(), "Export NRRD", "", "NRRD (*.nrrd)");
	if (filename.isEmpty() == false)
	{
        // QFileDialog does not enforce extensions on Linux, and so this check is necessary.
        QFileInfo info(filename);
        QString suffix = info.suffix();
        if(suffix != "nrrd")
        {
            if(suffix.isEmpty())
            {
                filename.append(".nrrd");
            }
            else
            {
                filename.replace(suffix, "nrrd");
            }
        }

        if(QFileInfo::exists(filename))
        {
            auto ans = QMessageBox::question(GetMainWindow(), "File Exists", "%1 already exists.\n\nWould you like to overwrite it?");

            if(ans != QMessageBox::Yes) return;
        }

        if (img->ExportSITKImage(filename.toStdString()))
        {
            QString msg = QString("Image exported successfully to file\n%1").arg(filename);
            QMessageBox::information(GetMainWindow(), "Export image", msg);
        }
        else
        {
            QString msg = QString("Failed exporting image to file\n%1").arg(filename);
            QMessageBox::critical(GetMainWindow(), "Export image", msg);
        }
    }
}