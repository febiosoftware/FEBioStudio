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

#ifdef HAS_PYTHON

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/Commands.h>
#include <MeshTools/GModel.h>
#include <GeomLib/GMeshObject.h>
#include "PythonTool.h"
#include "PythonToolsPanel.h"
#include <QEventLoop>
#include <GeomLib/GPrimitive.h>

#include <MeshTools/GDiscreteObject.h>
#include <MathLib/mat3d.h>

#include "PyCallBack.h"
#include "PythonInputHandler.h"
#include "PyOutput.h"


void openFile(const char *fileName)
{
    PRV::getMainWindow()->OpenFile(fileName);
}

CPythonDummyTool* PythonTool_init(const char* name, pybind11::function func)
{
    auto wnd = PRV::getMainWindow();
    CPythonToolsPanel* pythonToolsPanel = wnd->GetPythonToolsPanel();

    return pythonToolsPanel->addDummyTool(name, func);
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

GBox* GBox_init(vec3d pos, double width, double height, double depth)
{
    GBox* gbox = new GBox();

    gbox->SetFloatValue(GBox::WIDTH, width);
    gbox->SetFloatValue(GBox::HEIGHT, height);
    gbox->SetFloatValue(GBox::DEPTH, depth);

    gbox->Update();

    gbox->GetTransform().SetPosition(pos);

    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());

    doc->DoCommand(new CCmdAddAndSelectObject(doc->GetGModel(), gbox), gbox->GetName());

    return gbox;
}

PYBIND11_EMBEDDED_MODULE(fbs, m)
{
    pybind11::class_<CPyOutput>(m, "PyOutput")
        .def(pybind11::init())
        .def("write", &CPyOutput::write);

    pybind11::class_<GBox, std::unique_ptr<GBox, pybind11::nodelete>>(m, "GBox")
        .def(pybind11::init(&GBox_init))
        .def_property("position", 
                [](const GBox& g){
                    return g.GetTransform().GetPosition();
                }, 
                [](GBox* g, vec3d& pos){
                    g->GetTransform().SetPosition(pos);
                });

    pybind11::class_<GDiscreteSpringSet, std::unique_ptr<GDiscreteSpringSet, pybind11::nodelete>>(m, "SpringSet")
        .def(pybind11::init(&SpringSet_init))
        .def("addSpring", static_cast<void (GDiscreteSpringSet::*)(int,int)>(&GDiscreteSpringSet::AddElement));

    pybind11::class_<CPythonDummyTool, std::unique_ptr<CPythonDummyTool, pybind11::nodelete>>(m, "PythonTool")
        .def(pybind11::init(&PythonTool_init))
        .def("addBoolProperty", &CPythonDummyTool::addBoolProperty)
        .def("addIntProperty", &CPythonDummyTool::addIntProperty)
        .def("addDoubleProperty", &CPythonDummyTool::addDoubleProperty)
        .def("addVec3Property", &CPythonDummyTool::addVec3Property)
        .def("addEnumProperty", &CPythonDummyTool::addEnumProperty)
        .def("addStringProperty", &CPythonDummyTool::addStringProperty)
        .def("addResourceProperty", &CPythonDummyTool::addResourceProperty);

    m.def("openFile", openFile);
    m.def("FindOrMakeNode", FindOrMakeNode);

    m.def("getUserString", PyGetString);
    m.def("getUserInt", PyGetInt);

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
        .def_readwrite("z", &vec3d::z)
        .def("__repr__",
            [](const vec3d& v){
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
            }
        
    );
}

#endif