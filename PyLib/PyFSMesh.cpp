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

#include <MeshLib/FEMesh.h>
#include <MeshLib/FEItem.h>
#include <MeshLib/FENode.h>
#include <MeshLib/FEEdge.h>
#include <MeshLib/FEFace.h>
#include <MeshLib/FEElement.h>


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

    ///////////////// MeshItem /////////////////
	py::class_<MeshItem, std::unique_ptr<MeshItem, py::nodelete>>(mesh, "MeshItem")
        .def("is_hidden", &MeshItem::IsHidden)
        .def("is_selected", &MeshItem::IsSelected)
        .def("is_disabled", &MeshItem::IsDisabled)
        .def("is_active", &MeshItem::IsActive)
        .def("is_invisible", &MeshItem::IsInvisible)
        .def("is_visible", &MeshItem::IsVisible)

        .def("select", &MeshItem::Select)
        .def("unselect", &MeshItem::Unselect)

        .def("hide", &MeshItem::Hide)
        .def("unhide", &MeshItem::Unhide)

        .def("enable", &MeshItem::Enable)
        .def("disable", &MeshItem::Disable)

        .def("activate", &MeshItem::Activate)
        .def("deactivate", &MeshItem::Deactivate)

        .def("hide", &MeshItem::Hide)
        .def("unhide", &MeshItem::Unhide)

        .def("show", &MeshItem::Show)

        .def("id", &MeshItem::GetID)
        .def("set_id", &MeshItem::SetID)

        .def_readwrite("ntag", &MeshItem::m_ntag)
        .def_readwrite("gid", &MeshItem::m_gid)
        .def_readwrite("nid", &MeshItem::m_nid)
        ;
    ///////////////// MeshItem /////////////////

    ///////////////// FSElement /////////////////
	py::class_<FSElement, MeshItem, std::unique_ptr<FSElement, py::nodelete>>(mesh, "Element")
        .def("nodes", &FSElement::Nodes)
        .def("node", [](FSElement& self, int i){ return self.m_node[i]; })
        ;
        
    ///////////////// FSElement /////////////////


    ///////////////// FSNode /////////////////
	py::class_<FSNode, MeshItem, std::unique_ptr<FSNode, py::nodelete>>(mesh, "Node")
        .def_readwrite("pos", &FSNode::r)
        ;
    ///////////////// FSNode /////////////////
}

#else
void init_FSMesh(py::module_& m) {}
#endif