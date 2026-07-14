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

#include "PyFBSGeom.h"

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GBaseObject.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include <GeomLib/GItem.h>
#include <MeshLib/FSMesh.h>
#include <FEMLib/GMaterial.h>
#include <FECore/FETransform.h>
#include "DocHeaders/PyGeomDocs.h"
#include <MeshTools/FEMesher.h>
#include "PyUtil.h"

namespace py = pybind11;

class PyPartList
{
public:
	PyPartList(GObject* obj) : m_obj(obj) {}

	int size() const
	{
		return m_obj->Parts();
	}

	GPart* get(int i) const
	{
		int n = m_obj->Parts();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
		{
			throw py::index_error("part index out of range");
		}

		return m_obj->Part(i);
	}

	GPart* get(const std::string& name) const
	{
		GPart* part = m_obj->FindPartFromName(name.c_str());
		if (part == nullptr)
		{
			throw py::key_error("part not found: " + name);
		}

		return part;
	}

	void AssignMaterial(GMaterial* mat)
	{
		for (int i = 0; i < m_obj->Parts(); ++i)
		{
			m_obj->Part(i)->SetMaterialID(mat->GetID());
		}
	}

private:
	GObject* m_obj = nullptr;
};

class PySurfaceList
{
public:
	PySurfaceList(GObject* obj) : m_obj(obj) {}

	int size() const
	{
		return m_obj->Faces();
	}

	GFace* get(int i) const
	{
		int n = m_obj->Faces();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
		{
			throw py::index_error("surface index out of range");
		}

		return m_obj->Face(i);
	}

	GFace* get(const std::string& name) const
	{
		GFace* face = m_obj->FindFaceFromName(name.c_str());
		if (face == nullptr)
		{
			throw py::key_error("surface not found: " + name);
		}

		return face;
	}

private:
	GObject* m_obj = nullptr;
};

class PyEdgeList
{
public:
	PyEdgeList(GObject* obj) : m_obj(obj) {}

	int size() const
	{
		return m_obj->Edges();
	}

	GEdge* get(int i) const
	{
		int n = m_obj->Edges();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
		{
			throw py::index_error("edge index out of range");
		}

		return m_obj->Edge(i);
	}

	GEdge* get(const std::string& name) const
	{
		GEdge* edge = m_obj->FindEdgeFromName(name.c_str());
		if (edge == nullptr)
		{
			throw py::key_error("edge not found: " + name);
		}

		return edge;
	}

private:
	GObject* m_obj = nullptr;
};

class PyNodeList
{
public:
	PyNodeList(GObject* obj) : m_obj(obj) {}

	int size() const
	{
		return m_obj->Nodes();
	}

	GNode* get(int i) const
	{
		int n = m_obj->Nodes();
		if (i < 0) i += n;

		if (i < 0 || i >= n)
		{
			throw py::index_error("node index out of range");
		}

		return m_obj->Node(i);
	}

	GNode* get(const std::string& name) const
	{
		GNode* node = m_obj->FindNodeFromName(name.c_str());
		if (node == nullptr)
		{
			throw py::key_error("node not found: " + name);
		}

		return node;
	}

private:
	GObject* m_obj = nullptr;
};

