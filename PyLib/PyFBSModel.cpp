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
#include <FEMLib/FSModel.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GBaseObject.h>
#include <GeomLib/GItem.h>
#include "PySpringFunctions.h"
#include <FEBio/FEBioExport4.h>
#include "PyFSMesh.h"

namespace py = pybind11;

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
	return gbox;
}

// Create a box primitve
GObject* CreateMeshObject(const std::string& name, FSMesh* pm)
{
	GMeshObject* po = new GMeshObject(pm);
	po->SetName(name);
	return po;
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
void init_FBSModel(py::module& m)
{
	py::module mdl = m.def_submodule("mdl", "Module used to interact with an FEBio model.");


    ///////////////// FSModel /////////////////
    mdl.def("GetActiveFSModel", &GetActiveFSModel);

	mdl.def("active_model", GetActiveGModel);

	py::class_<GModel, std::unique_ptr<GModel, py::nodelete>>(mdl, "Model")
		.def("add_object", &GModel::AddObject);

	py::class_<FSModel, std::unique_ptr<FSModel, py::nodelete>>(mdl, "FSModel")
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

        // --- Object functions ---
        .def("Objects", [] (FSModel& self){return self.GetModel().Objects();})
        .def("Object", [] (FSModel& self, int i){return self.GetModel().Object(i);})
        .def("FindObject", [] (FSModel& self, int i){return self.GetModel().FindObject(i);})
        .def("FindObject", [] (FSModel& self, const string& str){return self.GetModel().FindObject(str);})
        .def("FindObjectIndex", [] (FSModel& self, GObject* obj){return self.GetModel().FindObjectIndex(obj);})
        .def("ReplaceObject", [] (FSModel& self, int i, GObject* obj){return self.GetModel().ReplaceObject(i, obj);})
        .def("ReplaceObject", [] (FSModel& self, GObject* obj1, GObject* obj2){return self.GetModel().ReplaceObject(obj1, obj2);})
        .def("AddObject", [] (FSModel& self, GObject* obj){return self.GetModel().AddObject(obj);})
        .def("RemoveObject", [] (FSModel& self, GObject* obj){return self.GetModel().RemoveObject(obj);})
        .def("InsertObject", [] (FSModel& self, GObject* obj, int i){return self.GetModel().InsertObject(obj, i);})
        ;
    ///////////////// FSModel /////////////////

    ///////////////// GObject /////////////////
	py::class_<GObject, std::unique_ptr<GObject, py::nodelete>>(mdl, "GObject")
		.def("Parts", &GBaseObject::Parts)
		.def("Faces", &GBaseObject::Faces)
		.def("Edges", &GBaseObject::Edges)
		.def("Nodes", &GBaseObject::Nodes)
		.def("Part", [](GObject& self, int i) {return self.Part(i); })
		.def("Face", [](GObject& self, int i) {return self.Face(i); })
		.def("Edge", [](GObject& self, int i) {return self.Edge(i); })
		.def("Node", [](GObject& self, int i) {return self.Node(i); })
		.def("Part", [](GObject& self, int i) {return self.Part(i); })
		.def("GetFEMesh", [](GObject& self) {return self.GetFEMesh(); })
		.def("BuildMesh", &GObject::BuildMesh);
//		.def("AssignMaterial", [](GObject& self, GMaterial* m) { self.AssignMaterial(m->GetID()); });
    ///////////////// GObject /////////////////

    ///////////////// GNode /////////////////
	py::class_<GNode, std::unique_ptr<GNode, py::nodelete>>(mdl, "GNode")
        .def("Type", &GNode::Type)
        .def("SetType", &GNode::SetType)
        .def("LocalPosition", static_cast<vec3d& (GNode::*)()>(&GNode::LocalPosition))
        .def("Position", &GNode::Position)
        .def("MakeRequired", &GNode::MakeRequired)
        ;
    ///////////////// GNode /////////////////
    

	mdl.def("create_box", CreateBox);
	mdl.def("create_mesh_object", CreateMeshObject);

	mdl.def("AddMaterial", AddMaterial);
	mdl.def("AddStep", AddStep);
	mdl.def("ExportFEB", ExportFEB);

	py::class_<FSStep, std::unique_ptr<FSStep, py::nodelete>>(mdl, "FSStep");

	py::class_<GMaterial, std::unique_ptr<GMaterial, py::nodelete>>(mdl, "GMaterial")
		.def("set", [](GMaterial* gm, std::string& paramName, float value) {
			FSMaterial* pm = gm->GetMaterialProperties();
			if (pm) {
				Param* p = pm->GetParam(paramName.c_str());
				if (p) p->SetFloatValue(value);
			}
		});

	mdl.def("mesh_from_curve", meshFromCurve, py::arg("points"), py::arg("radius"),
		py::arg("divisions") = 6, py::arg("segments") = 6, py::arg("ratio") = 0.5);

	py::class_<GDiscreteSpringSet, std::unique_ptr<GDiscreteSpringSet, py::nodelete>>(mdl, "SpringSet")
		.def(py::init(&SpringSet_init))
		.def("add_spring", static_cast<void (GDiscreteSpringSet::*)(int, int)>(&GDiscreteSpringSet::AddElement));

	mdl.def("find_or_make_node", FindOrMakeNode);
	mdl.def("intersect_with_object", IntersectWithObject);
}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
