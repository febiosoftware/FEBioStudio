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
#include "Document.h"
#include "CurveIntersectProps.h"
#include "GLHighlighter.h"
#include <GeomLib/GPrimitive.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <sstream>
#include <QtCore/QTimer>
#include "Command.h"

MeshingThread::MeshingThread(GObject* po)
{
	m_po = po;
	m_mesher = nullptr;
}

void MeshingThread::run()
{
	m_mesher = m_po->GetMesher();
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
REGISTER_CLASS(FEMMGRemesh, CLASS_FEMODIFIER, "Tet Refine", EDIT_MESH);
#endif

CMeshPanel::CMeshPanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CMeshPanel)
{
	m_mod = 0;
	m_nid = -1;
	ui->setupUi(this, wnd);
}

void CMeshPanel::Update()
{
	CDocument* doc = GetDocument();
	GObject* activeObject = doc->GetActiveObject();

	ui->obj->Update();

	// start by hiding everything
	ui->hideAllPanels();

	// if there is no active object, we're done
	if (activeObject == 0) return;

	if (dynamic_cast<GMeshObject*>(activeObject))
	{
		GMeshObject*  meshObject = dynamic_cast<GMeshObject*>(activeObject);

		// show the modifiers for editable meshes
		ui->showButtonsPanel(true);
	}
	else
	{
		FEMesher* mesher = activeObject->GetMesher();
		if (mesher)
		{
			ui->setMesherPropertyList(new CObjectProps(mesher));
			ui->showMesherParametersPanel(true);
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

			GModel* geo = &GetDocument()->GetFEModel()->GetModel();

			CPropertyList* pl = new CObjectProps(m_mod);

			ui->setModifierPropertyList(pl);
		}

		ui->showModifierParametersPanel(true);
	}
}

void CMeshPanel::on_buttons2_buttonSelected(int id)
{
	m_nid = id;
	if (id == -1) ui->showModifierParametersPanel(false);
	else
	{
		if (m_mod) delete m_mod;
		m_mod = 0;

		ui->setModifierPropertyList(0);

		ClassDescriptor* pcd = ui->buttons2->GetClassDescriptor(id);
		if (pcd)
		{
			m_mod = static_cast<FEModifier*>(pcd->Create()); assert(m_mod);

			GModel* geo = &GetDocument()->GetFEModel()->GetModel();

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
	CDocument* doc = GetDocument();
	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == 0) return;

	FEMesher* mesher = activeObject->GetMesher();
	if (mesher == 0) return;

	MeshingThread* thread = new MeshingThread(activeObject);
	CDlgStartThread dlg(this, thread);
	if (dlg.exec())
	{
		Update();
		CMainWindow* w = GetMainWindow();
		w->UpdateGLControlBar();
		CDocument* doc = GetDocument();
		GObject* activeObject = doc->GetActiveObject();
		w->UpdateModel(activeObject, false);
		w->RedrawGL();

		// clear any highlights
		GLHighlighter::ClearHighlights();
	}
}

void CMeshPanel::on_apply2_clicked(bool b)
{
	CDocument* doc = GetDocument();
	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == 0) return;

	// make sure we have a modifier
	if (m_mod == 0) return;

	FESelection* sel = doc->GetCurrentSelection();
	FEItemListBuilder* list = (sel ? sel->CreateItemList() : 0);
	FEGroup* g = dynamic_cast<FEGroup*>(list);
	if (g == 0) { delete list; list = 0; }

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
		if (dynamic_cast<GMeshObject*>(po)==0)
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
