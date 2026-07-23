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

#ifndef PY_EXTERNAL
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/PropertyList.h>
#endif
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/FSModel.h>
#include <FEMLib/FEInterface.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FEMeshAdaptor.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GGroup.h>
#include <GeomLib/FSGroup.h>
#include <GeomLib/GItem.h>
#include <MeshLib/FSItemListBuilder.h>
#include <FEBio/FEBioExport4.h>
#include <MeshIO/VTKExport.h>
#include "PyRunContext.h"
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FSCurveMesh.h>
#include <FEMLib/LogDataSettings.h>
#include <FSCore/ClassDescriptor.h>
#include "DocHeaders/PyModelDocs.h"
#include "PyUtil.h"

namespace py = pybind11;

bool ExportFEB(FSModel& fem, std::string& fileName)
{
	FEBioExport4 feb(fem);
	return feb.Write(fileName.c_str());
}

bool ExportVTK(FSModel& fem, std::string& fileName)
{
	VTKExport vtk(fem);
	return vtk.Write(fileName.c_str());
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

FSItemListBuilder* PyValueToItemList(py::handle value)
{
	if (value.is_none()) return nullptr;
	if (!py::isinstance<FSItemListBuilder>(value))
	{
		throw py::type_error("selection must be a named selection");
	}
	FSItemListBuilder* selection = value.cast<FSItemListBuilder*>();
	return selection;
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
		if (gmat == nullptr)
		{
			throw py::type_error("can't create material of type \"" + type + "\"");
		}

		FSMaterial* mat = gmat->GetMaterialProperties();
		if (mat) SetDynamicAttributes(*mat, kwargs);

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
		if (po == nullptr)
		{
			throw py::type_error("can't create object of type \"" + type + "\"");
		}

		if (kwargs.empty() == false)
		{
			SetDynamicAttributes(*po, kwargs);
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

#ifndef PY_EXTERNAL
	GObject* import_file(const std::string& fileName)
	{
		// TODO: I want to make changes here so I don't need the CModelDocument and CMainWindow classes. 
		// I want to be able to import a file directly into the model without needing to go through the document and window classes. 
		// This will make it easier to use this function in a headless mode or in a script.
		if (fileName.empty()) return nullptr;

		CModelDocument* doc = dynamic_cast<CModelDocument*>(PyRunContext::GetDocument());
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
#endif

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
		if (lc == nullptr)
		{
			throw py::type_error("can't create load controller of type \"" + type + "\"");
		}
		if (lc) SetDynamicAttributes(*lc, kwargs);
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
		if (step == nullptr)
		{
			throw py::type_error("can't create step of type \"" + type + "\"");
		}

		FEBio::InitDefaultProps(step);
		SetDynamicAttributes(*step, kwargs);
		
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
		if (bc == nullptr)
		{
			throw py::type_error("can't create boundary condition of type \"" + type + "\"");
		}
		SetDynamicAttributes(*bc, kwargs);
		return bc;
	}

private:
	FSStep* m_step = nullptr;
};

class PyLoadList : public PyNamedCollection<FSStep, FSLoad>
{
public:
	PyLoadList(FSStep* step)
		: PyNamedCollection<FSStep, FSLoad>(
			step,
			[](FSStep* step) { return step->Loads(); },
			[](FSStep* step, int i) { return step->Load(i); },
			[](FSStep* step, const std::string& name) { return step->FindLoad(name); },
			"load"
		),
		m_step(step) {}

	FSLoad* addNodalLoad(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSLoad* load = m_step->AddNodalLoad(name, type);
		if (load == nullptr)
		{
			throw py::type_error("can't create nodal load of type \"" + type + "\"");
		}
		SetDynamicAttributes(*load, kwargs);
		return load;
	}

	FSLoad* addSurfaceLoad(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSLoad* load = m_step->AddSurfaceLoad(name, type);
		if (load == nullptr)
		{
			throw py::type_error("can't create surface load of type \"" + type + "\"");
		}
		SetDynamicAttributes(*load, kwargs);
		return load;
	}

	FSLoad* addBodyLoad(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSLoad* load = m_step->AddBodyLoad(name, type);
		if (load == nullptr)
		{
			throw py::type_error("can't create body load of type \"" + type + "\"");
		}
		SetDynamicAttributes(*load, kwargs);
		return load;
	}

private:
	FSStep* m_step = nullptr;
};

class PyICList : public PyNamedCollection<FSStep, FSInitialCondition>
{
public:
	PyICList(FSStep* step)
		: PyNamedCollection<FSStep, FSInitialCondition>(
			step,
			[](FSStep* step) { return step->ICs(); },
			[](FSStep* step, int i) { return step->IC(i); },
			[](FSStep* step, const std::string& name) { return step->FindIC(name); },
			"initial condition"
		),
		m_step(step) {}

	FSInitialCondition* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSInitialCondition* ic = m_step->AddIC(name, type);
		if (ic == nullptr)
		{
			throw py::type_error("can't create initial condition of type \"" + type + "\"");
		}
		SetDynamicAttributes(*ic, kwargs);
		return ic;
	}

private:
	FSStep* m_step = nullptr;
};

class PyContactList : public PyNamedCollection<FSStep, FSPairedInterface>
{
public:
	PyContactList(FSStep* step)
		: PyNamedCollection<FSStep, FSPairedInterface>(
			step,
			[](FSStep* step) { return step->Interfaces(); },
			[](FSStep* step, int i) { return dynamic_cast<FSPairedInterface*>(step->Interface(i)); },
			[](FSStep* step, const std::string& name) { return dynamic_cast<FSPairedInterface*>(step->FindInterface(name)); },
			"interface"
		),
		m_step(step) {}

	FSPairedInterface* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSInterface* iface = m_step->AddInterface(name, type);

		FSPairedInterface* paired = dynamic_cast<FSPairedInterface*>(iface);
		if (paired == nullptr)
		{
			if (iface) delete iface;
			throw py::type_error("interface is not a paired interface");
		}

		SetDynamicAttributes(*paired, kwargs);
		return paired;
	}

private:
	FSStep* m_step = nullptr;
};

