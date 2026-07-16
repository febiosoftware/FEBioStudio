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

#include "PyFBSModel.h"

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/PropertyList.h>
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/FSModel.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/FSGroup.h>
#include <GeomLib/GItem.h>
#include <MeshLib/FSItemListBuilder.h>
#include <FEBio/FEBioExport4.h>
#include <MeshIO/VTKExport.h>
#include "PyExceptions.h"
#include "PyRunContext.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FSCurveMesh.h>
#include <FSCore/ClassDescriptor.h>
#include "DocHeaders/PyModelDocs.h"
#include "PyUtil.h"

namespace py = pybind11;

CModelDocument* GetActiveDocument()
{
	return dynamic_cast<CModelDocument*>(PyRunContext::GetDocument());
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

FSModel* GetActiveModel()
{
	CModelDocument* doc = GetActiveDocument();
	return (doc ? doc->GetFSModel() : nullptr);
}

enum class PySelectionItemType
{
	None,
	Node,
	Edge,
	Face,
	Part
};

std::vector<py::handle> SelectionItemsFromPython(py::handle value)
{
	std::vector<py::handle> items;

	if (py::isinstance<GNode>(value) || py::isinstance<GEdge>(value) ||
		py::isinstance<GFace>(value) || py::isinstance<GPart>(value))
	{
		items.push_back(value);
	}
	else if (py::isinstance<py::sequence>(value) && !py::isinstance<py::str>(value))
	{
		py::sequence seq = py::reinterpret_borrow<py::sequence>(value);
		for (py::handle item : seq)
		{
			items.push_back(item);
		}
	}
	else
	{
		throw py::type_error("selection items must be geometry items or a sequence of geometry items");
	}

	if (items.empty())
	{
		throw py::value_error("selection cannot be empty");
	}

	return items;
}

PySelectionItemType SelectionItemType(py::handle item)
{
	if (py::isinstance<GNode>(item)) return PySelectionItemType::Node;
	if (py::isinstance<GEdge>(item)) return PySelectionItemType::Edge;
	if (py::isinstance<GFace>(item)) return PySelectionItemType::Face;
	if (py::isinstance<GPart>(item)) return PySelectionItemType::Part;
	return PySelectionItemType::None;
}

py::object GetItemListSelection(FSHasOneItemList& self)
{
	FSItemListBuilder* selection = self.GetItemList();
	if (selection == nullptr) return py::none();
	return py::cast(selection, py::return_value_policy::reference);
}

void SetItemListSelection(FSHasOneItemList& self, py::handle value)
{
	if (value.is_none())
	{
		self.SetItemList(nullptr);
		return;
	}

	if (!py::isinstance<FSItemListBuilder>(value))
	{
		throw py::type_error("selection must be a named selection");
	}

	FSItemListBuilder* selection = value.cast<FSItemListBuilder*>();
	if (selection && !selection->Supports(self.GetMeshItemType()))
	{
		throw py::type_error("selection is not compatible with this model component");
	}

	self.SetItemList(selection);
}

class PyMaterialList : public PyNamedCollection<FSModel, GMaterial>
{
public:
	PyMaterialList(FSModel* model)
		: PyNamedCollection<FSModel, GMaterial>(
			model,
			[](FSModel* model) { return model->Materials(); },
			[](FSModel* model, int i) { return model->GetMaterial(i); },
			[](FSModel* model, const std::string& name) { return model->FindMaterial(name); },
			"material"
		),
		m_model(model) {}

	GMaterial* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		GMaterial* gmat = m_model->AddMaterial(name, type);
		if (gmat == nullptr) return nullptr;

		FSMaterial* mat = gmat->GetMaterialProperties();
		if (mat && kwargs.empty() == false)
		{
			for (auto item : kwargs)
			{
				std::string key = py::str(item.first);
				py::object value = py::reinterpret_borrow<py::object>(item.second);
				SetDynamicAttribute(*mat, key, value);
			}
		}

		return gmat;
	}

private:
	FSModel* m_model = nullptr;
};