// Initializes the fbs.geom module
void init_FBSGeom(py::module& m)
{
	py::module geom = m.def_submodule("geom", "Module used to create geometry.");

	// view wrapper for parts
	py::class_<PyPartList>(geom, "PartList")
		.def("__len__", &PyPartList::size)
		.def("__getitem__", py::overload_cast<int>(&PyPartList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyPartList::get, py::const_), py::return_value_policy::reference)
		.def("assign_material", &PyPartList::AssignMaterial);

	// view wrapper for surfaces
	py::class_<PySurfaceList>(geom, "SurfaceList")
		.def("__len__", &PySurfaceList::size)
		.def("__getitem__", py::overload_cast<int>(&PySurfaceList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PySurfaceList::get, py::const_), py::return_value_policy::reference);

	// view wrapper for edges
	py::class_<PyEdgeList>(geom, "EdgeList")
		.def("__len__", &PyEdgeList::size)
		.def("__getitem__", py::overload_cast<int>(&PyEdgeList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyEdgeList::get, py::const_), py::return_value_policy::reference);

	// view wrapper for nodes
	py::class_<PyNodeList>(geom, "NodeList")
		.def("__len__", &PyNodeList::size)
		.def("__getitem__", py::overload_cast<int>(&PyNodeList::get, py::const_), py::return_value_policy::reference)
		.def("__getitem__", py::overload_cast<const std::string&>(&PyNodeList::get, py::const_), py::return_value_policy::reference);

	py::class_<GObject, FSObject, std::unique_ptr<GObject, py::nodelete>>(geom, "GObject", DOC(GObject))
		.def("assign_material", [](GObject& self, GMaterial* mat) { for (int i = 0; i < self.Parts(); ++i) self.Part(i)->SetMaterialID(mat->GetID()); })
		.def_property("pos", &GBaseObject::GetPosition, &GBaseObject::SetPosition, "The position of the object in world coordinates. This is a property that can be set or retrieved.")
		.def("__getattr__", [](GObject& self, const std::string& name) { return GetDynamicAttribute(self, name); })
		.def("__setattr__", [](GObject& self, const std::string& name, py::object value) { SetDynamicAttribute(self, name, value); })
		.def_property_readonly(
			"nodes",
			[](GObject& self) { return PyNodeList(&self); },
			py::return_value_policy::reference_internal)
		.def_property_readonly(
			"edges",
			[](GObject& self) { return PyEdgeList(&self); },
			py::return_value_policy::reference_internal)
		.def_property_readonly(
			"surfaces",
			[](GObject& self) { return PySurfaceList(&self); },
			py::return_value_policy::reference_internal)
		.def_property_readonly(
			"parts",
			[](GObject& self) { return PyPartList(&self); },
			py::return_value_policy::reference_internal)
		.def_property_readonly("mesher", &GObject::GetFEMesher, DOC(GObject, GetFEMesher))
		.def("build_mesh", [](GObject& self, py::kwargs kwargs) 
			{
				FEMesher* mesher = self.GetFEMesher();
				if (!kwargs.empty() && mesher)
				{
					for (auto item : kwargs)
					{
						std::string key = py::str(item.first);
						py::object value = py::reinterpret_borrow<py::object>(item.second);
						SetDynamicAttribute(*mesher, key, value);
					}
				}
				self.BuildMesh();
			}, DOC(GObject, BuildMesh))
		.def("update", [](GObject& self) { self.Update(); }, DOC(GObject, Update))
		.def("assign_material", [](GObject& self, GMaterial* mat) { for (int i = 0; i < self.Parts(); ++i) self.Part(i)->SetMaterialID(mat->GetID()); })
		;

    py::class_<GSurfaceMeshObject, GObject, std::unique_ptr<GSurfaceMeshObject, py::nodelete>>(geom, "GSurfaceMeshObject")
        .def(py::init<FSSurfaceMesh*>())
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
		.def_property_readonly("type", &GNode::Type, DOC(GNode, Type))
		.def_property_readonly("local_position", static_cast<vec3d & (GNode::*)()>(&GNode::LocalPosition), DOC(GNode, LocalPosition))
		.def_property_readonly("position", [](GNode& self) { return self.Position(); }, DOC(GNode, Position))
		.def("make_required", &GNode::MakeRequired, DOC(GNode, MakeRequired))
		;

	py::class_<GEdge, std::unique_ptr<GEdge, py::nodelete>>(geom, "GEdge", DOC(GEdge));
	py::class_<GFace, std::unique_ptr<GFace, py::nodelete>>(geom, "GFace", DOC(GFace));
	py::class_<GPart, std::unique_ptr<GPart, py::nodelete>>(geom, "GPart", DOC(GPart))
		.def_property("material_id", &GPart::GetMaterialID, &GPart::SetMaterialID);
}
#else
void init_FBSGeom(pybind11::module_& m) {}
#endif
