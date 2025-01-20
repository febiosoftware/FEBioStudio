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
#include <pybind11/stl.h>
#include "PyExceptions.h"

#include <MeshLib/FSElementLibrary.h>
#include <FEMLib/FSProject.h>
#include <MeshIO/VTKExport.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GPrimitive.h>
#include <MeshTools/FEShellDisc.h>

#include <FECore/mat3d.h>

namespace py = pybind11;

void curveToVTKMesh(std::vector<vec3d> points, double radius, std::string name, int div, int seg, double ratio)
{
	GDisc disc;
	disc.SetFloatValue(GDisc::RADIUS, radius);
	disc.Update();

	auto mesher = dynamic_cast<FEShellDisc*>(disc.GetFEMesher());
	mesher->SetIntValue(FEShellDisc::NDIV, div);
	mesher->SetIntValue(FEShellDisc::NSEG, seg);
	mesher->SetFloatValue(FEShellDisc::RATIO, ratio);
	
	auto discMesh = disc.BuildMesh();

	vector<vec3d> nodePositions;
	for(int point = 0; point < points.size(); point++)
	{
		// Rotate the disc 
		if(point == 0)
		{
			vec3d vec1(0,0,1);
			vec3d vec2 = (points[point + 1] - points[point]).Normalize();

			disc.GetTransform().Rotate(quatd(vec1, vec2), vec3d(0,0,0));
		}
		else if(point != points.size() - 1)
		{
			vec3d vec1 = (points[point] - points[point - 1]).Normalize();
			vec3d vec2 = (points[point + 1] - points[point]).Normalize();

			disc.GetTransform().Rotate(quatd(vec1, vec2), points[point - 1]);
		}

		// Move the disc into position
		disc.GetTransform().SetPosition(points[point]);
		
		// Add all of the node locations to our vector
		for(int node = 0; node < discMesh->Nodes(); node++)
		{
			nodePositions.push_back(discMesh->NodePosition(node));
		}
	}

	// Create and allocate a new mesh. This is the mesh that we'll add to the model. 
	FSMesh* newMesh = new FSMesh();
	newMesh->Create(nodePositions.size(), discMesh->Elements() * (points.size() - 1));

	// Update the positions of all of the nodes in the new mesh
	for(int node = 0; node < newMesh->Nodes(); node++)
	{
		newMesh->Node(node).r = nodePositions[node];
	}

	for(int point = 0; point < points.size() - 1; point++)
	{
		for(int element = 0; element < discMesh->Elements(); element++)
		{
			// For each element, grab the corresponding element from the disc mesh so 
			// that we can use that node connectivity.
			auto discElement = discMesh->ElementPtr(element);

			// The current element on our new mesh corresponds to an element on the disc
			// mesh, but is offset by the number of elements in the disc mesh times the 
			// number of previous points in our curve
			auto current = newMesh->ElementPtr(element + discMesh->Elements() * point);
			current->SetType(FE_HEX8);

			for(int node = 0; node < discElement->Nodes(); node++)
			{
				// Grab the node number from the disc element, but them offset it by the 
				// number of nodes that were used in previous points in our curve
				current->m_node[node] = discElement->m_node[node] + discMesh->Nodes() * point;

				// Here we do the same, but the node in question lies on the next point, as it's
				// on the far face of the hex element
				current->m_node[node + discElement->Nodes()] = discElement->m_node[node] + discMesh->Nodes() * (point + 1);
			}
		}
	}

	newMesh->RebuildMesh();

	FSProject project;

	GMeshObject* gmesh = new GMeshObject(newMesh);
	gmesh->SetName(name);

	FSModel& fem = project.GetFSModel();
	fem.GetModel().AddObject(gmesh);

    VTKExport vtk(project);

    vtk.Write(name.c_str());
}



void init_FBSCore(py::module& m)
{
    py::module core = m.def_submodule("core", "Module used to interact with the FEBio and FEBio Studio core classes");

	py::class_<GLColor>(core, "color")
		.def_readwrite("r", &GLColor::r)
		.def_readwrite("g", &GLColor::g)
		.def_readwrite("b", &GLColor::b)
		.def_readwrite("a", &GLColor::a);

    core.def("curveToVTKMesh", curveToVTKMesh);//, py::arg("points"), py::arg("radius"), py::arg("name") = "Curve", 
        // py::arg("divisions") = 6, py::arg("segments") = 6, py::arg("ratio") = 0.5);

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

    FSElementLibrary::InitLibrary();
}
#else
void init_FBSCore(pybind11::module_& m) {}
#endif