class PyObjectList : public PyNamedCollection<GModel, GObject>
{
public:
	PyObjectList(FSModel* model) : 
		PyNamedCollection<GModel, GObject>(
			&model->GetModel(),
			[](GModel* model) { return model->Objects(); },
			[](GModel* model, int i) { return model->Object(i); },
			[](GModel* model, const std::string& name) { return model->FindObject(name); },
			"object"
		)
	{
		m_geom = &model->GetModel();
	}

	GObject* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		ClassKernel& fbs = *ClassKernel::GetInstance();
		std::unique_ptr<FSObject> obj(fbs.CreateClass(CLASS_OBJECT, type.c_str()));

		GObject* po = dynamic_cast<GObject*>(obj.get());
		if (po == nullptr) return nullptr;

		if (kwargs.empty() == false)
		{
			for (auto item : kwargs)
			{
				std::string key = py::str(item.first);
				py::object value = py::reinterpret_borrow<py::object>(item.second);

				SetDynamicAttribute(*po, key, value);
			}

			po->Update();
		}

		po->SetName(name);
		m_geom->AddObject(po);
		obj.release();
		return po;
	}

	GObject* addMeshObject(const std::string& name)
	{
		GMeshObject* po = new GMeshObject(new FSMesh());
		po->SetName(name);
		m_geom->AddObject(po);
		return po;
	}

	GObject* addCurveMeshObject(const std::string& name)
	{
		GCurveMeshObject* po = new GCurveMeshObject(new FSCurveMesh());
		po->SetName(name);
		m_geom->AddObject(po);
		return po;
	}

	void remove(GObject* po)
	{
		if (po == nullptr) return;
		m_geom->RemoveObject(po); // This does not delete the object!
		delete po;
	}

	GObject* import_file(const std::string& fileName)
	{
		// TODO: I want to make changes here so I don't need the CModelDocument and CMainWindow classes. 
		// I want to be able to import a file directly into the model without needing to go through the document and window classes. 
		// This will make it easier to use this function in a headless mode or in a script.
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
			if (m_geom->Objects())
			{
				po = m_geom->Object(m_geom->Objects() - 1);
			}
		}

		delete fileReader;
		return po;
	}

private:
	GModel* m_geom = nullptr;
};

class PyLoadControllerList : public PyNamedCollection<FSModel, FSLoadController>
{
public:
	PyLoadControllerList(FSModel* model)
		: PyNamedCollection<FSModel, FSLoadController>(
			model,
			[](FSModel* model) { return model->LoadControllers(); },
			[](FSModel* model, int i) { return model->GetLoadController(i); },
			[](FSModel* model, const std::string& name) { return model->FindLoadController(name); },
			"load controller"
		),
		m_model(model) {}

	FSLoadController* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSLoadController* lc = m_model->AddLoadController(name, type);
		if (lc && !kwargs.empty())
		{
			for (auto item : kwargs)
			{
				std::string key = py::str(item.first);
				py::object value = py::reinterpret_borrow<py::object>(item.second);
				SetDynamicAttribute(*lc, key, value);
			}
		}
		return lc;
	}

private:
	FSModel* m_model = nullptr;
};

class PyStepList : public PyNamedCollection<FSModel, FSStep>
{
public:
	PyStepList(FSModel* model)
		: PyNamedCollection<FSModel, FSStep>(
			model,
			[](FSModel* model) { return model->Steps(); },
			[](FSModel* model, int i) { return model->GetStep(i); },
			[](FSModel* model, const std::string& name) { return model->FindStep(name); },
			"step"
		),
		m_model(model) {}

