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

#include <MeshLib/FSMesh.h>
#include <MeshLib/FSMeshItem.h>
#include <MeshLib/FSNode.h>
#include <MeshLib/FSEdge.h>
#include <MeshLib/FSFace.h>
#include <MeshLib/FSElement.h>
#include <MeshLib/FSItemListBuilder.h>
#include <GeomLib/FSGroup.h>
#include <MeshLib/MeshTools.h>
#include <MeshLib/FSCurveMesh.h>
#include <MeshLib/FSSurfaceMesh.h>
#include <MeshTools/FEMesher.h>
#ifdef HAS_PYTHON
#include "PyFBSMesh.h"
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "DocHeaders/PyMeshDocs.h"
#include "PyUtil.h"

namespace py = pybind11;

class PyNodeRef : public PyMeshItemRef<FSLineMesh, FSNode> {
public:
	PyNodeRef(FSLineMesh* mesh, int index) : PyMeshItemRef<FSLineMesh, FSNode>(mesh, index, [](FSLineMesh* m, int i) -> FSNode& { return m->Node(i); }) {}

	vec3d pos() { return item().r; }
	void setPos(vec3d r) { item().r = r; }
};

class PyElementRef : public PyMeshItemRef<FSMesh, FSElement> {
public:
	PyElementRef(FSMesh* mesh, int index) : PyMeshItemRef<FSMesh, FSElement>(mesh, index, [](FSMesh* m, int i) -> FSElement& { return m->Element(i); }) {}

	void SetType(int ntype) { item().SetType(ntype); }
};

class PyElementNodeList
{
public:
	PyElementNodeList(FSElement* el) : m_el(el) {}

	int size() const { return m_el->Nodes(); }

	int get(int i) const
	{
		int n = size();
		if (i < 0) i += n;
		if (i < 0 || i >= n)
			throw py::index_error("node index out of range");
		return m_el->m_node[i];
	}

	void set(int i, int nodeId)
	{
		int n = size();
		if (i < 0) i += n;
		if (i < 0 || i >= n)
			throw py::index_error("node index out of range");
		m_el->m_node[i] = nodeId;
	}

private:
	FSElement* m_el;
};

class PyMeshEdgeList : public PyIndexedCollection<FSLineMesh, FSEdge>
{
public:
	PyMeshEdgeList(FSLineMesh* mesh) : PyIndexedCollection<FSLineMesh, FSEdge>(mesh,
		[](FSLineMesh* m) { return m->Edges(); },
		[](FSLineMesh* m, int i) { return &m->Edge(i); },
		"edge") {}
};

class PyMeshNodeList
{
public:
	PyMeshNodeList(FSLineMesh* mesh) : m_mesh(mesh) {}

	int size() const
	{
		return m_mesh->Nodes();
	}

	PyNodeRef get(int i) const
	{
		int n = size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
			throw py::index_error("node index out of range");

		return PyNodeRef(m_mesh, i);
	}

	py::iterator iter() const
	{
		py::list items;
		for (int i = 0; i < size(); ++i)
			items.append(py::cast(get(i)));

		return py::iter(items);
	}

private:
	FSLineMesh* m_mesh = nullptr;
};

class PyMeshFaceList
{
public:
	PyMeshFaceList(FSMesh* mesh) : m_mesh(mesh) {}

	int size() const
	{
		return m_mesh->Faces();
	}

	PyFaceRef get(int i) const
	{
		int n = size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
			throw py::index_error("face index out of range");

		return PyFaceRef(m_mesh, i);
	}

	py::iterator iter() const
	{
		py::list items;
		for (int i = 0; i < size(); ++i)
			items.append(py::cast(get(i)));

		return py::iter(items);
	}

private:
	FSMesh* m_mesh = nullptr;
};

class PySurfaceFaceList
{
public:
	PySurfaceFaceList(FSSurface* surf) : m_surf(surf) {}

	int size() const
	{
		return m_surf->size();
	}

