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

#include "PyFBSUI.h"

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <GeomLib/GPrimitive.h>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/Commands.h>
#include <FEBioLink/FEBioClass.h>
#include <MeshTools/GModel.h>
#include <GeomLib/GItem.h>
#include <FEBio/FEBioExport4.h>
#include <MeshTools/FEModel.h>

CModelDocument* GetActiveDocument()
{
	CMainWindow* wnd = FBS::getMainWindow();
	CModelDocument* doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
	return doc;
}

FSModel* GetActiveFSModel()
{
	CModelDocument* doc = GetActiveDocument();
	return doc->GetFSModel();
}

GModel* GetActiveGModel()
{
	CModelDocument* doc = GetActiveDocument();
	return doc->GetGModel();
}

// This function will add an object to the currently active model
void AddObjectToModel(GObject* po)
{
	GModel* gm = GetActiveGModel();
	gm->AddObject(po);
}

// Create a box primitve
GObject* CreateBox(std::string& name, vec3d pos, double width, double height, double depth)
{
	// create the box
	GBox* gbox = new GBox();
	gbox->SetName(name);
	gbox->SetFloatValue(GBox::WIDTH, width);
	gbox->SetFloatValue(GBox::HEIGHT, height);
	gbox->SetFloatValue(GBox::DEPTH, depth);
	gbox->Update();
	gbox->GetTransform().SetPosition(pos);

	// add it to the model
	AddObjectToModel(gbox);

	return gbox;
}

GMaterial* AddMaterial(std::string& name, std::string& type)
{
	FSModel* fem = GetActiveFSModel();
	FSMaterial* pm = FEBio::CreateMaterial(type, fem);
	if (pm)
	{
		GMaterial* gm = new GMaterial;
		gm->SetName(name);
		gm->SetMaterialProperties(pm);
		fem->AddMaterial(gm);
		return gm;
	}
	return nullptr;
}

FSStep* AddStep(std::string& name)
{
	FSModel* fem = GetActiveFSModel();
	FSStep* step = FEBio::CreateStep("solid", fem);
	if (step) fem->AddStep(step);
	return step;
}

bool ExportFEB(std::string& fileName)
{
	CModelDocument* doc = GetActiveDocument();
	FEBioExport4 feb(doc->GetProject());
	return feb.Write(fileName.c_str());
}

