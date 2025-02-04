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
#include <MeshLib/MeshTools.h>


#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_FSMesh(py::module_& m)
{
    py::module mesh = m.def_submodule("mesh", "Module used to interact with FE Meshes");

	mesh.def("FindAllIntersections", FindAllIntersections);

	py::class_<FSMeshBase, std::unique_ptr<FSMeshBase, py::nodelete>>(mesh, "MeshBase");

	py::class_<FSMesh, FSMeshBase, std::unique_ptr<FSMesh, py::nodelete>>(mesh, "Mesh")
		.def(py::init<>())
        .def("Create", &FSMesh::Create)
        .def("Clear", &FSMesh::Clear)

        .def("Nodes", &FSMesh::Nodes)
        .def("Node", [](FSMesh& self, int i) {return &self.Node(i);})

        .def("Edges", &FSMesh::Edges)
        .def("Edge", [](FSMesh& self, int i) {return &self.Edge(i);})

        .def("Faces", &FSMesh::Faces)
        .def("Face", [](FSMesh& self, int i) {return &self.Face(i);})

        .def("Elements", &FSMesh::Elements)
        .def("Element", [](FSMesh& self, int i) {return &self.Element(i); })

		.def("LocalToGlobal", &FSMesh::LocalToGlobal)
		.def("RebuildMesh", &FSMesh::RebuildMesh)
		.def("NodePosition", &FSMesh::NodePosition)

		.def("Surfaces", &FSMesh::FESurfaces)
		.def("Surface", &FSMesh::GetFESurface, py::return_value_policy::reference)
        ;

	py::class_<FSMeshItem, std::unique_ptr<FSMeshItem, py::nodelete>>(mesh, "MeshItem")
        .def("IsHidden", &FSMeshItem::IsHidden)
        .def("IsSelected", &FSMeshItem::IsSelected)
        .def("IsDisabled", &FSMeshItem::IsDisabled)
        .def("IsActive", &FSMeshItem::IsActive)
        .def("IsInvisible", &FSMeshItem::IsInvisible)
        .def("IsVisible", &FSMeshItem::IsVisible)
		.def("IsExterior", &FSMeshItem::IsExterior)

        .def("Select", &FSMeshItem::Select)
        .def("Unselect", &FSMeshItem::Unselect)

        .def("Hide", &FSMeshItem::Hide)
        .def("Unhide", &FSMeshItem::Unhide)
        .def("Show", &FSMeshItem::Show)

        .def("Enable", &FSMeshItem::Enable)
        .def("Disable", &FSMeshItem::Disable)

        .def("Activate", &FSMeshItem::Activate)
        .def("Deactivate", &FSMeshItem::Deactivate)

        .def("GetID", &FSMeshItem::GetID)
        .def("SetID", &FSMeshItem::SetID)

        .def_readwrite("tag", &FSMeshItem::m_ntag)
        .def_readwrite("gid", &FSMeshItem::m_gid)
        .def_readwrite("nid", &FSMeshItem::m_nid)
        ;

	py::enum_<FSElementType>(mesh, "ElementType")
		.value("FE_HEX8", FSElementType::FE_HEX8)
		;

	py::class_<FSElement, FSMeshItem, std::unique_ptr<FSElement, py::nodelete>>(mesh, "Element")
        .def("Nodes", &FSElement::Nodes)
		.def("Node", [](FSElement& self, int node) { return self.m_node[node]; })
		.def("SetNode", [](FSElement& self, int node, int val) { self.m_node[node] = val; })
		.def("SetType", &FSElement::SetType)
        ;

	py::class_<FSNode, FSMeshItem, std::unique_ptr<FSNode, py::nodelete>>(mesh, "Node")
        .def_readwrite("pos", &FSNode::r)
        ;

	py::class_<FSSurface>(mesh, "FESurface")
		.def("GetFaceIndices", [](FSSurface& self) { return self.CopyItems(); })
		.def("GetName", &FSSurface::GetName);

	py::class_<FSNodeSet>(mesh, "FSNodeSet")
	//        .def_readonly("nodes", &FSNodeSet::m_Item, py::return_value_policy::reference)
		.def("GetName", &FSNodeSet::GetName);

	py::class_<FSElemSet>(mesh, "FSElemSet")
	//        .def_readonly("elems", &FSElemSet::m_Item, py::return_value_policy::reference)
		.def("GetName", &FSElemSet::GetName);

}

#else
void init_FSMesh(pybind11::module_& m) {}
#endif