	FSStep* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSStep*step = m_model->AddStep(name, type);
		if (step)
		{
			FEBio::InitDefaultProps(step);
			if (!kwargs.empty())
			{
				for (auto item : kwargs)
				{
					std::string key = py::str(item.first);
					py::object value = py::reinterpret_borrow<py::object>(item.second);
					SetDynamicAttribute(*step, key, value);
				}
			}
		}
		return step;
	}

	FSStep* initial()
	{
		return m_model->GetStep(0);
	}

private:
	FSModel* m_model = nullptr;
};

class PyBCList : public PyNamedCollection<FSStep, FSBoundaryCondition>
{
public:
	PyBCList(FSStep* step)
		: PyNamedCollection<FSStep, FSBoundaryCondition>(
			step,
			[](FSStep* step) { return step->BCs(); },
			[](FSStep* step, int i) { return step->BC(i); },
			[](FSStep* step, const std::string& name) { return step->FindBC(name); },
			"boundary condition"
		),
		m_step(step) {}

	FSBoundaryCondition* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSBoundaryCondition* bc = m_step->AddBC(name, type);
		if (bc && !kwargs.empty())
		{
			for (auto item : kwargs)
			{
				std::string key = py::str(item.first);
				py::object value = py::reinterpret_borrow<py::object>(item.second);
				SetDynamicAttribute(*bc, key, value);
			}
		}
		return bc;
	}

private:
	FSStep* m_step = nullptr;
};

class PySelectionList : public PyNamedCollection<FSModel, FSItemListBuilder>
{
public:
	PySelectionList(FSModel* model) :
		PyNamedCollection<FSModel, FSItemListBuilder>(
			model,
			[](FSModel* model) { return model->GetModel().CountNamedSelections(); },
			[](FSModel* model, int i) { return model->GetModel().NamedSelection(i); },
			[](FSModel* model, const std::string& name) { return model->GetModel().FindNamedSelection(name); },
			"named selection"
		),
		m_model(model) {}

	FSItemListBuilder* add(const std::string& name, py::handle value)
	{
		if (name.empty())
		{
			throw py::value_error("selection name cannot be empty");
		}

		GModel& gmodel = m_model->GetModel();
		if (gmodel.FindNamedSelection(name) != nullptr)
		{
			throw py::value_error("selection already exists: " + name);
		}

		std::vector<py::handle> items = SelectionItemsFromPython(value);
		PySelectionItemType itemType = SelectionItemType(items[0]);

		FSItemListBuilder* selection = nullptr;
		switch (itemType)
		{
		case PySelectionItemType::Node:
			selection = new GNodeList(&gmodel);
			break;
		case PySelectionItemType::Edge:
			selection = new GEdgeList(&gmodel);
			break;
		case PySelectionItemType::Face:
			selection = new GFaceList(&gmodel);
			break;
		case PySelectionItemType::Part:
			selection = new GPartList(&gmodel);
			break;
		default:
			throw py::type_error("selection items must be geometry items");
		}

		try
		{
			for (py::handle item : items)
			{
				if (SelectionItemType(item) != itemType)
				{
					throw py::type_error("selection items must all have the same geometry type");
				}

				switch (itemType)
				{
				case PySelectionItemType::Node:
					selection->add(item.cast<GNode*>()->GetID());
					break;
				case PySelectionItemType::Edge:
					selection->add(item.cast<GEdge*>()->GetID());
					break;
				case PySelectionItemType::Face:
					selection->add(item.cast<GFace*>()->GetID());
					break;
				case PySelectionItemType::Part:
					selection->add(item.cast<GPart*>()->GetID());
					break;
				default:
					break;
				}
			}

			selection->SetName(name);
			gmodel.AddNamedSelection(selection);
			return selection;
		}
		catch (...)
		{
			delete selection;
			throw;
		}
	}

private:
	FSModel* m_model = nullptr;
};

