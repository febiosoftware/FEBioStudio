#include "stdafx.h"
#include "ui_editpanel.h"
#include "MainWindow.h"
#include "Document.h"
#include "ObjectProps.h"
#include "GLHighlighter.h"
#include "CurveIntersectProps.h"
#include <MeshTools/FECurveIntersect.h>
#include <MeshTools/FESmoothSurfaceMesh.h>
#include <MeshTools/FEEdgeCollapse.h>
#include <MeshTools/FEFixMesh.h>
#include <MeshTools/FECVDDecimationModifier.h>
#include <MeshTools/FEEdgeFlip.h>
#include <MeshTools/FERefineSurface.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GMeshObject.h>
#include <QMessageBox>
#include "Commands.h"

REGISTER_CLASS(FESurfaceAutoPartition     , CLASS_SURFACE_MODIFIER, "Auto partition", 0xFF);
REGISTER_CLASS(FESurfacePartitionSelection, CLASS_SURFACE_MODIFIER, "Partition"    , 0xFF);
REGISTER_CLASS(FESmoothSurfaceMesh        , CLASS_SURFACE_MODIFIER, "Smooth"       , 0xFF);
REGISTER_CLASS(FEEdgeCollapse             , CLASS_SURFACE_MODIFIER, "Edge Collapse", 0xFF);
REGISTER_CLASS(FEFixMesh                  , CLASS_SURFACE_MODIFIER, "Fix Mesh"     , 0xFF);
REGISTER_CLASS(FECVDDecimationModifier    , CLASS_SURFACE_MODIFIER, "Decimate"     , 0xFF);
REGISTER_CLASS(FEEdgeFlip                 , CLASS_SURFACE_MODIFIER, "Flip edges"   , 0xFF);
REGISTER_CLASS(FERefineSurface            , CLASS_SURFACE_MODIFIER, "Refine"       , 0xFF);
REGISTER_CLASS(FECurveIntersect           , CLASS_SURFACE_MODIFIER, "Project Curve", 0xFF);

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

CEditPanel::CEditPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CEditPanel)
{
	ui->setupUi(this, wnd);
}

void CEditPanel::Update()
{
	CDocument* doc = GetDocument();

	// make sure the active object has changed
	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == ui->m_currenObject) return;

	ui->m_currenObject = activeObject;

	ui->obj->Update();

	if (activeObject == 0)
	{
		ui->showParametersPanel(false);
		ui->showButtonsPanel(false);
	}
	else
	{
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
	FEMesh* pm = activeObject->GetFEMesh();
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
			CDocument* doc = GetDocument();
			FESelection* sel = doc->GetCurrentSelection();
			FEItemListBuilder* list = sel->CreateItemList();
			FEGroup* g = 0;
			if (sel->Size() > 0)
			{
				g = dynamic_cast<FEGroup*>(list);
				if (g == 0) { delete list; list = 0; }
			}

			bool ret = doc->ApplyFESurfaceModifier(*ui->m_mod, surfaceObject, g);
			if (ret == false)
			{
				std::string err;
				if (ui->m_mod) err = ui->m_mod->GetErrorString();
				if (err.empty()) err = "(unknown)";
				QString errStr = QString::fromStdString(err);
				QMessageBox::critical(this, "Apply modifier", "Cannot apply this modifier to this selection.\nERROR: " + errStr);
			}
			GetMainWindow()->RedrawGL();
			GetMainWindow()->UpdateModel(activeObject, true);

			// don't forget to cleanup
			if (g) delete g;
		}
	}
	else
	{
		CCmdGroup* cmd = new CCmdGroup("Change mesh");
		cmd->AddCommand(new CCmdChangeFEMesh(activeObject, nullptr));
		cmd->AddCommand(new CCmdChangeObjectParams(activeObject));
		GetDocument()->DoCommand(cmd);
	}

	// clear any highlights
	GLHighlighter::ClearHighlights();
}

void CEditPanel::on_menu_triggered(QAction* pa)
{
	CDocument* pdoc = GetDocument();
	GObject* po = pdoc->GetActiveObject();

	if (pa->objectName() == "convert1")
	{
		FEMesh* mesh = po->GetFEMesh();
		if (mesh == 0)
		{
			QMessageBox::critical(this, "Convert", "This object does not have a mesh and cannot be converted to an editable mesh.");
			return;
		}

		// convert to editable surface
		if (dynamic_cast<GSurfaceMeshObject*>(po) == 0)
		{
			CCmdConvertToEditableSurface* pcmd = new CCmdConvertToEditableSurface(po);
			pdoc->DoCommand(pcmd);

			// update the modify panel
			Update();

			GetMainWindow()->Update(this, true);
		}
	}
	else
	{
		// convert to editable mesh
		if (dynamic_cast<GMeshObject*>(po) == 0)
		{
			FEMesh* mesh = po->GetFEMesh();
			if (mesh == 0)
			{
				// for editable surfaces, we'll use the surface mesh for converting
				if (dynamic_cast<GSurfaceMeshObject*>(po))
				{
					CCmdConvertSurfaceToEditableMesh* pcmd = new CCmdConvertSurfaceToEditableMesh(po);
					pdoc->DoCommand(pcmd);

					// update the modify panel
					Update();

					GetMainWindow()->Update(this, true);
				}
				else QMessageBox::critical(this, "Convert", "This object does not have a mesh and cannot be converted to an editable mesh.");
			}
			else
			{
				CCmdConvertToEditableMesh* pcmd = new CCmdConvertToEditableMesh(po);
				pdoc->DoCommand(pcmd);

				// update the modify panel
				Update();

				GetMainWindow()->Update(this, true);
			}
		}
	}
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

			GModel* geo = &GetDocument()->GetFEModel()->GetModel();

			CPropertyList* pl = 0;
			if (dynamic_cast<FECurveIntersect*>(ui->m_mod)) pl = new CCurveIntersectProps(geo, dynamic_cast<FECurveIntersect*>(ui->m_mod));
			else if (dynamic_cast<FESurfacePartitionSelection*>(ui->m_mod)) pl = new CPartitionProps(dynamic_cast<FESurfacePartitionSelection*>(ui->m_mod));
			else pl = new CObjectProps(ui->m_mod);

			ui->setPropertyList(pl);
		}

		ui->showParametersPanel(true);
	}
}
