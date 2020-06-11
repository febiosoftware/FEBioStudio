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
#include <GeomLib/GCurveMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include "ui_meshpanel.h"
#include "ObjectProps.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "CurveIntersectProps.h"
#include "GLHighlighter.h"
#include <GeomLib/GPrimitive.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <sstream>
#include <QtCore/QTimer>
#include <GeomLib/MeshLayer.h>
#include <MeshTools/FEShellMesher.h>
#include <MeshTools/FETetGenMesher.h>
#include "Commands.h"

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

MeshingThread::MeshingThread(GObject* po)
{
	m_po = po;
	m_mesher = nullptr;
}

void MeshingThread::run()
{
	m_mesher = m_po->GetFEMesher();
	m_po->BuildMesh();
	emit resultReady();
}

double MeshingThread::progress()
{
	return (m_mesher ? m_mesher->Progress().percent : 0.0);
}

const char* MeshingThread::currentTask()
{
	return (m_mesher ? m_mesher->Progress().task : "");
}

void MeshingThread::stop()
{
	if (m_mesher) m_mesher->Terminate();
}

//=============================================================================
CDlgStartThread::CDlgStartThread(QWidget* parent, MeshingThread* thread)
{
	m_thread = thread;

	m_szcurrentTask = 0;

	QVBoxLayout* l = new QVBoxLayout;
	l->addWidget(new QLabel("Meshing in progress. Please wait."));
	l->addWidget(m_task = new QLabel(""));

	l->addWidget(m_progress = new QProgressBar);
	m_progress->setRange(0, 100);
	m_progress->setValue(0);

	QHBoxLayout* h = new QHBoxLayout;
	h->addStretch();
	h->addWidget(m_stop = new QPushButton("Cancel"));

	l->addLayout(h);
	setLayout(l);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	QObject::connect(m_thread, SIGNAL(resultReady()), this, SLOT(threadFinished()));
	QObject::connect(m_stop, SIGNAL(clicked()), this, SLOT(cancel()));

	QTimer::singleShot(100, this, SLOT(checkProgress()));

	m_bdone = false;
	m_thread->start();
}

void CDlgStartThread::accept()
{
	QDialog::accept();
}

void CDlgStartThread::cancel()
{
	m_stop->setEnabled(false);
	m_thread->stop();
}

void CDlgStartThread::checkProgress()
{
	if (m_bdone) accept();
	else
	{
		double p = m_thread->progress();
		m_progress->setValue((int) p);

		const char* sztask = m_thread->currentTask();
		if (sztask != m_szcurrentTask)
		{
			m_szcurrentTask = sztask;
			m_task->setText(m_szcurrentTask);
		}

		QTimer::singleShot(100, this, SLOT(checkProgress()));
	}
}

void CDlgStartThread::threadFinished()
{
	m_bdone = true;
	m_thread->deleteLater();
	checkProgress();
}

//=============================================================================

REGISTER_CLASS(FERebuildMesh          , CLASS_FEMODIFIER, "Rebuild Mesh"   , EDIT_MESH);
REGISTER_CLASS(FEAutoPartition        , CLASS_FEMODIFIER, "Auto Partition" , EDIT_MESH);
REGISTER_CLASS(FEPartitionSelection   , CLASS_FEMODIFIER, "Partition"      , EDIT_ELEMENT | EDIT_FACE | EDIT_EDGE | EDIT_NODE);
REGISTER_CLASS(FESmoothMesh           , CLASS_FEMODIFIER, "Smooth"         , EDIT_MESH);
REGISTER_CLASS(FEDiscardMesh          , CLASS_FEMODIFIER, "Discard Mesh"   , EDIT_MESH);
REGISTER_CLASS(FEMirrorMesh           , CLASS_FEMODIFIER, "Mirror"         , EDIT_MESH);
REGISTER_CLASS(FETetGenModifier       , CLASS_FEMODIFIER, "TetGen"         , EDIT_MESH);
REGISTER_CLASS(FEExtrudeFaces         , CLASS_FEMODIFIER, "Extrude Faces"  , EDIT_FACE);
REGISTER_CLASS(FERevolveFaces         , CLASS_FEMODIFIER, "Revolve Faces"  , EDIT_FACE);
REGISTER_CLASS(FEWeldNodes            , CLASS_FEMODIFIER, "Weld nodes"     , EDIT_MESH);
REGISTER_CLASS(FERefineMesh			  , CLASS_FEMODIFIER, "Refine Mesh"    , EDIT_MESH | EDIT_SAFE);
REGISTER_CLASS(FEConvertMesh		  , CLASS_FEMODIFIER, "Convert"        , EDIT_MESH | EDIT_SAFE);
REGISTER_CLASS(FEAddNode              , CLASS_FEMODIFIER, "Add Node"       , EDIT_MESH);
REGISTER_CLASS(FEInvertMesh           , CLASS_FEMODIFIER, "Invert"         , EDIT_MESH | EDIT_ELEMENT | EDIT_SAFE);
REGISTER_CLASS(FEDetachElements	      , CLASS_FEMODIFIER, "Detach"         , EDIT_ELEMENT);
REGISTER_CLASS(FEBoundaryLayerMesher  , CLASS_FEMODIFIER, "PostBL"         , EDIT_FACE | EDIT_SAFE);
REGISTER_CLASS(FESetShellThickness    , CLASS_FEMODIFIER, "Shell Thickness", EDIT_MESH | EDIT_ELEMENT);
REGISTER_CLASS(FESetFiberOrientation  , CLASS_FEMODIFIER, "Set Fibers"     , EDIT_MESH | EDIT_ELEMENT | EDIT_SAFE);
REGISTER_CLASS(FESetAxesOrientation   , CLASS_FEMODIFIER, "Set Axes"       , EDIT_MESH | EDIT_ELEMENT | EDIT_SAFE);
REGISTER_CLASS(FEAxesCurvature        , CLASS_FEMODIFIER, "Set Axes from Curvature" , EDIT_MESH | EDIT_ELEMENT | EDIT_FACE | EDIT_SAFE);
REGISTER_CLASS(FEAlignNodes           , CLASS_FEMODIFIER, "Align"          , EDIT_NODE);
REGISTER_CLASS(FECreateShells         , CLASS_FEMODIFIER, "Create Shells from Faces"  , EDIT_FACE | EDIT_MESH);
#ifdef HAS_MMG
REGISTER_CLASS(FEMMGRemesh, CLASS_FEMODIFIER, "Tet Remesh", EDIT_MESH | EDIT_SAFE);
#endif

