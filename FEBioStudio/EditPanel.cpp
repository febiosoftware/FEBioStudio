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
#include "ui_editpanel.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "ObjectProps.h"
#include "GLHighlighter.h"
#include "CurveIntersectProps.h"
#include <MeshTools/FECurveIntersect.h>
#include <MeshTools/FESmoothSurfaceMesh.h>
#include <MeshTools/FEEdgeCollapse.h>
#include <MeshTools/FEFixSurfaceMesh.h>
#include <MeshTools/FECVDDecimationModifier.h>
#include <MeshTools/FEEdgeFlip.h>
#include <MeshTools/FERefineSurface.h>
#include <MeshTools/FEWeldModifier.h>
#include <MeshTools/FEMMGRemesh.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GOCCObject.h>
#include <QMessageBox>
#include "Commands.h"

//=======================================================================================
SurfaceModifierThread::SurfaceModifierThread(CModelDocument* doc, FESurfaceModifier* mod, GSurfaceMeshObject* po, FSGroup* pg)
{
	m_doc = doc;
	m_mod = mod;
	m_pg = pg;
	m_po = po;
}

void SurfaceModifierThread::run()
{
	bool bsuccess = m_doc->ApplyFESurfaceModifier(*m_mod, m_po, m_pg);
	emit resultReady(bsuccess);
}

bool SurfaceModifierThread::hasProgress()
{
	return (m_mod ? m_mod->GetProgress().valid : false);
}

double SurfaceModifierThread::progress()
{
	return (m_mod ? m_mod->GetProgress().percent : 0.0);
}

const char* SurfaceModifierThread::currentTask()
{
	return (m_mod ? m_mod->GetName().c_str() : "");
}

void SurfaceModifierThread::stop()
{

}

//=======================================================================================
REGISTER_CLASS(FESurfaceAutoPartition     , CLASS_SURFACE_MODIFIER, "Auto partition", 0xFF);
REGISTER_CLASS(FESurfacePartitionSelection, CLASS_SURFACE_MODIFIER, "Partition"    , 0xFF);
REGISTER_CLASS(FESmoothSurfaceMesh        , CLASS_SURFACE_MODIFIER, "Smooth"       , 0xFF);
REGISTER_CLASS(FEEdgeCollapse             , CLASS_SURFACE_MODIFIER, "Edge Collapse", 0xFF);
REGISTER_CLASS(FEFixSurfaceMesh           , CLASS_SURFACE_MODIFIER, "Fix Mesh"     , 0xFF);
REGISTER_CLASS(FECVDDecimationModifier    , CLASS_SURFACE_MODIFIER, "Decimate"     , 0xFF);
REGISTER_CLASS(FEEdgeFlip                 , CLASS_SURFACE_MODIFIER, "Flip edges"   , 0xFF);
REGISTER_CLASS(FERefineSurface            , CLASS_SURFACE_MODIFIER, "Refine"       , 0xFF);
REGISTER_CLASS(FECurveIntersect           , CLASS_SURFACE_MODIFIER, "Project Curve", 0xFF);
REGISTER_CLASS(FEWeldSurfaceNodes         , CLASS_SURFACE_MODIFIER, "Weld Nodes"   , 0xFF);
REGISTER_CLASS(MMGSurfaceRemesh         , CLASS_SURFACE_MODIFIER, "MMG Remesh"   , 0xFF);

class CPartitionProps : public CDataPropertyList
{
public:
	CPartitionProps(FESurfacePartitionSelection* mod) : m_mod(mod)
	{
		m_createNew = true;
		m_partition = 0;

		addBoolProperty(&m_createNew, "Create New");
		addIntProperty(&m_partition, "Partition");

		if (m_createNew) mod->assignToPartition(-1);
	}

	void SetPropertyValue(int i, const QVariant& value)
	{
		switch (i)
		{
		case 0: m_createNew = value.toBool(); break;
		case 1: m_partition = value.toInt(); break;
		}

		if (m_createNew) m_mod->assignToPartition(-1);
		else m_mod->assignToPartition(m_partition);
	}

