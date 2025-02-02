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

FEPostModel* readPlotFile(std::string filename)
{
    FEPostModel* model = new FEPostModel;
    xpltFileReader reader(model);

    reader.Load(filename.c_str());

    model->SetDisplacementField(BUILD_FIELD(DATA_CLASS::NODE_DATA, 0, 0));

    return model;
}

#ifndef PY_EXTERNAL

FEPostModel* GetActiveModel()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(PyRunContext::GetDocument());
	if (doc == nullptr)
	{
		throw pyGenericExcept("There is no active post document.");
	}
	return (doc ? doc->GetFSModel() : nullptr);
}
#endif

ModelDataField* runDistanceMap(FEPostModel* model, std::vector<int>& sel1, std::vector<int>& sel2, bool sign)
{
    FEDistanceMap* distanceMap = new FEDistanceMap(model, 0);
    model->AddDataField(distanceMap);
    
    distanceMap->SetSelection1(sel1);
    distanceMap->SetSelection2(sel2);
    distanceMap->SetSigned(sign);

    distanceMap->Apply();

    return distanceMap;
}

void init_FBSPost(py::module& m)
{
	py::module post = m.def_submodule("post", "Module used to interact with plot files");

    post.def("readPlotFile", &readPlotFile);
    post.def("runDistanceMap", &runDistanceMap);

    InitStandardDataFields();
    post.def("AddStandardDataField", pybind11::overload_cast<FEPostModel&, const std::string&>(&AddStandardDataField));

	py::class_<Material>(post, "Material")
		.def("setColor", &Material::setColor)
		.def("name", &Material::GetName)
		.def("show", &Material::show)
		.def("hide", &Material::hide);

	py::class_<FEPostModel>(post, "FEPostModel")
		.def("Materials", &FEPostModel::Materials)
		.def("GetMaterial", &FEPostModel::GetMaterial, py::return_value_policy::reference)
        .def("GetFEMesh", &FEPostModel::GetFEMesh, py::return_value_policy::reference)
        .def("GetStates", &FEPostModel::GetStates)
        .def("GetState", &FEPostModel::GetStates, py::return_value_policy::reference)
        .def("GetDataManager", &FEPostModel::GetDataManager, py::return_value_policy::reference)
        .def("Evaluate", [](FEPostModel& self, ModelDataField& field, int component, int time)
            {
                self.Evaluate(field.GetFieldID() | component, time);

                return self.GetState(time);
            }, py::return_value_policy::reference);

#ifndef PY_EXTERNAL
	post.def("GetActiveModel", &GetActiveModel, py::return_value_policy::reference);
#endif

	py::enum_<Data_Tensor_Type>(post, "DataTensorType")
        .value("DATA_SCALAR", Data_Tensor_Type::TENSOR_SCALAR)
        .value("DATA_VECTOR", Data_Tensor_Type::TENSOR_VECTOR)
        .value("DATA_TENSOR2", Data_Tensor_Type::TENSOR_TENSOR2);

	py::class_<FEDataManager>(post, "FEDataManager")
        .def("datafields", &FEDataManager::DataFields)
        .def("datafield", [](FEDataManager& self, int i){return *self.DataField(i); }, py::return_value_policy::reference)
        .def("find_datafield", &FEDataManager::FindDataField);

	py::class_<ModelDataField, std::unique_ptr<ModelDataField, py::nodelete>>(post, "ModelDataField")
        .def("components", &ModelDataField::components)
        .def("component_name", &ModelDataField::componentName)
        .def("name", &ModelDataField::GetName)
        .def("set_name", &ModelDataField::SetName);

	py::class_<FEState>(post, "State")
        .def_readonly("NodeData", &FEState::m_NODE, py::return_value_policy::reference)
        .def_readonly("EdgeData", &FEState::m_EDGE, py::return_value_policy::reference)
        .def_readonly("FaceData", &FEState::m_FACE, py::return_value_policy::reference)
        .def_readonly("ElemData", &FEState::m_ELEM, py::return_value_policy::reference)
        .def("NodePosition", [](FEState& self, int index) { return to_vec3d(self.NodePosition(index));});

	py::class_<NODEDATA>(post, "NODEDATA")
        .def("r",  [](NODEDATA& self){return to_vec3d(self.m_rt);})
        .def_readonly("val", &NODEDATA::m_val)
        .def_readonly("tag", &NODEDATA::m_ntag);

	py::class_<EDGEDATA>(post, "EDGEDATA")
        .def_readonly("val", &EDGEDATA::m_val)
        .def_readonly("tag", &EDGEDATA::m_ntag)
        .def_readonly("nodeVals", &EDGEDATA::m_nv);

	py::class_<ELEMDATA>(post, "ELEMDATA")
        .def_readonly("val", &ELEMDATA::m_val)
        .def_readonly("state", &ELEMDATA::m_state)
        .def_readonly("shellThickness", &ELEMDATA::m_h);

	py::class_<FACEDATA>(post, "FACEDATA")
        .def_readonly("val", &FACEDATA::m_val)
        .def_readonly("tag", &FACEDATA::m_ntag);

}

#else
void init_FBSPost(pybind11::module_& m) {}
#endif