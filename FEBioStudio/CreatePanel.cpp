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
#include "CreatePanel.h"
#include "ui_createpanel.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GPrimitive.h>
#include <QMessageBox>
#include "Commands.h"
#include "MainWindow.h"
#include "CreateP2PlinePane.h"
#include "CreateGeodesicCurvePane.h"
#include "CreateSpringPane.h"
#include <GeomLib/GOCCObject.h>
#include <GeomLib/GModel.h>
#include "CommandWindow.h"
#include <GLLib/GLScene.h>

//------------------------------------------------------------------------------------------------------------
REGISTER_CLASS2(GBox               , CLASS_OBJECT, "box"            , ":/icons/box.png"          , 0);
REGISTER_CLASS2(GCylinder          , CLASS_OBJECT, "cylinder"       , ":/icons/cylinder.png"     , 0);
REGISTER_CLASS2(GTube              , CLASS_OBJECT, "tube"           , ":/icons/tube.png"         , 0);
REGISTER_CLASS2(GSphere            , CLASS_OBJECT, "sphere"         , ":/icons/sphere.png"       , 0);
REGISTER_CLASS2(GCone              , CLASS_OBJECT, "cone"           , ":/icons/cone.png"         , 0);
REGISTER_CLASS2(GTruncatedEllipsoid, CLASS_OBJECT, "ellipsoid"      , ":/icons/ellipsoid.png"    , 0);
REGISTER_CLASS2(GTorus             , CLASS_OBJECT, "torus"          , ":/icons/torus.png"        , 0);
REGISTER_CLASS2(GSlice             , CLASS_OBJECT, "slice"          , ":/icons/slice.png"        , 0);
REGISTER_CLASS2(GSolidArc          , CLASS_OBJECT, "solid_arc"      , ":/icons/solidarc.png"     , 0);
REGISTER_CLASS2(GHexagon           , CLASS_OBJECT, "hexagon"        , ":/icons/hexagon.png"      , 0);
REGISTER_CLASS2(GQuartDogBone      , CLASS_OBJECT, "dog_bone"       , ":/icons/dogbone.png"      , 0);
REGISTER_CLASS2(GCylinderInBox     , CLASS_OBJECT, "cylinder_in_box", ":/icons/cylinderinbox.png", 0);
REGISTER_CLASS2(GSphereInBox       , CLASS_OBJECT, "sphere_in_box"  , ":/icons/sphereinbox.png"  , 0);
REGISTER_CLASS2(GHollowSphere      , CLASS_OBJECT, "hollow_sphere"  , ":/icons/hollowsphere.png" , 0);
REGISTER_CLASS2(GBoxInBox          , CLASS_OBJECT, "box_in_box"     , ":/icons/boxinbox.png"     , 0);
REGISTER_CLASS2(GThinTube          , CLASS_OBJECT, "thin_tube"      , ":/icons/thintube.png"     , 0);
REGISTER_CLASS2(GPatch             , CLASS_OBJECT, "patch"          , ":/icons/square.png"       , 0);
REGISTER_CLASS2(GDisc              , CLASS_OBJECT, "disc"           , ":/icons/disc.png"         , 0);
REGISTER_CLASS2(GRing              , CLASS_OBJECT, "ring"           , ":/icons/ring.png"         , 0);
REGISTER_CLASS2(GExtrudeModifier   , CLASS_MODIFIER, "extrude", ":/icons/extrude.png", 0);
REGISTER_CLASS2(GRevolveModifier   , CLASS_MODIFIER, "revolve", ":/icons/revolve.png", 0);
REGISTER_CLASS2(GBendModifier      , CLASS_MODIFIER, "bend"   , ":/icons/bend.png"   , 0);

#ifndef NDEBUG
REGISTER_CLASS2(GCylindricalPatch  , CLASS_OBJECT, "cylindrical_patch", ":/icons/cylpatch.png"     , 0);

#ifdef HAS_OCC
REGISTER_CLASS2(GOCCBottle			, CLASS_OBJECT, "bottle", ":/icons/bottle.png", 0);
REGISTER_CLASS2(GOCCBox             , CLASS_OBJECT, "occ_box"   , ":/icons/box.png", 0);
#endif

#endif // NDEBUG

//------------------------------------------------------------------------------------------------------------
CCreatePanel::CCreatePanel(CMainWindow* wnd, QWidget* parent) : CWindowPanel(wnd, parent), ui(new Ui::CCreatePanel)
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

	for (it = ClassKernel::FirstCD(); it != ClassKernel::LastCD(); ++it)
	{
		ClassDescriptor* pcd = *it;
		if (pcd->GetType() == CLASS_MODIFIER)
		{
			ui->AddCreateOption(0, pcd->GetName(), QIcon(pcd->GetResourceName()), new CGeoModifierPane(this, pcd));
		}
	}

	// CAD objects
	ui->AddCreateOption(2, "Point-to-point curve", QIcon(":/icons/p2pline.png" ), new CCreateP2PLinePane(this));
	ui->AddCreateOption(2, "Geodesic curve"      , QIcon(":/icons/geodesic.png"), new CCreateGeodesicCurvePane(this));
	ui->AddCreateOption(2, "Loft surface"        , QIcon(":/icons/loft.png"    ), new CCreateLoftSurface(this));
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

			CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

			GObject* activeObject = doc->GetActiveObject();
			if ((pane->createPolicy() == CCreatePane::ADD_NEW_OBJECT) || (activeObject == nullptr))
			{
				doc->AddObject(go);
			}
			else if (pane->createPolicy() == CCreatePane::REPLACE_ACTIVE_OBJECT)
			{
				go->SetName(activeObject->GetName());
				doc->DoCommand(new CCmdSwapObjects(doc->GetGModel(), activeObject, go));
			}
			
			CMainWindow* wnd = GetMainWindow();
			wnd->UpdateModel(go);
			wnd->Update(this);

			// make sure the object is visible
			GLScene* scene = doc->GetScene();
			if (scene)
			{
				scene->ZoomSelection(false);
				wnd->RedrawGL();
			}

			CCommandLine cmd("create");
			CCommandLogger::Log(cmd << po->GetTypeString() << Stringify(*po));
		}
		else if (dynamic_cast<GDiscreteElement*>(po))
		{
			GetMainWindow()->UpdateModel(go);
			GetMainWindow()->Update(this);
		}
		else if (dynamic_cast<GDiscreteObject*>(po))
		{
			GDiscreteObject* go = dynamic_cast<GDiscreteObject*>(po);

			CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
			GModel& geo = doc->GetFSModel()->GetModel();
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
