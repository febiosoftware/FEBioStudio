/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2023 University of Utah, The Trustees of Columbia University in
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
#include "PyFBSPost.h"

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <XPLTLib/xpltFileReader.h>
#include <PostLib/FEPostModel.h>
#include <PostLib/FEState.h>
#include <PostLib/FEDataManager.h>
#include <PostLib/FEDataField.h>
#include <PostLib/FEMeshData.h>
#include <PostLib/constants.h>
#include <PostLib/FEDistanceMap.h>
#include <PostLib/DataFilter.h>
#include <PostLib/FEVTKExport.h>
#include <PostGL/GLModel.h>
#include "DocHeaders/PyPostDocs.h"
#include "PyExceptions.h"
#ifndef PY_EXTERNAL
#include <FEBioStudio/PostDocument.h>
#include "PyRunContext.h"
#include "PyExceptions.h"
#endif
#include "PyUtil.h"

#include <vector>
#include <string>

using namespace std;
using namespace Post;
namespace py = pybind11;

class PyPostMaterialList : public PyNamedCollection<CGLModel, Post::Material>
{
public:
	PyPostMaterialList(CGLModel* model) : PyNamedCollection<CGLModel, Post::Material>
		(model, [](CGLModel* m) { return m->GetFSModel()->Materials(); },
			[](CGLModel* m, int i) { return m->GetFSModel()->GetMaterial(i); },
			[](CGLModel* m, const std::string& name) { return m->GetFSModel()->FindMaterial(name); },
			"material") {}
private:
	CGLModel* m_model = nullptr;
};

class PyPostDataFieldList : public PyNamedCollection<CGLModel, Post::ModelDataField>
{
public:
	PyPostDataFieldList(CGLModel* model) : PyNamedCollection<CGLModel, Post::ModelDataField>
		(model, [](CGLModel* m) { return m->GetFSModel()->GetDataManager()->DataFields(); },
			[](CGLModel* m, int i) { return *m->GetFSModel()->GetDataManager()->DataField(i); },
			[](CGLModel* m, const std::string& name) { 
				int index = m->GetFSModel()->GetDataManager()->FindDataField(name); 
				return (index >= 0) ? *m->GetFSModel()->GetDataManager()->DataField(index) : nullptr; },
			"data field") {}
private:
	CGLModel* m_model = nullptr;
};

class PyPostStateList : public PyIndexedCollection<CGLModel, Post::FEState>
{
public:
	PyPostStateList(CGLModel* model) : PyIndexedCollection<CGLModel, Post::FEState>
		(model, [](CGLModel* m) { return m->GetFSModel()->GetStates(); },
			[](CGLModel* m, int i) { return m->GetFSModel()->GetState(i); },
			"state") {}
private:
	CGLModel* m_model = nullptr;
};

class PyPostPlotList : public PyNamedCollection<CGLModel, Post::CGLPlot>
{
public:
	PyPostPlotList(CGLModel* model) : PyNamedCollection<CGLModel, Post::CGLPlot>
		(model, [](CGLModel* m) { return m->Plots(); },
			[](CGLModel* m, int i) { return m->Plot(i); },
			[](CGLModel* m, const std::string& name) { return m->FindPlot(name); },
			"plot"), m_model(model) {}

	CGLPlot* add(const std::string& name, const std::string& type, py::kwargs kwargs)
	{
		CGLPlot* plot = m_model->AddPlot(name.c_str(), type.c_str());
		if (plot == nullptr)
		{
			throw pyGenericExcept("Failed to add plot.");
		}
		if (!kwargs.empty())
		{
			SetDynamicAttributes(*plot, kwargs);
		}
		return plot;
	}

private:
	CGLModel* m_model = nullptr;
};

// TODO: I'm pretty sure this is a memory leak since no one is deleting the FEPostModel
CGLModel* ReadPlotFile(std::string filename)
{
    FEPostModel* model = new FEPostModel;
    xpltFileReader reader(model);

	if (reader.Load(filename.c_str()) == false)
	{
		throw pyGenericExcept("Failed to read plot file.");
	}

    model->SetDisplacementField(BUILD_FIELD(DATA_CLASS::NODE_DATA, 0, 0));

	CGLModel* glm = new CGLModel(model);
    return glm;
}

