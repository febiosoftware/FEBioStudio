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

#include "PyFSMesh.h"

#include <MeshLib/FSMesh.h>
#include <MeshLib/FSMeshItem.h>
#include <MeshLib/FSNode.h>
#include <MeshLib/FSEdge.h>
#include <MeshLib/FSFace.h>
#include <MeshLib/FSElement.h>


#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

void init_FSMesh(py::module_& m)
{
    py::module mesh = m.def_submodule("mesh", "Module used to interact with FE Meshes");

    ///////////////// FSMesh /////////////////
	py::class_<FSMesh, std::unique_ptr<FSMesh, py::nodelete>>(mesh, "Mesh")
        .def("create", &FSMesh::Create)
        .def("clear", &FSMesh::Clear)

        .def("nodes", &FSMesh::Nodes)
        .def("node", [](FSMesh& self, int i) {return &self.Node(i);})

        .def("edges", &FSMesh::Edges)
        .def("edge", [](FSMesh& self, int i) {return &self.Edge(i);})

        .def("faces", &FSMesh::Faces)
        .def("face", [](FSMesh& self, int i) {return &self.Face(i);})

        .def("elements", &FSMesh::Elements)
        .def("element", [](FSMesh& self, int i) {return &self.Element(i);})
        ;

    ///////////////// FSMesh /////////////////

    ///////////////// FSMeshItem /////////////////
	py::class_<FSMeshItem, std::unique_ptr<FSMeshItem, py::nodelete>>(mesh, "FSMeshItem")
        .def("is_hidden", &FSMeshItem::IsHidden)
        .def("is_selected", &FSMeshItem::IsSelected)
        .def("is_disabled", &FSMeshItem::IsDisabled)
        .def("is_active", &FSMeshItem::IsActive)
        .def("is_invisible", &FSMeshItem::IsInvisible)
        .def("is_visible", &FSMeshItem::IsVisible)

        .def("select", &FSMeshItem::Select)
        .def("unselect", &FSMeshItem::Unselect)

        .def("hide", &FSMeshItem::Hide)
        .def("unhide", &FSMeshItem::Unhide)

        .def("enable", &FSMeshItem::Enable)
        .def("disable", &FSMeshItem::Disable)

        .def("activate", &FSMeshItem::Activate)
        .def("deactivate", &FSMeshItem::Deactivate)

        .def("hide", &FSMeshItem::Hide)
        .def("unhide", &FSMeshItem::Unhide)

        .def("show", &FSMeshItem::Show)

        .def("id", &FSMeshItem::GetID)
        .def("set_id", &FSMeshItem::SetID)

        .def_readwrite("ntag", &FSMeshItem::m_ntag)
        .def_readwrite("gid", &FSMeshItem::m_gid)
        .def_readwrite("nid", &FSMeshItem::m_nid)
        ;
    ///////////////// FSMeshItem /////////////////

    ///////////////// FSElement /////////////////
	py::class_<FSElement, FSMeshItem, std::unique_ptr<FSElement, py::nodelete>>(mesh, "Element")
        .def("nodes", &FSElement::Nodes)
        .def("node", [](FSElement& self, int i){ return self.m_node[i]; })
        ;
        
    ///////////////// FSElement /////////////////


    ///////////////// FSNode /////////////////
	py::class_<FSNode, FSMeshItem, std::unique_ptr<FSNode, py::nodelete>>(mesh, "Node")
        .def_readwrite("pos", &FSNode::r)
        ;
    ///////////////// FSNode /////////////////
}

#else
void init_FSMesh(pybind11::module_& m) {}
#endif