class PyConstraintList : public PyNamedCollection<FSStep, FSModelConstraint>
{
public:
	PyConstraintList(FSStep* step)
		: PyNamedCollection<FSStep, FSModelConstraint>(
			step,
			[](FSStep* step) { return step->Constraints(); },
			[](FSStep* step, int i) { return step->Constraint(i); },
			[](FSStep* step, const std::string& name) { return step->FindConstraint(name); },
			"constraint"
		),
		m_step(step) {}

	FSModelConstraint* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSModelConstraint* constraint = m_step->AddConstraint(name, type);
		if (constraint == nullptr)
		{
			throw py::type_error("can't create constraint of type \"" + type + "\"");
		}
		SetDynamicAttributes(*constraint, kwargs);
		return constraint;
	}

private:
	FSStep* m_step = nullptr;
};

class PyMeshAdaptorList : public PyNamedCollection<FSStep, FSMeshAdaptor>
{
public:
	PyMeshAdaptorList(FSStep* step)
		: PyNamedCollection<FSStep, FSMeshAdaptor>(
			step,
			[](FSStep* step) { return step->MeshAdaptors(); },
			[](FSStep* step, int i) { return step->MeshAdaptor(i); },
			[](FSStep* step, const std::string& name) { return step->FindMeshAdaptor(name); },
			"mesh adaptor"
		),
		m_step(step) {}

	FSMeshAdaptor* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		FSMeshAdaptor* adaptor = m_step->AddMeshAdaptor(name, type);
		if (adaptor == nullptr)
		{
			throw py::type_error("can't create mesh adaptor of type \"" + type + "\"");
		}
		SetDynamicAttributes(*adaptor, kwargs);
		return adaptor;
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
			throw py::type_error("can't create spring set of type \"" + type + "\"");
		}

		set->SetName(name);

		gmodel.AddDiscreteObject(set);
		return set;
	}