	PyFaceRef get(int i) const
	{
		int n = size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
			throw py::index_error("face index out of range");

		return PyFaceRef(m_surf->GetMesh(), (*m_surf)[i]);
	}

	py::iterator iter() const
	{
		py::list items;
		for (int i = 0; i < size(); ++i)
			items.append(py::cast(get(i)));

		return py::iter(items);
	}

private:
	FSSurface* m_surf = nullptr;
};


class PyMeshElementList : public PyIndexedCollection<FSMesh, FSElement>
{
public:
	PyMeshElementList(FSMesh* mesh) : PyIndexedCollection<FSMesh, FSElement>(mesh,
		[](FSMesh* m) { return m->Elements(); },
		[](FSMesh* m, int i) { return &m->Element(i); },
		"element") {}
private:
	FSMesh* m_mesh = nullptr;
};

class PyMeshSurfaceList : public PyNamedCollection<FSMesh, FSSurface>
{
public:
	PyMeshSurfaceList(FSMesh* mesh) : PyNamedCollection<FSMesh, FSSurface>(
		mesh, 
		[](FSMesh* m) { return m->FESurfaces(); },
		[](FSMesh* m, int i) { return m->GetFESurface(i); },
		[](FSMesh* m, const std::string& name) { return m->FindFESurface(name); },
		"surface") {}
};

