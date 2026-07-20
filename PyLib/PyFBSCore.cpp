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

#include "PyFBSCore.h"
#include "PyFBSPost.h"

#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <MeshLib/FSElementLibrary.h>
#include <FECore/mat3d.h>
#include <FSCore/color.h>
#include <FSCore/FSObject.h>
#include <FECore/FETransform.h>
#include "DocHeaders/PyCoreDocs.h"
#include "PyUtil.h"

namespace py = pybind11;

void init_FBSCore(py::module& m)
{
    py::module core = m.def_submodule("core", "Module used to interact with the FEBio and FEBio Studio core classes");

	py::class_<GLColor>(core, "Color", DOC(GLColor))
		.def(py::init<>(), DOC(GLColor, GLColor))
		.def(py::init<uint8_t, uint8_t, uint8_t>(), DOC(GLColor, GLColor, 3))
		.def_readwrite("r", &GLColor::r, DOC(GLColor, r))
		.def_readwrite("g", &GLColor::g, DOC(GLColor, g))
		.def_readwrite("b", &GLColor::b, DOC(GLColor, b))
		.def_readwrite("a", &GLColor::a, DOC(GLColor, a));

	py::class_<vec3d>(core, "Vec3")
        .def(py::init<>(), DOC(vec3d, vec3d))
        .def(py::init<double, double, double>(), DOC(vec3d, vec3d, 3))
        .def(py::self + py::self, DOC(vec3d, operator_add))
        .def(py::self - py::self, DOC(vec3d, operator_sub))
        .def(py::self * py::self, DOC(vec3d, operator_mul_2))
        .def(py::self ^ py::self, DOC(vec3d, operator))
        .def(py::self == py::self, DOC(vec3d, operator_eq))
        .def(-py::self, DOC(vec3d, operator_sub_2))
        .def(py::self * double(), DOC(vec3d, operator_mul))
        .def(py::self / double(), DOC(vec3d, operator_div))
        .def("length", &vec3d::Length, DOC(vec3d, Length))
        .def("sqr_length", &vec3d::SqrLength, DOC(vec3d, SqrLength))
        .def("normalize", &vec3d::Normalize, DOC(vec3d, Normalize))
        .def("normalized", &vec3d::Normalized, DOC(vec3d, Normalized))
        .def_readwrite("x", &vec3d::x, DOC(vec3d, x))
        .def_readwrite("y", &vec3d::y, DOC(vec3d, y))
        .def_readwrite("z", &vec3d::z, DOC(vec3d, z))
        .def("__repr__",
            [](const vec3d& v){
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
            });

	py::class_<quatd>(core, "Quaternion", DOC(quatd))
		.def(py::init<>(), DOC(quatd, quatd))
		.def(py::init<const vec3d&, const vec3d&>(), DOC(quatd, quatd, 4))
		.def("__repr__",
			[](const quatd& q) {
				return "(" + std::to_string(q.x) + ", " + std::to_string(q.y) + ", " + std::to_string(q.z) + ", " + std::to_string(q.w) + ")";
			})
		.def("inverse", &quatd::Inverse, DOC(quatd, Inverse))
		;

	py::class_<Transform>(core, "Transform", DOC(Transform))
		.def_property(
			"position",
			[](Transform& self) { return self.GetPosition(); },
			[](Transform& self, py::handle value) {
				self.SetPosition(Vec3dFromPython(value, "position"));
			}
		)
		.def("rotate", py::overload_cast<quatd, vec3d>(&Transform::Rotate), DOC(Transform, Rotate))
		.def("set_euler_angles_deg", static_cast<void(Transform::*)(double,double,double)>(&Transform::SetRotation))
		.def("local_to_global", &Transform::LocalToGlobal, DOC(Transform, LocalToGlobal))
		.def("global_to_local", &Transform::GlobalToLocal, DOC(Transform, GlobalToLocal))
		;

	py::class_<PyVec2dList>(core, "PointList")
		.def("__len__", &PyVec2dList::size)
		.def("__getitem__", &PyVec2dList::get)
		.def("__iter__", &PyVec2dList::iter)
		.def("add", py::overload_cast<double, double>(&PyVec2dList::add))
		.def("add", py::overload_cast<py::handle>(&PyVec2dList::add))
		.def("clear", &PyVec2dList::clear)
		;

	py::class_<PyParameter>(core, "Parameter")
		.def_property("value", &PyParameter::value, &PyParameter::setValue)
		.def_property("lc_id", &PyParameter::lcID, &PyParameter::setLCID)
		.def_property("lc", &PyParameter::getLC, &PyParameter::setLC)
		.def_property_readonly("name", &PyParameter::name)
		.def_property_readonly("long_name", &PyParameter::longName)
		.def_property_readonly("unit", &PyParameter::unit);

	py::class_<PyParameterList>(core, "ParameterList")
		.def("__len__", &PyParameterList::size)
		.def("__getitem__", py::overload_cast<int>(&PyParameterList::get, py::const_))
		.def("__getitem__", py::overload_cast<const std::string&>(&PyParameterList::get, py::const_))
		.def("__iter__", &PyParameterList::iter)
		.def("__contains__", py::overload_cast<const std::string&>(&PyParameterList::contains, py::const_))
		;

	// minimal property slot wrapper for model components
	py::class_<PyPropertySlot>(core, "PropertySlot")
		.def("create", &PyPropertySlot::create)
		.def("clear", &PyPropertySlot::clear)
		.def_property_readonly("is_set", &PyPropertySlot::isSet)
		.def_property_readonly("type_str", &PyPropertySlot::typeStr);

	py::class_<FSObject, std::unique_ptr<FSObject, py::nodelete>>(core, "Base", "Base class for all FEBio Studio objects")
		.def_property("name", &FSObject::GetName, &FSObject::SetName, "Get or set the name of the object")
		;

    FSElementLibrary::InitLibrary();
}
#else
void init_FBSCore(pybind11::module_& m) {}
#endif