class PyDiscreteObjectList : public PyNamedCollection<FSModel, GDiscreteObject>
{
public:
	PyDiscreteObjectList(FSModel* model) :
		PyNamedCollection<FSModel, GDiscreteObject>(
			model,
			[](FSModel* model) { return model->GetModel().DiscreteObjects(); },
			[](FSModel* model, int i) { return model->GetModel().DiscreteObject(i); },
			[](FSModel* model, const std::string& name) { return model->GetModel().FindDiscreteObject(name); },
			"discrete object"
		),
		m_model(model) {}

	GDiscreteObject* addSpringSet(const std::string& name, const std::string& type)
	{
		GModel& gmodel = m_model->GetModel();

		auto set = new GDiscreteSpringSet(&gmodel);

		if (type == "Linear")
		{
			set->SetMaterial(new FSLinearSpringMaterial(m_model));
		}
		else if (type == "Nonlinear")
		{
			set->SetMaterial(new FSNonLinearSpringMaterial(m_model));
		}
		else if (type == "Hill")
		{
			set->SetMaterial(new FSHillContractileMaterial(m_model));
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

private:
	FSModel* m_model = nullptr;
};

// Initializes the fbs.mdl module
void init_FBSModel(py::module& m)
{
	py::module mdl = m.def_submodule("mdl", "Module used to interact with an FEBio Studio model.");

	// collection wrapper for materials
	py::class_<PyMaterialList>(mdl, "MaterialList")
		.def("__len__", &PyMaterialList::size)
		.def("__getitem__", py::overload_cast<int>(&PyMaterialList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyMaterialList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", &PyMaterialList::iter)
		.def("add", &PyMaterialList::add, py::return_value_policy::reference);

	// collection wrapper for objects
	py::class_<PyObjectList>(mdl, "ObjectList")
		.def("__len__", &PyObjectList::size)
		.def("__getitem__", py::overload_cast<int>(&PyObjectList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyObjectList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyObjectList& self) {
			py::list items;
			for (int i = 0; i < self.size(); ++i)
			{
				items.append(py::cast(self.get(i), py::return_value_policy::reference));
			}
			return py::iter(items);
		})
		.def("add", &PyObjectList::add, py::return_value_policy::reference)
		.def("add_mesh_object", &PyObjectList::addMeshObject, py::return_value_policy::reference, py::arg("name"))
		.def("add_curve_mesh_object", &PyObjectList::addCurveMeshObject, py::return_value_policy::reference, py::arg("name"))
		.def("remove", &PyObjectList::remove)
		.def("import_file", &PyObjectList::import_file, py::return_value_policy::reference)
		;

	// collection wrapper for discrete objects
	py::class_<PyDiscreteObjectList>(mdl, "DiscreteObjectList")
		.def("__len__", &PyDiscreteObjectList::size)
		.def("__getitem__", py::overload_cast<int>(&PyDiscreteObjectList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyDiscreteObjectList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyDiscreteObjectList& self) {
				py::list items;
				for (int i = 0; i < self.size(); ++i)
				{
					items.append(py::cast(self.get(i), py::return_value_policy::reference));
				}
				return py::iter(items);
			})
		.def("add_spring_set", &PyDiscreteObjectList::addSpringSet, py::return_value_policy::reference)
		;

	// collection wrapper for steps
	py::class_<PyStepList>(mdl, "StepList")
		.def("__len__", &PyStepList::size)
		.def("__getitem__", py::overload_cast<int>(&PyStepList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyStepList::get, py::const_), py::return_value_policy::reference)
		.def("add", &PyStepList::add, py::return_value_policy::reference)
		.def_property_readonly("initial", &PyStepList::initial, py::return_value_policy::reference)
		.def("__iter__", [](const PyStepList& self) {
			py::list items;
			for (int i = 0; i < self.size(); ++i)
			{
				items.append(py::cast(self.get(i), py::return_value_policy::reference));
			}
			return py::iter(items);
		});

	// collection wrapper for boundary conditions
	py::class_<PyBCList>(mdl, "BCList")
		.def("__len__", &PyBCList::size)
		.def("__getitem__", py::overload_cast<int>(&PyBCList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyBCList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyBCList& self) {
			py::list items;
			for (int i = 0; i < self.size(); ++i)
			{
				items.append(py::cast(self.get(i), py::return_value_policy::reference));
			}
			return py::iter(items);
		})
		.def("add", &PyBCList::add, py::return_value_policy::reference)
		;

	// collection wrapper for load controllers
	py::class_<PyLoadControllerList>(mdl, "LoadControllerList")
		.def("__len__", &PyLoadControllerList::size)
		.def("__getitem__", py::overload_cast<int>(&PyLoadControllerList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyLoadControllerList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyLoadControllerList& self) {
			py::list items;
			for (int i = 0; i < self.size(); ++i)
			{
				items.append(py::cast(self.get(i), py::return_value_policy::reference));
			}
			return py::iter(items);
		})
		.def("add", &PyLoadControllerList::add, py::return_value_policy::reference)
		;

	// collection wrapper for named selections
	py::class_<PySelectionList>(mdl, "SelectionList")
		.def("__len__", &PySelectionList::size)
		.def("__getitem__", py::overload_cast<int>(&PySelectionList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PySelectionList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", &PySelectionList::iter)
		.def("add", &PySelectionList::add, py::return_value_policy::reference)
		;

	// view wrapper for parameters
	py::class_<PyParameter>(mdl, "Parameter")
		.def_property("value", &PyParameter::value, &PyParameter::setValue)
		.def_property("lc_id", &PyParameter::lcID, &PyParameter::setLCID)
		.def_property("lc", &PyParameter::getLC, &PyParameter::setLC)
		.def_property_readonly("name", &PyParameter::name)
		.def_property_readonly("long_name", &PyParameter::longName)
		.def_property_readonly("unit", &PyParameter::unit);

	py::class_<PyParameterList>(mdl, "ParameterList")
		.def("__len__", &PyParameterList::size)
		.def("__getitem__", py::overload_cast<int>(&PyParameterList::get, py::const_))
		.def("__getitem__", py::overload_cast<const std::string&>(&PyParameterList::get, py::const_))
		.def("__iter__", [](const PyParameterList& self) {
			py::list items;
			for (int i = 0; i < self.size(); ++i)
			{
				items.append(self.get(i));
			}
			return py::iter(items);
		});

	py::class_<PyVec2dList>(mdl, "PointList")
		.def("__len__", &PyVec2dList::size)
		.def("__getitem__", &PyVec2dList::get)
		.def("__iter__", &PyVec2dList::iter)
		.def("add", py::overload_cast<double, double>(&PyVec2dList::add))
		.def("add", py::overload_cast<py::handle>(&PyVec2dList::add))
		.def("clear", &PyVec2dList::clear)
		;

	// minimal property slot wrapper for model components
	py::class_<PyPropertySlot>(mdl, "PropertySlot")
		.def("create", &PyPropertySlot::create)
		.def("clear", &PyPropertySlot::clear)
		.def_property_readonly("is_set", &PyPropertySlot::isSet)
		.def_property_readonly("type_str", &PyPropertySlot::typeStr);

	mdl.def("active_model", GetActiveModel, "Returns the active FSModel instance.", py::return_value_policy::reference);
	mdl.def("active_object", &PyRunContext::GetActiveObject, "Returns the active GObject instance.", py::return_value_policy::reference);

	py::class_<FSModel, std::unique_ptr<FSModel, py::nodelete>>(mdl, "Model", DOC(FSModel))
		.def("clear", &FSModel::Reset, DOC(FSModel, Clear))
		.def("purge", &FSModel::Purge, DOC(FSModel, Purge))

		.def("export_feb", &ExportFEB, "Export the model to a FEBio file.")
		.def("export_vtk", &ExportVTK, "Export the model to a VTK file.")

		.def("add_mesh_object", [](FSModel& self, FSMesh* mesh) {
				GMeshObject* po = new GMeshObject(mesh);
				self.GetModel().AddObject(po);
				return po;
			}, "Add a mesh object to the model.", py::return_value_policy::reference)

		.def("add_curve_mesh_object", [](FSModel& self, FSCurveMesh* mesh) {
				GObject* po = new GCurveMeshObject(mesh);
				self.GetModel().AddObject(po);
				return po;
			}, "Add a curve mesh object to the model.", py::return_value_policy::reference)

		.def_property_readonly(
			"objects",
			[](FSModel& self) { return PyObjectList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"discrete_objects",
			[](FSModel& self) { return PyDiscreteObjectList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"materials",
			[](FSModel& self) { return PyMaterialList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"load_controllers",
			[](FSModel& self) { return PyLoadControllerList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"steps",
			[](FSModel& self) { return PyStepList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"selections",
			[](FSModel& self) { return PySelectionList(&self); },
			py::return_value_policy::reference_internal
		)
		;

	py::class_<FSStep, FSObject, std::unique_ptr<FSStep, py::nodelete>>(mdl, "FSStep", "A class representing an analysis step in the FEBio model.")
		.def_property_readonly("params", [](FSStep& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def("__getattr__", [](FSStep& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSStep& self, const std::string& name, py::object value) { SetDynamicAttribute(self, name, value); })
		.def_property_readonly(
			"bcs",
			[](FSStep& self) { return PyBCList(&self); },
			py::return_value_policy::reference_internal
		)
		;

	py::class_<GMaterial, FSObject, std::unique_ptr<GMaterial, py::nodelete>>(mdl, "Material", "A class representing a material in the FEBio model.")
		.def_property_readonly("id", &GMaterial::GetID, "The unique ID of the material.")
		.def_property_readonly("params", [](GMaterial& self) {
		FSMaterial* material = self.GetMaterialProperties();
		if (material == nullptr)
			throw py::attribute_error("material has no FEBio properties");

		return PyParameterList(material);
			}, py::return_value_policy::reference_internal)
		.def("__getattr__", [](GMaterial& self, const std::string& name) -> py::object {
		FSMaterial* material = self.GetMaterialProperties();
		if (material == nullptr)
			throw py::attribute_error("material has no FEBio properties");

		return GetDynamicAttribute(*material, name);
			})
		.def("__setattr__", [](GMaterial& self, const std::string& name, py::object value) {
			if (name == "name") {
				self.SetName(value.cast<std::string>());
				return;
			}

			FSMaterial* material = self.GetMaterialProperties();
			if (material == nullptr)
				throw py::attribute_error("material has no FEBio properties");

			SetDynamicAttribute(*material, name, value);
		});

	py::class_<FSLoadController, FSObject, std::unique_ptr<FSLoadController>>(mdl, "LoadController")
		.def_property_readonly("id", &FSLoadController::GetID, "The unique ID of the load controller.")
		.def("__getattr__", [](FSLoadController& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSLoadController& self, const std::string& name, py::object value) { SetDynamicAttribute(self, name, value); })
		;

	py::class_<FSBoundaryCondition, FSObject, std::unique_ptr<FSBoundaryCondition>>(mdl, "BoundaryCondition")
		.def_property_readonly("params", [](FSBoundaryCondition& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def_property(
			"selection",
			[](FSBoundaryCondition& self) { return GetItemListSelection(self); },
			[](FSBoundaryCondition& self, py::handle value) { SetItemListSelection(self, value); })
		.def("__getattr__", [](FSBoundaryCondition& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSBoundaryCondition& self, const std::string& name, py::object value) {
			if (name == "selection")
			{
				SetItemListSelection(self, value);
				return;
			}

			SetDynamicAttribute(self, name, value);
		})
		;
}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
