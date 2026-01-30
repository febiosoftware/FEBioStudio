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
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/FSModel.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <FEBio/FEBioExport4.h>
#include <MeshIO/VTKExport.h>
#include "PyExceptions.h"
#include "PyRunContext.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FSCurveMesh.h>
#include "DocHeaders/PyModelDocs.h"

namespace py = pybind11;

CModelDocument* GetActiveDocument()
{
	return dynamic_cast<CModelDocument*>(PyRunContext::GetDocument());
}

GMaterial* AddMaterial(FSModel& fem, const std::string& name, const std::string& type)
{
	FSMaterial* pm = FEBio::CreateMaterial(type, &fem);
	if (pm)
	{
		GMaterial* gm = new GMaterial;
		gm->SetName(name);
		gm->SetMaterialProperties(pm);
		fem.AddMaterial(gm);
		return gm;
	}
	return nullptr;
}

FSStep* AddStep(FSModel& fem, const std::string& name, const std::string& typeString)
{
	FSStep* step = FEBio::CreateStep(typeString, &fem);
	if (step)
	{
		step->SetName(name);
		fem.AddStep(step);
	}
	return step;
}

GDiscreteSpringSet* AddSpringSet(FSModel& fem, const std::string& name, const std::string& typeStr)
{
	GModel& gmodel = fem.GetModel();

	auto set = new GDiscreteSpringSet(&gmodel);

	if (typeStr == "Linear")
	{
		set->SetMaterial(new FSLinearSpringMaterial(&fem));
	}
	else if (typeStr == "Nonlinear")
	{
		set->SetMaterial(new FSNonLinearSpringMaterial(&fem));
	}
	else if (typeStr == "Hill")
	{
		set->SetMaterial(new FSHillContractileMaterial(&fem));
	}
	else
	{
		delete set;
		return nullptr;
	}

	set->SetName(name);

	gmodel.AddDiscreteObject(set);

	return set;
}

bool ExportFEB(FSModel& fem, std::string& fileName)
{
	CModelDocument* doc = GetActiveDocument();
	assert(doc->GetFSModel() == &fem);
	FEBioExport4 feb(doc->GetProject());
	return feb.Write(fileName.c_str());
}

bool ExportVTK(FSModel& fem, std::string& fileName)
{
	CModelDocument* doc = GetActiveDocument();
	assert(doc->GetFSModel() == &fem);
	VTKExport vtk(doc->GetProject());
	return vtk.Write(fileName.c_str());
}

GObject* ImportGeometryFromFile(FSModel& fem, std::string& fileName)
{
	if (fileName.empty()) return nullptr;

	CModelDocument* doc = GetActiveDocument();
	if (doc == nullptr) return nullptr;

	CMainWindow* wnd = doc->GetMainWindow();
	if (wnd == nullptr) return nullptr;

	FileReader* fileReader = wnd->CreateFileReader(QString::fromStdString(fileName));
	if (fileReader == nullptr) return nullptr;

	GObject* po = nullptr;
	if (fileReader->Load(fileName.c_str()))
	{
		GModel& mdl = fem.GetModel();
		if (mdl.Objects())
		{
			po = mdl.Object(mdl.Objects() - 1);
		}
	}

	delete fileReader;
	return po;
}

FSModel* GetActiveModel()
{
	CModelDocument* doc = GetActiveDocument();
	return (doc ? doc->GetFSModel() : nullptr);
}

