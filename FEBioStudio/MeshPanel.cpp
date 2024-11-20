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
#include <MeshTools/FEModifier.h>
#include <MeshTools/FEDiscardMesh.h>
#include <MeshTools/FETetGenModifier.h>
#include <MeshTools/FEMMGRemesh.h>
#include <MeshTools/FEWeldModifier.h>
#include <MeshTools/FEAutoPartition.h>
#include <MeshTools/FEBoundaryLayerMesher.h>
#include <MeshTools/FEAxesCurvature.h>
#include <MeshTools/FEExtrudeFaces.h>
#include <MeshTools/FECreateShells.h>
#include <MeshTools/FERevolveFaces.h>
#include <MeshTools/FEMesher.h>
#include <MeshTools/FEMultiBlockMesh.h>
#include <MeshTools/FESelection.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GMultiBox.h>
#include <GeomLib/GMultiPatch.h>
#include "ui_meshpanel.h"
#include "ObjectProps.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "GLHighlighter.h"
#include <GeomLib/GPrimitive.h>
#include <QMessageBox>
#include <MeshTools/FEShellMesher.h>
#include <MeshTools/FETetGenMesher.h>
#include <MeshTools/FEFixMesh.h>
#include "Commands.h"
#include "Tool.h"
#include <GLLib/GDecoration.h>
#include "CommandWindow.h"

class CSurfaceMesherProps : public CObjectProps
{
public:
	CSurfaceMesherProps(GSurfaceMeshObject* po) : CObjectProps(nullptr), m_po(po)
	{
		BuildParameterList();
	}

	void BuildParameterList()
	{
		Clear();
		addProperty("Meshing Method", CProperty::Enum)->setEnumValues(QStringList() << "TetGen" << "Shell Mesh");
		addProperty("Properties", CProperty::Group);
		BuildParamList(m_po->GetFEMesher());
	}

	QVariant GetPropertyValue(int i)
	{
		FEMesher* mesher = m_po->GetFEMesher();

		if (i == 0)
		{
			if (dynamic_cast<FEShellMesher*>(mesher)) return 1; else return 0;
		}
		else if (i > 1) return CObjectProps::GetPropertyValue(i - 2);
		else return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		FEMesher* mesher = m_po->GetFEMesher();
		if (i == 0)
		{
			int val = v.toInt();
			if ((val == 0) && (dynamic_cast<FETetGenMesher*>(mesher) == nullptr))
			{
				m_po->SetFEMesher(new FETetGenMesher(m_po));
				BuildParameterList();
				SetModified(true);
			}
			else if (dynamic_cast<FEShellMesher*>(mesher) == nullptr)
			{
				m_po->SetFEMesher(new FEShellMesher(m_po));
				BuildParameterList();
				SetModified(true);
			}
		}
		else if (i > 1) CObjectProps::SetPropertyValue(i - 2, v);
	}

private:
	GSurfaceMeshObject*	m_po;
};


//=======================================================================================
MeshingThread::MeshingThread(GObject* po)
{
	m_po = po;
	m_mesher = nullptr;
}

void MeshingThread::run()
{
	m_mesher = m_po->GetFEMesher();
	if (m_mesher) m_mesher->SetErrorMessage("");
	FSMesh* mesh = m_po->BuildMesh();
	if (m_mesher && (mesh == nullptr)) SetErrorString(QString::fromStdString(m_mesher->GetErrorMessage()));
	emit resultReady(mesh != nullptr);
}

bool MeshingThread::hasProgress()
{
	return (m_mesher ? m_mesher->GetProgress().valid : false);
}

double MeshingThread::progress()
{
	return (m_mesher ? m_mesher->GetProgress().percent : 0.0);
}

const char* MeshingThread::currentTask()
{
	return (m_mesher ? m_mesher->GetProgress().task : "");
}

void MeshingThread::stop()
{
	if (m_mesher) m_mesher->Terminate();
}

//=======================================================================================
ModifierThread::ModifierThread(CModelDocument* doc, FEModifier* mod, GObject* po, FESelection* sel)
{
	m_doc = doc;
	m_mod = mod;
	m_sel = sel;
	m_po = po;
	m_newMesh = nullptr;
}

