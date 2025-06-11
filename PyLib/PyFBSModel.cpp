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

bool ExportFEB(std::string& fileName)
{
	CModelDocument* doc = GetActiveDocument();
	FEBioExport4 feb(doc->GetProject());
	return feb.Write(fileName.c_str());
}

bool ExportVTK(std::string& fileName)
{
	CModelDocument* doc = GetActiveDocument();
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

	mdl.def("GetActiveModel", GetActiveModel);
	mdl.def("GetActiveObject", &PyRunContext::GetActiveObject);

	py::class_<FSModel, std::unique_ptr<FSModel, py::nodelete>>(mdl, "Model")
		.def("Clear", &FSModel::Clear)
		.def("Purge", &FSModel::Purge)

		.def("ExportFEB", &ExportFEB)
		.def("ExportVTK", &ExportVTK)

		.def("ImportGeometryFromFile", &ImportGeometryFromFile)

		// functions for adding geometry
		.def("AddBox", [](FSModel& self, double W, double H, double D) {
				GBox* box = new GBox(W, H, D);
				self.GetModel().AddObject(box);
				return box;}, py::return_value_policy::reference)
		
		.def("AddDisc", [](FSModel& self, double R) {
				GDisc* disc = new GDisc(R);
				self.GetModel().AddObject(disc);
				return disc;}, py::return_value_policy::reference)

		.def("AddMeshObject", [](FSModel& self, FSMesh* mesh) {
				GMeshObject* po = new GMeshObject(mesh);
				self.GetModel().AddObject(po);
				return po;
			}, py::return_value_policy::reference)

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

        // --- material functions ---
		.def("AddMaterial", [](FSModel& self, const std::string& name, const std::string& type) { return AddMaterial(self, name, type); })
		.def("GetMaterial", &FSModel::GetMaterial)
        .def("AssignMaterial", static_cast<void (FSModel::*)(GObject*, GMaterial*)>(&FSModel::AssignMaterial))
        .def("ReplaceMaterial", &FSModel::ReplaceMaterial)
        .def("CanDeleteMaterial", &FSModel::CanDeleteMaterial)
        .def("DeleteMaterial", &FSModel::DeleteMaterial)
        .def("InsertMaterial", &FSModel::InsertMaterial)
        .def("Materials", &FSModel::Materials)
        .def("GetMaterialFromID", &FSModel::GetMaterialFromID)
        .def("FindMaterial", &FSModel::FindMaterial)
        .def("GetRigidConnectorFromID", &FSModel::GetRigidConnectorFromID)
        
        // --- Analysis steps ---
		.def("AddStep", [](FSModel& self, const std::string& name, const std::string& type) { return AddStep(self, name, type); })
		.def("Steps", &FSModel::Steps)
        .def("GetStep", &FSModel::GetStep)
        .def("FindStep", &FSModel::FindStep)
        .def("GetStepIndex", &FSModel::GetStepIndex)
        .def("DeleteStep", &FSModel::DeleteStep)
        .def("InsertStep", &FSModel::InsertStep)
        .def("SwapSteps", &FSModel::SwapSteps)
        .def("ReplaceStep", &FSModel::ReplaceStep)
        .def("AssignComponentToStep", &FSModel::AssignComponentToStep)

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

	py::class_<FSStep, std::unique_ptr<FSStep, py::nodelete>>(mdl, "FSStep")
		.def("SetName", &FSObject::SetName);

	py::class_<GMaterial, std::unique_ptr<GMaterial, py::nodelete>>(mdl, "GMaterial")
		.def("set", [](GMaterial* gm, std::string& paramName, float value) {
			FSMaterial* pm = gm->GetMaterialProperties();
			if (pm) {
				Param* p = pm->GetParam(paramName.c_str());
				if (p) p->SetFloatValue(value);
			}
		})
		.def("SetName", &FSObject::SetName);

	py::class_<GDiscreteSpringSet, std::unique_ptr<GDiscreteSpringSet, py::nodelete>>(mdl, "SpringSet")
		.def("AddSpring", static_cast<void (GDiscreteSpringSet::*)(int, int)>(&GDiscreteSpringSet::AddElement));
}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