	QVariant GetPropertyValue(int i)
	{
		switch (i)
		{
		case 0: return m_createNew; break;
		case 1: return m_partition; break;
		}

		return QVariant();
	}


public:
	bool	m_createNew;
	int		m_partition;
	FESurfacePartitionSelection*	m_mod;
};

CEditPanel::CEditPanel(CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CEditPanel)
{
	ui->setupUi(this, wnd);
}

void CEditPanel::Update(bool breset)
{
	CGLDocument* doc = GetDocument();
	if (doc == nullptr) return;

	// make sure the active object has changed
	GObject* activeObject = doc->GetActiveObject();
	if (breset) ui->m_currenObject = nullptr;

	if (activeObject && (activeObject == ui->m_currenObject))
	{
		// only update position
		vec3d r = activeObject->GetTransform().GetPosition();
		ui->SetObjectPosition(r);
		return;
	}

	ui->m_currenObject = activeObject;

	ui->obj->Update();

	if (activeObject == 0)
	{
		ui->showParametersPanel(false);
		ui->showPositionPanel(false);
		ui->showButtonsPanel(false);
	}
	else
	{
		ui->showPositionPanel(true);

		vec3d r = activeObject->GetTransform().GetPosition();
		ui->SetObjectPosition(r);

		if (activeObject->Parameters() > 0)
		{
			ui->setPropertyList(new CObjectProps(activeObject));
			ui->showParametersPanel(true);
		}
		else
		{
			ui->setPropertyList(0);
			ui->showParametersPanel(false);
		}

		GSurfaceMeshObject* surfaceMesh = dynamic_cast<GSurfaceMeshObject*>(activeObject);
		if (surfaceMesh)
		{
			ui->showButtonsPanel(true);
		}
		else 
		{
			ui->showButtonsPanel(false);
		}
	}
}

void CEditPanel::Apply()
{
	on_apply_clicked(true);
}

void CEditPanel::on_apply_clicked(bool b)
{
	// get the acrive object
	GObject* activeObject = ui->m_currenObject;
	if (activeObject == 0) return;

	// check for a FE mesh
	FSMesh* pm = activeObject->GetFEMesh();
	if (pm)
	{
		if (QMessageBox::question(this, "Apply Changes", "This object has a mesh. This mesh has to be discarded before the changes can be applied.\nDo you wish to discard the mesh?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
	}

	GSurfaceMeshObject* surfaceObject = dynamic_cast<GSurfaceMeshObject*>(activeObject);
	if (surfaceObject)
	{
		if (ui->m_mod)
		{
			CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
			FESelection* sel = doc->GetCurrentSelection();
			FEItemListBuilder* list = sel->CreateItemList();
			FSGroup* g = 0;
			if (sel->Size() > 0)
			{
				g = dynamic_cast<FSGroup*>(list);
				if (g == 0) { delete list; list = 0; }
			}

			SurfaceModifierThread* thread = new SurfaceModifierThread(doc, ui->m_mod, surfaceObject, g);
			CDlgStartThread dlg(this, thread);
			dlg.setTask(QString::fromStdString(ui->m_mod->GetName()));
			if (dlg.exec())
			{
				bool bsuccess = dlg.GetReturnCode();
				if (bsuccess == false)
				{
					std::string err;
					if (ui->m_mod) err = ui->m_mod->GetErrorString();
					if (err.empty()) err = "(unknown)";
					QString errStr = QString::fromStdString(err);
					QMessageBox::critical(this, "Apply modifier", "Cannot apply this modifier to this selection.\nERROR: " + errStr);
				}
				else
				{
					std::string log = ui->m_mod->GetErrorString();
					if (log.empty() == false)
					{
						GetMainWindow()->AddLogEntry(QString::fromStdString(log) + "\n");
					}
				}
				GetMainWindow()->RedrawGL();
				GetMainWindow()->UpdateModel(activeObject, true);
			}

			// don't forget to cleanup
			if (g) delete g;
		}
	}
	else
	{
		CCmdGroup* cmd = new CCmdGroup("Generate geometry");
		cmd->AddCommand(new CCmdChangeFEMesh(activeObject, nullptr));
		cmd->AddCommand(new CCmdChangeObjectParams(activeObject));
		GetDocument()->DoCommand(cmd, activeObject->GetName());
	}

	// clear any highlights
	GLHighlighter::ClearHighlights();
}

void CEditPanel::on_menu_triggered(QAction* pa)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	GObject* po = pdoc->GetActiveObject();

	int convertOption = pa->data().toInt();

	if (convertOption == CObjectPanel::CONVERT_TO_EDITABLE_SURFACE)
	{
		if (dynamic_cast<GSurfaceMeshObject*>(po) == nullptr)
		{
			GSurfaceMeshObject* pnew = ConvertToEditableSurface(po);
			if (pnew)
			{
				pdoc->DoCommand(new CCmdSwapObjects(pdoc->GetGModel(), po, pnew));
			}
			else
			{
				QMessageBox::critical(this, "Convert", "Unable to convert to editable surface.");
				return;
			}
		}
	}
	else if (convertOption == CObjectPanel::CONVERT_TO_EDITABLE_MESH)
	{
		// convert to editable mesh
		if (dynamic_cast<GMeshObject*>(po) == 0)
		{
			GMeshObject* pnew = ConvertToEditableMesh(po);
			if (pnew)
			{
				pdoc->DoCommand(new CCmdSwapObjects(pdoc->GetGModel(), po, pnew));
			}
			else
			{
				QMessageBox::critical(this, "Convert", "Unable to convert to editable mesh.");
				return;
			}
		}
	}
	else
	{
		QMessageBox::critical(this, "FEBio Studio", "Don't know how to convert object.");
	}

	Update();
	GetMainWindow()->Update(this, true);
}

void CEditPanel::updateObjectPosition()
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	GObject* po = pdoc->GetActiveObject();
	if (po == nullptr) return;

	vec3d r = ui->objectPosition();
	Transform t = po->GetTransform();
	t.SetPosition(r);

	pdoc->DoCommand(new CCmdTransformObject(po, t), po->GetName());
	
	GetMainWindow()->RedrawGL();
}