void ModifierThread::run()
{
	bool bsuccess = false;

	// get the mesh
	FSMesh* pm = m_po->GetFEMesh();
	if (pm && m_mod)
	{
		// apply modifier and create new mesh
		m_newMesh = nullptr;
		try {
			if (m_sel && (m_sel->Type() != SELECT_OBJECTS))
				m_newMesh = m_mod->Apply(m_po, m_sel);
			else
				m_newMesh = m_mod->Apply(pm);
		}
		catch (...)
		{
			m_mod->SetError("Exception detected.");
		}

		SetErrorString(QString::fromStdString(m_mod->GetErrorString()));
		if ((m_newMesh == nullptr) && !m_mod->AllowNullMesh()) bsuccess = false;
		else bsuccess = true;
	}
	emit resultReady(bsuccess);
}

bool ModifierThread::hasProgress()
{
	return (m_mod ? m_mod->GetProgress().valid : false);
}

double ModifierThread::progress()
{
	return (m_mod ? m_mod->GetProgress().percent : 0.0);
}

const char* ModifierThread::currentTask()
{
	return (m_mod ? m_mod->GetName().c_str() : "");
}

void ModifierThread::stop()
{
	
}

//=============================================================================
// NOTE: Try to keep these in alphabetical order!
REGISTER_CLASS(FEAddNode              , CLASS_FEMODIFIER, "Add Node"       , EDIT_MESH);
REGISTER_CLASS(FEAddTriangle          , CLASS_FEMODIFIER, "Add Triangle"   , EDIT_MESH);
REGISTER_CLASS(FEAlignNodes           , CLASS_FEMODIFIER, "Align"          , EDIT_NODE);
REGISTER_CLASS(FEAutoPartition        , CLASS_FEMODIFIER, "Auto Partition" , EDIT_MESH);
REGISTER_CLASS(FEBoundaryLayerMesher  , CLASS_FEMODIFIER, "Boundary Layer" , EDIT_FACE | EDIT_SAFE);
REGISTER_CLASS(FEConvertMesh		  , CLASS_FEMODIFIER, "Convert Mesh"   , EDIT_MESH | EDIT_SAFE);
REGISTER_CLASS(FECreateShells         , CLASS_FEMODIFIER, "Create Shells from Faces"  , EDIT_FACE | EDIT_MESH);
REGISTER_CLASS(FEDetachElements	      , CLASS_FEMODIFIER, "Detach Elements", EDIT_ELEMENT);
REGISTER_CLASS(FEDiscardMesh          , CLASS_FEMODIFIER, "Discard Mesh"   , EDIT_MESH | EDIT_SAFE);
REGISTER_CLASS(FEExtrudeFaces         , CLASS_FEMODIFIER, "Extrude Faces"  , EDIT_FACE);
REGISTER_CLASS(FEFixMesh              , CLASS_FEMODIFIER, "Fix Mesh"       , EDIT_MESH);
REGISTER_CLASS(FEInflateMesh          , CLASS_FEMODIFIER, "Inflate"        , EDIT_FACE);
REGISTER_CLASS(FEInvertMesh           , CLASS_FEMODIFIER, "Invert"         , EDIT_MESH | EDIT_ELEMENT | EDIT_SAFE);
REGISTER_CLASS(FEMirrorMesh           , CLASS_FEMODIFIER, "Mirror"         , EDIT_MESH);
#ifdef HAS_MMG
REGISTER_CLASS(MMGRemesh            , CLASS_FEMODIFIER, "MMG Remesh"     , EDIT_MESH | EDIT_SAFE);
#endif
REGISTER_CLASS(FEPartitionSelection   , CLASS_FEMODIFIER, "Partition"      , EDIT_ELEMENT | EDIT_FACE | EDIT_EDGE | EDIT_NODE);
REGISTER_CLASS(FERebuildMesh          , CLASS_FEMODIFIER, "Rebuild Mesh"   , EDIT_MESH);
REGISTER_CLASS(RefineMesh			  , CLASS_FEMODIFIER, "Refine Mesh"    , EDIT_MESH | EDIT_SAFE);
REGISTER_CLASS(FERevolveFaces         , CLASS_FEMODIFIER, "Revolve Faces"  , EDIT_FACE);
REGISTER_CLASS(FERezoneMesh           , CLASS_FEMODIFIER, "Rezone"         , EDIT_FACE | EDIT_SAFE);
REGISTER_CLASS(FESetAxesOrientation   , CLASS_FEMODIFIER, "Set Axes"       , EDIT_MESH | EDIT_ELEMENT | EDIT_SAFE);
REGISTER_CLASS(FEAxesCurvature        , CLASS_FEMODIFIER, "Set Axes from Curvature" , EDIT_MESH | EDIT_ELEMENT | EDIT_FACE | EDIT_SAFE);
REGISTER_CLASS(FESetFiberOrientation  , CLASS_FEMODIFIER, "Set Fibers"     , EDIT_MESH | EDIT_ELEMENT | EDIT_SAFE);
REGISTER_CLASS(FESetShellThickness    , CLASS_FEMODIFIER, "Shell Thickness", EDIT_MESH | EDIT_ELEMENT);
REGISTER_CLASS(FESmoothMesh           , CLASS_FEMODIFIER, "Smooth"         , EDIT_MESH);
REGISTER_CLASS(FETetGenModifier       , CLASS_FEMODIFIER, "TetGen"         , EDIT_MESH);
REGISTER_CLASS(FEWeldNodes            , CLASS_FEMODIFIER, "Weld nodes"     , EDIT_MESH);
REGISTER_CLASS(FESetMBWeight          , CLASS_FEMODIFIER, "Set MB Weight"  , EDIT_MESH | EDIT_SAFE);

