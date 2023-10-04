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

void openFile(const char *fileName)
{
    FBS::getMainWindow()->OpenFile(fileName);
}

CPythonDummyTool* PythonTool_init(const char* name, pybind11::function func)
{
    auto wnd = FBS::getMainWindow();
    CPythonToolsPanel* pythonToolsPanel = wnd->GetPythonToolsPanel();

    return pythonToolsPanel->addDummyTool(name, func);
}

GBox* GBox_init(vec3d pos, double width, double height, double depth)
{
    GBox* gbox = new GBox();

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

    return gbox;
}

void init_FBSUI(pybind11::module& m)
{
    pybind11::module ui = m.def_submodule("ui", "Module used to interact with the FEBio Studio GUI");

    pybind11::class_<CPyOutput>(ui, "PyOutput")
        .def(pybind11::init())
        .def("write", &CPyOutput::write)
        .def("flush", &CPyOutput::flush);

    pybind11::class_<GDiscreteSpringSet, std::unique_ptr<GDiscreteSpringSet, pybind11::nodelete>>(ui, "SpringSet")
        .def(pybind11::init(&SpringSet_init))
        .def("addSpring", static_cast<void (GDiscreteSpringSet::*)(int,int)>(&GDiscreteSpringSet::AddElement));

    ui.def("openFile", openFile);

    ui.def("FindOrMakeNode", FindOrMakeNode);
    ui.def("IntersectWithObject", IntersectWithObject);
    ui.def("MeshFromCurve", meshFromCurve, pybind11::arg("points"), pybind11::arg("radius"), pybind11::arg("name") = "Curve", 
        pybind11::arg("divisions") = 6, pybind11::arg("segments") = 6, pybind11::arg("ratio") = 0.5);

    ui.def("setProgressText", PySetProgressText);
    ui.def("setProgress", static_cast<void (*) (int)>(PySetProgress));
    ui.def("setProgress", static_cast<void (*) (float)>(PySetProgress));
    ui.def("getUserString", PyGetString);
    ui.def("getUserInt", PyGetInt);
    ui.def("getUserSelection", PyGetSelection);

    pybind11::class_<CPythonDummyTool, std::unique_ptr<CPythonDummyTool, pybind11::nodelete>>(ui, "PythonTool")
        .def(pybind11::init(&PythonTool_init))
        .def("addBoolParameter", &CPythonDummyTool::addBoolProperty, pybind11::arg("name"), pybind11::arg("value") = true)
        .def("addIntParameter", &CPythonDummyTool::addIntProperty, pybind11::arg("name"), pybind11::arg("value") = 0)
        .def("addDoubleParameter", &CPythonDummyTool::addDoubleProperty, pybind11::arg("name"), pybind11::arg("value") = 0)
        .def("addVec3Parameter", &CPythonDummyTool::addVec3Property, pybind11::arg("name"), pybind11::arg("value") = vec3d())
        .def("addEnumParameter", &CPythonDummyTool::addEnumProperty, pybind11::arg("name"), pybind11::arg("labels"), pybind11::arg("value") = 0)
        .def("addStringParameter", &CPythonDummyTool::addStringProperty, pybind11::arg("name"), pybind11::arg("value") = "")
        .def("addResourceParameter", &CPythonDummyTool::addResourceProperty, pybind11::arg("name"), pybind11::arg("value") = "");
}

#else
void init_FBSUI(pybind11::module_& m) {}
#endif