void init_FSMesh(py::module_& m)
{
    py::module mesh = m.def_submodule("mesh", "Module used to interact with FE Meshes");

	mesh.def("find_all_intersections", FindAllIntersections);

	py::class_<PyNodeRef>(mesh, "NodeRef")
		.def_property_readonly("index", &PyNodeRef::index)
		.def_property_readonly("id", &PyNodeRef::id)
		.def_property_readonly(
			"node",
			[](PyNodeRef& self) -> FSNode& { return self.item(); },
			py::return_value_policy::reference_internal
		)
		.def_property("pos", &PyNodeRef::pos, &PyNodeRef::setPos)
		;


	py::class_<PyFaceRef>(mesh, "FaceRef")
		.def_property_readonly("index", &PyFaceRef::index)
		.def_property_readonly("id", &PyFaceRef::id)
		.def_property_readonly(
			"face",
			[](PyFaceRef& self) -> FSFace& { return self.item(); },
			py::return_value_policy::reference_internal
		);

	py::class_<PyElementRef>(mesh, "ElementRef")
		.def_property_readonly("index", &PyElementRef::index)
		.def_property_readonly("id", &PyElementRef::id)
		.def_property_readonly(
			"element",
			[](PyElementRef& self) -> FSElement& { return self.item(); },
			py::return_value_policy::reference_internal
		)
		.def_property_readonly( "nodes", [](PyElementRef& self) -> PyElementNodeList { return PyElementNodeList(&self.item()); }, py::return_value_policy::reference_internal)
		.def("set_type", &PyElementRef::SetType);


	py::class_<PyElementNodeList>(mesh, "ElementNodeList")
		.def("__len__", &PyElementNodeList::size)
		.def("__getitem__", &PyElementNodeList::get)
		.def("__setitem__", &PyElementNodeList::set);

	py::class_<PyMeshNodeList>(mesh, "MeshNodeList")
		.def("__len__", &PyMeshNodeList::size)
		.def("__getitem__", &PyMeshNodeList::get, py::return_value_policy::reference)
		;

	py::class_<PyMeshEdgeList>(mesh, "MeshEdgeList")
		.def("__len__", &PyMeshEdgeList::size)
		.def("__getitem__", &PyMeshEdgeList::get, py::return_value_policy::reference)
		;

	py::class_<PyMeshFaceList>(mesh, "MeshFaceList")
		.def("__len__", &PyMeshFaceList::size)
		.def("__getitem__", &PyMeshFaceList::get, py::return_value_policy::reference)
		;

	py::class_<PyMeshElementList>(mesh, "MeshElementList")
		.def("__len__", &PyMeshElementList::size)
		.def("__getitem__", &PyMeshElementList::get, py::return_value_policy::reference)
		;

	py::class_<PyMeshSurfaceList>(mesh, "MeshSurfaceList")
		.def("__len__", &PyMeshSurfaceList::size)
		.def("__getitem__", py::overload_cast<int>(&PyMeshSurfaceList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyMeshSurfaceList::get, py::const_), py::return_value_policy::reference)
		;

	py::class_<PySurfaceFaceList>(mesh, "SurfaceFaceList")
		.def("__len__", &PySurfaceFaceList::size)
		.def("__getitem__", &PySurfaceFaceList::get, py::return_value_policy::reference)
		;

	py::class_<FSLineMesh, std::unique_ptr<FSLineMesh, py::nodelete>>(mesh, "LineMesh", DOC(FSLineMesh))

		.def_property_readonly( "nodes", [](FSLineMesh& self) { return PyMeshNodeList(&self); }, py::return_value_policy::reference_internal)
		.def_property_readonly( "edges", [](FSLineMesh& self) { return PyMeshEdgeList(&self); }, py::return_value_policy::reference_internal)
		;

	py::class_<FSMeshBase, FSLineMesh, std::unique_ptr<FSMeshBase, py::nodelete>>(mesh, "MeshBase", DOC(FSMeshBase))
		.def_property_readonly("faces", [](FSMesh& self) { return PyMeshFaceList(&self); }, py::return_value_policy::reference_internal)
		;

    py::class_<FSSurfaceMesh, FSMeshBase, std::unique_ptr<FSSurfaceMesh, py::nodelete>>(mesh, "SurfaceMesh", DOC(FSSurfaceMesh))
        .def(py::init<>())
        .def("Create", &FSSurfaceMesh::Create)
        .def("RebuildMesh", &FSSurfaceMesh::RebuildMesh)
        ;

	py::class_<FSMesh, FSMeshBase, std::unique_ptr<FSMesh, py::nodelete>>(mesh, "Mesh", DOC(FSMesh))
		// new interface
		.def("clear", &FSMesh::Clear, DOC(FSMesh, Clear))
		.def("create", &FSMesh::Create, DOC(FSMesh, Create))
		.def("rebuild_mesh", &FSMesh::RebuildMesh, DOC(FSMesh, RebuildMesh))

		.def_property_readonly("elements", [](FSMesh& self) { return PyMeshElementList(&self); }, py::return_value_policy::reference_internal)

		.def_property_readonly("surfaces", [](FSMesh& self) { return PyMeshSurfaceList(&self); }, py::return_value_policy::reference_internal)

		// old interface 
		.def("NodeIndexFromID", &FSMesh::NodeIndexFromID, DOC(FSMesh, NodeIndexFromID))

		.def("MeshDataFields", &FSMesh::MeshDataFields, DOC(FSMesh, MeshDataFields))
		.def("GetMeshDataField", &FSMesh::GetMeshDataField, py::return_value_policy::reference, DOC(FSMesh, GetMeshDataField))

        .def("MeshPartitions", &FSMesh::MeshPartitions, DOC(FSMesh, MeshPartitions))
        .def("MeshPartition", &FSMesh::MeshPartition, py::return_value_policy::reference, DOC(FSMesh, MeshPartition))

        .def("ElemSets", &FSMesh::FEElemSets, DOC(FSMesh, FEElemSets))
        .def("ElemSet", &FSMesh::GetFEElemSet, py::return_value_policy::reference, DOC(FSMesh, GetFEElemSet))
		.def("FindElemSet", &FSMesh::FindFEElemSet, py::return_value_policy::reference)

        .def("NodeSets", &FSMesh::FENodeSets, DOC(FSMesh, FENodeSets))
        .def("NodeSet", &FSMesh::GetFENodeSet, py::return_value_policy::reference, DOC(FSMesh, GetFENodeSet))
        ;

	py::class_<FSCurveMesh, FSLineMesh, std::unique_ptr<FSCurveMesh, py::nodelete>>(mesh, "CurveMesh", DOC(FSCurveMesh))
		.def(py::init<>(), DOC(FSCurveMesh, FSCurveMesh))
		.def("create_from_points", &FSCurveMesh::CreateFromPoints, DOC(FSCurveMesh, CreateFromPoints))
		;

	py::class_<FSMeshData, FSObject, std::unique_ptr<FSMeshData, py::nodelete>>(mesh, "MeshData", DOC(FSMeshData))
		.def("getVec3d", &FSMeshData::getVec3d, DOC(FSMeshData, getVec3d))
		;

	py::class_<FSMeshItem, std::unique_ptr<FSMeshItem, py::nodelete>>(mesh, "MeshItem", DOC(FSMeshItem))
        .def("IsHidden", &FSMeshItem::IsHidden, DOC(FSMeshItem, IsHidden))
        .def("IsSelected", &FSMeshItem::IsSelected, DOC(FSMeshItem, IsSelected))
        .def("IsDisabled", &FSMeshItem::IsDisabled, DOC(FSMeshItem, IsDisabled))
        .def("IsActive", &FSMeshItem::IsActive, DOC(FSMeshItem, IsActive))
        .def("IsInvisible", &FSMeshItem::IsInvisible, DOC(FSMeshItem, IsInvisible))
        .def("IsVisible", &FSMeshItem::IsVisible, DOC(FSMeshItem, IsVisible))
		.def("IsExterior", &FSMeshItem::IsExterior, DOC(FSMeshItem, IsExterior))

        .def("Select", &FSMeshItem::Select, DOC(FSMeshItem, Select))
        .def("Unselect", &FSMeshItem::Unselect, DOC(FSMeshItem, Unselect))

        .def("Hide", &FSMeshItem::Hide, DOC(FSMeshItem, Hide))
        .def("Unhide", &FSMeshItem::Unhide, DOC(FSMeshItem, Unhide))
        .def("Show", &FSMeshItem::Show, DOC(FSMeshItem, Show))

        .def("Enable", &FSMeshItem::Enable, DOC(FSMeshItem, Enable))
        .def("Disable", &FSMeshItem::Disable, DOC(FSMeshItem, Disable))

        .def("Activate", &FSMeshItem::Activate, DOC(FSMeshItem, Activate))
        .def("Deactivate", &FSMeshItem::Deactivate, DOC(FSMeshItem, Deactivate))

        .def("GetID", &FSMeshItem::GetID, DOC(FSMeshItem, GetID))
        .def("SetID", &FSMeshItem::SetID, DOC(FSMeshItem, SetID))

        .def_readwrite("tag", &FSMeshItem::m_ntag, DOC(FSMeshItem, m_ntag))
        .def_readwrite("gid", &FSMeshItem::m_gid, DOC(FSMeshItem, m_gid))
        .def_readwrite("nid", &FSMeshItem::m_nid, DOC(FSMeshItem, m_nid))
        ;

	py::enum_<FSElementType>(mesh, "ElementType")
		.value("FE_HEX8", FSElementType::FE_HEX8)
		;

	py::class_<FSElement, FSMeshItem, std::unique_ptr<FSElement, py::nodelete>>(mesh, "Element", DOC(FSElement))
        .def("Nodes", &FSElement::Nodes, DOC(FSElement, Nodes))
		.def("Node", [](FSElement& self, int node) { return self.m_node[node]; }, "Get the node ID at the specified index.")
		.def("SetNode", [](FSElement& self, int node, int val) { self.m_node[node] = val; }, "Set the node ID at the specified index.")
		.def("SetType", &FSElement::SetType, DOC(FSElement, SetType))
		.def("SetAxes", &FSElement::setAxes, DOC(FSElement, setAxes))
        ;

    py::class_<FSFace, FSMeshItem, std::unique_ptr<FSFace, py::nodelete>>(mesh, "Face", DOC(FSFace))
        .def("Nodes", &FSFace::Nodes, DOC(FSFace, Nodes))
		.def("Node", [](FSFace& self, int node) { return self.n[node]; }, "Get the node ID at the specified index.")
        .def("SetNode", [](FSFace& self, int node, int val) { self.n[node] = val; }, "Set the node ID at the specified index.")
        .def("Edges", &FSFace::Edges, DOC(FSFace, Edges))
		.def("Edge", &FSFace::GetEdge, DOC(FSFace, GetEdge))
        .def("Type", &FSFace::Type, DOC(FSFace, Type))
        .def("SetType", &FSFace::SetType, DOC(FSFace, SetType))
        ;

    py::class_<FSEdge, FSMeshItem, std::unique_ptr<FSEdge, py::nodelete>>(mesh, "Edge", "Class representing a mesh edge.")
        .def("Nodes", &FSEdge::Nodes, "Get the node IDs of the edge.")
		.def("Node", [](FSEdge& self, int node) { return self.n[node]; }, "Get the node ID at the specified index.")
        ;

	py::class_<FSNode, FSMeshItem, std::unique_ptr<FSNode, py::nodelete>>(mesh, "Node", "Class representing a mesh node.")
        .def_readwrite("pos", &FSNode::r, "Get the position of the node.")
        ;

    py::class_<FSItemListBuilder, FSObject>(mesh, "FSItemListBuilder", "Class for building a list of mesh items.")
        .def("CopyItems", &FSItemListBuilder::CopyItems, "Get a copy of the items in the list.")
        ;

    py::class_<FSGroup, FSItemListBuilder, std::unique_ptr<FSGroup, py::nodelete>>(mesh, "Group", "Class representing a group of mesh items.")
        .def("GetMesh", &FSGroup::GetMesh, py::return_value_policy::reference, "Get the mesh associated with this group.")
        ;

	py::class_<FSSurface, FSGroup>(mesh, "FESurface", "Class representing a mesh surface.")
		.def_property_readonly("faces", [](FSSurface& self) { return PySurfaceFaceList(&self); }, py::return_value_policy::reference_internal)
		;

	py::class_<FSNodeSet, FSGroup>(mesh, "FSNodeSet", "Class representing a set of mesh nodes.");

	py::class_<FSElemSet, FSGroup>(mesh, "FSElemSet", "Class representing a set of mesh elements.");

    py::class_<FSMeshPartition, FSObject, std::unique_ptr<FSMeshPartition, py::nodelete>>(mesh, "MeshPartition", "Class representing a partition of a mesh.")
        .def("SetMatID", &FSMeshPartition::SetMatID, "Set the material ID for this partition.")
        .def("GetMatID", &FSMeshPartition::GetMatID, "Get the material ID for this partition.")
        .def("Faces", &FSMeshPartition::Faces, "Get the number of faces in this partition.")
        .def("Face", &FSMeshPartition::Face, py::return_value_policy::reference, "Get the face at the specified index.")
        .def("Elements", &FSMeshPartition::Elements, "Get the number of elements in this partition.")
        .def("Element", &FSMeshPartition::Element, py::return_value_policy::reference, "Get the element at the specified index.")
        .def("FaceList", &FSMeshPartition::FaceList, "Get the list of face indices in this partition.")
        .def("ElementList", &FSMeshPartition::ElementList, "Get the list of element indices in this partition.")
        ;

	py::class_<FEMesher, FSObject, std::unique_ptr<FEMesher, py::nodelete>>(mesh, "Mesher", "Class for generating meshes.")
		.def_property_readonly("params", [](FEMesher& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def("__getattr__", [](FEMesher& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FEMesher& self, const std::string& name, py::object value) { SetDynamicAttribute(self, name, value); })
		;

}

#else
void init_FSMesh(pybind11::module_& m) {}
#endif
