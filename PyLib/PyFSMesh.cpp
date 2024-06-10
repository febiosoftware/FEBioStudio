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

void init_FSMesh(pybind11::module_& m)
{
    ///////////////// FSMesh /////////////////
    pybind11::class_<FSMesh, std::unique_ptr<FSMesh, pybind11::nodelete>>(m, "FSMesh")
        .def("Create", &FSMesh::Create)
        .def("Clear", &FSMesh::Clear)

        .def("Nodes", &FSMesh::Nodes)
        .def("Node", [](FSMesh& self, int i) {return &self.Node(i);})

        .def("Edges", &FSMesh::Edges)
        .def("Edge", [](FSMesh& self, int i) {return &self.Edge(i);})

        .def("Faces", &FSMesh::Faces)
        .def("Face", [](FSMesh& self, int i) {return &self.Face(i);})

        .def("Elements", &FSMesh::Elements)
        .def("Element", [](FSMesh& self, int i) {return &self.Element(i);})

        ;

    ///////////////// FSMesh /////////////////

    ///////////////// MeshItem /////////////////
    pybind11::class_<MeshItem, std::unique_ptr<MeshItem, pybind11::nodelete>>(m, "MeshItem")
        .def("IsHidden", &MeshItem::IsHidden)
        .def("IsSelected", &MeshItem::IsSelected)
        .def("IsDisabled", &MeshItem::IsDisabled)
        .def("IsActive", &MeshItem::IsActive)
        .def("IsInvisible", &MeshItem::IsInvisible)
        .def("IsVisible", &MeshItem::IsVisible)

        .def("Select", &MeshItem::Select)
        .def("Unselect", &MeshItem::Unselect)

        .def("Hide", &MeshItem::Hide)
        .def("Unhide", &MeshItem::Unhide)

        .def("Enable", &MeshItem::Enable)
        .def("Disable", &MeshItem::Disable)

        .def("Activate", &MeshItem::Activate)
        .def("Deactivate", &MeshItem::Deactivate)

        .def("Hide", &MeshItem::Hide)
        .def("Unhide", &MeshItem::Unhide)

        .def("Show", &MeshItem::Show)

        .def("GetID", &MeshItem::GetID)
        .def("SetID", &MeshItem::SetID)

        .def_readwrite("ntag", &MeshItem::m_ntag)
        .def_readwrite("gid", &MeshItem::m_gid)
        .def_readwrite("nid", &MeshItem::m_nid)
        ;
    ///////////////// MeshItem /////////////////

    ///////////////// FSNode /////////////////
    pybind11::class_<FSNode, MeshItem, std::unique_ptr<FSNode, pybind11::nodelete>>(m, "FSNode")
        .def_readwrite("pos", &FSNode::r);
        ;
    ///////////////// FSNode /////////////////

    // ///////////////// FSElement /////////////////
    // pybind11::class_<FSElement, MeshItem, std::unique_ptr<FSElement, pybind11::nodelete>>(m, "FSElement")
    //     .def_readwrite("pos", &FSNode::r);
    //     ;
    // ///////////////// FSElement /////////////////
}

#else
void init_FSMesh(pybind11::module_& m) {}
#endif