CMeshPanel::CMeshPanel(CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CMeshPanel)
{
	ui->setupUi(this, wnd);
}

void CMeshPanel::Update(bool breset)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	GModel* gm = doc->GetGModel();
	GObject* activeObject = doc->GetActiveObject();

	// make sure this object is made the active object
	GObject::SetActiveObject(activeObject);

	// only update if reset is true or the active object changed
	if ((breset == false) && (activeObject == ui->m_currentObject)) return;

	// keep track of the object
	ui->m_currentObject = activeObject;

	ui->obj->Update();

	// start by hiding everything
	ui->hideAllPanels();

	// if there is no active object, we're done
	if (activeObject == nullptr) return;

	if (dynamic_cast<GMeshObject*>(activeObject))
	{
		GMeshObject*  meshObject = dynamic_cast<GMeshObject*>(activeObject);

		// show the modifiers for editable meshes
		// only for the default mesh layer, otherwise
		// we show only the non-editable mesh modifiers
		if (gm->GetActiveMeshLayer() == 0)
			ui->showButtonsPanel(true);
		else
			ui->showButtonsPanel2(true);
	}
	else
	{
		GSurfaceMeshObject* surfaceMeshObject = dynamic_cast<GSurfaceMeshObject*>(activeObject);
		if (surfaceMeshObject)
		{
			ui->setMesherPropertyList(new CSurfaceMesherProps(surfaceMeshObject));
			ui->showMesherParametersPanel(true);
		}
		else
		{
			FEMesher* mesher = activeObject->GetFEMesher();
			if (mesher)
			{
				ui->setMesherPropertyList(new CObjectProps(mesher));
				ui->showMesherParametersPanel(true);
			}
		}

		FSMesh* mesh = activeObject->GetFEMesh();
		if (mesh)
		{
			// show modifiers for non-editable meshes
			ui->showButtonsPanel2(true);
		}
	}
}

void CMeshPanel::on_buttons2_idClicked(int id)
{
	ui->activateTool(id);
}

void CMeshPanel::on_buttons_idClicked(int id)
{
	ui->activateTool(id);
}

void CMeshPanel::Apply()
{
	on_apply_clicked(true);
}

bool CMeshPanel::OnPickEvent(const FESelection& sel)
{
	if (ui->m_activeTool)
	{
		bool b = ui->m_activeTool->onPickEvent(sel);
		if (b) ui->m_activeTool->updateUi();
		return b;
	}
	else return false;
}

bool CMeshPanel::OnUndo()
{
	if (ui->m_activeTool)
	{
		bool b = ui->m_activeTool->onUndoEvent();
		if (b) ui->m_activeTool->updateUi();
		return b;
	}
	else return false;
}