private:
	FSModel* m_model = nullptr;
};

class PyPlotVariableList : public PyNamedCollection<FSModel, CPlotVariable>
{
public:
	PyPlotVariableList(FSModel* model) :
		PyNamedCollection<FSModel, CPlotVariable>(
			model,
			[](FSModel* model) { return model->GetPlotDataSettings().PlotVariables(); },
			[](FSModel* model, int i) { return &model->GetPlotDataSettings().PlotVariable(i); },
			[](FSModel* model, const std::string& name) { return model->GetPlotDataSettings().FindVariable(name); },
			"plot variable"
		),
		m_model(model) {}

	CPlotVariable* add(const std::string& name)
	{
		CPlotDataSettings& plt = m_model->GetPlotDataSettings();
		CPlotVariable* var = plt.FindVariable(name);
		if (var == nullptr)
		{
			// see if we can create a new plot variable
			std::vector<FEBio::FEBioClassInfo> pltClasses = FEBio::FindAllClasses(m_model->GetModule(), FEPLOTDATA_ID);
			for (auto& cls : pltClasses)
			{
				if (std::string(cls.sztype) == name)
				{
					DOMAIN_TYPE dom = DOMAIN_MESH;
					if (cls.baseClassId == FEBio::GetBaseClassIndex("FEPlotSurfaceData")) dom = DOMAIN_SURFACE;
					if (cls.baseClassId == FEBio::GetBaseClassIndex("FEPlotDomainData")) dom = DOMAIN_PART;

					var = plt.AddPlotVariable(name, true, true, dom);
					break;
				}
			}

			if (var == nullptr)
			{
				throw py::value_error("plot variable does not exist: " + name);
			}
		}
		return var;
	}

private:
	FSModel* m_model = nullptr;
};

class PyLogVariableList : public PyIndexedCollection<FSModel, FSLogData>
{
public:
	PyLogVariableList(FSModel* model) :
		PyIndexedCollection<FSModel, FSLogData>(
			model,
			[](FSModel* model) { return model->GetLogDataSettings().LogDataSize(); },
			[](FSModel* model, int i) { return &model->GetLogDataSettings().LogData(i); },
			"log variable"
		),
		m_model(model) {}

private:
	FSModel* m_model = nullptr;
};

