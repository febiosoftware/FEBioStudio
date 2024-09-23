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
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/Commands.h>
#include "PythonTool.h"
#include "PythonToolsPanel.h"
#include "PyExceptions.h"
#include "PySpringFunctions.h"
#include <GeomLib/GPrimitive.h>

#include <FEMLib/GDiscreteObject.h>

#include "PyCallBack.h"
#include "PythonInputHandler.h"
#include "PyOutput.h"
#include <sstream>

namespace py = pybind11;
using namespace pybind11::literals;

void openFile(const char *fileName)
{
	FBS::getMainWindow()->OpenFile(fileName);
}

void GBox_init(vec3d pos, double width, double height, double depth)
{
	GBox* gbox = new GBox();

	static int n = 1;
	std::stringstream ss;
	ss << "box" << n++;
	gbox->SetName(ss.str());

    gbox->SetFloatValue(GBox::WIDTH, width);
    gbox->SetFloatValue(GBox::HEIGHT, height);
    gbox->SetFloatValue(GBox::DEPTH, depth);

    gbox->Update();

    gbox->GetTransform().SetPosition(pos);

    auto wnd = FBS::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
    if(!doc)
    {
        throw pyNoModelDocExcept();
    }

    doc->DoCommand(new CCmdAddAndSelectObject(doc->GetGModel(), gbox), gbox->GetName());
}

void init_FBSUI(py::module& m)
{
    py::module ui = m.def_submodule("ui", "Module used to interact with the FEBio Studio GUI");

    py::class_<CPyOutput>(ui, "PyOutput")
        .def(py::init())
        .def("write", &CPyOutput::write)
        .def("flush", &CPyOutput::flush);

    py::class_<GDiscreteSpringSet, std::unique_ptr<GDiscreteSpringSet, py::nodelete>>(ui, "SpringSet")
        .def(py::init(&SpringSet_init))
        .def("addSpring", static_cast<void (GDiscreteSpringSet::*)(int,int)>(&GDiscreteSpringSet::AddElement));

    ui.def("openFile", openFile);

	ui.def("GBox", GBox_init, "pos"_a, "W"_a, "H"_a, "D"_a);

    ui.def("FindOrMakeNode", FindOrMakeNode);
    ui.def("IntersectWithObject", IntersectWithObject);
    ui.def("MeshFromCurve", meshFromCurve, py::arg("points"), py::arg("radius"), py::arg("name") = "Curve",
		py::arg("divisions") = 6, py::arg("segments") = 6, py::arg("ratio") = 0.5);

    ui.def("setProgressText", PySetProgressText);
    ui.def("setProgress", static_cast<void (*) (int)>(PySetProgress));
    ui.def("setProgress", static_cast<void (*) (float)>(PySetProgress));
//    ui.def("getUserString", PyGetString);
//    ui.def("getUserInt", PyGetInt);
//    ui.def("getUserSelection", PyGetSelection);
}

#else
void init_FBSUI(py::module_& m) {}
#endif