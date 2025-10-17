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
#include "DocHeaders/PyPostDocs.h"

#ifndef PY_EXTERNAL
#include <FEBioStudio/PostDocument.h>
#include "PyRunContext.h"
#include "PyExceptions.h"
#endif

#include <vector>
#include <string>

using namespace std;
using namespace Post;
namespace py = pybind11;

FEPostModel* ReadPlotFile(std::string filename)
{
    FEPostModel* model = new FEPostModel;
    xpltFileReader reader(model);

    reader.Load(filename.c_str());

    model->SetDisplacementField(BUILD_FIELD(DATA_CLASS::NODE_DATA, 0, 0));

    return model;
}

#ifndef PY_EXTERNAL

FEPostModel* GetActivePostModel()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(PyRunContext::GetDocument());
	if (doc == nullptr)
	{
		throw pyGenericExcept("There is no active post document.");
	}
	return (doc ? doc->GetFSModel() : nullptr);
}
#endif

void init_FBSPost(py::module& m)
{
	py::module post = m.def_submodule("post", "Module used to interact with plot files");

    post.def("ReadPlotFile", &ReadPlotFile, "Reads a plot file and returns a FEPostModel object.");
#ifndef PY_EXTERNAL
	post.def("GetActiveModel", &GetActivePostModel, "Returns the active FEPostModel instance.", py::return_value_policy::reference);
#endif

    InitStandardDataFields();
    post.def("AddStandardDataField", pybind11::overload_cast<FEPostModel&, const std::string&>(&AddStandardDataField), 
        "Adds a standard data field to the model.", py::arg("model"), py::arg("dataField"));

	post.def("SurfaceNormalProjection", &Post::SurfaceNormalProjection, "Projects the surface normals onto a specified plane.",
        py::return_value_policy::reference);

	py::class_<Material>(post, "Material", "Material class representing a material in the post-processing model.")
		.def_property("name", &Material::GetName, &Material::SetName, "Name of the material.")
		.def("SetColor", static_cast<void(Material::*)(uint8_t, uint8_t, uint8_t)>(&Material::setColor), "Sets the color of the material.")
		.def("Show", &Material::show, "Shows the material.")
		.def("Hide", &Material::hide, "Hides the material.")
        .def("Enabled", &Material::enabled, "Checks if the material is enabled.")
        ;

	py::class_<FEPostModel>(post, "PostModel", DOC(Post, FEPostModel))
		.def("Materials", &FEPostModel::Materials, DOC(Post, FEPostModel, Materials))
		.def("Material", &FEPostModel::GetMaterial, DOC(Post, FEPostModel, GetMaterial), py::return_value_policy::reference)
        .def("EnableMaterial", &FEPostModel::EnableMaterial, DOC(Post, FEPostModel, EnableMaterial))
        .def("GetFEMesh", &FEPostModel::GetFEMesh, DOC(Post, FEPostModel, GetFEMesh), py::return_value_policy::reference)
        .def("States", &FEPostModel::GetStates, DOC(Post, FEPostModel, GetStates))
        .def("State", &FEPostModel::GetState, DOC(Post, FEPostModel, GetState), py::return_value_policy::reference)
		.def("DataFields", [](FEPostModel& self) { return self.GetDataManager()->DataFields(); }, "Returns a list of data fields in the model.", py::return_value_policy::reference)
		.def("DataField", [](FEPostModel& self, int n) { return *self.GetDataManager()->DataField(n); }, py::return_value_policy::reference, "Returns a specific data field from the model.")
		.def("AddDataField", static_cast<void(FEPostModel::*)(ModelDataField*, const std::string&)>(&FEPostModel::AddDataField), "Adds a data field to the model.")
		.def("GetDataField", [](FEPostModel& self, const std::string& dataField) {
				FEDataManager* dm = self.GetDataManager();
				int index = dm->FindDataField(dataField);
				return *dm->DataField(index);
			}, py::return_value_policy::reference, "Returns a specific data field from the model.")
        .def("GetDataManager", &FEPostModel::GetDataManager, DOC(Post, FEPostModel, GetDataManager), py::return_value_policy::reference)
        .def("Evaluate", [](FEPostModel& self, ModelDataField& field, int component, int time)
            {
                self.Evaluate(field.GetFieldID() | component, time);
                return self.GetState(time);
            }, "Evaluates the model at a specific time step. Returns a reference to the state and updates the values stored in the state's member variables.", 
            py::return_value_policy::reference);

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
		.def_readonly("nodeData", &FEState::m_NODE, DOC(Post, FEState, m_NODE), py::return_value_policy::reference)
		.def_readonly("edgeData", &FEState::m_EDGE, DOC(Post, FEState, m_EDGE), py::return_value_policy::reference)
		.def_readonly("faceData", &FEState::m_FACE, DOC(Post, FEState, m_FACE), py::return_value_policy::reference)
		.def_readonly("elemData", &FEState::m_ELEM, DOC(Post, FEState, m_ELEM), py::return_value_policy::reference)
		.def("NodePosition", [](FEState& self, int index) { return to_vec3d(self.NodePosition(index)); }, "Returns the position of a node at the specified index.", py::return_value_policy::reference)
		.def_readonly("time", &FEState::m_time, DOC(Post, FEState, m_time));
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

	py::class_<FEDistanceMap, ModelDataField, std::unique_ptr<FEDistanceMap, py::nodelete>>(post, "DistanceMap")
		.def(py::init<FEPostModel*, int>())
		.def("SetSelection1", &FEDistanceMap::SetSelection1)
		.def("SetSelection2", &FEDistanceMap::SetSelection2)
		.def("SetSigned", &FEDistanceMap::SetSigned)
		.def("FlipPrimary", &FEDistanceMap::FlipPrimary)
		.def("FlipSecondary", &FEDistanceMap::FlipSecondary)
		.def("SetMethod", &FEDistanceMap::SetMethod)
		.def("Apply", &FEDistanceMap::Apply)
		;
}

#else
void init_FBSPost(pybind11::module_& m) {}
#endif