void CEditPanel::on_posX_editingFinished()
{
	updateObjectPosition();
}

void CEditPanel::on_posY_editingFinished()
{
	updateObjectPosition();
}

void CEditPanel::on_posZ_editingFinished()
{
	updateObjectPosition();
}

void CEditPanel::on_buttons_buttonSelected(int id)
{
	ui->m_nid = id;
	if (id == -1) ui->showParametersPanel(false);
	else
	{
		if (ui->m_mod) delete ui->m_mod;
		ui->m_mod = 0;

		ui->form->setPropertyList(0);

		ClassDescriptor* pcd = ui->buttons->GetClassDescriptor(id);
		if (pcd)
		{
			ui->m_mod = static_cast<FESurfaceModifier*>(pcd->Create()); assert(ui->m_mod);

			CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

			GModel* geo = &doc->GetFSModel()->GetModel();

			CPropertyList* pl = 0;
			if (dynamic_cast<FECurveIntersect*>(ui->m_mod)) pl = new CCurveIntersectProps(geo, dynamic_cast<FECurveIntersect*>(ui->m_mod));
			else if (dynamic_cast<FESurfacePartitionSelection*>(ui->m_mod)) pl = new CPartitionProps(dynamic_cast<FESurfacePartitionSelection*>(ui->m_mod));
			else pl = new CObjectProps(ui->m_mod);

			ui->setPropertyList(pl);
		}

		ui->showParametersPanel(true);
	}
}

void CEditPanel::on_form_dataChanged(bool itemModified, int index)
{
	CPropertyList* pl = ui->form->getPropertyList();
	if (pl == nullptr) return;
	if ((index >= 0) && (index < pl->Properties()))
	{
		CProperty& p = pl->Property(index);
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		if (doc)
		{
			GObject* poa = doc->GetActiveObject(); assert(poa);
			if (poa == nullptr) return;

			QVariant v = pl->GetPropertyValue(index);
			QString msg = QString("Object parameter %1 changed to %2 (%3)").arg(p.name).arg(v.toString()).arg(QString::fromStdString(poa->GetName()));
			doc->AppendChangeLog(msg);
		}
	}
}