// helper function for converting a py::handle to a datafield code
int PyHandleToDataFieldCode(FEPostModel& model, py::handle fieldSpec)
{
	FEDataManager* dm = model.GetDataManager();

	std::string fieldName;
	std::string componentName;

	if (py::isinstance<py::str>(fieldSpec))
	{
		fieldName = fieldSpec.cast<std::string>();
	}
	else if (py::isinstance<py::tuple>(fieldSpec))
	{
		py::tuple t = py::reinterpret_borrow<py::tuple>(fieldSpec);
		if (t.size() != 2)
		{
			throw py::type_error("field must be a string or a (field, component) tuple");
		}

		if (!py::isinstance<py::str>(t[0]) || !py::isinstance<py::str>(t[1]))
		{
			throw py::type_error("field tuple must contain two strings: (field, component)");
		}

		fieldName = t[0].cast<std::string>();
		componentName = t[1].cast<std::string>();
	}
	else
	{
		throw py::type_error("field must be a string or a (field, component) tuple");
	}

	int fieldIndex = dm->FindDataField(fieldName);
	if (fieldIndex < 0)
	{
		throw py::value_error("Data field not found: " + fieldName);
	}

	Post::ModelDataField* field = *dm->DataField(fieldIndex);

	int component = 0;
	if (!componentName.empty())
	{
		component = field->componentCode(componentName, Post::TENSOR_SCALAR);
		if (component == -1)
		{
			throw py::value_error("Invalid component name: " + componentName);
		}
	}

	return field->GetFieldID() | component;
}

#ifndef PY_EXTERNAL

CGLModel* GetActivePostModel()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(PyRunContext::GetDocument());
	if (doc == nullptr)
	{
		throw pyGenericExcept("There is no active post document.");
	}
	return (doc ? doc->GetGLModel() : nullptr);
}
#endif

double PostIntegrateElements(FEPostModel& model, const std::string& elsetName, int fieldCode, int state)
{
	FEState* ps = model.GetState(state);
	if (ps == nullptr)
	{
		throw pyGenericExcept("Invalid state index.");
	}

	FSMesh* mesh = ps->GetFEMesh();
	if (mesh == nullptr) return 0.0;
	FSElemSet* set = mesh->FindFEElemSet(elsetName);
	if (set == nullptr)
	{
		throw pyGenericExcept("Invalid element set name.");
	}

	std::vector<int> elemList = set->CopyItems();

	// update the model's state
	Post::FEDataManager* DM = model.GetDataManager();
	if (DM)
	{
		int dispField = DM->GetFieldCode("displacement");
		if (dispField >= 0)
		{
			if (!model.EvaluateNodalPosition(dispField, state))
			{
				return 0.0;
			}
		}
	}

	if (!model.Evaluate(fieldCode, state))
	{
		return 0.0;
	}

	return Post::IntegrateElems(*mesh, elemList, ps);
}

double PostIntegrateFaces(FEPostModel& model, const std::string& surfName, int fieldCode, int state)
{
	FEState* ps = model.GetState(state);
	if (ps == nullptr)
	{
		throw pyGenericExcept("Invalid state index.");
	}

	FSMesh* mesh = ps->GetFEMesh();
	if (mesh == nullptr) return 0.0;

	FSSurface* surf = mesh->FindFESurface(surfName);
	if (surf == nullptr)
	{
		throw pyGenericExcept("Invalid surface name.");
	}

	std::vector<int> faceList = surf->CopyItems();

	// update the model's state
	Post::FEDataManager* DM = model.GetDataManager();
	if (DM)
	{
		int dispField = DM->GetFieldCode("displacement");
		if (dispField >= 0)
		{
			if (!model.EvaluateNodalPosition(dispField, state))
			{
				return 0.0;
			}
		}
	}

	if (!model.Evaluate(fieldCode, state, true))
	{
		return 0.0;
	}

	return Post::IntegrateFaces(*mesh, faceList, ps);
}