CMeshPanel::CMeshPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CMeshPanel)
{
	m_mod = 0;
	m_nid = -1;
	m_currentObject = nullptr;
	ui->setupUi(this, wnd);
}

void CMeshPanel::Update(bool breset)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GModel* gm = doc->GetGModel();
	GObject* activeObject = doc->GetActiveObject();

	// only update if reset is true or the active object changed
	if ((breset == false) && (activeObject == m_currentObject)) return;

	// keep track of the object
	m_currentObject = activeObject;

	ui->obj->Update();

	// start by hiding everything
	ui->hideAllPanels();

	// if there is no active object, we're done
	if (activeObject == 0) return;

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

		FEMesh* mesh = activeObject->GetFEMesh();
		if (mesh)
		{
			// show modifiers for non-editable meshes
			ui->showButtonsPanel2(true);
		}
	}
}

void CMeshPanel::on_buttons_buttonSelected(int id)
{
	m_nid = id;
	if (id == -1) ui->showModifierParametersPanel(false);
	else
	{
		if (m_mod) delete m_mod;
		m_mod = 0;

		ui->setModifierPropertyList(0);

		ClassDescriptor* pcd = ui->buttons->GetClassDescriptor(id);
		if (pcd)
		{
			m_mod = static_cast<FEModifier*>(pcd->Create()); assert(m_mod);

			CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

			GModel* geo = &doc->GetFEModel()->GetModel();

			CPropertyList* pl = new CObjectProps(m_mod);

			ui->setModifierPropertyList(pl);
		}

		ui->showModifierParametersPanel(true);
	}
}

void CMeshPanel::on_buttons2_buttonSelected(int id)
{
	// If the ID is the same and we already have a modifier 
	// allocated, then return.
	if ((id == m_nid) && m_mod) return;

	// delete modifier
	if (m_mod) delete m_mod;
	m_mod = nullptr;

	// store button id
	m_nid = id;

	// update modifier and panel
	if (id == -1) ui->showModifierParametersPanel(false);
	else
	{
		ui->setModifierPropertyList(0);

		ClassDescriptor* pcd = ui->buttons2->GetClassDescriptor(id);
		if (pcd)
		{
			m_mod = static_cast<FEModifier*>(pcd->Create()); assert(m_mod);

			CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

			GModel* geo = &doc->GetFEModel()->GetModel();

			CPropertyList* pl = new CObjectProps(m_mod);

			ui->setModifierPropertyList(pl);
		}

		ui->showModifierParametersPanel(true);
	}
}


void CMeshPanel::Apply()
{
	on_apply_clicked(true);
}

void CMeshPanel::on_apply_clicked(bool b)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == 0) return;

	FEMesher* mesher = activeObject->GetFEMesher();
	if (mesher == 0) return;

	MeshingThread* thread = new MeshingThread(activeObject);
	CDlgStartThread dlg(this, thread);
	if (dlg.exec())
	{
		Update();
		CMainWindow* w = GetMainWindow();
		w->UpdateGLControlBar();
		w->UpdateModel(activeObject, false);
		w->RedrawGL();

		// clear any highlights
		GLHighlighter::ClearHighlights();
	}
}

void CMeshPanel::on_apply2_clicked(bool b)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == 0) return;

	// make sure we have a modifier
	if (m_mod == 0) return;

	FESelection* sel = doc->GetCurrentSelection();
	FEItemListBuilder* list = (sel ? sel->CreateItemList() : 0);
	FEGroup* g = dynamic_cast<FEGroup*>(list);
	if (g == 0) 
	{ 
		if (dynamic_cast<GEdgeList*>(list) && (list->size() == 1))
		{
			GEdgeList* edgeList = dynamic_cast<GEdgeList*>(list);
			GEdge* ge = dynamic_cast<GEdgeList*>(list)->GetEdge(0);
			g = ge->GetFEEdgeSet();
		}
		else { delete list; list = 0; }
	}

	bool bsuccess = doc->ApplyFEModifier(*m_mod, activeObject, g);
	if (bsuccess == false)
	{
		QString err = QString("Error while applying %1:\n%2").arg(QString::fromStdString(m_mod->GetName())).arg(QString::fromStdString(m_mod->GetErrorString()));
		QMessageBox::critical(this, "Error", err);
	}

	// don't forget to cleanup
	if (g) delete g;

	CMainWindow* w = GetMainWindow();
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
	GModel* mdl = pdoc->GetGModel();

	if (pa->objectName() == "convert1")
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
	else
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
	Update();
	GetMainWindow()->Update(this, true);
}
