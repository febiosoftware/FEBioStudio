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
#include "DocTemplate.h"
#include "ModelDocument.h"
#include <QtCore/QDir>
#include <XML/XMLReader.h>
#include <FEMLib/FSProject.h>
#include <FEBioLink/FEBioModule.h>
#include <FEBioLink/FEBioClass.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GGroup.h>
#include <MeshTools/FEBox.h>
#include <MeshTools/FEMesher.h>
#include <GeomLib/GPrimitive.h>

std::vector<DocTemplate*> TemplateManager::m_doc;
string TemplateManager::m_path = "$(PREVIEW_PATH)\\templates\\";

DocTemplate::DocTemplate()
{
}

DocTemplate::~DocTemplate()
{

}


string TemplateManager::TemplatePath() { return m_path; }

int TemplateManager::Templates() { return (int) m_doc.size(); }

DocTemplate& TemplateManager::GetTemplate(int i) { return *m_doc[i]; }

void TemplateManager::Init()
{
	DocTemplate* tmp = new DocTemplateUniAxialStrain;
	tmp->title = "uniaxial strain";
	AddTemplate(tmp);
}

void TemplateManager::AddTemplate(DocTemplate* tmp)
{
	assert(tmp);
	if (tmp) m_doc.push_back(tmp);
}

//================================================================================
DocTemplateUniAxialStrain::DocTemplateUniAxialStrain()
{
	title = "uniaxial strain";
}

bool DocTemplateUniAxialStrain::Load(CModelDocument* doc)
{
	// First, set the correct module
	FSProject& prj = doc->GetProject();
	prj.SetModule(FEBio::GetModuleId("solid"));

	// get the models
	FSModel& fsm = prj.GetFSModel();
	GModel& gm = fsm.GetModel();

	// add a box
	GBox* po = new GBox;
	po->SetName("box");
	po->m_w = po->m_h = po->m_d = 1.0;
	po->Update();
	gm.AddObject(po);

	// create unit mesh
	FEBoxMesher* box = dynamic_cast<FEBoxMesher*>(po->GetFEMesher());
	box->SetResolution(1, 1, 1);
	po->BuildMesh();

	// get the surfaces
	GFace* xn = po->Face(3);
	GFace* xp = po->Face(1);
	GFace* yn = po->Face(0);
	GFace* zn = po->Face(4);

	// add a step
	FSStep* step = FEBio::CreateStep("solid", &fsm);
	step->SetName("Step1");
	FEBio::InitDefaultProps(step);
	fsm.AddStep(step);

	// add boundary conditions
	FSBoundaryCondition* bcx = FEBio::CreateBoundaryCondition("zero displacement", &fsm);
	bcx->SetName("fix-x");
	bcx->SetParamBool("x_dof", true);
	bcx->SetItemList(new GFaceList(&gm, xn));
	step->AddBC(bcx);

	FSBoundaryCondition* bcy = FEBio::CreateBoundaryCondition("zero displacement", &fsm);
	bcy->SetName("fix-y");
	bcy->SetParamBool("y_dof", true);
	bcy->SetItemList(new GFaceList(&gm, yn));
	step->AddBC(bcy);

	FSBoundaryCondition* bcz = FEBio::CreateBoundaryCondition("zero displacement", &fsm);
	bcz->SetName("fix-z");
	bcz->SetParamBool("z_dof", true);
	bcz->SetItemList(new GFaceList(&gm, zn));
	step->AddBC(bcz);

	FSBoundaryCondition* dcx = FEBio::CreateBoundaryCondition("prescribed displacement", &fsm);
	dcx->SetName("displace");
	dcx->SetParamInt("dof", 0);
	dcx->SetParamFloat("value", 1.0);
	dcx->SetItemList(new GFaceList(&gm, xp));
	step->AddBC(dcx);

	// create load controller and assign to parameter
	LoadCurve lc;
	lc.Add(0, 0);
	lc.Add(1, 1);
	FSLoadController* plc = fsm.AddLoadCurve(lc);
	dcx->GetParam("value")->SetLoadCurveID(plc->GetID());

	return true;
}
