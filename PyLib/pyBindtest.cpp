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

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <MeshTools/GModel.h>
#include <GeomLib/GMeshObject.h>
#include "PythonTool.h"
#include <FEBioStudio/PythonToolsPanel.h>

#include <MeshTools/GDiscreteObject.h>
#include <MathLib/mat3d.h>

#include <iostream>


void openFile(const char *fileName)
{
    PRV::getMainWindow()->OpenFile(fileName);
}

CPythonTool* PythonTool_init(const char* name, pybind11::function func)
{
    auto wnd = PRV::getMainWindow();
    CPythonToolsPanel* pythonToolsPanel = wnd->GetPythonToolsPanel();

    return pythonToolsPanel->addTool(name, func.ptr());
}

void finalizePythonTool(CPythonTool* tool)
{
    auto wnd = PRV::getMainWindow();
    CPythonToolsPanel* pythonToolsPanel = wnd->GetPythonToolsPanel();

    pythonToolsPanel->finalizeTool(tool);
}


GDiscreteSpringSet* SpringSet_init(const char* name, char* type)
{
    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
    auto gmodel = doc->GetGModel();

    auto set = new GDiscreteSpringSet(gmodel);

    if(strcmp(type, "Linear") == 0)
    {
        set->SetMaterial(new FELinearSpringMaterial);
    }
    else if(strcmp(type, "Nonlinear") == 0)
    {
        set->SetMaterial(new FENonLinearSpringMaterial);
    }
    else if(strcmp(type, "Hill") == 0)
    {
        set->SetMaterial(new FEHillContractileMaterial);
    }
    else
    {
        delete set;
        return nullptr;
    }

    set->SetName(name);

    gmodel->AddDiscreteObject(set);

    return set;
}

int FindOrMakeNode(double x, double y, double z, double tol)
{
    vec3d r(x,y,z);

    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
    auto po = dynamic_cast<GMeshObject*>(doc->GetActiveObject());

	// find closest node
	int imin = -1;
	double l2min = 0.0;
	FEMesh* m = po->GetFEMesh();
	int N = m->Nodes();
	imin = -1;
	for (int i = 0; i < N; ++i)
	{
		FENode& ni = m->Node(i);
		if (ni.IsExterior())
		{
			vec3d ri = m->LocalToGlobal(ni.r);

			double l2 = (r - ri).SqrLength();
			if ((imin == -1) || (l2 < l2min))
			{
				imin = i;
				l2min = l2;
			}
		}
	}
	if ((imin!=-1) && (l2min < tol*tol))
	{
		return po->MakeGNode(imin);
	}

    return po->AddNode(r);
}

PYBIND11_MODULE(fbs2, m)
{
    pybind11::class_<GDiscreteSpringSet, std::unique_ptr<GDiscreteSpringSet, pybind11::nodelete>>(m, "SpringSet")
        .def(pybind11::init(&SpringSet_init))
        .def("addSpring", static_cast<void (GDiscreteSpringSet::*)(int,int)>(&GDiscreteSpringSet::AddElement));

    pybind11::class_<CPythonTool, std::unique_ptr<CPythonTool, pybind11::nodelete>>(m, "PythonTool")
        .def(pybind11::init(&PythonTool_init))
        .def("addBoolProperty", &CPythonTool::addBoolProperty)
        .def("addIntProperty", &CPythonTool::addIntProperty)
        .def("addDoubleProperty", &CPythonTool::addDoubleProperty)
        .def("addEnumProperty", &CPythonTool::addEnumProperty)
        .def("addResourceProperty", &CPythonTool::addResourceProperty)
        .def("finalize", finalizePythonTool);

    m.def("openFile", openFile);
    m.def("FindOrMakeNode", FindOrMakeNode);

    pybind11::class_<vec3d>(m, "vec3d")
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
        .def_readwrite("z", &vec3d::z);
}
