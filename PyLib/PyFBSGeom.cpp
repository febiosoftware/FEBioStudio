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

#include "PyFBSUI.h"

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GBaseObject.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GItem.h>
#include <MeshLib/FSMesh.h>
#include <FECore/FETransform.h>
#include "DocHeaders/PyGeomDocs.h"

namespace py = pybind11;

// Initializes the fbs.geom module
void init_FBSGeom(py::module& m)
{
	py::module geom = m.def_submodule("geom", "Module used to create geometry.");

	py::class_<GObject, FSObject, std::unique_ptr<GObject, py::nodelete>>(geom, "GObject", DOC(GObject))
		.def("Parts", &GBaseObject::Parts, DOC(GBaseObject, Parts))
		.def("Faces", &GBaseObject::Faces, DOC(GBaseObject, Faces))
		.def("Edges", &GBaseObject::Edges, DOC(GBaseObject, Edges))
		.def("Nodes", &GBaseObject::Nodes, DOC(GBaseObject, Nodes))
		.def("Part", [](GObject& self, int i) {return self.Part(i); }, DOC(GBaseObject, Part))
		.def("Face", [](GObject& self, int i) {return self.Face(i); }, DOC(GBaseObject, Face))
		.def("Edge", [](GObject& self, int i) {return self.Edge(i); }, DOC(GBaseObject, Edge))
		.def("Node", [](GObject& self, int i) {return self.Node(i); }, DOC(GBaseObject, Node))
		.def("Part", [](GObject& self, int i) {return self.Part(i); }, DOC(GBaseObject, Part))
		.def("GetFEMesh", [](GObject& self) {return self.GetFEMesh(); }, DOC(GObject, GetFEMesh))
		.def("BuildMesh", &GObject::BuildMesh, DOC(GObject, BuildMesh))
		.def("GetTransform", static_cast<Transform&(GBaseObject::*)()>(&GBaseObject::GetTransform), py::return_value_policy::reference, DOC(GBaseObject, GetTransform))
		.def_property("pos", &GBaseObject::GetPosition, &GBaseObject::SetPosition, "The position of the object in world coordinates. This is a property that can be set or retrieved.")
		;

	py::class_<GMeshObject, GObject, std::unique_ptr<GMeshObject, py::nodelete>>(geom, "GMeshObject", DOC(GMeshObject))
		.def("AddNode", static_cast<int (GMeshObject::*)(vec3d)>(&GMeshObject::AddNode), DOC(GMeshObject, AddNode))
		.def("MakeGNode", &GMeshObject::MakeGNode, DOC(GMeshObject, MakeGNode))
		;

	py::class_<GBox, GObject, std::unique_ptr<GBox, py::nodelete>>(geom, "GBox", DOC(GBox));

	py::class_<GDisc, GObject, std::unique_ptr<GDisc, py::nodelete>>(geom, "GDisc", DOC(GDisc))
		.def("CreateMesh", &GDisc::CreateMesh, DOC(GDisc, CreateMesh))
		;

	py::class_<GNode, std::unique_ptr<GNode, py::nodelete>>(geom, "GNode", DOC(GNode))
		.def("Type", &GNode::Type, DOC(GNode, Type))
		.def("SetType", &GNode::SetType, DOC(GNode, SetType))
		.def("LocalPosition", static_cast<vec3d & (GNode::*)()>(&GNode::LocalPosition), DOC(GNode, LocalPosition))
		.def("Position", &GNode::Position, DOC(GNode, Position))
		.def("MakeRequired", &GNode::MakeRequired, DOC(GNode, MakeRequired))
		;
}
#else
void init_FBSGeom(pybind11::module_& m) {}
#endif
