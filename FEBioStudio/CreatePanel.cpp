#include "stdafx.h"
#include "CreatePanel.h"
#include "ui_createpanel.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GPrimitive.h>
#include <QMessageBox>
#include "ObjectProps.h"
#include "Document.h"
#include "Command.h"
#include "MainWindow.h"
#include "GLView.h"
#include "CreateP2PlinePane.h"
#include "CreateSpringPane.h"
#include <GeomLib/GOCCObject.h>

//------------------------------------------------------------------------------------------------------------
REGISTER_CLASS2(GBox               , CLASS_OBJECT, "Box"            , ":/icons/box.png"          , 0);
REGISTER_CLASS2(GCylinder          , CLASS_OBJECT, "Cylinder "      , ":/icons/cylinder.png"     , 0);
REGISTER_CLASS2(GTube              , CLASS_OBJECT, "Tube"           , ":/icons/tube.png"         , 0);
REGISTER_CLASS2(GSphere            , CLASS_OBJECT, "Sphere"         , ":/icons/sphere.png"       , 0);
REGISTER_CLASS2(GCone              , CLASS_OBJECT, "Cone"           , ":/icons/cone.png"         , 0);
REGISTER_CLASS2(GTruncatedEllipsoid, CLASS_OBJECT, "Ellipsoid"      , ":/icons/ellipsoid.png"    , 0);
REGISTER_CLASS2(GTorus             , CLASS_OBJECT, "Torus"          , ":/icons/torus.png"        , 0);
REGISTER_CLASS2(GSlice             , CLASS_OBJECT, "Slice"          , ":/icons/slice.png"        , 0);
REGISTER_CLASS2(GSolidArc          , CLASS_OBJECT, "Solid Arc"      , ":/icons/solidarc.png"     , 0);
REGISTER_CLASS2(GHexagon           , CLASS_OBJECT, "Hexagon"        , ":/icons/hexagon.png"      , 0);
REGISTER_CLASS2(GQuartDogBone      , CLASS_OBJECT, "Dog bone"       , ":/icons/dogbone.png"      , 0);
REGISTER_CLASS2(GCylinderInBox     , CLASS_OBJECT, "Cylinder in Box", ":/icons/cylinderinbox.png", 0);
REGISTER_CLASS2(GSphereInBox       , CLASS_OBJECT, "Sphere in Box"  , ":/icons/sphereinbox.png"  , 0);
REGISTER_CLASS2(GHollowSphere      , CLASS_OBJECT, "Hollow Sphere"  , ":/icons/hollowsphere.png" , 0);
REGISTER_CLASS2(GThinTube          , CLASS_OBJECT, "Thin Tube"      , ":/icons/thintube.png"     , 0);
REGISTER_CLASS2(GPatch             , CLASS_OBJECT, "Patch"          , ":/icons/square.png"       , 0);
REGISTER_CLASS2(GDisc              , CLASS_OBJECT, "Disc"           , ":/icons/disc.png"         , 0);
REGISTER_CLASS2(GRing              , CLASS_OBJECT, "Ring"           , ":/icons/ring.png"         , 0);

#ifdef _DEBUG
REGISTER_CLASS2(GCylindricalPatch  , CLASS_OBJECT, "Cylindrical Patch", ":/icons/cylpatch.png"     , 0);
#endif

#ifdef HAS_OCC
REGISTER_CLASS2(GOCCBottle			, CLASS_OBJECT, "Bottle", ":/icons/bottle.png", 0);
REGISTER_CLASS2(GOCCBox             , CLASS_OBJECT, "Box"   , ":/icons/box.png", 0);
#endif

//------------------------------------------------------------------------------------------------------------
CCreatePanel::CCreatePanel(CMainWindow* wnd, QWidget* parent) : CCommandPanel(wnd, parent), ui(new Ui::CCreatePanel)
{
	ui->setupUi(this);
	m_tmpObject = 0;

	// fill the button panels
	// Primitives
	Class_Iterator it;
	for (it = ClassKernel::FirstCD(); it != ClassKernel::LastCD(); ++it)
	{
		ClassDescriptor* pcd = *it;
		if (pcd->GetType() == CLASS_OBJECT)
		{
			ui->AddCreateOption(0, pcd->GetName(), QIcon(pcd->GetResourceName()), new CDefaultCreatePane(this, pcd));
		}
	}

	// Springs
	CCreatePane* springPane = new CCreateSpringPane(this); springPane->setObjectName("spring");
	ui->AddCreateOption(1, "Spring", QIcon(":/icons/spring.png"), springPane);

//	ui->AddCreateOption(1, "Deformable spring", QIcon(":/icons/wire.png"), new CCreateSpringPane(this));

	// CAD objects
	ui->AddCreateOption(2, "Point-to-point curve", QIcon(":/icons/p2pline.png"), new CCreateP2PLinePane(this));
	ui->AddCreateOption(2, "Loft surface"        , QIcon(":/icons/loft.png"), new CCreateLoftSurface(this));
	ui->AddCreateOption(2, "Extrude"             , QIcon(":/icons/extrude.png"), new CCreateExtrude(this));
}

GObject* CCreatePanel::GetTempObject()
{
	return m_tmpObject;
}

void CCreatePanel::SetTempObject(GObject* po)
{
	m_tmpObject = po;
	GetMainWindow()->RedrawGL();
}

void CCreatePanel::setInput(const vec3d& p)
{
	if (ui->activePane && ui->activePane->isVisible()) ui->activePane->setInput(p);
	GetMainWindow()->RedrawGL();
}

void CCreatePanel::setInput(FESelection* sel)
{
	if (ui->activePane && ui->activePane->isVisible()) ui->activePane->setInput(sel);
	GetMainWindow()->RedrawGL();
}

bool CCreatePanel::OnEscapeEvent()
{
	CCreatePane* p = ui->currentPage();
	if (p) return p->Clear();
	else return false;
}

bool CCreatePanel::OnDeleteEvent()
{
	CCreatePane* p = ui->currentPage();
	if (p) return p->Undo();
	else return false;
}

void CCreatePanel::Apply()
{
	if (ui->currentPage())
		on_create_clicked();
}

void CCreatePanel::on_create_clicked()
{
	CCreatePane* pane = ui->currentPage(); assert(pane);
	FSObject* po = pane->Create();

	if (po)
	{
		GObject* go = dynamic_cast<GObject*>(po);
		if (go)
		{
			if (go->Update(true) == false)
			{
				QMessageBox::critical(this, "Create Object", "Cannot create object.\nInvalid object parameters");
				return;
			}

			CDocument* doc = GetDocument();
			doc->DoCommand(new CCmdAddAndSelectObject(go), go->GetName());
			CMainWindow* wnd = GetMainWindow();
			wnd->UpdateModel(go);
			wnd->Update(this);

			// make sure the object is visible
			wnd->GetGLView()->ZoomSelection(false);
		}
		else if (dynamic_cast<GDiscreteElement*>(po))
		{
			GetMainWindow()->UpdateModel(go);
			GetMainWindow()->Update(this);
		}
		else if (dynamic_cast<GDiscreteObject*>(po))
		{
			GDiscreteObject* go = dynamic_cast<GDiscreteObject*>(po);

			CDocument* doc = GetDocument();
			GModel& geo = doc->GetFEModel()->GetModel();
			geo.AddDiscreteObject(go);
			GetMainWindow()->UpdateModel(go);
			GetMainWindow()->Update(this);
		}
		else 
		{
			assert(false);
			delete po;
		}
	}
}

void CCreatePanel::select_option(int id)
{
	ui->setActivePane(id);
}
