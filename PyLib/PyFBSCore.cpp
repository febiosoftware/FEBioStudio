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

#ifdef HAS_PYTHON

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "PyExceptions.h"

#include <MathLib/mat3d.h>

void init_FBSCore(pybind11::module& m)
{
    pybind11::module core = m.def_submodule("core", "Module used to interact with the FEBio Studio GUI");

    pybind11::class_<vec3d>(core, "vec3d")
        .def(pybind11::init<>())
        .def(pybind11::init<double, double, double>())
        .def(pybind11::self + pybind11::self)
        .def(pybind11::self - pybind11::self)
        .def(pybind11::self * pybind11::self)
        .def(pybind11::self ^ pybind11::self)
        .def(pybind11::self == pybind11::self)
        .def(-pybind11::self)
        .def(pybind11::self * double())
        .def(pybind11::self / double())
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
            }
        
    );
}

#else
void init_FBSCore(pybind11::module_& m) {}
#endif