// Initializes the fbs.mdl module
void init_FBSModel(py::module& m)
{
	py::module mdl = m.def_submodule("mdl", "Module used to interact with an FEBio Studio model.");

	mdl.def("GetActiveModel", GetActiveModel, "Returns the active FSModel instance.", py::return_value_policy::reference);
	mdl.def("GetActiveObject", &PyRunContext::GetActiveObject, "Returns the active GObject instance.", py::return_value_policy::reference);

	py::class_<FSModel, std::unique_ptr<FSModel, py::nodelete>>(mdl, "Model", DOC(FSModel))
		.def("Clear", &FSModel::Clear, DOC(FSModel, Clear))
		.def("Purge", &FSModel::Purge, DOC(FSModel, Purge))

		.def("ExportFEB", &ExportFEB, "Export the model to a FEBio file.")
		.def("ExportVTK", &ExportVTK, "Export the model to a VTK file.")

		.def("ImportGeometryFromFile", &ImportGeometryFromFile, "Import geometry from a file.")

		// functions for adding geometry
		.def("AddBox", [](FSModel& self, double W, double H, double D) {
				GBox* box = new GBox(W, H, D);
				self.GetModel().AddObject(box);
				return box;}, "Add a box to the model.", py::return_value_policy::reference)

		.def("AddDisc", [](FSModel& self, double R) {
				GDisc* disc = new GDisc(R);
				self.GetModel().AddObject(disc);
				return disc;}, "Add a disc to the model.", py::return_value_policy::reference)

		.def("AddMeshObject", [](FSModel& self, FSMesh* mesh) {
				GMeshObject* po = new GMeshObject(mesh);
				self.GetModel().AddObject(po);
				return po;
			}, "Add a mesh object to the model.", py::return_value_policy::reference)

		.def("AddCurveMeshObject", [](FSModel& self, FSCurveMesh* mesh) {
				GObject* po = new GCurveMeshObject(mesh);
				self.GetModel().AddObject(po);
				return po;
			}, "Add a curve mesh object to the model.", py::return_value_policy::reference)

        // --- functions to delete all components ---
        .def("DeleteAllMaterials", &FSModel::DeleteAllMaterials, DOC(FSModel, DeleteAllMaterials))
        .def("DeleteAllBC", &FSModel::DeleteAllBC, DOC(FSModel, DeleteAllBC))
        .def("DeleteAllLoads", &FSModel::DeleteAllLoads, DOC(FSModel, DeleteAllLoads))
        .def("DeleteAllIC", &FSModel::DeleteAllIC, DOC(FSModel, DeleteAllIC))
        .def("DeleteAllContact", &FSModel::DeleteAllContact, DOC(FSModel, DeleteAllContact))
        .def("DeleteAllConstraints", &FSModel::DeleteAllRigidBCs, DOC(FSModel, DeleteAllConstraints))
        .def("DeleteAllRigidICs", &FSModel::DeleteAllRigidICs, DOC(FSModel, DeleteAllRigidICs))
        .def("DeleteAllRigidLoads", &FSModel::DeleteAllRigidLoads, DOC(FSModel, DeleteAllRigidLoads))
        .def("DeleteAllRigidConnectors", &FSModel::DeleteAllRigidConnectors, DOC(FSModel, DeleteAllRigidConnectors))
        .def("DeleteAllSteps", &FSModel::DeleteAllSteps, DOC(FSModel, DeleteAllSteps))
        .def("DeleteAllLoadControllers", &FSModel::DeleteAllLoadControllers, DOC(FSModel, DeleteAllLoadControllers))
        .def("DeleteAllMeshDataGenerators", &FSModel::DeleteAllMeshDataGenerators, DOC(FSModel, DeleteAllMeshDataGenerators))
        .def("ClearSelections", &FSModel::ClearSelections, DOC(FSModel, ClearSelections))

        // --- material functions ---
		.def("AddMaterial", [](FSModel& self, const std::string& name, const std::string& type) { return AddMaterial(self, name, type); })
		.def("GetMaterial", &FSModel::GetMaterial, DOC(FSModel, GetMaterial))
        .def("AssignMaterial", static_cast<void (FSModel::*)(GObject*, GMaterial*)>(&FSModel::AssignMaterial), DOC(FSModel, AssignMaterial))
        .def("ReplaceMaterial", &FSModel::ReplaceMaterial, DOC(FSModel, ReplaceMaterial))
        .def("CanDeleteMaterial", &FSModel::CanDeleteMaterial, DOC(FSModel, CanDeleteMaterial))
        .def("DeleteMaterial", &FSModel::DeleteMaterial, DOC(FSModel, DeleteMaterial))
        .def("InsertMaterial", &FSModel::InsertMaterial, DOC(FSModel, InsertMaterial))
        .def("Materials", &FSModel::Materials, DOC(FSModel, Materials))
        .def("GetMaterialFromID", &FSModel::GetMaterialFromID, DOC(FSModel, GetMaterialFromID))
        .def("FindMaterial", &FSModel::FindMaterial, DOC(FSModel, FindMaterial))
        .def("GetRigidConnectorFromID", &FSModel::GetRigidConnectorFromID, DOC(FSModel, GetRigidConnectorFromID))

        // --- Analysis steps ---
		.def("AddStep", [](FSModel& self, const std::string& name, const std::string& type) { return AddStep(self, name, type); }, DOC(FSModel, AddStep))
		.def("Steps", &FSModel::Steps, DOC(FSModel, Steps))
        .def("GetStep", &FSModel::GetStep, DOC(FSModel, GetStep))
        .def("FindStep", &FSModel::FindStep, DOC(FSModel, FindStep))
        .def("GetStepIndex", &FSModel::GetStepIndex, DOC(FSModel, GetStepIndex))
        .def("DeleteStep", &FSModel::DeleteStep, DOC(FSModel, DeleteStep))
        .def("InsertStep", &FSModel::InsertStep, DOC(FSModel, InsertStep))
        .def("SwapSteps", &FSModel::SwapSteps, DOC(FSModel, SwapSteps))
        .def("ReplaceStep", &FSModel::ReplaceStep, DOC(FSModel, ReplaceStep))
        .def("AssignComponentToStep", &FSModel::AssignComponentToStep, DOC(FSModel, AssignComponentToStep))

        // --- Object functions ---
        .def("Objects", [] (FSModel& self){return self.GetModel().Objects();})
        .def("Object", [] (FSModel& self, int i){return self.GetModel().Object(i);})
		.def("AddObject", [](FSModel& self, GObject * po) { self.GetModel().AddObject(po); })
        .def("FindObject", [] (FSModel& self, int i){return self.GetModel().FindObject(i);})
        .def("FindObject", [] (FSModel& self, const string& str){return self.GetModel().FindObject(str);})
        .def("FindObjectIndex", [] (FSModel& self, GObject* obj){return self.GetModel().FindObjectIndex(obj);})
        .def("ReplaceObject", [] (FSModel& self, int i, GObject* obj){return self.GetModel().ReplaceObject(i, obj);})
        .def("ReplaceObject", [] (FSModel& self, GObject* obj1, GObject* obj2){return self.GetModel().ReplaceObject(obj1, obj2);})
        .def("RemoveObject", [] (FSModel& self, GObject* obj){return self.GetModel().RemoveObject(obj);})
        .def("InsertObject", [] (FSModel& self, GObject* obj, int i){return self.GetModel().InsertObject(obj, i);})
		.def("DeleteObject", [](FSModel& self, GObject* po) { self.GetModel().RemoveObject(po); delete po; })
		.def("AddSpringSet", [](FSModel& self, const std::string& name, const std::string& type) { return AddSpringSet(self, name, type); })
		;

	py::class_<FSStep, std::unique_ptr<FSStep, py::nodelete>>(mdl, "FSStep", "A class representing an analysis step in the FEBio model.")
		.def("SetName", &FSObject::SetName, "Set the name of the step.");

	py::class_<GMaterial, std::unique_ptr<GMaterial, py::nodelete>>(mdl, "GMaterial", "A class representing a material in the FEBio model.")
		.def("set", [](GMaterial* gm, std::string& paramName, float value) {
			FSMaterial* pm = gm->GetMaterialProperties();
			if (pm) {
				Param* p = pm->GetParam(paramName.c_str());
				if (p) p->SetFloatValue(value);
			}
		})
		.def("SetName", &FSObject::SetName, "Set the name of the material.");

	py::class_<GDiscreteSpringSet, std::unique_ptr<GDiscreteSpringSet, py::nodelete>>(mdl, "SpringSet", "A class representing a set of discrete springs in the FEBio model.")
		.def("AddSpring", static_cast<void (GDiscreteSpringSet::*)(int, int)>(&GDiscreteSpringSet::AddElement), "Add a spring between two nodes by their indices.");
}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
