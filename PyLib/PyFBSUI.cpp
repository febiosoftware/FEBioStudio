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
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include "PyCallBack.h"
#include "PyOutput.h"
#include <sstream>

namespace py = pybind11;
using namespace pybind11::literals;

void openFile(const char *fileName)
{
	FBS::getMainWindow()->OpenFile(fileName);
}

void init_FBSUI(py::module& m)
{
    py::module ui = m.def_submodule("ui", "Module used to interact with the FEBio Studio GUI");

	py::module panels = ui.def_submodule("panels", "Module used for interacting with FBS panels");
	py::module pytools = panels.def_submodule("pytools", "Module used for interacting with Python panel");

    py::class_<CPyOutput>(ui, "PyOutput")
        .def(py::init())
        .def("write", &CPyOutput::write)
        .def("flush", &CPyOutput::flush);

    ui.def("openFile", openFile);

    pytools.def("set_progress_text", PySetProgressText);
    pytools.def("set_progress", static_cast<void (*) (int)>(PySetProgress));
    pytools.def("set_progress", static_cast<void (*) (float)>(PySetProgress));
}

#else
void init_FBSUI(py::module_& m) {}
#endif