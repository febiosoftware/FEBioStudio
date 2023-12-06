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

#include <XPLTLib/xpltFileReader.h>
#include <PostLib/FEPostModel.h>
#include <PostLib/FEPostMesh.h>
#include <PostLib/FEGroup.h>
#include <PostLib/FEState.h>
#include <PostLib/FEDataManager.h>
#include <PostLib/FEDataField.h>
#include <PostLib/FEMeshData.h>
#include <PostLib/constants.h>

#include <iostream>

#include <vector>
#include <string>

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace Post;

FEPostModel* readPlotFile(std::string filename)
{
    FEPostModel* model = new FEPostModel;
    xpltFileReader reader(model);

    reader.Load(filename.c_str());

    return model;
}

void init_FBSPost(pybind11::module& m)
{
    pybind11::module post = m.def_submodule("post", "Module used to interact with plot files");

    post.def("readPlotFile", &readPlotFile);

    pybind11::class_<FEPostModel>(post, "FEPostModel")
        .def("GetFEMesh", &FEPostModel::GetFEMesh, pybind11::return_value_policy::reference)
        .def("GetStates", &FEPostModel::GetStates)
        .def("GetState", &FEPostModel::GetStates, pybind11::return_value_policy::reference)
        .def("GetDataManager", &FEPostModel::GetDataManager, pybind11::return_value_policy::reference)
        .def("Evaluate", [](FEPostModel& self, ModelDataField& field, int component, int time)
            {
                self.Evaluate(field.GetFieldID() | component, time);

                return self.GetState(time);
            }, pybind11::return_value_policy::reference);

    pybind11::class_<FEPostMesh>(post, "FEPostMesh")
        .def("Surfaces", &FEPostMesh::Surfaces)
        .def("Surface", &FEPostMesh::Surface, pybind11::return_value_policy::reference);

    pybind11::class_<Post::FSSurface>(post, "FESurface")
        .def_readonly("Faces", &Post::FSSurface::m_Face)
        .def("GetName", &Post::FSSurface::GetName);

    pybind11::enum_<Data_Tensor_Type>(post, "DataTensorType")
        .value("DATA_SCALAR", DATA_SCALAR)
        .value("DATA_VECTOR", DATA_VECTOR)
        .value("DATA_TENSOR2", DATA_TENSOR2);

    pybind11::class_<FEDataManager>(post, "FEDataManager")
        .def("DataFields", &FEDataManager::DataFields)
        .def("DataField", [](FEDataManager& self, int i){return *self.DataField(i); }, pybind11::return_value_policy::reference)
        .def("FindDataField", &FEDataManager::FindDataField);

    pybind11::class_<ModelDataField>(post, "ModelDataField")
        .def("components", &ModelDataField::components)
        .def("componentName", &ModelDataField::componentName)
        .def("GetName", &ModelDataField::GetName);

    pybind11::class_<FEState>(post, "FEState")
        .def_readonly("NodeData", &FEState::m_NODE, pybind11::return_value_policy::reference)
        .def_readonly("EdgeData", &FEState::m_EDGE, pybind11::return_value_policy::reference)
        .def_readonly("FaceData", &FEState::m_FACE, pybind11::return_value_policy::reference)
        .def_readonly("ElemData", &FEState::m_ELEM, pybind11::return_value_policy::reference);

    pybind11::class_<NODEDATA>(post, "NODEDATA")
        .def_readonly("val", &NODEDATA::m_val)
        .def_readonly("tag", &NODEDATA::m_ntag);

    pybind11::class_<EDGEDATA>(post, "EDGEDATA")
        .def_readonly("val", &EDGEDATA::m_val)
        .def_readonly("tag", &EDGEDATA::m_ntag)
        .def_readonly("nodeVals", &EDGEDATA::m_nv);

    pybind11::class_<ELEMDATA>(post, "ELEMDATA")
        .def_readonly("val", &ELEMDATA::m_val)
        .def_readonly("state", &ELEMDATA::m_state)
        .def_readonly("shellThickness", &ELEMDATA::m_h);

    pybind11::class_<FACEDATA>(post, "FACEDATA")
        .def_readonly("val", &FACEDATA::m_val)
        .def_readonly("tag", &FACEDATA::m_ntag);

}

#else
void init_FBSPost(pybind11::module_& m) {}
#endif