void init_FBSPost(py::module& m)
{
	py::module post = m.def_submodule("post", "Module used to interact with plot files");

	// view wrapper for materials
	py::class_<PyPostMaterialList>(post, "MaterialList")
		.def("__len__", &PyPostMaterialList::size)
		.def("__getitem__", py::overload_cast<int>(&PyPostMaterialList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyPostMaterialList::get, py::const_), py::return_value_policy::reference)
	;

	py::class_<PyPostStateList>(post, "StateList")
		.def("__len__", &PyPostStateList::size)
		.def("__getitem__", &PyPostStateList::get, py::return_value_policy::reference)
		;

	py::class_<PyPostDataFieldList>(post, "DataFieldList")
		.def("__len__", &PyPostDataFieldList::size)
		.def("__getitem__", py::overload_cast<int>(&PyPostDataFieldList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyPostDataFieldList::get, py::const_), py::return_value_policy::reference)
		;

	py::class_<PyPostPlotList>(post, "PlotList")
		.def("__len__", &PyPostPlotList::size)
		.def("__getitem__", py::overload_cast<int>(&PyPostPlotList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyPostPlotList::get, py::const_), py::return_value_policy::reference)
		.def("add", &PyPostPlotList::add, "Adds a new plot to the model.")
		;

    post.def("read_plot_file", &ReadPlotFile, "Reads a plot file and returns a post::PostModel object.");

#ifndef PY_EXTERNAL
	post.def("active_model", &GetActivePostModel, "Returns the active post::PostModel instance.", py::return_value_policy::reference);
#endif

    InitStandardDataFields();

    post.def("AddStandardDataField", [](CGLModel& model, const std::string& dataField) { return AddStandardDataField(*model.GetFSModel(), dataField); },
        "Adds a standard data field to the model.", py::arg("model"), py::arg("dataField"));

	post.def("SurfaceNormalProjection", [](CGLModel& model, ModelDataField& df, const std::string& plane) { return Post::SurfaceNormalProjection(*model.GetFSModel(), &df, plane); },
        "Projects the surface normals onto a specified plane.", py::return_value_policy::reference);

	py::class_<Material>(post, "Material", "Material class representing a material in the post-processing model.")
		.def_property("name", &Material::GetName, &Material::SetName, "Name of the material.")

		.def_readwrite("visible", &Material::bvisible, "Visibility of the material.")
		.def("show", &Material::show, "Shows the material.")
		.def("hide", &Material::hide, "Hides the material.")

		.def_readwrite("enabled", &Material::benable, "Enables or disables the material.")
		.def("enable", &Material::enable, "Enables the material.")
		.def("disable", &Material::disable, "Disables the material.")

		.def_property("color", [](Material& self) { return self.diffuse; }, py::overload_cast<GLColor>(&Material::setColor))
        ;

	py::class_<FEPostModel::PlotObject>(post, "PlotObject")
		.def("Name", &FEPostModel::PlotObject::GetName, "Returns the name of the plot object.")
		.def("GetDataField", &FEPostModel::PlotObject::FindObjectData, "Returns the name of the plot object.")
		;

	py::class_<CGLModel, std::unique_ptr<CGLModel, py::nodelete>>(post, "PostModel")

		// new interface
		.def_property_readonly( "materials"  , [](CGLModel& self) { return PyPostMaterialList (&self); }, py::return_value_policy::reference_internal)
		.def_property_readonly( "states"     , [](CGLModel& self) { return PyPostStateList    (&self); }, py::return_value_policy::reference_internal)
		.def_property_readonly( "data_fields", [](CGLModel& self) { return PyPostDataFieldList(&self); }, py::return_value_policy::reference_internal)
		.def_property_readonly( "plots"      , [](CGLModel& self) { return PyPostPlotList     (&self); }, py::return_value_policy::reference_internal)

		// old interface (TODO: refactor and remove)
		.def("AddDataField", [](CGLModel& self, ModelDataField* df, const std::string& name) { self.GetFSModel()->AddDataField(df, name); }, "Adds a data field to the model.")
        .def("GetDataManager", [](CGLModel& self) { return self.GetFSModel()->GetDataManager(); }, py::return_value_policy::reference)
		.def("GetPlotObject", [](CGLModel& self, const std::string& name) { return self.GetFSModel()->FindPlotObject(name); }, py::return_value_policy::reference)
		.def("EvaluatePlotObject", [](CGLModel& self, FEPostModel::PlotObject* po, ModelDataField* data, int comp, int ntime) { return self.GetFSModel()->EvaluatePlotObject(po, *data, comp, ntime); }, py::return_value_policy::reference)
		;

	py::class_<CGLPlot, FSObject, std::unique_ptr<CGLPlot, py::nodelete>>(post, "Plot")
		.def("__getattr__", [](CGLPlot& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](CGLPlot& self, const std::string& name, py::object value) { SetDynamicAttribute(self, name, value); })
		;

	py::enum_<Data_Tensor_Type>(post, "DataTensorType")
        .value("DATA_SCALAR", Data_Tensor_Type::TENSOR_SCALAR)
        .value("DATA_VECTOR", Data_Tensor_Type::TENSOR_VECTOR)
        .value("DATA_TENSOR2", Data_Tensor_Type::TENSOR_TENSOR2);

	py::class_<FEDataManager>(post, "DataManager", DOC(Post, FEDataManager))
        .def("DataFields", &FEDataManager::DataFields, DOC(Post, FEDataManager, DataFields))
        .def("DataField", [](FEDataManager& self, int i){return *self.DataField(i); }, DOC(Post, FEDataManager, DataField), py::return_value_policy::reference)
        .def("FindDataField", &FEDataManager::FindDataField, DOC(Post, FEDataManager, FindDataField))
        ;

	py::class_<ModelDataField, std::unique_ptr<ModelDataField, py::nodelete>>(post, "ModelDataField", DOC(Post, ModelDataField))
		.def_property("name", &ModelDataField::GetName, &ModelDataField::SetName, "Name of the data field.")
		.def("Components", &ModelDataField::components, DOC(Post, ModelDataField, components))
		.def("ComponentName", &ModelDataField::componentName, DOC(Post, ModelDataField, componentName))
		;

	py::class_<FEState>(post, "State", DOC(Post, FEState))
		.def_readonly("time", &FEState::m_time, DOC(Post, FEState, m_time))

		.def_property_readonly("fe_mesh", &FEState::GetFEMesh, DOC(Post, FEState, GetFEMesh), py::return_value_policy::reference)

		.def("evaluate", [](FEState& self, py::handle fieldRef) -> FEState&
			{
				FEPostModel* model = self.GetFSModel();
				int fieldCode = PyHandleToDataFieldCode(*model, fieldRef);
				model->Evaluate(fieldCode, self.m_id);
				return self;
			}, "Evaluates a specific data field on the state.",
			py::return_value_policy::reference)

		.def_readonly("node_data", &FEState::m_NODE, DOC(Post, FEState, m_NODE), py::return_value_policy::reference)
		.def_readonly("edge_data", &FEState::m_EDGE, DOC(Post, FEState, m_EDGE), py::return_value_policy::reference)
		.def_readonly("face_data", &FEState::m_FACE, DOC(Post, FEState, m_FACE), py::return_value_policy::reference)
		.def_readonly("elem_data", &FEState::m_ELEM, DOC(Post, FEState, m_ELEM), py::return_value_policy::reference)

		.def("integrate_elements", [](FEState& self, const std::string& elsetName, py::handle fieldRef) {
			FEPostModel* fem = self.GetFSModel();
			int fieldCode = PyHandleToDataFieldCode(*fem, fieldRef);
			return PostIntegrateElements(*fem, elsetName, fieldCode, self.m_id); },
			py::arg("elset"), py::arg("field"),
			"Integrates a data field over an element set.")
	
		.def("integrate_faces", [](FEState& self, const std::string& surfName, py::handle fieldRef) {
			FEPostModel* fem = self.GetFSModel();
			int fieldCode = PyHandleToDataFieldCode(*fem, fieldRef);
			return PostIntegrateFaces(*fem, surfName, fieldCode, self.m_id); },
			py::arg("surf"), py::arg("field"),
			"Integrates a data field over a surface.")

		.def("NodePosition", [](FEState& self, int index) { return to_vec3d(self.NodePosition(index)); }, "Returns the position of a node at the specified index.", py::return_value_policy::reference)
		;

	py::class_<NODEDATA>(post, "NODEDATA", DOC(Post, NODEDATA))
        .def("r",  [](NODEDATA& self){return to_vec3d(self.m_rt);}, "Returns the position of the node.")
        .def_readonly("val", &NODEDATA::m_val, DOC(Post, NODEDATA, m_val))
        .def_readonly("tag", &NODEDATA::m_ntag, DOC(Post, NODEDATA, m_ntag));

	py::class_<EDGEDATA>(post, "EDGEDATA", DOC(Post, EDGEDATA))
        .def_readonly("val", &EDGEDATA::m_val, DOC(Post, EDGEDATA, m_val))
        .def_readonly("tag", &EDGEDATA::m_ntag, DOC(Post, EDGEDATA, m_ntag))
        .def_readonly("nodeVals", &EDGEDATA::m_nv, DOC(Post, EDGEDATA, m_nv));

	py::class_<ELEMDATA>(post, "ELEMDATA", DOC(Post, ELEMDATA))
        .def_readonly("val", &ELEMDATA::m_val, DOC(Post, ELEMDATA, m_val))
        .def_readonly("state", &ELEMDATA::m_state, DOC(Post, ELEMDATA, m_state))
        .def_readonly("shellThickness", &ELEMDATA::m_h, DOC(Post, ELEMDATA, m_h));

	py::class_<FACEDATA>(post, "FACEDATA", DOC(Post, FACEDATA))
        .def_readonly("val", &FACEDATA::m_val, DOC(Post, FACEDATA, m_val))
        .def_readonly("tag", &FACEDATA::m_ntag, DOC(Post, FACEDATA, m_ntag));

	py::enum_<Data_Mat3ds_Component>(post, "MAT3DS")
		.value("XX", Data_Mat3ds_Component::MAT3DS_XX)
		.value("YY", Data_Mat3ds_Component::MAT3DS_YY)
		.value("ZZ", Data_Mat3ds_Component::MAT3DS_ZZ)
		.value("XY", Data_Mat3ds_Component::MAT3DS_XY)
		.value("YZ", Data_Mat3ds_Component::MAT3DS_YZ)
		.value("XZ", Data_Mat3ds_Component::MAT3DS_XZ)
		.value("EFFECTIVE", Data_Mat3ds_Component::MAT3DS_EFFECTIVE)
		.value("P1", Data_Mat3ds_Component::MAT3DS_P1)
		.value("P2", Data_Mat3ds_Component::MAT3DS_P2)
		.value("P3", Data_Mat3ds_Component::MAT3DS_P3)
		.value("DEV_P1", Data_Mat3ds_Component::MAT3DS_DEV_P1)
		.value("DEV_P2", Data_Mat3ds_Component::MAT3DS_DEV_P2)
		.value("DEV_P3", Data_Mat3ds_Component::MAT3DS_DEV_P3)
		.value("MAX_SHEAR", Data_Mat3ds_Component::MAT3DS_MAX_SHEAR)
		.value("MAGNITUDE", Data_Mat3ds_Component::MAT3DS_MAGNITUDE)
		.value("I1", Data_Mat3ds_Component::MAT3DS_I1)
		.value("I2", Data_Mat3ds_Component::MAT3DS_I2)
		.value("I3", Data_Mat3ds_Component::MAT3DS_I3)
		;

/*	py::class_<FEDistanceMap, ModelDataField, std::unique_ptr<FEDistanceMap, py::nodelete>>(post, "DistanceMap")
		.def(py::init<CGLModel*, int>())
        .def("Init", &FEDistanceMap::Init)
		.def("SetSelection1", &FEDistanceMap::SetSelection1)
		.def("SetSelection2", &FEDistanceMap::SetSelection2)
		.def("SetSigned", &FEDistanceMap::SetSigned)
		.def("FlipPrimary", &FEDistanceMap::FlipPrimary)
		.def("FlipSecondary", &FEDistanceMap::FlipSecondary)
		.def("SetMethod", &FEDistanceMap::SetMethod)
		.def("Apply", &FEDistanceMap::Apply)
        .def("ApplyState", &FEDistanceMap::ApplyState)
		;
*/
	py::class_<FEVTKExport>(post, "VTKExport", "class for exporting post-model to vtk file")
		.def(py::init<>())
		.def_readwrite("export_all_states", &FEVTKExport::m_bwriteAllStates)
		.def_readwrite("export_selected_elements_only", &FEVTKExport::m_bselElemsOnly)
		.def_readwrite("write_series_file", &FEVTKExport::m_bwriteSeriesFile)
		.def_readwrite("write_part_ids", &FEVTKExport::m_bwritePartIDs)
		.def("save", [](FEVTKExport& self, CGLModel& model, const char* szfile) { 
			bool b = self.Save(*model.GetFSModel(), szfile); 
			if (!b) throw pyGenericExcept("Failed to save VTK file.");
			})
		;
}

#else
void init_FBSPost(pybind11::module_& m) {}
#endif