// Initializes the fbs.mdl module
void init_FBSModel(py::module& m)
{
	py::module mdl = m.def_submodule("model", "Module used to interact with an FEBio Studio model.");

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
#ifndef PY_EXTERNAL
		.def("import_file", &PyObjectList::import_file, py::return_value_policy::reference)
#endif
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

	// collection wrapper for loads
	py::class_<PyLoadList>(mdl, "LoadList")
		.def("__len__", &PyLoadList::size)
		.def("__getitem__", py::overload_cast<int>(&PyLoadList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyLoadList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyLoadList& self) {
		py::list items;
		for (int i = 0; i < self.size(); ++i)
		{
			items.append(py::cast(self.get(i), py::return_value_policy::reference));
		}
		return py::iter(items);
			})
		.def("add_nodal_load"  , &PyLoadList::addNodalLoad  , py::return_value_policy::reference)
		.def("add_surface_load", &PyLoadList::addSurfaceLoad, py::return_value_policy::reference)
		.def("add_body_load"   , &PyLoadList::addBodyLoad   , py::return_value_policy::reference)
		;

	// collection wrapper for initial conditions
	py::class_<PyICList>(mdl, "ICList")
		.def("__len__", &PyICList::size)
		.def("__getitem__", py::overload_cast<int>(&PyICList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyICList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyICList& self) {
		py::list items;
		for (int i = 0; i < self.size(); ++i)
		{
			items.append(py::cast(self.get(i), py::return_value_policy::reference));
		}
		return py::iter(items);
			})
		.def("add", &PyICList::add, py::return_value_policy::reference)
		;


	// collection wrapper for contact definitions
	py::class_<PyContactList>(mdl, "ContactList")
		.def("__len__", &PyContactList::size)
		.def("__getitem__", py::overload_cast<int>(&PyContactList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyContactList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyContactList& self) {
		py::list items;
		for (int i = 0; i < self.size(); ++i)
		{
			items.append(py::cast(self.get(i), py::return_value_policy::reference));
		}
		return py::iter(items);
			})
		.def("add", &PyContactList::add, py::return_value_policy::reference)
		;

	// collection wrapper for nonlinear constraints
	py::class_<PyConstraintList>(mdl, "ConstraintList")
		.def("__len__", &PyConstraintList::size)
		.def("__getitem__", py::overload_cast<int>(&PyConstraintList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyConstraintList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyConstraintList& self) {
		py::list items;
		for (int i = 0; i < self.size(); ++i)
		{
			items.append(py::cast(self.get(i), py::return_value_policy::reference));
		}
		return py::iter(items);
			})
		.def("add", &PyConstraintList::add, py::return_value_policy::reference)
		;

	// collection wrapper for mesh adaptors
	py::class_<PyMeshAdaptorList>(mdl, "MeshAdaptorList")
		.def("__len__", &PyMeshAdaptorList::size)
		.def("__getitem__", py::overload_cast<int>(&PyMeshAdaptorList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyMeshAdaptorList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyMeshAdaptorList& self) {
		py::list items;
		for (int i = 0; i < self.size(); ++i)
		{
			items.append(py::cast(self.get(i), py::return_value_policy::reference));
		}
		return py::iter(items);
			})
		.def("add", &PyMeshAdaptorList::add, py::return_value_policy::reference)
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

	py::class_<PyPlotVariableList>(mdl, "PlotVariableList")
		.def("__len__", &PyPlotVariableList::size)
		.def("__getitem__", py::overload_cast<int>(&PyPlotVariableList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyPlotVariableList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyPlotVariableList& self) {
				py::list items;
				for (int i = 0; i < self.size(); ++i)
				{
					items.append(py::cast(self.get(i), py::return_value_policy::reference));
				}
				return py::iter(items);
			})
		.def("add", &PyPlotVariableList::add, py::return_value_policy::reference)
		;

	py::class_<PyLogVariableList>(mdl, "LogVariableList")
		.def("__len__", &PyLogVariableList::size)
		.def("__getitem__", py::overload_cast<int>(&PyLogVariableList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", [](const PyLogVariableList& self) {
				py::list items;
				for (int i = 0; i < self.size(); ++i)
				{
					items.append(py::cast(self.get(i), py::return_value_policy::reference));
				}
				return py::iter(items);
			})
		;

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
		.def_property_readonly(
			"plot_variables",
			[](FSModel& self) { return PyPlotVariableList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"log_variables",
			[](FSModel& self) { return PyLogVariableList(&self); },
			py::return_value_policy::reference_internal
		)
		;

	py::class_<FSStep, FSObject, std::unique_ptr<FSStep, py::nodelete>>(mdl, "Step", "A class representing an analysis step in the FEBio model.")
		.def_property_readonly("params", [](FSStep& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def("__getattr__", [](FSStep& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSStep& self, const std::string& name, py::object value) { SetDynamicAttribute(self, name, value); })
		.def_property_readonly(
			"boundary_conditions",
			[](FSStep& self) { return PyBCList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"loads",
			[](FSStep& self) { return PyLoadList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"initial_conditions",
			[](FSStep& self) { return PyICList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"contacts",
			[](FSStep& self) { return PyContactList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"constraints",
			[](FSStep& self) { return PyConstraintList(&self); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly(
			"mesh_adaptors",
			[](FSStep& self) { return PyMeshAdaptorList(&self); },
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

	py::class_<FSLoad, FSObject, std::unique_ptr<FSLoad>>(mdl, "Load")
		.def_property_readonly("params", [](FSLoad& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def_property(
			"selection",
			[](FSLoad& self) { return GetItemListSelection(self); },
			[](FSLoad& self, py::handle value) { SetItemListSelection(self, value); })
		.def("__getattr__", [](FSLoad& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSLoad& self, const std::string& name, py::object value) {
				if (name == "selection")
				{
					SetItemListSelection(self, value);
					return;
				}

				SetDynamicAttribute(self, name, value);
				}
			)
		;

	py::class_<FSInitialCondition, FSObject, std::unique_ptr<FSInitialCondition>>(mdl, "InitialCondition")
		.def_property_readonly("params", [](FSInitialCondition& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def_property(
			"selection",
			[](FSInitialCondition& self) { return GetItemListSelection(self); },
			[](FSInitialCondition& self, py::handle value) { SetItemListSelection(self, value); })
		.def("__getattr__", [](FSInitialCondition& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSInitialCondition& self, const std::string& name, py::object value) {
			if (name == "selection")
			{
				SetItemListSelection(self, value);
				return;
			}

			SetDynamicAttribute(self, name, value);
		})
		;

	py::class_<FSPairedInterface, FSObject, std::unique_ptr<FSPairedInterface>>(mdl, "PairedInterface")
		.def_property_readonly("params", [](FSPairedInterface& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def_property(
			"primary_selection",
			[](FSPairedInterface& self) { return self.GetPrimarySurface(); },
			[](FSPairedInterface& self, FSItemListBuilder* selection) { self.SetPrimarySurface(selection); })
		.def_property(
			"secondary_selection",
			[](FSPairedInterface& self) { return self.GetSecondarySurface(); },
			[](FSPairedInterface& self, FSItemListBuilder* selection) { self.SetSecondarySurface(selection); })
		.def("__getattr__", [](FSPairedInterface& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSPairedInterface& self, const std::string& name, py::object value) {
			if (name == "primary_selection")
			{
				self.SetPrimarySurface(PyValueToItemList(value));
				return;
			}
			else if (name == "secondary_selection")
			{
				self.SetSecondarySurface(PyValueToItemList(value));
				return;
			}

			SetDynamicAttribute(self, name, value);
		})
		;

	py::class_<FSModelConstraint, FSObject, std::unique_ptr<FSModelConstraint>>(mdl, "ModelConstraint")
		.def_property_readonly("params", [](FSModelConstraint& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def("__getattr__", [](FSModelConstraint& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSModelConstraint& self, const std::string& name, py::object value) {
		SetDynamicAttribute(self, name, value);
			})
		;

	py::class_<FSMeshAdaptor, FSObject, std::unique_ptr<FSMeshAdaptor>>(mdl, "MeshAdaptor")
		.def_property_readonly("params", [](FSMeshAdaptor& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def_property(
			"selection",
			[](FSMeshAdaptor& self) { return GetItemListSelection(self); },
			[](FSMeshAdaptor& self, py::handle value) { SetItemListSelection(self, value); })
		.def("__getattr__", [](FSMeshAdaptor& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FSMeshAdaptor& self, const std::string& name, py::object value) {
				if (name == "selection")
				{
					SetItemListSelection(self, value);
					return;
				}

				SetDynamicAttribute(self, name, value);
			})
		;

	py::class_<CPlotVariable, std::unique_ptr<CPlotVariable, py::nodelete>>(mdl, "PlotVariable")
		.def_property_readonly("name", &CPlotVariable::name);

	py::class_<FSLogData, std::unique_ptr<FSLogData, py::nodelete>>(mdl, "LogVariable");
}
#else
void init_FBSModel(pybind11::module_& m) {}
#endif