void CMeshPanel::on_apply_clicked(bool b)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == 0) return;

	FEMesher* mesher = activeObject->GetFEMesher();
	if (mesher == 0) return;

	// check if the current mesh has any dependencies
	if (activeObject->GetFEMesh())
	{
		GObject* o = activeObject;
		if (o->CanDeleteMesh() == false)
		{
			QString msg("This mesh has dependencies in the model. Modifying it could invalidate the model and cause problems.\nDo you wish to continue?");
			if (QMessageBox::warning(this, "FEBio Studio", msg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}
		}
	}

	MeshingThread* thread = new MeshingThread(activeObject);
	CDlgStartThread dlg(this, thread);
	if (dlg.exec())
	{
		// see if the meshing was successful
		FSMesh* mesh = activeObject->GetFEMesh();
		if (mesh == nullptr)
		{
			QString errMsg = QString::fromStdString(mesher->GetErrorMessage());
			QString error = QString("Meshing Failed:\n") + errMsg;
			QMessageBox::critical(this, "Meshing", error);
		}
		else doc->AppendChangeLog(QString("Object \"%1\" meshed").arg(QString::fromStdString(activeObject->GetName())));

		Update();
		CMainWindow* w = GetMainWindow();
		w->UpdateModel(activeObject, false);
		w->Update();
		w->RedrawGL();

		// clear any highlights
		GLHighlighter::ClearHighlights();

		if (mesh)
		{
			// create a report: 
			QString report = QString("Meshing finished for %1:\n").arg(QString::fromStdString(activeObject->GetName()));
			report += QString("- Nodes    = %1\n").arg(mesh->Nodes());
			report += QString("- Faces    = %1\n").arg(mesh->Faces());
			report += QString("- Elements = %1\n").arg(mesh->Elements());
			QMessageBox::information(this, "FEBio Studio", report);
			w->AddLogEntry(report);
			CCommandLogger::Log("genmesh");
		}
	}
}

void CMeshPanel::on_modParams_apply()
{
	CMainWindow* w = GetMainWindow();

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == 0) return;

	// make sure we have a modifier
	ModifierTool* modTool = dynamic_cast<ModifierTool*>(ui->m_activeTool);
	if (modTool == nullptr) return;

	FEModifier* mod = modTool->GetModifier();
	if (mod == nullptr) return;

	// check if the current mesh has any dependencies
	if (activeObject->GetFEMesh())
	{
		GObject* o = activeObject;
		if (o->CanDeleteMesh() == false)
		{
			QString msg("This mesh has dependencies in the model. Modifying it could invalidate the model and cause problems.\nDo you wish to continue?");
			if (QMessageBox::warning(this, "FEBio Studio", msg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}
		}
	}

	FESelection* sel = doc->GetCurrentSelection();
	FEItemListBuilder* list = (sel ? sel->CreateItemList() : 0);
	FSGroup* g = dynamic_cast<FSGroup*>(list);
	if (g == 0) 
	{ 
		if (dynamic_cast<GEdgeList*>(list) && (list->size() == 1))
		{
			GEdge* ge = dynamic_cast<GEdgeList*>(list)->GetEdge(0);
			g = ge->GetFEEdgeSet();
		}
		else { delete list; list = 0; }
	}

	ModifierThread* thread = new ModifierThread(doc, mod, activeObject, sel);
	CDlgStartThread dlg(this, thread);
	dlg.setTask(QString::fromStdString(mod->GetName()));
	if (dlg.exec())
	{
		bool bsuccess = dlg.GetReturnCode();
		if (bsuccess == false)
		{
			QString err = QString("Error while applying %1:\n%2").arg(QString::fromStdString(mod->GetName())).arg(QString::fromStdString(mod->GetErrorString()));
			QMessageBox::critical(this, "Error", err);
		}
		else
		{
			FSMesh* newMesh = thread->GetNewMesh();
			if (newMesh)
			{
				newMesh->ClearFaceSelection();

				// swap the meshes
				string ss = mod->GetName();
				doc->DoCommand(new CCmdChangeFEMesh(activeObject, thread->GetNewMesh()), ss.c_str(), false);

				std::string err = mod->GetErrorString();
				if (err.empty() == false)
				{
					w->AddLogEntry(QString::fromStdString(err) + QString("\n"));
				}
			}
		}
	}

	modTool->Reset();

	w->UpdateModel(activeObject, true);
	w->UpdateGLControlBar();
	w->RedrawGL();

	// clear any highlights
	GLHighlighter::ClearHighlights();
}

void CMeshPanel::on_menu_triggered(QAction* pa)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(GetDocument());
	GObject* po = pdoc->GetActiveObject();

	// Make sure that this object is not the active item 
	if (pdoc->GetActiveItem() == po) pdoc->SetActiveItem(nullptr);

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

void CMeshPanel::on_form_dataChanged(bool itemModified, int index)
{
	CPropertyList* pl = ui->mesherParams->GetPropertyList();
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
			QString msg = QString("Mesher parameter %1 changed to %2 (%3)").arg(p.name).arg(v.toString()).arg(QString::fromStdString(poa->GetName()));
			doc->AppendChangeLog(msg);
		}
	}
}
