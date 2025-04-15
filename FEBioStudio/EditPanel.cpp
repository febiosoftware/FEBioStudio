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
#include <MeshTools/FEFixJaggedEdges.h>
#include <MeshTools/FEExtrudeEdges.h>
#include <MeshTools/FEFixSurfaceMesh.h>
#include <MeshTools/FECVDDecimationModifier.h>
#include <MeshTools/FEEdgeFlip.h>
#include <MeshTools/FERefineSurface.h>
#include <MeshTools/FEWeldModifier.h>
#include <MeshTools/FEMMGRemesh.h>
#include <MeshTools/FEFillHole.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMultiBox.h>
#include <GeomLib/GMultiPatch.h>
#include <GeomLib/GOCCObject.h>
#include <QMessageBox>
#include "Commands.h"

//=======================================================================================
SurfaceModifierThread::SurfaceModifierThread(FESurfaceModifier* mod, GSurfaceMeshObject* po, FSGroup* pg)
{
	m_mod = mod;
	m_pg = pg;
	m_po = po;
	m_newMesh = nullptr;
}

void SurfaceModifierThread::run()
{
	if ((m_po == nullptr) || (m_mod == nullptr))
	{
		emit resultReady(false);
		return;
	}

	// get the surface mesh
	FSSurfaceMesh* mesh = m_po->GetSurfaceMesh();
	if (mesh == nullptr)
	{
		emit resultReady(false);
		return;
	}

	// create a new mesh
	m_newMesh = nullptr;
	try {
		m_newMesh = m_mod->Apply(mesh, m_pg);
		SetErrorString(QString::fromStdString(m_mod->GetErrorString()));
	}
	catch (...)
	{
		SetErrorString("Exception detected.");
	}

	emit resultReady(m_newMesh != nullptr);
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
REGISTER_CLASS(FESurfaceAutoPartition     , CLASS_SURFACE_MODIFIER, "Auto partition"  , 0xFF);
REGISTER_CLASS(FESurfacePartitionSelection, CLASS_SURFACE_MODIFIER, "Partition"       , 0xFF);
REGISTER_CLASS(FESmoothSurfaceMesh        , CLASS_SURFACE_MODIFIER, "Smooth"          , 0xFF);
REGISTER_CLASS(FEEdgeCollapse             , CLASS_SURFACE_MODIFIER, "Edge Collapse"   , 0xFF);
REGISTER_CLASS(FEFixSurfaceMesh           , CLASS_SURFACE_MODIFIER, "Fix Mesh"        , 0xFF);
REGISTER_CLASS(FECVDDecimationModifier    , CLASS_SURFACE_MODIFIER, "Decimate"        , 0xFF);
REGISTER_CLASS(FEEdgeFlip                 , CLASS_SURFACE_MODIFIER, "Flip edges"      , 0xFF);
REGISTER_CLASS(FERefineSurface            , CLASS_SURFACE_MODIFIER, "Refine"          , 0xFF);
REGISTER_CLASS(FECurveIntersect           , CLASS_SURFACE_MODIFIER, "Project Curve"   , 0xFF);
REGISTER_CLASS(FEWeldSurfaceNodes         , CLASS_SURFACE_MODIFIER, "Weld Nodes"      , 0xFF);
REGISTER_CLASS(MMGSurfaceRemesh           , CLASS_SURFACE_MODIFIER, "MMG Remesh"      , 0xFF);
REGISTER_CLASS(FEFixJaggedEdges           , CLASS_SURFACE_MODIFIER, "Fix Jagged Edges", 0xFF);
REGISTER_CLASS(FEExtrudeEdges             , CLASS_SURFACE_MODIFIER, "Extrude Edges"   , 0xFF);
REGISTER_CLASS(FEFillHole                 , CLASS_SURFACE_MODIFIER, "Fill Holes"      , 0xFF);

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
	if (breset) ui->m_currentObject = nullptr;

	if (activeObject && (activeObject == ui->m_currentObject))
	{
		vec3d r = activeObject->GetTransform().GetPosition();
		ui->pos->SetObjectPosition(r);
		ui->editParams->updateData();
		return;
	}

	ui->m_currentObject = activeObject;

	ui->obj->Update();

	if (activeObject == nullptr)
	{
		ui->showPositionPanel(false);
		ui->showObjectParametersPanel(false);
		ui->showButtonsPanel(false);
		ui->showModifierParametersPanel(false);
	}
	else
	{
		ui->showPositionPanel(true);

		vec3d r = activeObject->GetTransform().GetPosition();
		ui->pos->SetObjectPosition(r);

		if (activeObject->Parameters() > 0)
		{
			ui->editParams->SetPropertyList(new CObjectProps(activeObject));
			ui->showObjectParametersPanel(true);
			ui->showModifierParametersPanel(false);
		}
		else
		{
			ui->editParams->SetPropertyList(nullptr);
			ui->showObjectParametersPanel(false);
			ui->showModifierParametersPanel(false);
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
	GObject* activeObject = ui->m_currentObject;
	if (activeObject == nullptr) return;

	// check for a FE mesh
	FSMesh* pm = activeObject->GetFEMesh();
	if (pm)
	{
		if (QMessageBox::question(this, "Apply Changes", "This object has a mesh. This mesh has to be discarded before the changes can be applied.\nDo you wish to discard the mesh?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
	}

	CCmdGroup* cmd = new CCmdGroup("Change mesh");
	cmd->AddCommand(new CCmdChangeFEMesh(activeObject, nullptr));
	cmd->AddCommand(new CCmdChangeObjectParams(activeObject));
	GetDocument()->DoCommand(cmd);

	// clear any highlights
	GLHighlighter::ClearHighlights();
}

void CEditPanel::on_modParams_apply()
{
	// get the active object
	GObject* activeObject = ui->m_currentObject;
	if (activeObject == nullptr) return;

	// check for a FE mesh
	FSMesh* pm = activeObject->GetFEMesh();
	if (pm)
	{
		if (QMessageBox::question(this, "Apply Changes", "This object has a mesh. This mesh has to be discarded before the changes can be applied.\nDo you wish to discard the mesh?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
		{
			return;
		}
	}

	// make sure we have a modifier
	SurfaceModifierTool* modTool = dynamic_cast<SurfaceModifierTool*>(ui->m_activeTool);
	if (modTool == nullptr) return;

	FESurfaceModifier* mod = modTool->GetModifier();
	if (mod == nullptr) return;

	GSurfaceMeshObject* surfaceObject = dynamic_cast<GSurfaceMeshObject*>(activeObject);
	if (surfaceObject == nullptr) return;

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	FESelection* sel = doc->GetCurrentSelection();
	FSItemListBuilder* list = sel->CreateItemList();
	FSGroup* g = 0;
	if (sel->Size() > 0)
	{
		g = dynamic_cast<FSGroup*>(list);
		if (g == 0) { delete list; list = 0; }
	}

	SurfaceModifierThread* thread = new SurfaceModifierThread(mod, surfaceObject, g);
	CDlgStartThread dlg(GetMainWindow(), thread);
	dlg.setTask(QString::fromStdString(mod->GetName()));
	if (dlg.exec())
	{
		bool bsuccess = dlg.GetReturnCode();
		if (bsuccess == false)
		{
			std::string err;
			if (mod) err = mod->GetErrorString();
			if (err.empty()) err = "(unknown)";
			QString errStr = QString::fromStdString(err);
			QMessageBox::critical(this, "Apply modifier", "Cannot apply this modifier to this selection.\nERROR: " + errStr);
		}
		else
		{
			std::string log = mod->GetErrorString();
			if (log.empty() == false)
			{
				GetMainWindow()->AddLogEntry(QString::fromStdString(log) + "\n");
			}

			// if the object has an FE mesh, we need to delete it
			CCmdGroup* cmdg = new CCmdGroup("Apply surface modifier");
			cmdg->AddCommand(new CCmdChangeFEMesh(surfaceObject, nullptr));
			cmdg->AddCommand(new CCmdChangeFESurfaceMesh(surfaceObject, thread->newMesh()));
			doc->DoCommand(cmdg);
		}
		GetMainWindow()->RedrawGL();
		GetMainWindow()->UpdateModel(activeObject, true);
	}
	thread->deleteLater();

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
	else if (convertOption == CObjectPanel::CONVERT_TO_MULTIBLOCK)
	{
		GPrimitive* primitive = dynamic_cast<GPrimitive*>(po);
		if (primitive == nullptr) QMessageBox::information(this, "Convert", "Cannot convert this to a multiblock object.");

		GMultiBox* newObject = new GMultiBox(primitive);
		pdoc->DoCommand(new CCmdSwapObjects(pdoc->GetGModel(), po, newObject));
	}
	else if (convertOption == CObjectPanel::CONVERT_TO_MULTIPATCH)
	{
		GShellPrimitive* primitive = dynamic_cast<GShellPrimitive*>(po);
		if (primitive == nullptr) QMessageBox::information(this, "Convert", "Cannot convert this to a multi-patch object.");

		GMultiPatch* newObject = new GMultiPatch(primitive);
		pdoc->DoCommand(new CCmdSwapObjects(pdoc->GetGModel(), po, newObject));
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

	vec3d r = ui->pos->objectPosition();
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

void CEditPanel::on_buttons_idClicked(int id)
{
	ui->activateTool(id);
}

void CEditPanel::on_form_dataChanged(bool itemModified, int index)
{
	CPropertyList* pl = ui->editParams->GetPropertyList();
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
