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

namespace py = pybind11;

void init_FBSCore(py::module& m)
{
    py::module core = m.def_submodule("core", "Module used to interact with the FEBio and FEBio Studio core classes");

	py::class_<GLColor>(core, "color")
		.def(py::init<>())
		.def(py::init<uint8_t, uint8_t, uint8_t>())
		.def_readwrite("r", &GLColor::r)
		.def_readwrite("g", &GLColor::g)
		.def_readwrite("b", &GLColor::b)
		.def_readwrite("a", &GLColor::a);

	py::class_<vec3d>(core, "vec3d")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self * py::self)
        .def(py::self ^ py::self)
        .def(py::self == py::self)
        .def(-py::self)
        .def(py::self * double())
        .def(py::self / double())
        .def("Length", &vec3d::Length)
        .def("SqrLength", &vec3d::SqrLength)
        .def("Normalize", &vec3d::Normalize)
        .def("Normalized", &vec3d::Normalized)
        .def_readwrite("x", &vec3d::x)
        .def_readwrite("y", &vec3d::y)
        .def_readwrite("z", &vec3d::z)
        .def("__repr__",
            [](const vec3d& v){
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
            });

	py::class_<quatd>(core, "quatd")
		.def(py::init<>())
		.def(py::init<const vec3d&, const vec3d&>())
		.def("__repr__",
			[](const quatd& q) {
				return "(" + std::to_string(q.x) + ", " + std::to_string(q.y) + ", " + std::to_string(q.z) + ", " + std::to_string(q.w) + ")";
			})
		.def("Inverse", &quatd::Inverse)
		;

	py::class_<Transform>(core, "Transform")
		.def("Rotate", static_cast<void (Transform::*)(quatd, vec3d)> (&Transform::Rotate))
		.def("SetPosition", &Transform::SetPosition)
		.def("SetEulerAngles", static_cast<void (Transform::*)(double, double, double)> (&Transform::SetRotation))
		;

	py::class_<FSObject, std::unique_ptr<FSObject, py::nodelete>>(core, "FSObject")
		.def_property("name", &FSObject::GetName, &FSObject::SetName)
		;

    FSElementLibrary::InitLibrary();
}
#else
void init_FBSCore(pybind11::module_& m) {}
#endif