// Initializes the fbs.mdl module
void init_FBSModel(pybind11::module& m)
{
	pybind11::module mdl = m.def_submodule("mdl", "Module used to interact with an FEBio model.");


    ///////////////// FSModel /////////////////
    mdl.def("GetActiveFSModel", &GetActiveFSModel);

    pybind11::class_<FSModel, std::unique_ptr<FSModel, pybind11::nodelete>>(mdl, "FSModel")
        .def("Clear", &FSModel::Clear)
        .def("Purge", &FSModel::Purge)

        // --- functions to delete all components ---
        .def("DeleteAllMaterials", &FSModel::DeleteAllMaterials)
        .def("DeleteAllBC", &FSModel::DeleteAllBC)
        .def("DeleteAllLoads", &FSModel::DeleteAllLoads)
        .def("DeleteAllIC", &FSModel::DeleteAllIC)
        .def("DeleteAllContact", &FSModel::DeleteAllContact)
        .def("DeleteAllConstraints", &FSModel::DeleteAllRigidBCs)
        .def("DeleteAllRigidICs", &FSModel::DeleteAllRigidICs)
        .def("DeleteAllRigidLoads", &FSModel::DeleteAllRigidLoads)
        .def("DeleteAllRigidConnectors", &FSModel::DeleteAllRigidConnectors)
        .def("DeleteAllSteps", &FSModel::DeleteAllSteps)
        .def("DeleteAllLoadControllers", &FSModel::DeleteAllLoadControllers)
        .def("DeleteAllMeshDataGenerators", &FSModel::DeleteAllMeshDataGenerators)
        
        .def("ClearSelections", &FSModel::ClearSelections)
        .def("New", &FSModel::New)
        .def("GetModel", &FSModel::GetModel)

        // --- material functions ---
        .def("GetMaterial", &FSModel::GetMaterial)
        .def("AddMaterial", &FSModel::AddMaterial)
        .def("ReplaceMaterial", &FSModel::ReplaceMaterial)
        .def("CanDeleteMaterial", &FSModel::CanDeleteMaterial)
        .def("DeleteMaterial", &FSModel::DeleteMaterial)
        .def("InsertMaterial", &FSModel::InsertMaterial)
        .def("Materials", &FSModel::Materials)
        .def("GetMaterialFromID", &FSModel::GetMaterialFromID)
        .def("FindMaterial", &FSModel::FindMaterial)
        .def("GetRigidConnectorFromID", &FSModel::GetRigidConnectorFromID)
        .def("GetModel", &FSModel::GetModel)
        
        // --- Analysis steps ---
        .def("Steps", &FSModel::Steps)
        .def("GetStep", &FSModel::GetStep)
        .def("FindStep", &FSModel::FindStep)
        .def("GetStepIndex", &FSModel::GetStepIndex)
        .def("AddStep", &FSModel::AddStep)
        .def("DeleteStep", &FSModel::DeleteStep)
        .def("InsertStep", &FSModel::InsertStep)
        .def("SwapSteps", &FSModel::SwapSteps)
        .def("ReplaceStep", &FSModel::ReplaceStep)
        .def("AssignComponentToStep", &FSModel::AssignComponentToStep)
        ;
    ///////////////// FSModel /////////////////

    ///////////////// GModel /////////////////
    mdl.def("GetActiveGModel", &GetActiveGModel);

    pybind11::class_<GModel, std::unique_ptr<GModel, pybind11::nodelete>>(mdl, "GModel")
        .def("Clear", &GModel::Clear)
        .def("ClearGroups", &GModel::ClearGroups)
        .def("ClearDiscrete", &GModel::ClearDiscrete)

        // --- Object functions ---
        .def("Objects", &GModel::Objects)
        .def("Object", &GModel::Object)
        .def("FindObject", static_cast<GObject* (GModel::*)(int)>(&GModel::FindObject))
        .def("FindObject", static_cast<GObject* (GModel::*)(const string&)>(&GModel::FindObject))
        .def("FindObjectIndex", &GModel::FindObjectIndex)
        .def("ReplaceObject", static_cast<void (GModel::*)(int, GObject*)>(&GModel::ReplaceObject))
        .def("ReplaceObject", static_cast<void (GModel::*)(GObject*, GObject*)>(&GModel::ReplaceObject))
        .def("AddObject", &GModel::AddObject)
        .def("RemoveObject", &GModel::RemoveObject)
        .def("InsertObject", &GModel::InsertObject)

        // --- part functions ---
        .def("Parts", &GModel::Parts)
        .def("Part", &GModel::Part)
        .def("FindPart", &GModel::FindPart)

        // --- surface functions ---
        .def("Surfaces", &GModel::Surfaces)
        .def("Surface", &GModel::Surface)
        .def("FindSurface", &GModel::FindSurface)

        // --- edge functions ---
        .def("Edges", &GModel::Edges)
        .def("Edge", &GModel::Edge)
        .def("FindEdge", &GModel::FindEdge)
        .def("FindEdgeFromName", &GModel::FindEdgeFromName)

        // --- node functions ---
        .def("Nodes", &GModel::Nodes)
        .def("Node", &GModel::Node)
        .def("FindNode", &GModel::FindNode)
        ;
    ///////////////// GModel /////////////////

    ///////////////// GNode /////////////////
    pybind11::class_<GNode, std::unique_ptr<GNode, pybind11::nodelete>>(mdl, "GNode")
        .def("Type", &GNode::Type)
        .def("SetType", &GNode::SetType)
        .def("LocalPosition", static_cast<vec3d& (GNode::*)()>(&GNode::LocalPosition))
        .def("Position", &GNode::Position)
        .def("MakeRequired", &GNode::MakeRequired)
        ;
    ///////////////// GNode /////////////////

	mdl.def("CreateBox", CreateBox);
	mdl.def("AddMaterial", AddMaterial);
	mdl.def("AddStep", AddStep);
	mdl.def("ExportFEB", ExportFEB);

	pybind11::class_<FSStep, std::unique_ptr<FSStep, pybind11::nodelete>>(mdl, "FSStep");

	pybind11::class_<GMaterial, std::unique_ptr<GMaterial, pybind11::nodelete>>(mdl, "GMaterial")
		.def("set", [](GMaterial* gm, std::string& paramName, float value) {
			FSMaterial* pm = gm->GetMaterialProperties();
			if (pm) {
				Param* p = pm->GetParam(paramName.c_str());
				if (p) p->SetFloatValue(value);
			}
		});

	pybind11::class_<GObject, std::unique_ptr<GObject, pybind11::nodelete>>(mdl, "GObject")
		.def("AssignMaterial", [](GObject* po, GMaterial* pm) {
			for (int i = 0; i < po->Parts(); ++i) po->Part(i)->SetMaterialID(pm->GetID());
		})
        .def("BuildMesh", &GObject::BuildMesh);
			// });
		// .def("BuildMesh", [](GObject* po) {
		// 	po->BuildMesh();
		// 	});

}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
