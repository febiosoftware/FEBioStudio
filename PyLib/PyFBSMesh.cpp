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

class PyEdgeRef : public PyMeshItemRef<FSLineMesh, FSEdge> {
public:
	PyEdgeRef(FSLineMesh* mesh, int index) : PyMeshItemRef<FSLineMesh, FSEdge>(mesh, index, [](FSLineMesh* m, int i) -> FSEdge& { return m->Edge(i); }) {}
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

	py::list iter() const
	{
		py::list items;
		for (int i = 0; i < size(); ++i)
			items.append(get(i));
		return items;
	}

private:
	FSElement* m_el;
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

class PyMeshEdgeList
{
public:
	PyMeshEdgeList(FSLineMesh* mesh) : m_mesh(mesh) {}

	int size() const
	{
		return m_mesh->Edges();
	}

	PyEdgeRef get(int i) const
	{
		int n = size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
			throw py::index_error("edge index out of range");

		return PyEdgeRef(m_mesh, i);
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
	PyMeshFaceList(FSMeshBase* mesh) : m_mesh(mesh) {}

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
	FSMeshBase* m_mesh = nullptr;
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


class PyMeshElementList
{
public:
	PyMeshElementList(FSMesh* mesh) : m_mesh(mesh) {}

	int size() const
	{
		return m_mesh->Elements();
	}

	PyElementRef get(int i) const
	{
		int n = size();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
			throw py::index_error("element index out of range");

		return PyElementRef(m_mesh, i);
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

class PyMeshNodeSetList : public PyNamedCollection<FSMesh, FSNodeSet>
{
public:
	PyMeshNodeSetList(FSMesh* mesh) : PyNamedCollection<FSMesh, FSNodeSet>(
		mesh,
		[](FSMesh* m) { return m->FENodeSets(); },
		[](FSMesh* m, int i) { return m->GetFENodeSet(i); },
		[](FSMesh* m, const std::string& name) { return m->FindFENodeSet(name); },
		"nodeset") {
	}
};

class PyMeshElemSetList : public PyNamedCollection<FSMesh, FSElemSet>
{
public:
	PyMeshElemSetList(FSMesh* mesh) : PyNamedCollection<FSMesh, FSElemSet>(
		mesh,
		[](FSMesh* m) { return m->FEElemSets(); },
		[](FSMesh* m, int i) { return m->GetFEElemSet(i); },
		[](FSMesh* m, const std::string& name) { return m->FindFEElemSet(name); },
		"elemset") {
	}
};

class PyMeshDataFieldList : public PyNamedCollection<FSMesh, FSMeshData>
{
public:
	PyMeshDataFieldList(FSMesh* mesh) : PyNamedCollection<FSMesh, FSMeshData>(
		mesh,
		[](FSMesh* m) { return m->MeshDataFields(); },
		[](FSMesh* m, int i) { return m->GetMeshDataField(i); },
		[](FSMesh* m, const std::string& name) { return m->FindMeshDataField(name); },
		"meshdata") {
	}
};

py::object GetMeshDataValue(FSMeshData& self, int i)
{
	int n = self.DataItems();
	if (i < 0) i += n;

	if (i < 0 || i >= n)
	{
		throw py::index_error("mesh data index out of range");
	}

	switch (self.GetDataType())
	{
	case DATA_SCALAR: return py::float_(self.getScalar(i)); break;
	case DATA_VEC3: return py::cast(self.getVec3d(i)); break;
	default:
		throw std::runtime_error("Unsupported data type");
	};
}

void SetMeshDataValue(FSMeshData& self, int i, py::object value)
{
	int n = self.DataItems();
	if (i < 0) i += n;

	if (i < 0 || i >= n)
	{
		throw py::index_error("mesh data index out of range");
	}

	switch (self.GetDataType())
	{
	case DATA_SCALAR: return self.setScalar(i, py::float_(value)); break;
	case DATA_VEC3: return self.setVec3d(i, py::cast<vec3d>(value)); break;
	default:
		throw std::runtime_error("Unsupported data type");
	};
}


void init_FSMesh(py::module_& m)
{
    py::module mesh = m.def_submodule("mesh", "Module used to interact with FE Meshes");

	py::class_<PyNodeRef>(mesh, "Node")
		.def_property_readonly("index", &PyNodeRef::index)
		.def_property_readonly("id", &PyNodeRef::id)
		.def_property("pos", &PyNodeRef::pos, &PyNodeRef::setPos)
		;

	py::class_<PyEdgeRef>(mesh, "Edge")
		.def_property_readonly("index", &PyEdgeRef::index)
		.def_property_readonly("id", &PyEdgeRef::id)
		;

	py::class_<PyFaceRef>(mesh, "Face")
		.def_property_readonly("index", &PyFaceRef::index)
		.def_property_readonly("id", &PyFaceRef::id)
		;
	
	py::class_<PyElementRef>(mesh, "Element")
		.def_property_readonly("index", &PyElementRef::index)
		.def_property_readonly("id", &PyElementRef::id)
		.def_property_readonly( "nodes", [](PyElementRef& self) -> PyElementNodeList { return PyElementNodeList(&self.item()); }, py::return_value_policy::reference_internal)
		.def("set_type", &PyElementRef::SetType);

	py::class_<PyElementNodeList>(mesh, "ElementNodeList")
		.def("__len__", &PyElementNodeList::size)
		.def("__getitem__", &PyElementNodeList::get)
		.def("__setitem__", &PyElementNodeList::set)
		.def("__iter__", &PyElementNodeList::iter)
		;

	py::class_<PyMeshNodeList>(mesh, "MeshNodeList")
		.def("__len__", &PyMeshNodeList::size)
		.def("__getitem__", &PyMeshNodeList::get, py::return_value_policy::reference)
		.def("__iter__", &PyMeshNodeList::iter)
		;

	py::class_<PyMeshEdgeList>(mesh, "MeshEdgeList")
		.def("__len__", &PyMeshEdgeList::size)
		.def("__getitem__", &PyMeshEdgeList::get, py::return_value_policy::reference)
		.def("__iter__", &PyMeshEdgeList::iter)
		;

	py::class_<PyMeshFaceList>(mesh, "MeshFaceList")
		.def("__len__", &PyMeshFaceList::size)
		.def("__getitem__", &PyMeshFaceList::get, py::return_value_policy::reference)
		.def("__iter__", &PyMeshFaceList::iter)
		;

	py::class_<PyMeshElementList>(mesh, "MeshElementList")
		.def("__len__", &PyMeshElementList::size)
		.def("__getitem__", &PyMeshElementList::get, py::return_value_policy::reference)
		.def("__iter__", &PyMeshElementList::iter)
		;

	py::class_<PyMeshSurfaceList>(mesh, "MeshSurfaceList")
		.def("__len__", &PyMeshSurfaceList::size)
		.def("__getitem__", py::overload_cast<int>(&PyMeshSurfaceList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyMeshSurfaceList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", &PyMeshSurfaceList::iter)
		;

	py::class_<PyMeshNodeSetList>(mesh, "MeshNodeSetList")
		.def("__len__", &PyMeshNodeSetList::size)
		.def("__getitem__", py::overload_cast<int>(&PyMeshNodeSetList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyMeshNodeSetList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", &PyMeshNodeSetList::iter)
		;

	py::class_<PyMeshElemSetList>(mesh, "MeshElemSetList")
		.def("__len__", &PyMeshElemSetList::size)
		.def("__getitem__", py::overload_cast<int>(&PyMeshElemSetList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyMeshElemSetList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", &PyMeshElemSetList::iter)
		;

	py::class_<PyMeshDataFieldList>(mesh, "MeshDataFieldList")
		.def("__len__", &PyMeshDataFieldList::size)
		.def("__getitem__", py::overload_cast<int>(&PyMeshDataFieldList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyMeshDataFieldList::get, py::const_), py::return_value_policy::reference)
		.def("__iter__", &PyMeshDataFieldList::iter)
		;

	py::class_<PySurfaceFaceList>(mesh, "SurfaceFaceList")
		.def("__len__", &PySurfaceFaceList::size)
		.def("__getitem__", &PySurfaceFaceList::get, py::return_value_policy::reference)
		.def("__iter__", &PySurfaceFaceList::iter)
		;

	py::class_<FSLineMesh, std::unique_ptr<FSLineMesh, py::nodelete>>(mesh, "LineMesh", DOC(FSLineMesh))
		.def_property_readonly( "nodes", [](FSLineMesh& self) { return PyMeshNodeList(&self); }, py::return_value_policy::reference_internal)
		.def_property_readonly( "edges", [](FSLineMesh& self) { return PyMeshEdgeList(&self); }, py::return_value_policy::reference_internal)
		;

	py::class_<FSMeshBase, FSLineMesh, std::unique_ptr<FSMeshBase, py::nodelete>>(mesh, "MeshBase", DOC(FSMeshBase))
		.def_property_readonly("faces", [](FSMeshBase& self) { return PyMeshFaceList(&self); }, py::return_value_policy::reference_internal)
		.def("find_all_intersections", FindAllIntersections);
		;

    py::class_<FSSurfaceMesh, FSMeshBase, std::unique_ptr<FSSurfaceMesh, py::nodelete>>(mesh, "SurfaceMesh", DOC(FSSurfaceMesh))
        .def(py::init<>())
        .def("create", &FSSurfaceMesh::Create)
        .def("rebuild_mesh", &FSSurfaceMesh::RebuildMesh)
        ;

	py::class_<FSMesh, FSMeshBase, std::unique_ptr<FSMesh, py::nodelete>>(mesh, "Mesh", DOC(FSMesh))
		// new interface
		.def("clear", &FSMesh::Clear, DOC(FSMesh, Clear))
		.def("create", &FSMesh::Create, DOC(FSMesh, Create))
		.def("rebuild_mesh", &FSMesh::RebuildMesh, DOC(FSMesh, RebuildMesh))

		.def_property_readonly("elements", [](FSMesh& self) { return PyMeshElementList(&self); }, py::return_value_policy::reference_internal)

		.def_property_readonly("node_sets", [](FSMesh& self) { return PyMeshNodeSetList(&self); }, py::return_value_policy::reference_internal)
		.def_property_readonly("surfaces", [](FSMesh& self) { return PyMeshSurfaceList(&self); }, py::return_value_policy::reference_internal)
		.def_property_readonly("element_sets", [](FSMesh& self) { return PyMeshElemSetList(&self); }, py::return_value_policy::reference_internal)

		.def_property_readonly("data_fields", [](FSMesh& self) { return PyMeshDataFieldList(&self); }, py::return_value_policy::reference_internal)
		;

	py::class_<FSCurveMesh, FSLineMesh, std::unique_ptr<FSCurveMesh, py::nodelete>>(mesh, "CurveMesh", DOC(FSCurveMesh))
		.def(py::init<>(), DOC(FSCurveMesh, FSCurveMesh))
		.def("create_from_points", &FSCurveMesh::CreateFromPoints, DOC(FSCurveMesh, CreateFromPoints))
		;

	py::class_<FSMeshData, FSObject, std::unique_ptr<FSMeshData, py::nodelete>>(mesh, "MeshData", DOC(FSMeshData))
		.def("__len__", &FSMeshData::DataItems, DOC(FSMeshData, DataItems))
		.def("__getitem__", &GetMeshDataValue, DOC(FSMeshData, get))
		.def("__setitem__", &SetMeshDataValue, DOC(FSMeshData, set))
		.def("__iter__", [](FSMeshData& self) {
			py::list items;
			for (int i = 0; i < self.DataItems(); ++i)
				items.append(GetMeshDataValue(self, i));
			return py::iter(items);
			})
		;

	py::enum_<FSElementType>(mesh, "ElementType")
		.value("FE_HEX8", FSElementType::FE_HEX8)
		;

    py::class_<FSItemListBuilder, FSObject>(mesh, "ItemListBuilder", "Class for building a list of mesh items.")
        .def("copy_items", &FSItemListBuilder::CopyItems, "Get a copy of the items in the list.")
        ;

    py::class_<FSGroup, FSItemListBuilder, std::unique_ptr<FSGroup, py::nodelete>>(mesh, "Group", "Class representing a group of mesh items.")
        .def_property_readonly("mesh", &FSGroup::GetMesh, py::return_value_policy::reference, "Get the mesh associated with this group.")
        ;

	py::class_<FSSurface, FSGroup>(mesh, "Surface", "Class representing a mesh surface.")
		.def_property_readonly("faces", [](FSSurface& self) { return PySurfaceFaceList(&self); }, py::return_value_policy::reference_internal)
		;

	py::class_<FSNodeSet, FSGroup>(mesh, "NodeSet", "Class representing a set of mesh nodes.");

	py::class_<FSElemSet, FSGroup>(mesh, "ElementSet", "Class representing a set of mesh elements.");

	py::class_<FEMesher, FSObject, std::unique_ptr<FEMesher, py::nodelete>>(mesh, "Mesher", "Class for generating meshes.")
		.def_property_readonly("params", [](FEMesher& self) { return PyParameterList(&self); }, py::return_value_policy::reference_internal)
		.def("__getattr__", [](FEMesher& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](FEMesher& self, const std::string& name, py::object value) { SetDynamicAttribute(self, name, value); })
		;
}

#else
void init_FSMesh(pybind11::module_& m) {}
#endif
